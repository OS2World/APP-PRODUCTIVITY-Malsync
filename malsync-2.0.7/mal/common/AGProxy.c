/* The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mobile Application Link.
 *
 * The Initial Developer of the Original Code is AvantGo, Inc.
 * Portions created by AvantGo, Inc. are Copyright (C) 1997-1999
 * AvantGo, Inc. All Rights Reserved.
 *
 * Contributor(s):
 */

#include <AGProxy.h>
#include <AGBase64.h>
#include <AGNet.h>
#include <AGTypes.h>
#include <AGUtil.h>

#ifndef __palmos__
//#include <stdio.h>	(adam) temporary hack - compile this out to build on ce
#endif

typedef enum _sockreturncodes {
    SOCKS_OK = 90,
    SOCKS_DENIED = 91,
    SOCKS_COULDNT_CONNECT = 92,
    SOCKS_BAD_ID = 93,
    /* Non protocol errors */
    SOCKS_BAD_VERSION,
    SOCKS_UNKNOWN_ERROR

} SocksCode;

static SocksCode proxySocksParseResponse(void *buffer);
static char *authEncodePassword(char *name, char *password);

#define AUTH_STRING "Proxy-authorization: Basic %s\r\n"
#define BASIC_AUTH_STRING "Authorization: Basic %s\r\n"

/*---------------------------------------------------------------------------*/
AGBool AGProxyCheckExclusionArray(AGArray *array, char *addrString)
{
    char *token;
    int len1, len2, i;
    char *ptr;
    
    for (i=0; i < AGArrayCount(array); i++) {
        
        token = (char *)array->elements[i];

        len1 = strlen( token );
        len2 = strlen( addrString );
        if( len1 > len2 )
            continue;

        ptr = addrString + ( len2 - len1 );
        if( strcmp( ptr, token) == 0 ) {
            return TRUE;
        }
    
    }
    return FALSE;

}
/*---------------------------------------------------------------------------*/
char *AGProxyCreateAuthHeader(char *user, char *pass, AGBool dobasic)
{
    char *authString;
    char *header = NULL;
    int len;

    authString = authEncodePassword(user, pass);
    if (authString) {
        len = strlen(AUTH_STRING) + strlen(authString) + 3;
        header = (char *)malloc(len);
        if (!header) {
            free(authString);
            return NULL;
        }
        if (dobasic)
            sprintf(header, BASIC_AUTH_STRING, authString);
        else
            sprintf(header, AUTH_STRING, authString);
        free(authString);
    }
    return header;

}
/*---------------------------------------------------------------------------*/
char *AGSocksBufCreate(unsigned long laddr, short _port, int *buflen)
{
    int minlen;
    uint8 *buf, *buffer;
    short port;
    char *userid = "AGUser";

    minlen = 9 + strlen(userid);
    buffer = buf = (uint8*)malloc(minlen);

    if (!buf)
        return NULL;
    
    /* Version */
    *buf++ = 4; 

    /* Socks connect request */
    *buf++ = 1;
 
    /* Write port in network byte order */
    port = htons(_port);
    memcpy(buf, &port, 2);
    buf += 2;

    /* Write internet addr (Its already in network order) */
    memcpy(buf, &laddr, 4);
    buf += 4;

    memcpy(buf, userid, strlen(userid));
    buf += strlen(userid);

    *buf = 0;
    *buflen = minlen;
    return (char *)buffer;

}
/*---------------------------------------------------------------------------*/
int AGSocksGetResponse(char *buffer)
{
    int rc = 0;
    
    switch(proxySocksParseResponse(buffer)) {
        case SOCKS_OK:
            return 0;
        case SOCKS_COULDNT_CONNECT:
            rc = AG_NET_SOCKS_COULDNT_CONNECT;
            break;
        case SOCKS_BAD_ID:
            rc = AG_NET_SOCKS_BAD_ID;
            break;        
        default:
            rc = AG_NET_SOCKS_ERROR;
            break;
    }
    return 0;

}
/*---------------------------------------------------------------------------*/
static SocksCode proxySocksParseResponse(void *buffer)
{
    uint8* buf = (uint8*)buffer;

    /* Ignore the version string */

    buf++;
    switch (*buf) {
      case SOCKS_OK:
      case SOCKS_DENIED:
      case SOCKS_COULDNT_CONNECT:
      case SOCKS_BAD_ID:
        return (SocksCode)*buf;
      default:
        return SOCKS_UNKNOWN_ERROR;
    }
    

}
/*---------------------------------------------------------------------------*/
static char *authEncodePassword(char *name, char *password)
{
    char *baseAuthString;
    int len;
    char *authString;
    
    len = strlen(name) + strlen(password) + 2;

    baseAuthString = (char*)malloc(len);
    sprintf(baseAuthString, "%s:%s", name, password);

    /* create base 64 encoding of "userid:password" */
    authString = AGBase64Encode((uint8 *)baseAuthString, 0);

    return authString;

}

