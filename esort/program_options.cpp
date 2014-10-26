#include <sstream>

#include "platform.h"
#include "program_options.h"

#define DEFAULT_MEM_LIMIT_MB 10
#define MIN_MEM_LIMIT 1
#define MAX_MEM_LIMIT 4096


program_options::program_options(int argc,char** argv):memory_limit_b(DEFAULT_MEM_LIMIT_MB){
	if (argc<2)
		throw program_options_exception("filename not specified");
	filename = std::string(argv[1]);

	if (argc > 2){
		memory_limit_b = atoi(argv[2]);
		if ((memory_limit_b<MIN_MEM_LIMIT)||(memory_limit_b>MAX_MEM_LIMIT))
			throw program_options_exception("incorrect memory limit");
	}
	memory_limit_b *= 1024*1024;// in bytes
}

std::string program_options::usage(){
	std::ostringstream oss;
	oss << "Usage: " <<
#if defined(WindowsPlatform)
		" exsort.exe "
#elif defined(LinuxPlatform)
		" exsort "
#endif
	<<"<fname> [<mem_limit>] [<cache_path>]"<<std::endl;
	oss<<"Utility for external sorting of file"<<std::endl;
	oss<<std::endl;
	oss<<"Arguments:"<<std::endl;
	
	oss<<"fname: full path to target file"<<std::endl;
	oss<<"mem_limit: memory limit, Mb ("<<MIN_MEM_LIMIT<<".."<<MAX_MEM_LIMIT<<", default:"<<DEFAULT_MEM_LIMIT_MB<<")"<<std::endl;
	oss<<"cache_path: path for cache-files (separate storage increases sorting speed)"<<std::endl;

	return oss.str();
}