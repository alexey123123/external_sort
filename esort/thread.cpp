#include "platform.h"
#include "thread.h"

#if defined(LinuxPlatform)
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#elif defined(WindowsPlatform)
#include <windows.h>
#include <process.h> // for _beginthreadex, _endthreadex
#else
#error Not implemented
#endif // Platform

#include <string>
#include <cassert>
#include <cstdint>


////////////////////////////////////////////////
// реализация потока для
// поддерживаемых платформ
//


#if defined(LinuxPlatform)

// реализация для Posix (Linux)
// с использованием posix-threads
class thread::impl {
public:
	impl(): pt(0) {}
	~impl() { join(-1); }

	static void* thread_proc(void* param) {
		thread* threadobj=(thread*)param;
		threadobj->do_thread();
		pthread_exit(NULL);
		return NULL;
	}

	bool create(thread* threadobj) {
		if (pt) return false;
		pthread_create(&pt,NULL,thread_proc,threadobj);

		usleep(500000); // 500 ms, for possible join call
		return true;
	}

	bool cancel() {
		if (!pt) return false;
		pthread_cancel(pt);
		return true;
	}

	bool join(int timeout_ms) {
		if (!pt) return true;
		if (timeout_ms >= 0){
			int t = timeout_ms;
			if (t==0)
				t = 1;


			struct timespec ts;
			int s;

			if (clock_gettime(CLOCK_REALTIME, &ts) == -1) 
				return false;

			ts.tv_sec += timeout_ms / 1000;
			ts.tv_sec += (timeout_ms % 1000) * 1000;

			s = pthread_timedjoin_np(pt, NULL, &ts);
			if (s != 0) 
				return false;
		} else
			pthread_join(pt,NULL);
		pt=0;
		return true;
	}

	bool is_terminated(){
		return join(0);
	}

	void test_cancel() {
		pthread_testcancel();
	}

	void disable_cancel() {
		int oldstate;
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&oldstate);
	}

	void enable_cancel() {
		int oldstate;
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&oldstate);
	}

protected:



private:
	pthread_t pt;
};

#elif defined(WindowsPlatform)

// 
class thread::impl {
public:
	impl(): threadhandle(0) {}
	~impl() { join(-1); }

	static std::uint32_t __stdcall thread_proc(void* param) {
		thread* threadobj=(thread*)param;
		threadobj->do_thread();
		::_endthreadex(0);
		return 0;
	}

	bool create(thread* threadobj) {
		if (threadhandle) return false;
		cancelevent=::CreateEvent(NULL,FALSE,FALSE,NULL);
		threadhandle=(HANDLE)::_beginthreadex(0,0,thread_proc,threadobj,0,&threadid);
		::Sleep(100);
		return true;
	}

	bool cancel() {
		if (!threadhandle) return false;
		::SetEvent(cancelevent);
		return true;
	}

	bool join(int timeout_ms) {
		if (!threadhandle) return true;		

		if (timeout_ms < 0)
			timeout_ms = INFINITE;

		DWORD res=::WaitForSingleObject(threadhandle,timeout_ms);
		if (res!=WAIT_OBJECT_0) return false;
		::CloseHandle(threadhandle);
		::CloseHandle(cancelevent);
		threadhandle=0;
		return true;
	}

	bool is_terminated(){
		return join(0);
	}

	void test_cancel() {
		DWORD res=::WaitForSingleObject(cancelevent,0);
		if (res==WAIT_OBJECT_0)
			::_endthreadex(0);
	}

	void disable_cancel() {}
	void enable_cancel() {}

private:
	std::uint32_t threadid;
	HANDLE threadhandle;
	HANDLE cancelevent;
};

#endif // WindowsPlatform



thread::thread(){
	pimpl = new impl();
}


thread::~thread() {
	delete pimpl;
}

bool thread::start() {
	return pimpl->create(this);
}

bool thread::cancel() {
	return pimpl->cancel();
}

void thread::check_cancel() {
	pimpl->test_cancel();
}


bool thread::wait(){
	return pimpl->join(-1);
}

bool thread::is_terminated() const{
	return pimpl->is_terminated();
}
