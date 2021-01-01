#include "Game.hpp"
#define NEIGH_ARR(f, line_idx, col_idx, h, w)					\
		{	(*f)[line_idx + 1 % w][col_idx + 1 % h],	\
			(*f)[line_idx + 1 % w][col_idx],			\
			(*f)[line_idx + 1 % w][col_idx - 1 % h],	\
			(*f)[line_idx - 1 % w][col_idx - 1 % h],	\
			(*f)[line_idx - 1 % w][col_idx],			\
			(*f)[line_idx - 1 % w][col_idx + 1 % h],	\
			(*f)[line_idx][col_idx + 1 % h],			\
			(*f)[line_idx][col_idx - 1 % h]			\
		}

static const char *colors[7] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
Game::Game(game_params parms):
		parms(parms),
		m_gen_num(parms.n_gen),
		interactive_on(parms.interactive_on),
		print_on(parms.print_on),
		m_gen_hist(vector<float>()),
		crr_gen_num(0),
		gen_count_lock{pthread_mutex_init(&gen_count_lock, NULL)} {}

const vector<float> Game::gen_hist() const { return m_gen_hist; }

const vector<float> Game::tile_hist() const { return m_tile_hist; }

void Game::set_tile_hist(uint tile_idx, float time){
	m_tile_hist[get_crr_gen() * m_thread_num + tile_idx] = time;
}


uint Game::thread_num() const { return m_thread_num; } 

field Game::get_crr_fld() { return crr_fld; }

field Game::get_nxt_fld() { return nxt_fld; }

uint Game::get_crr_gen() { 
	uint res;
	pthread_mutex_lock(&gen_count_lock);
	res = crr_gen_num; 
	pthread_mutex_unlock(&gen_count_lock);
	return res;
	}

uint Game::get_gen_num() { return m_gen_num; }

static uint dominant(uint hist[8]){
	uint max = 0;
	for(int i = 1; i < 8; i++){
		if(max < hist[i]) max = hist[i];
	}
	return max;
}

static void eval_cell(Game * game, uint line_idx, uint col_idx){
	uint w = game->width, h = game->height, alive = 0, 
	neigh_histo[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	field crr = game->get_crr_fld(), nxt = game->get_nxt_fld();
	uint neigh[9] = NEIGH_ARR(crr, line_idx, col_idx, h, w);
	for(int i = 0; i < 9; i++){
		if(neigh[i]){
			neigh_histo[neigh[i]]++;
			alive++;
		}
	}
	if(alive < 3 && alive > 1){
		if((*crr)[line_idx][col_idx] != 0){ //Was alive
			(*nxt)[line_idx][col_idx] = (*crr)[line_idx][col_idx];
		}else if(alive == 3){ //Was dead
			(*nxt)[line_idx][col_idx] = dominant(neigh_histo);
		}else{
			(*nxt)[line_idx][col_idx] = 0;
		}
	}else{
		(*nxt)[line_idx][col_idx] = 0;
	}
}

static void eval_cell_color(Game * game, uint line_idx, uint col_idx){
	uint w = game->width, h = game->height, alive = 0, sum = 0;
	field nxt = game->get_nxt_fld();
	uint neigh[9] = NEIGH_ARR(nxt, line_idx, col_idx, h, w);
	for(int i = 0; i < 9; i++){
		if(neigh[i]){
			sum += neigh[i];
			alive++;
		}
	}
	(*nxt)[line_idx][col_idx] = floor(sum / alive);
}

static void set_start_end_bound(int * start, int * end, int tile_id, Game * game){
	int offset = floor(game->height / game->thread_num());
	*start = (tile_id == 0) ? 0 : offset * tile_id;
	*end = (tile_id == game->thread_num()) ? game->height - 1 : offset * (tile_id + 1); 
}

static void task_phase1(Game * game, uint tile_id){
	int start_line_idx, end_line_idx;
	set_start_end_bound(&start_line_idx, &end_line_idx, tile_id, game);
	for(int line_idx = start_line_idx; line_idx <= end_line_idx; line_idx++){
		for(int col_idx = 0; col_idx < game->width; col_idx++){
			eval_cell(game, line_idx, col_idx);
		}
	} 
}

static void task_phase2(Game * game, uint tile_id){
	int start_line_idx, end_line_idx;
	set_start_end_bound(&start_line_idx, &end_line_idx, tile_id, game);
	for(int line_idx = start_line_idx; line_idx <= end_line_idx; line_idx++){
		for(int col_idx = 0; col_idx < game->width; col_idx++){
			eval_cell_color(game, line_idx, col_idx);
		}
	} 
}

void Game::run() {

	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation 
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((float)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(NULL);
	} // generation loop
	print_board("Final Board");
	_destroy_game();
}

void Game::_init_game() {
	//Read file
	vector<string> str_field = utils::read_lines(parms.filename);
	height = str_field.size();
	width = str_field[0].size();
	//update m_thread_num
	m_thread_num = (str_field.size() < parms.n_thread) ? str_field.size() : parms.n_thread; 
	m_tile_hist = vector<float>(m_thread_num * m_gen_num, 0);
	// Create threads
	m_threadpool = vector<GOLThread*>(m_thread_num, nullptr);
	for(int i = 0; i < m_thread_num; i++){
		m_threadpool[i] = new GOLThread(i, this);
	}
	// Create game fields
	crr_fld = new vector<vector<uint>>(str_field.size(), vector<uint>(str_field[0].size(), 0));
	nxt_fld = new vector<vector<uint>>(*crr_fld);
	uint count_lines = 0, count_chars;
	for(auto str_line : str_field){
		count_chars = 0;
		for(auto c : str_line){
			((*crr_fld)[count_lines])[count_chars++] = (uint) c;
		}
	}
	// Start the threads
	for(auto thread : m_threadpool){
		thread->start();
	}
	// Testing of your implementation will presume all threads are started here
}

void Game::_step(uint curr_gen) {
	// Push phase 1 jobs to queue
	for(int i = 0; i < m_thread_num; i++){
		t_queue.push({.tile_idx = i, .task = task_phase1});
	}
	// Wait for the workers to finish calculating 
	while(t_queue.size() > 0) {}
	// Push phase 2 jobs to queue
	for(int i = 0; i < m_thread_num; i++){
		t_queue.push({.tile_idx = i, .task = task_phase2});
	}
	// Wait for the workers to finish calculating 
	while(t_queue.size() > 0) {}
	// Swap pointers between current and next field 
	auto tmp = crr_fld;
	crr_fld = nxt_fld;
	nxt_fld = tmp;
	pthread_mutex_lock(&gen_count_lock);
	crr_gen_num++;
	pthread_mutex_unlock(&gen_count_lock);
}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// Testing of your implementation will presume all threads are joined here
	for(auto thread : m_threadpool){
		pthread_join(thread->get_pthread(), NULL);
	}
	pthread_mutex_destroy(&gen_count_lock);
	free(crr_fld);
	free(nxt_fld);
}

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

	if(print_on){ 

		// Clear the screen, to create a running animation 
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != NULL)
			cout << "<------------" << header << "------------>" << endl;
		
		// TODO: Print the board 
		print_the_board(crr_fld, crr_fld->size(), (*crr_fld)[0].size());

		// Display for GEN_SLEEP_USEC micro-seconds on screen 
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}

static void print_the_board(field f, uint field_width, uint field_height){
	vector<vector<uint>> field = *f;
	cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height; ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
                if (field[i][j] > 0)
                    cout << colors[field[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
}


