#ifndef __arguments_h__
#define __arguments_h__

#include <string>
#include <stdexcept>

class program_options_exception: public std::runtime_error{
public:
	program_options_exception(const std::string& message): std::runtime_error(message){};
};

struct program_options{
	std::string		filename;
	unsigned long	memory_limit_b;
	std::string		cache_path;
	
	program_options(int argc,char** argv);//throws
	
	static std::string usage();
};


#endif//__arguments_h__