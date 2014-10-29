#ifndef __sort_thread_h__
#define __sort_thread_h__

#include <cstdint>
#include <string>
#include <fstream>
#include <vector>


#include "thread.h"

class sort_thread: public thread{
public:
	sort_thread();
	~sort_thread();

	std::streamsize read(std::ifstream* ifstr, std::streamsize count);
	void sort_and_save(const std::string& filename);

	std::string check_result_and_get_filename();

	std::string& error()
		{return error_message;}
protected:
	void do_thread();
private:
	sort_thread(const sort_thread&);
	sort_thread& operator=(const sort_thread&);

	std::vector<std::uint64_t> data;
	unsigned int values_count;
	std::string _filename;
	std::string error_message;
};

#endif//__sort_thread_h__