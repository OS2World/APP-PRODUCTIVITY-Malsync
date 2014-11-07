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

#include <AGMsg.h>

#define AGMSGStartingStringRsrc		"" \
        "Starting MAL Synchronization."
#define AGMSGRetryingStringRsrc		"" \
        "Retrying MAL Synchronization."
#define AGMSGLookupStringRsrc		"" \
        "Looking up the MAL Server."
#define AGMSGLookupFailedStringRsrc	"" \
        "Error 5453: Could not resolve the MAL Server name."
#define AGMSGConnectingStringRsrc	"" \
        "Connecting to MAL Server."
#define AGMSGConnectingFailedStringRsrc	"" \
        "Error 5455: Failed to connect to the MAL Server."
#define AGMSGCancellingStringRsrc	"" \
        "Canceling."
#define AGMSGDisconnectingRsrc		"" \
        "Disconnecting."
#define AGMSGConnectionClosedStringRsrc "" \
        "Error 5458: The MAL Server unexpectedly closed the connection."
#define AGMSGUnknownFailureStringRsrc		"" \
        "Error 5459: Unknown network error has occurred."
#define AGMSGSendingChangedStringRsrc	"" \
        "Sending data to MAL Server."
#define AGMSGSendingFailedStringRsrc      "" \
        "Error 5461: Could not send data to MAL Server."
#define AGMSGHostLookupFailedStringRsrc    "" \
        "Error 5470: Could not resolve the MAL Server name from the Internet."
#define AGMSGConnectFailedStringRsrc       "" \
        "Error 5471: The MAL Server did not respond to our connect request."
#define AGMSGWaitingForServerStringRsrc		"" \
        "Waiting for MAL Server."
#define AGMSGReadingFailedStringRsrc		"" \
        "Error 5463: Error while reading from the MAL Server."
#define AGMSGProxyDNSErrorStringRsrc		"" \
        "Error 5477: Error resolving proxy host."
#define AGMSGSocksDNSErrorStringRsrc		"" \
        "Error 5478: Error resolving proxy server."
#define AGMSGProxyConnectFailedStringRsrc ""\
        "Error 5472: Error connecting to proxy server."
#define AGMSGBadProxyAuthStringRsrc ""\
        "Error 5479: Bad Proxy Authorization."
#define AGMSGIncompatibleVersionStringRsrc ""\
        "Error 5480: Versions numbers do not match between this server and client."
#define AGMSGInvalidMagicStringRsrc ""\
        "Error 5481: Invalid MAL identification code from server. It is unlikely this is a MAL server."
#define AGMSGServerReadingFailedStringRsrc ""\
        "Error 5482: Error while reading from the device."
#define AGMSGServerInvalidMagicStringRsrc ""\
        "Error 5483: Invalid MAL identification code from device, it is unlikely this is a MAL client."

ExportFunc char *AGGetMsg(uint32 msgId)
{
    switch(msgId) {
    case AGMSGStartingStringId:	
        return AGMSGStartingStringRsrc;
    case AGMSGRetryingStringId:	
        return AGMSGRetryingStringRsrc;
    case AGMSGLookupStringId:	
        return AGMSGLookupStringRsrc;
    case AGMSGLookupFailedStringId:	
        return AGMSGLookupFailedStringRsrc;
    case AGMSGConnectingStringId:	
        return AGMSGConnectingStringRsrc;
    case AGMSGConnectingFailedStringId:	
        return AGMSGConnectingFailedStringRsrc;
    case AGMSGCancellingStringId:	
        return AGMSGCancellingStringRsrc;
    case AGMSGDisconnectingId:	
        return AGMSGDisconnectingRsrc;
    case AGMSGConnectionClosedStringId:	
        return AGMSGConnectionClosedStringRsrc;
    case AGMSGUnknownFailureStringId:	
        return AGMSGUnknownFailureStringRsrc;
    case AGMSGSendingChangedStringId:	
        return AGMSGSendingChangedStringRsrc;
    case AGMSGSendingFailedStringId:    
        return AGMSGSendingFailedStringRsrc;
    case AGMSGHostLookupFailedStringId: 
        return AGMSGHostLookupFailedStringRsrc;
    case AGMSGConnectFailedStringId:    
        return AGMSGConnectFailedStringRsrc;
    case AGMSGReadingFailedStringId:    
        return AGMSGReadingFailedStringRsrc;
    case AGMSGProxyDNSErrorStringId:    
        return AGMSGProxyDNSErrorStringRsrc;
    case AGMSGSocksDNSErrorStringId:    
        return AGMSGSocksDNSErrorStringRsrc;
    case AGMSGProxyConnectFailedStringId: 
        return AGMSGProxyConnectFailedStringRsrc;
    case AGMSG401StringId: 
    case AGMSGBadProxyAuthStringId: 
        return AGMSGBadProxyAuthStringRsrc;
    case AGMSGIncompatibleVersionStringId: 
        return AGMSGIncompatibleVersionStringRsrc;
    case AGMSGInvalidMagicStringId: 
        return AGMSGInvalidMagicStringRsrc;
    case AGMSGServerReadingFailedStringId: 
        return AGMSGServerReadingFailedStringRsrc;
    case AGMSGServerInvalidMagicStringId: 
        return AGMSGServerInvalidMagicStringRsrc;
    }
    return NULL;
}

