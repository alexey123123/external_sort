#include <vector>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "../../esort/merge_thread.h"
#include "../../esort/clock_ms.h"


/*
	Test utility for merge_thread
	1. Generate test data & save it to separate chunks
		1 2 3 4 5 6 .....
		1 2 3 3 4 5 .....
		.....
	2. Merge chunks
	3. Check result:
		1 1 1 1...... 2 2 2 2 2.....
*/



#define CHUNK_LEN		1000000
#define CHUNKS_COUNT	10
#define MEMORY_LIMIT	15*1024*1024	
#define RESULT_FILENAME "result.bin"



int main(int argc,char** argv){
	
	std::deque<std::string> chunk_fnames;

	try{
		//generate data chunks and 
		std::cout<<"generate data...";
		std::vector<std::uint64_t> data;
		data.reserve(CHUNK_LEN);
		for (int i=0;i!=CHUNK_LEN;i++)
			data.push_back(i);
		
		
		
		int ch_count = 0;
		while(ch_count < CHUNKS_COUNT){
			std::ostringstream oss;
			oss << "chunk"<<ch_count<<".bin";
			std::string fname = oss.str();
			std::ofstream ofstr(fname, std::ios::binary | std::ios::out);
			if (!ofstr)
				throw std::runtime_error("cannot create chunk file "+fname);
			ofstr.write((char*)&data[0], data.size() * sizeof(std::uint64_t));
			if (!ofstr)
				throw std::runtime_error("cannot save data in "+fname);
			ofstr.close();
			chunk_fnames.push_back(fname);
			ch_count++;
		}
		data.clear();
		data.shrink_to_fit();
		std::cout<<"Ok ("<<chunk_fnames.size() << " files)"<<std::endl;

		//split it

		merge_thread mt;
		mt.prepare( merge_thread::task(chunk_fnames, RESULT_FILENAME, MEMORY_LIMIT) );

		std::cout<<"merge...";
		unsigned int start_clockms = clock_ms();
		mt.run();

		mt.wait();

		if (mt.error_message() != "")
			throw std::runtime_error("merge error:" + mt.error_message());

		std::cout<<"Ok (in "<<(float)from_clock_ms(start_clockms) / 1000<<" seconds)"<<std::endl;


		// check result
		std::cout<<"check result...";
		std::ifstream result_ifstr(RESULT_FILENAME,std::ios::binary);
		if (!result_ifstr)
			throw std::runtime_error("cannot open result filename");


		// reading portions of CHUNKS_COUNT values		
		std::vector<std::uint64_t> portion_data;
		portion_data.resize(CHUNKS_COUNT * sizeof(std::uint64_t));
		for (int i=0;i!=CHUNK_LEN;i++){

			result_ifstr.read((char*)&portion_data[0],CHUNKS_COUNT);
			if (result_ifstr.gcount() != CHUNKS_COUNT)
				throw std::runtime_error("invalid output file size");
		
			// all values in portion should be equals
			std::uint64_t check_value = 0;
			std::for_each(portion_data.begin(),portion_data.end(),[&](std::uint64_t n){
				check_value |= n;
			});
			check_value ^= portion_data[0];

			if (check_value != 0)
				throw std::runtime_error("invalid data in result file");
		}
		std::cout<<"Ok"<<std::endl;


	}
	catch(std::runtime_error& ex){
		std::cerr<<"test failed: "<<ex.what()<<std::endl;
	}

	std::cout<<"cleaning up..."<<std::endl;
	std::for_each(chunk_fnames.begin(),chunk_fnames.end(),[](std::string fname){
		std::remove(fname.c_str());
	});
	std::remove(RESULT_FILENAME);





	return 0;
}