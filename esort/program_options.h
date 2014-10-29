#ifndef __arguments_h__
#define __arguments_h__

#include <string>
#include <cstdint>
#include <stdexcept>

class program_options_exception: public std::runtime_error{
public:
	program_options_exception(const std::string& message): std::runtime_error(message){};
};

struct program_options{
	std::string		filename;
	std::int64_t	memory_limit_b;
	int				threads_count;
	
	program_options(int argc,char** argv);//throws
	
	static std::string usage();
};


#endif//__arguments_h__