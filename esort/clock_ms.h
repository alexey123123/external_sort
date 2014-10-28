#ifndef __clock_ms_h__
#define __clock_ms_h__

// timeout measurement, 
// sleeping


// ms count from program start
unsigned int clock_ms();

// interval
unsigned int diff_clock_ms(unsigned int from,unsigned int to);

inline unsigned int from_clock_ms(unsigned int from) 
	{ return diff_clock_ms(from, clock_ms() ); }

// 
void sleep_ms(int timeout);

#endif//__clock_ms_h__