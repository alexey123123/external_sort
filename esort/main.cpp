#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <iterator>

#include "program_options.h"

#include "thread_pool.h"
#include "sort_thread.h"
#include "merge_thread.h"


/*
	Utility for sorting large amount of data with limited memory resources.
	File split into chunks, sorted and merged.
	Realization of algorithm: http://en.wikipedia.org/wiki/External_sorting
*/



std::string get_chunk_filename(int num);
void delete_files(const std::deque<std::string>& fnames);






int main(int argc,char** argv){

	std::cout << "File sort utility (v1.0)"<<std::endl;


	std::deque<std::string> chunk_filenames;
	try{

		unsigned int start_clockms = clock_ms();

		//parse options
		program_options po(argc,argv);

		std::ifstream fstr(po.filename, std::ios::binary );
		if (!fstr)
			throw program_options_exception("cannot open <"+po.filename+">");


		std::cout<<"sorting file ["<<po.filename<<"] in "<<
			po.threads_count<<" threads with memory limit="<<
			po.memory_limit_b<<" bytes"<<std::endl;

		thread_pool<sort_thread> tp(po.threads_count);

		// calculate chunk size
		std::int64_t chunk_size = po.memory_limit_b / po.threads_count;
		chunk_size -= chunk_size % sizeof(std::uint64_t);
		std::cout << chunk_size<<" bytes for 1 thread"<<std::endl;
		int chunk_num = 0;
		

		std::streamsize rbytes = 0;
		do{
			sort_thread* t = tp.find_ready();
			if (!t)
				throw std::runtime_error("cannot get thread");

			std::string fname = t->check_result_and_get_filename();//previous completed chunk filename
			if (fname!="")
				chunk_filenames.push_back(fname);

			//read next chunk
			rbytes = t->read(&fstr,chunk_size);
			if (rbytes==0)
				break;

			fname = get_chunk_filename(chunk_num);
			std::cout<<"started sort of "<<rbytes<<" bytes to "<<fname<<std::endl;
			chunk_num++;

			t->sort_and_save(fname);

		}
		while (!fstr.eof());

		std::cout<<"waiting for all threads...";

		//wait all threads for finish and get chunk filenames
		tp.wait_all();
		std::vector<sort_thread*> threads = tp.get_threads();
		std::for_each(threads.begin(),threads.end(),[&](sort_thread* t){
			std::string fname = t->check_result_and_get_filename();
			if (fname!=""){
				chunk_filenames.push_back(fname);
			}
				
		});
		tp.clear();
		std::cout<<"Ok"<<std::endl;

		std::cout<<"splitted to "<<chunk_filenames.size()<<" chunks"<<std::endl;
		int chunks_count = chunk_filenames.size();

		//------- merge chunks ---------		
		while (chunk_filenames.size() > 1){
			
			// how many threads we need ?
			unsigned int threads_count, chunks_in_thread;
			if (chunks_count < po.threads_count){
				//2 chunks in 1 thread
				threads_count = chunks_count / 2;
				threads_count += chunks_count % 2;	//+1
				chunks_in_thread = 2;
			} else{
				threads_count = po.threads_count;
				chunks_in_thread = chunks_count / threads_count;
			}
			
			
			std::uint64_t memory_for_thread = po.memory_limit_b / threads_count;

			thread_pool<merge_thread> sp_pool(threads_count);

			std::deque<std::string> new_chunks;

			//collect chunks in portions (chunks_in_thread) and submit to thread
			std::deque<std::string>::iterator it = chunk_filenames.begin();
			while (it != chunk_filenames.end()){

				std::deque<std::string> portion;
				while ((portion.size() < chunks_in_thread)&&(it!=chunk_filenames.end())){
					portion.push_back(*it);
					it++;
				}


				if (portion.size() == 1){
					std::cout<<"last chunk ("<<portion[0]<<") saved to next iteration"<<std::endl;
					new_chunks.push_back( portion[0] );
					break;
				}

				merge_thread* st = sp_pool.find_ready();
				if (!st)
					throw std::runtime_error("cannot get split_thread");

				merge_thread::task prev_t = st->check_result_and_get_task();
				if (!prev_t.is_clear()){
					std::cout<< prev_t.output_filename <<" ready"<<std::endl;
					new_chunks.push_back( prev_t.output_filename );
					delete_files(prev_t.chunks);
				}
					

				st->clean();

				//get filename
				std::string result_chunk_filename = get_chunk_filename(chunk_num);
				chunk_num++;

				//prepare thread
				st->prepare( merge_thread::task(portion, result_chunk_filename, memory_for_thread) );

				std::cout<<"splitting "<<portion.size()<<" chunks to "<<result_chunk_filename<<"..."<<std::endl;
				//run....
				st->run();
			}
			//wait all threads
			sp_pool.wait_all();
			//fetching results
			std::vector<merge_thread*> threads = sp_pool.get_threads();
			std::for_each(threads.begin(),threads.end(),[&](merge_thread* t){
				merge_thread::task prev_t = t->check_result_and_get_task();
				if (!prev_t.is_clear()){
					std::cout<< prev_t.output_filename <<" ready"<<std::endl;
					new_chunks.push_back( prev_t.output_filename );
					delete_files(prev_t.chunks);
				}			
			});
			sp_pool.clear();
			std::cout<<"Ok"<<std::endl;
			chunk_filenames = new_chunks;		
		}

		//last chunk contains result file
		std::string r_fname = po.filename+".sorted";
		std::remove(r_fname.c_str());
		if (std::rename(chunk_filenames[0].c_str(),r_fname.c_str())!=0)
			throw std::runtime_error("cannot rename "+chunk_filenames[0]+" to "+r_fname);
		std::cout<<"result file: "<<r_fname<<std::endl;
		std::cout<<"generated in: "<<(double)(from_clock_ms(start_clockms))/1000<<" sec"<<std::endl;

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


std::string get_chunk_filename(int num){
	std::ostringstream oss;
	oss << "chunk" << num << ".bin";
	return oss.str();
}

void delete_files(const std::deque<std::string>& fnames){
	std::for_each(fnames.begin(),fnames.end(),[](std::string fname){
		if (std::remove(fname.c_str()) != 0)
			throw std::runtime_error("cannot remove file:" + fname);
		std::cout<<"deleted "<<fname<<std::endl;
	});

}




