#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <limits>
#include "sort_thread.h"

sort_thread::sort_thread():values_count(0){

}

sort_thread::~sort_thread(){
	wait();
}


std::streamsize sort_thread::read(std::ifstream* ifstr, std::streamsize count){
	if (count % sizeof(std::uint64_t) != 0)
		throw std::runtime_error("count is not aligned to uint64");

	//check vector size

	if (count / sizeof(std::uint64_t) > std::numeric_limits<unsigned int>().max())
		throw std::runtime_error("values count overhead");

	values_count = (unsigned int)(count / sizeof(std::uint64_t));
	
	if (data.size() < values_count)
		try{
			data.resize(values_count);
		}
		catch(...){
			std::ostringstream oss;
			oss << "cannot allocate vector<std::uint64_t> for "<<values_count<<" values";
			throw std::runtime_error(oss.str());
		}
		

	ifstr->read((char*)&data[0], count);
	std::streamsize readed_b = ifstr->gcount();
	if (readed_b != count)
		values_count = (std::vector<std::uint64_t>::size_type)(readed_b / sizeof(std::uint64_t)); //possible cut data to 8 bytes

	return readed_b;
}

void sort_thread::sort_and_save(const std::string& filename){
	_filename = filename;

	start();
}

void sort_thread::do_thread(){
	try{
		//sort
		std::sort(data.begin(),
			data.begin() + values_count,
			std::less<std::uint64_t>());

		//save
		std::ofstream ofstr(_filename,std::ios::binary | std::ios::trunc);
		if (!ofstr)
			throw std::runtime_error("cannot open file");
		ofstr.write((char*)&data[0],values_count * sizeof(std::uint64_t));
		if (!ofstr)
			throw std::runtime_error("cannot write data");

		ofstr.close();

	}
	catch(std::runtime_error& ex){
		error_message = std::string(ex.what());
	}
}

std::string sort_thread::check_result_and_get_filename(){
	if (error_message != "")
		throw std::runtime_error("save data error:" + error_message);

	std::string f1 = _filename;
	_filename = "";
	return f1;
}


