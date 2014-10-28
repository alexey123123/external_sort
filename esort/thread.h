#ifndef __Thread_h__
#define __Thread_h__


// abstract class for thread (Win/linux)

class thread{
public:
	thread();
	virtual ~thread();

	// start thread
	// (true - thread is already started)
	bool start();
	// stop thread in check_cancel() point
	bool cancel();

	// join to terminated thread
	bool wait();

	// only check
	bool is_terminated() const;

protected:

	void check_cancel();
	virtual void do_thread()=0;

private:
	thread(const thread&);
	thread& operator=(const thread&);

	class impl;
	impl* pimpl;
	friend class impl;
};
#endif//__Thread_h__


