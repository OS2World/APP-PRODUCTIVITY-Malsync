#if defined(_WIN32) && !defined(_WCE)

#ifndef __AGPROXYDEBUG_H__
#define __AGPROXYDEBUG_H__
/*
 * Copyright 1999 AvantGo, Inc. All rights reserved.
 *
 * AGProxyDebug.h
 *
 * This file contains the functions will write to a buffer 
 * which is protected by a mutex.
 *
 */

#include <AGClientProcessor.h>

#ifdef __cplusplus
extern "C" {
#endif

ExportFunc void proxyDebugInit(AGClientProcessor *cp);
ExportFunc int  proxyDebug(AGClientProcessor *cp, const char *fmt, ...);
ExportFunc void proxyDebugCleanup(AGClientProcessor *cp);

#ifdef __cplusplus
}
#endif 

#endif
#endif 