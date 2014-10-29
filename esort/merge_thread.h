#ifndef __split_thread_h__
#define __split_thread_h__

#include <queue>
#include <deque>
#include <string>
#include <cstdint>
#include <fstream>

#include "thread.h"

// Class for chunks splitting.
// Read chunks value-by-value and write to out buffer

class merge_thread: public thread{
public:
	merge_thread();
	~merge_thread();

	void clean();

	struct task{
		std::deque<std::string> chunks;
		std::string output_filename;
		std::int64_t memory_limit;

		void clear();
		bool is_clear() const;
		task();
		task(const std::deque<std::string>& _chunks, 
			const std::string& _output_filename,
			std::int64_t _memory_limit);
	};
	void prepare(const task& t);
	void run();

	const std::string& error_message() const
		{return _error_message;}

	task check_result_and_get_task();


protected:
	void do_thread();
private:
	merge_thread(const merge_thread&);
	merge_thread& operator=(const merge_thread&);


	task _task;

	std::ofstream ofstr;
	std::vector<char> ofstr_buffer;//increase ofstream buffer for better write-perfomance
	
	// chunk stream
	struct data_source{
		std::string filename;
		std::ifstream ifstr;
		std::uint64_t value;

		bool read_value();

		data_source():value(0){};
	};

	// std::priority_queue for chunk values sorting
	struct compare{
		bool operator()(data_source* l,data_source* r) const{
			return l->value > r->value;
		};
	};
	typedef std::priority_queue<data_source*, 
		std::vector<data_source*>, compare > sorted_sources_queue;
	sorted_sources_queue sort_queue;

	std::string _error_message;
};

#endif//__split_thread_h__