#include <sstream>

#include "platform.h"
#include "program_options.h"

#define DEFAULT_MEM_LIMIT_MB 10
#define MIN_MEM_LIMIT 1
#define MAX_MEM_LIMIT 4096
#define DEFAULT_THREADS_COUNT 5
#define MAX_THREADS_COUNT 100


program_options::program_options(int argc,char** argv):
											memory_limit_b(DEFAULT_MEM_LIMIT_MB),
											threads_count(DEFAULT_THREADS_COUNT){
	if (argc<2)
		throw program_options_exception("filename not specified");
	filename = std::string(argv[1]);

	if (argc > 2){
		memory_limit_b = atoi(argv[2]);
		if ((memory_limit_b<MIN_MEM_LIMIT)||(memory_limit_b>MAX_MEM_LIMIT))
			throw program_options_exception("incorrect memory limit");
	}
	memory_limit_b *= 1024*1024;// in bytes

	if (argc > 3){
		threads_count = atoi(argv[3]);
		if ((threads_count<1)||(threads_count>MAX_THREADS_COUNT))
			throw program_options_exception("incorrect threads count");
	}

}

std::string program_options::usage(){
	std::ostringstream oss;
	oss << "Usage: " <<
#if defined(WindowsPlatform)
		" exsort.exe "
#elif defined(LinuxPlatform)
		" exsort "
#endif
	<<"<fname> [<mem_limit> [<cache_path> [<th_count>]"<<std::endl;
	oss<<"Utility for external sorting of file"<<std::endl;
	oss<<std::endl;
	oss<<"Arguments:"<<std::endl;
	
	oss<<"fname: full path to target file"<<std::endl;
	oss<<"mem_limit: memory limit, Mb ("<<MIN_MEM_LIMIT<<".."<<MAX_MEM_LIMIT<<", default:"<<DEFAULT_MEM_LIMIT_MB<<")"<<std::endl;
	oss<<"th_count: count of threads (default:"<<DEFAULT_THREADS_COUNT<<")"<<std::endl;

	return oss.str();
}