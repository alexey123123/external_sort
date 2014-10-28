#include <cstdio>
#include <stdexcept>
#include <algorithm>

#include "merge_thread.h"
#include "clock_ms.h"

merge_thread::merge_thread(){

}


merge_thread::~merge_thread(){
	wait();
	clean();
}

void merge_thread::clean(){
	ofstr.close();
	while (!sort_queue.empty()){
		data_source* s = sort_queue.top();
		delete (s);
		sort_queue.pop();
	}
	_error_message = "";
	_output_filename = "";
}

void merge_thread::prepare(const std::deque<std::string>& chunks, 
						const std::string& filename, std::uint64_t mem_limit){
	// some cleanup
	wait();
	clean();

	if (chunks.size()<2)
		throw std::runtime_error("chunks < 2");

	_output_filename = filename;

	// write buffer	
	ofstr_buffer.resize(mem_limit);
	std::streambuf* sb = ofstr.rdbuf();
	sb->pubsetbuf(&ofstr_buffer[0],ofstr_buffer.size());

	ofstr.open(filename, std::ios::binary);
	if (!ofstr)
		throw std::runtime_error("cannot open file("+filename+") for write");

	// create data_source for each chunk
	std::for_each(chunks.begin(),chunks.end(),[&](std::string c_fname){
		data_source* s = new data_source();

		s->ifstr.open(c_fname, std::ios::binary);
		if (!s->ifstr){
			delete (s);
			throw std::runtime_error("cannot open file ("+c_fname+")");
		}
		s->filename = c_fname;
		//initial read
		if (s->read_value())
			sort_queue.push(s); else
			delete (s); //file is empty ?
	});
	
}


void merge_thread::run(){
	_error_message = "";
	start();
}

void merge_thread::do_thread(){
	try{
		
		std::uint64_t data_count = 0;

		// TODO: optimization:
		// if (sort_queue.size()==1) => write all remained data from s->ifstream >> ofstream

		while (sort_queue.size() > 0){
			
			data_source* s = sort_queue.top();		// get front(lowest) value
			sort_queue.pop();

			ofstr.write((char*)&s->value,sizeof(std::uint64_t));
			if (!ofstr)
				throw std::runtime_error("cannot write data to "+_output_filename);
		
			// have another value? 
			if (s->read_value())
				sort_queue.push(s); //put data_source back in queue
			else
				delete s;
		}
		ofstr.close();		
	}
	catch(std::runtime_error& ex){
		_error_message = std::string(ex.what());
	}
}



bool merge_thread::data_source::read_value(){
	ifstr.read((char*)&value, sizeof(std::uint64_t));
	return ifstr.gcount() == sizeof(std::uint64_t);
}

std::string merge_thread::check_result_and_get_filename(){
	if (!is_terminated())
		throw std::runtime_error("thread is not terminated");
	if (_error_message != "")
		throw std::runtime_error("save data error:" + _error_message);

	std::string f1 = _output_filename;
	_output_filename = "";
	return f1;
}
