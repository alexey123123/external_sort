#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "program_options.h"
#include "sort_thread.h"
#include "thread_pool.h"




int main(int argc,char** argv){

	std::cout << "File sort utility (v1.0)"<<std::endl;

	try{
		program_options po(argc,argv);

		std::ifstream fstr(po.filename, std::ios::binary);
		if (!fstr)
			throw program_options_exception("cannot open <"+po.filename+">");

		//TODO: split to chunks
		//TODO: merge chunks

	}
	catch(program_options_exception& ex){
		std::cerr<<"invalid argument: "<<ex.what()<<std::endl;
		std::cerr << program_options::usage() << std::endl;
	}
	catch(std::runtime_error& ex){
		std::cerr<<"runtime error:"<<ex.what()<<std::endl;
	}

	return 0;
}

