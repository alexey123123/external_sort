#ifndef __platform_h__
#define __platform_h__



#if defined(WIN32)
# define WindowsPlatform
#elif defined(__linux)
# define LinuxPlatform
#endif

#if defined(WindowsPlatform) || defined(LinuxPlatform)
#else
#error Platform not detected
#endif


#endif//__platform_h__