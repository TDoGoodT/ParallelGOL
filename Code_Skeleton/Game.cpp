#include "utils.hpp"
#include "Game.hpp"

#define NEIGH_IDX(line, col) 			\
	{ 										\
		{(line) , (col + 1) },		\
		{(line) , (col - 1) },		\
		{(line + 1) , (col + 1) },	\
		{(line + 1) , (col) },		\
		{(line + 1) , (col - 1) },	\
		{(line - 1) , (col + 1) },	\
		{(line - 1) , (col) },		\
		{(line - 1) , (col - 1) }		\
	}

#define INIT_XY(neigh, i, x, y, h, w)									\
	{															\
		x = (neigh[i][0] < 0) ? (h - 1) : (neigh[i][0] % h),	\
		y = (neigh[i][1] < 0) ? (w - 1) : (neigh[i][1] % w);	\
	}

static const char *colors[7] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/

static uint dominant(vector<int> hist){
	int max = 0, max_idx = 0;
	for(int i = 1; i <= 7; i++){
		if(max < (hist[i] * i)) {
			max = hist[i] * i;
			max_idx = i;
		}
	}
	return static_cast<uint>(max_idx);
}

static void eval_cell(Game * game, int line_idx, int col_idx){
	vector<int> neigh_histo(8,0);
	field crr = game->get_crr_fld(), nxt = game->get_nxt_fld();
	int x, y, w = game->width, h = game->height, alive = 0,
	 neigh[8][2] = NEIGH_IDX(line_idx, col_idx);
	for(int i = 0; i < 8; i++){
		if(neigh[i][0] >=0 && neigh[i][0]<h && neigh[i][1] >= 0 && neigh[i][1] < w) {
		    x = neigh[i][0];
		    y = neigh[i][1];
            if ((*crr)[x][y] > 0) {
                neigh_histo[(*crr)[x][y]]++;
                alive++;
            }
        }
	}
	if(alive < 4 && alive > 1){
		if((*crr)[line_idx][col_idx] > 0){ //Was alive
			(*nxt)[line_idx][col_idx] = (*crr)[line_idx][col_idx];
		}else if(alive == 3){ //Was dead
		/*if(line_idx == 7 && col_idx == 10) {
			for(auto x : neigh_histo) cerr << x;
			cerr << endl << dominant(neigh_histo) << endl;
		}*/
			(*nxt)[line_idx][col_idx] = dominant(neigh_histo);
		}else{
			(*nxt)[line_idx][col_idx] = 0;
		}
	}else{
		(*nxt)[line_idx][col_idx] = 0;
	}
}

static void eval_cell_color(Game * game, int line_idx, int col_idx){
	field crr = game->get_crr_fld();
	field nxt = game->get_nxt_fld();
	if((*nxt)[line_idx][col_idx] == 0){
		(*crr)[line_idx][col_idx] = 0;
		return;
	}
	int x, y, w = game->width, h = game->height, alive = 1,
	neigh[8][2] = NEIGH_IDX(line_idx, col_idx);
	uint  sum = (*nxt)[line_idx][col_idx];
	for(int i = 0; i < 8; i++){
		if(neigh[i][0] >= 0  && neigh[i][0]<h && neigh[i][1] >= 0 && neigh[i][1] < w){
			x = neigh[i][0];
			y = neigh[i][1];
			if((*nxt)[x][y] > 0){
				sum += (*nxt)[x][y];
				alive++;
			}
		}
	}
	(*crr)[line_idx][col_idx] = std::round(((double) sum) / ((double) alive));
}

static void set_start_end_bound(uint * start, uint * end, uint tile_id, Game * game){
	uint offset = floor(game->height / game->thread_num());
	(*start) = offset * tile_id;
	(*end) = (tile_id == game->thread_num() - 1) ? game->height - 1 : offset * (tile_id + 1); 
}

static void task_phase1(Game * game, uint tile_id, uint start, uint end){
	for(uint line = start; line <= end; line++){
		for(uint col = 0; col < game->width; col++){
			eval_cell(game, static_cast<int>(line), static_cast<int>(col));
		}
	}
}

static void task_phase2(Game * game, uint tile_id, uint start, uint end){
	for(uint line = start; line <= end; line++){
		for(uint col = 0; col < game->width; col++){
			eval_cell_color(game, static_cast<int>(line), static_cast<int>(col));
		}
	} 
}

