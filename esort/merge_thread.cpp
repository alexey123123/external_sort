#include <cstdio>
#include <stdexcept>
#include <sstream>
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
	_task.clear();
	_error_message = "";
}

void merge_thread::prepare(const task& t){

	// some cleanup
	wait();
	clean();

	if (t.chunks.size()<2)
		throw std::runtime_error("chunks < 2");



	// write buffer	
	try{
		ofstr_buffer.resize((std::vector<char>::size_type)t.memory_limit);
	} 
	catch(...){
		std::ostringstream oss;
		oss << "cannot allocate vector<char> for "<<t.memory_limit<<" values";
		throw std::runtime_error(oss.str());
	}
	
	//override ofstream buffer
	std::streambuf* sb = ofstr.rdbuf();
	sb->pubsetbuf(&ofstr_buffer[0],ofstr_buffer.size());

	ofstr.open(t.output_filename, std::ios::binary);
	if (!ofstr)
		throw std::runtime_error("cannot open file("+t.output_filename+") for write");

	// create data_source for each chunk
	std::for_each(t.chunks.begin(),t.chunks.end(),[&](std::string c_fname){
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


	_task = t;
	
}


void merge_thread::run(){
	_error_message = "";
	start();
}

void merge_thread::do_thread(){
	try{
		
		// TODO: optimization:
		// if (sort_queue.size()==1) => write all remained data from s->ifstream >> ofstream

		while (sort_queue.size() > 0){
			
			data_source* s = sort_queue.top();		// get front(lowest) value
			sort_queue.pop();

			ofstr.write((char*)&s->value,sizeof(std::uint64_t));
			if (!ofstr)
				throw std::runtime_error("cannot write data to "+_task.output_filename);
		
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

merge_thread::task merge_thread::check_result_and_get_task(){
	if (!is_terminated())
		throw std::runtime_error("thread is not terminated");
	if (_error_message != "")
		throw std::runtime_error("save data error:" + _error_message);

	task t = _task;
	_task.clear();
	return t;
}

void merge_thread::task::clear(){
	output_filename = "";
	memory_limit = 0;
	chunks.clear();
}
bool merge_thread::task::is_clear() const{
	return (chunks.size()==0) && (output_filename=="") && (memory_limit==0);
}
merge_thread::task::task():memory_limit(0){

}

merge_thread::task::task(const std::deque<std::string>& _chunks, 
	const std::string& _output_filename,
	std::int64_t _memory_limit):chunks(_chunks),output_filename(_output_filename),memory_limit(_memory_limit){

}


