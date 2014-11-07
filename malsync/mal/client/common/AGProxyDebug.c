#if defined(_WIN32) && !defined(_WIN32_WCE)
/*
 * Copyright 1999 AvantGo, Inc. All rights reserved.
 *
 * AGProxyDebug.c
 *
 * This file contains the functions will write to a buffer 
 * which is protected by a mutex.
 *
 */
#include <stdio.h> 
#include <stdarg.h>
#include <AGProxyDebug.h>

static int bufsize = 4096;
static int bib; 

/*---------------------------------------------------------------------------*/
void proxyDebugInit(AGClientProcessor *cp)
{

    /* Make a mutex cause we are running two threads on win32. */
    if (!cp->logMutex)
        cp->logMutex = CreateMutex(NULL, FALSE, NULL);

}
/*---------------------------------------------------------------------------*/
void proxyDebugCleanup(AGClientProcessor *cp)
{
    if (cp->logMutex)
        CloseHandle(cp->logMutex);

    /* can't free read buffer here */
}
/*---------------------------------------------------------------------------*/
int proxyDebug(AGClientProcessor *cp, const char *fmt, ...)
{
    int ret;
    va_list  ap;
    uint8 *ptr;
    int bf; 

    if (!cp->logMutex)
        return -1;


    /* Lock mutex */
    WaitForSingleObject(cp->logMutex, INFINITE);

    if (!cp->debugBuffer) {
        bib = 0;
        cp->debugBuffer = malloc(bufsize);
        if (!cp->debugBuffer) {
            ReleaseMutex(cp->logMutex);
            return -1;
        }
    }

    /* Compute bytes free */
    bf = bufsize - bib;

    /* Allright this is pretty hokey, but i really can't imagine
    needing to write more than a KB of crap. */
    if (bufsize < bf) {
        bufsize = bufsize + 4096;
        ptr = cp->debugBuffer;
        cp->debugBuffer = realloc(ptr, bufsize);
        if (!cp->debugBuffer) {
            free(ptr);
            ReleaseMutex(cp->logMutex);
            return -1;
        }
        /* need to recaluate bytes free */
        bf = bufsize - bib;
    }

    ptr = cp->debugBuffer + bib;

    va_start(ap, fmt);
    ret = _vsnprintf(ptr, bf, fmt, ap);
    va_end( ap );    

    bib = strlen(cp->debugBuffer);
    ReleaseMutex(cp->logMutex);
    return ret;

}

#endif