static void next_gen(Game * game, uint tile_id, Semaphore * sem){
	uint start, end, gen = game->get_crr_gen(), t_num = game->thread_num();
	set_start_end_bound(&start, &end, tile_id, game);
	task_phase1(game,tile_id, start, end);
	sem->up();
	while((uint) sem->get_val() < (2 * (gen+1) * t_num) - t_num) {}
	task_phase2(game,tile_id, start, end);
	sem->up();
	while((uint) sem->get_val() > 2 * (gen+1) * t_num) {}
}

/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/

Game::Game(game_params parms):
		parms(parms),
		m_gen_num(parms.n_gen),
		interactive_on(parms.interactive_on),
		print_on(parms.print_on),
		m_tile_hist_sem(1){}

const vector<double> Game::gen_hist() const { return m_gen_hist; }

const vector<double> Game::tile_hist() const { return m_tile_hist; }

void Game::push_tile_time(float time){
	m_tile_hist_sem.down();
	m_tile_hist.push_back(time);
	m_tile_hist_sem.up();
}


uint Game::thread_num() const { return m_thread_num; } 

field Game::get_crr_fld() { return crr_fld; }

field Game::get_nxt_fld() { return nxt_fld; }

uint Game::get_crr_gen() { return m_gen.get_val(); }

uint Game::get_gen_num() { return m_gen_num; }

void Game::run() {

	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation 
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((float)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(NULL);
		m_gen.up();
	} // generation loop
	print_board("Final Board");
	_destroy_game();
}

void Game::_init_game() {
	//Read file
	vector<vector<string>> str_field;
	vector<string> tmp = utils::read_lines(parms.filename);
	for(auto s : tmp){
		str_field.push_back(utils::split(s,' '));
	}
	height = str_field.size();
	width = str_field[0].size();
	//update m_thread_num
	m_thread_num = (str_field.size() < parms.n_thread) ? str_field.size() : parms.n_thread; 
	// Create threads
	m_threadpool = vector<GOLThread*>(m_thread_num, nullptr);
	for(uint i = 0; i < m_thread_num; i++){
		m_threadpool[i] = new GOLThread(i, this, &m_sem);
	}
	// Create game fields
	crr_fld = new vector<vector<uint>>(height, vector<uint>(width));
	nxt_fld = new vector<vector<uint>>(height, vector<uint>(width));
	uint count_lines = 0, count_chars;
	for(auto str_line : str_field){
		count_chars = 0;
		for(auto c : str_line){
			(*crr_fld)[count_lines][count_chars++] = ((uint) c[0]) - 48;
		}count_lines++;
	}
	// Start the threads
	for(auto thread : m_threadpool){
		thread->start();
	}
	// Testing of your implementation will presume all threads are started here
}

void Game::_step(uint curr_gen) {
	// Push phase 1 jobs to queue
	vector<task_struct> tasks;
	for(uint i = 0; i < m_thread_num; i++){
        tasks.push_back({i, next_gen});
	}
	t_queue.multi_push(tasks);
	// Wait for the workers to finish calculating
	while((uint) m_sem.get_val() < 2 * ((curr_gen + 1) * m_thread_num) ){}
}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// Testing of your implementation will presume all threads are joined here
	for(auto thread : m_threadpool){
		thread->join();
	}
}

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/


static void print_the_board1(field f, uint field_height, uint field_width){
	cout  << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
	for (auto line : (*f)) {
		cout << u8"║";
		for (auto x : line) {
            if (x > 0){
                cout << colors[x % 7] << u8"█" << RESET;
			}
            else
                cout << u8"░";
		}
		cout << u8"║" << endl;
	}
	cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
}
/*
static void print_the_board2(field f, uint field_height, uint field_width){
	for (auto line : (*f)) {
		for (auto x : line) {
			cout << "(" << x << ")";
		}
		cout << endl;
	}
}*/


inline void Game::print_board(const char* header) {

	if(print_on){ 

		// Clear the screen, to create a running animation 
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != NULL)
			cout << "<------------" << header << "------------>" << endl;

		print_the_board1(crr_fld, (*crr_fld).size(), (*crr_fld)[0].size());

		// Display for GEN_SLEEP_USEC micro-seconds on screen 
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}


