#ifndef __GAMERUN_H
#define __GAMERUN_H
#include "utils.hpp"
#include "Thread.hpp"
#include "PCQueue.hpp"
/*--------------------------------------------------------------------------------
								  Species colors
--------------------------------------------------------------------------------*/
#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red - 1*/
#define GREEN   "\033[32m"      /* Green - 2*/
#define YELLOW  "\033[33m"      /* Yellow - 3*/
#define BLUE    "\033[34m"      /* Blue - 4*/
#define MAGENTA "\033[35m"      /* Magenta - 5*/
#define CYAN    "\033[36m"      /* Cyan - 6*/
#define BLACK   "\033[30m"      /* Black - 7 */


/*--------------------------------------------------------------------------------
								  Auxiliary Structures
--------------------------------------------------------------------------------*/
typedef vector<vector<uint>>* 	field;
typedef void 					(*task)(Game*, uint);
typedef struct {
	uint 	tile_idx;
	task 	task;
} 								task_srct;
typedef PCQueue<task_srct> 		tasks_queue;

struct game_params {
	// All here are derived from ARGV, the program's input parameters. 
	uint 		n_gen;
	uint 		n_thread;
	string 		filename;
	bool 		interactive_on; 
	bool 		print_on; 
};

class GOLThread : public Thread{
public:
    GOLThread(uint thread_id, Game * g):
        Thread(thread_id),
		game(g){
			//TODO: Any vars??
        }
protected:
    virtual void thread_workload(){
		//TODO: add the routine for each worker in the game
		while(game->get_crr_gen() < game->get_gen_num()) {
			auto tile_start = std::chrono::system_clock::now();
			task_srct t = game->t_queue.pop();
			(t.task)(game, t.tile_idx);
			auto tile_end = std::chrono::system_clock::now();
			auto time = (float)std::chrono::duration_cast<std::chrono::microseconds>(tile_end - tile_start).count();
			game->set_tile_hist(t.tile_idx, time);
		}
		pthread_exit(NULL);
    }
	//TODO: VARS ???
	Game* game;
};
/*--------------------------------------------------------------------------------
									Class Declaration
--------------------------------------------------------------------------------*/
class Game {
public:

	Game(game_params);
	~Game();
	void 				run(); // Runs the game
	const vector<float> gen_hist() const; // Returns the generation timing histogram  
	const vector<float> tile_hist() const; // Returns the tile timing histogram
	void 				set_tile_hist(uint tile_idx, float time);
	uint 				thread_num() const; //Returns the effective number of running threads = min(thread_num, field_height)
	field 				get_crr_fld();
	field 				get_nxt_fld();
	uint 				get_crr_gen();
	uint 				get_gen_num();

public:
	tasks_queue 		t_queue;
	uint 				width;
	uint 				height;
protected: // All members here are protected, instead of private for testing purposes
	// See Game.cpp for details on these three functions
	void 				_init_game(); 
	void 				_step(uint curr_gen); 
	void 				_destroy_game(); 

	game_params 		parms;
	uint 				m_gen_num; 			 // The number of generations to run
	uint 				m_thread_num; 			 // Effective number of threads = min(thread_num, field_height)
	vector<float> 		m_tile_hist; 	 // Shared Timing history for tiles: First m_gen_num cells are the calculation durations for tiles in generation 1 and so on. 
							   	 // Note: In your implementation, all m_thread_num threads must write to this structure. 
	vector<float> 		m_gen_hist;  	 // Timing history for generations: x=m_gen_hist[t] iff generation t was calculated in x microseconds
	vector<GOLThread*> 	m_threadpool; // A storage container for your threads. This acts as the threadpool. 

	bool 				interactive_on; // Controls interactive mode - that means, prints the board as an animation instead of a simple dump to STDOUT 
	bool 				print_on; // Allows the printing of the board. Turn this off when you are checking performance (Dry 3, last question)
	void 				print_board(const char* header);
	// TODO: Add in your variables and synchronization primitives  
	field 				crr_fld;
	field 				nxt_fld;
	uint 				crr_gen_num;
	mutex_t 			gen_count_lock;
	
};
#endif
