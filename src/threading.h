#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

//Needed because mingw-w64's implementation is awful.
#ifdef ALT_MINGW_THREADS

//supress pragma message
#define MINGW_STDTHREAD_REDUNDANCY_WARNING 
#include "mingw.thread.h"
#undef MINGW_STDTHREAD_REDUNDANCY_WARNING

#pragma message "Alternate thread for mingw implementation enabled"
#define threadNS mingw_stdthread
#define SLEEP_FOR(x) mingw_stdthread::this_thread::sleep_for(x)

#else

#include <thread>

#define threadNS std
#define SLEEP_FOR(x) std::this_thread::sleep_for(x)

#endif

#endif