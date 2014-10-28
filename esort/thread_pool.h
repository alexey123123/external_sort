#ifndef __thread_pool_h__
#define __thread_pool_h__

#include <vector>
#include <algorithm>
#include "clock_ms.h"

template<typename T>
class thread_pool{
public:

	thread_pool(int num){
		while(num){
			threads.push_back(new T());
			num--;
		}
	}

	~thread_pool(){
		threads.clear();
	};

	
	void wait_all(){
		std::for_each(threads.begin(),threads.end(),[](T* t){
			t->wait();
		});

	}

	// delete all threads
	void clear(){
		for (int i=0;i!=threads.size();i++){
			T* t = threads[i];
			delete t;
		}

	}


	// return first terminated(free-to-use) thread
	T* find_ready(){
		while(true){
			for (int i=0;i!=threads.size();i++)
				if (threads[i]->is_terminated())
					return threads[i];
			//sleep 100ms
			//sleep_ms(100);
		}
	}

	std::vector<T*> get_threads()
		{return threads; }

private:
	std::vector<T*> threads;
};

#endif//__thread_pool_h__
