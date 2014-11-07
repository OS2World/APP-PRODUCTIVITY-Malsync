/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

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
 */
/*

  See the README file in this directory for info on compiling, using
  and the state of this program. If the readme file is for some reason
  not there, it can be found at the MAL website:

  http://corp.avantgo.com/mal/

*/
/* Thu, 29 Jul 1999 12:43:01 +0200
 *     Turbo Fredriksson <turbo@nocrew.org>
 *     Added daemon mode (close STDIO and disconnect from the shell
 *     Rewrote the command line parsing (to use getopt_long)
 */
#include <stdio.h>
#include <signal.h>

#ifdef _WIN32
/* Fucking windows doesn't have getopt? */
#include <getopt.h>
#else
#include <unistd.h>
#if !defined(_HPUX_SOURCE) && !defined(__EMX__)
	#include <dlfcn.h>
#endif
#endif

#ifdef HAVE_GETOPTLONG
#include <getopt.h>
#endif

#ifdef __EMX__
	#define INCL_DOSERRORS
	#define INCL_DOSMODULEMGR
	#include <os2.h>
#endif

#include <AGNet.h>
#include <AGUtil.h>
#include <AGClientProcessor.h>
#include <AGProtocol.h>
#include <AGBufferReader.h>
#include <AGPalmProtocol.h>
#include <AGUserConfig.h>
#include <AGServerConfig.h>
#include <AGSyncCommon.h>
#include <AGCommandProcessor.h>
#include <AGDesktopInfoPalm.h>
#include <AGTimeoutMessageBox.h>
#include <AGMobileLinkSettings.h>
#include <MAL31UserConfig.h>

#include <pi-source.h>
#include <pi-socket.h>
#include <pi-file.h>
#include <pi-dlp.h>
#include <pi-version.h>

#define DEFAULT_CARD_NUM 0

#define VERSION_STRING "malsync version 2.0.7"

static void Connect();
static char *device = "/dev/pilot";
static char *httpProxy      = NULL;
static int   httpProxyPort  = 0;
static char *socksProxy     = NULL;
static int   socksProxyPort = 0;
static char *proxyUsername  = NULL;
static char *proxyPassword  = NULL;
static char *progname       = NULL;
#ifdef _DEBUG
#define debug_print(x) printf("%s\n", (x));
#else
#define debug_print(x)
#endif

typedef struct {

    AGDeviceInfo * deviceInfo;
    AGUserConfig * userConfig;
    AGServerConfig * currentServerConfig;
    AGClientProcessor * clientProcessor;
    AGPlatformCalls * platform;
    AGRecord * record;
    AGDBConfig * currentDb;
    AGCommandProcessor *commandProcessor;
    AGBool quit;
    
    /* Pilot-specific */
    AGBool currentDBIsResourceType;
    int pilot_rHandle;
    int pilot_RecIndex;
    recordid_t id;
    uint8  *pilot_buffer;
    int pilot_buffer_size;
    
    /* Secure Network Library Stuff */
    AGBool          hasseclib;

} PalmSyncInfo;


#ifdef HAVE_GETOPTLONG
static struct option long_options[] =
{
    {"proxyaddress", 1, 0, 1},
    {"proxyport",    1, 0, 2},
    {"proxyname",    1, 0, 3},
    {"proxypasswd",  1, 0, 4},
    
    {"socksproxy",   1, 0, 5},
    {"socksport",    1, 0, 6},
    
    {"debug",        0, 0, 7},
    {"daemon",       0, 0, 8},
    {"help",         0, 0, 9},
    {"lowres",       0, 0, 10},
    
    {0, 0, 0, 0}
};
#endif

static int sd = 0;
static int verbose = 0;
static int daemon_mode = 0;
static int threeone = 0;
static int lowres = 0;

static int32 initAndOpenDatabase(void *out, AGDBConfig *theDatabase, 
                                 int32 *errCode);
static int32 getNextModifiedRecord(void * out, AGRecord ** record, 
                                   int32 *errCode);
static int32 getNextRecord(void * out, AGRecord ** record,
                           int32 *errCode);

static int32 cmdTASK(void *out, int32 *returnErrorCode, char *currentTask,
                     AGBool bufferable);
static int32 cmdITEM(void *out, int32 *returnErrorCode,
                                   int32 currentItemNumber,
                                   int32 totalItemCount,
                                   char *currentItem);
static int32 cmdDELETEDATABASE(void *out, int32 *returnErrorCode,
                                             char *dbname);
static int32 cmdOPENDATABASE(void *out, int32 *returnErrorCode,
                                           char *dbname);
static int32 cmdCLOSEDATABASE(void *out, int32 *returnErrorCode);
static int32 cmdCLEARMODS(void *out, int32 *returnErrorCode);
static int32 cmdGOODBYE(void *out, int32 *returnErrorCode,
                        AGSyncStatus syncStatus,
                        int32 errorCode,
                        char *errorMessage);
static int32 cmdRECORD(void *out, int32 *returnErrorCode,
                       int32 *newUID,
                       int32 uid,
                       AGRecordStatus mod,
                       int32 recordDataLength,
                       void *recordData,
                       int32 platformDataLength,
                       void *platformData);

static int processCommandLine(int argc, char *argv[]);

typedef sword (*netInitFunc)(AGNetCtx *ctx);
typedef sword (*netCloseFunc)(AGNetCtx *ctx);
typedef int32 (*netCtxSizeFunc)(void);
typedef void  (*netPreSyncHook) (AGNetCtx *ctx, 
                                 AGServerConfig *sc,
                                 AGLocationConfig *lc,
                                 AGSyncProcessor *sp,
                                 AGBool connectSecure);
typedef void  (*netPostSyncHook) (AGNetCtx *ctx, 
                                 AGServerConfig *sc,
                                 AGLocationConfig *lc,
                                 AGSyncProcessor *sp,
                                 AGBool connectSecure);

static netInitFunc     secnetinit = NULL;
static netCloseFunc    secnetclose = NULL;
static netCtxSizeFunc  secctxsize = NULL;
static netPreSyncHook  secnetpresync = NULL;
static netPostSyncHook  secnetpostsync = NULL;
/*----------------------------------------------------------------------------*/
static int
loadSecLib(AGNetCtx **ctx)
{
    char *seclib;
    
    seclib = getenv("MALSYNC_SECURITYLIB");

    if (!seclib) {
        if (verbose) {
            printf("MALSYNC_SECURITYLIB env variable not set\n");
        }
        return 0;
    }

#ifdef _WIN32
    {
        HINSTANCE h = LoadLibrary(seclib);
        if (h) {
            secnetinit = (netInitFunc)GetProcAddress(h, "NetInit");
            secnetclose = (netCloseFunc)GetProcAddress(h, "NetClose");
            secctxsize = (netCtxSizeFunc)GetProcAddress(h, "NetGetCtxSize");
            secnetpostsync = 
                (netPostSyncHook)GetProcAddress(h, "NetPostSyncHook");
            secnetpresync = (netPreSyncHook)GetProcAddress(h, "NetPreSyncHook");
        }
    }
#else /* _WIN32 */
#ifdef __EMX__
	{
		APIRET rc;
		HMODULE h;
		char load_error[CCHMAXPATH];

		rc = DosLoadModule(load_error, sizeof(load_error), seclib, &h);
		if (rc != NO_ERROR) {
			fprintf(stderr, "malsync: DosLoadModule() failed (rc = %d); failure at loading \"%s\"\n", rc, load_error);
		} else {
			rc = DosQueryProcAddr(h, 0, "NetInit", &secnetinit);
			rc = DosQueryProcAddr(h, 0, "NetClose", &secnetclose);
			rc = DosQueryProcAddr(h, 0, "NetGetCtxSize", &secctxsize);
			rc = DosQueryProcAddr(h, 0, "NetPostSyncHook", &secnetpostsync);
			rc = DosQueryProcAddr(h, 0, "NetPreSyncHook", &secnetpresync);
		}
	}
#else
    {   
#ifndef _HPUX_SOURCE
        void *h;
#ifdef __linux__
        h  = dlopen(seclib, RTLD_GLOBAL|RTLD_NOW);
#else /* __linux__ */
        h  = dlopen(seclib, RTLD_NOW);
#endif /* __linux */
        
        if (h) {
            secnetinit = dlsym(h, "NetInit");
            secnetclose = dlsym(h, "NetClose");
            secctxsize = dlsym(h, "NetGetCtxSize");
            secnetpostsync = dlsym(h, "NetPostSyncHook");
            secnetpresync = dlsym(h, "NetPreSyncHook");
        } else {
            if (verbose)
                printf("%s\n", dlerror());
        }
#endif
    }
#endif /* !__EMX__ */
#endif /* !_WIN32 */

    if (secnetinit && secnetclose && secctxsize ) {
        if (verbose) {
            printf("Found security library, initalizing\n");
        }
        *ctx = calloc(1, (*secctxsize)());
        (*secnetinit)(*ctx);
        return 1;
    }
    return 0;
}


/*---------------------------------------------------------------------------*/
void 
syncInfoFree(PalmSyncInfo * pInfo)
{


    if (NULL != pInfo) {

        if (NULL != pInfo->platform)
            free(pInfo->platform);

        if (NULL != pInfo->userConfig)
            AGUserConfigFree(pInfo->userConfig);

        if (NULL != pInfo->pilot_buffer)
            free(pInfo->pilot_buffer);

        if (NULL != pInfo->commandProcessor)
            AGCommandProcessorFree(pInfo->commandProcessor);

        free(pInfo);
    }
}

/*---------------------------------------------------------------------------*/
PalmSyncInfo *
syncInfoNew()
{
    PalmSyncInfo * pInfo;

    /* Allocate the PalmSyncInfo structure. */
    pInfo = (PalmSyncInfo *)malloc(sizeof(PalmSyncInfo));
    if (NULL != pInfo) {

        const int pbs = 65535;

        bzero(pInfo, sizeof(PalmSyncInfo));

        pInfo->pilot_buffer_size    = pbs;
        pInfo->pilot_buffer         = (uint8 *)malloc(pbs);
        if (NULL == pInfo->pilot_buffer)
            goto fail;

        /* Allocate the platform calls record. */
        pInfo->platform = (AGPlatformCalls *)malloc(sizeof(AGPlatformCalls));
        bzero(pInfo->platform, sizeof(AGPlatformCalls));
        if (NULL == pInfo->platform)
            goto fail;

        return pInfo;
    }

fail:
    if (NULL != pInfo) {
        if (verbose) {
            printf("Error in syncInfoNew\n");
        }
        syncInfoFree(pInfo);
    }

    return NULL;
}
/*---------------------------------------------------------------------------*/
static AGBool setupPlatformCalls(PalmSyncInfo * pInfo)
{
    pInfo->platform->out = pInfo;
    pInfo->platform->nextModifiedRecordFunc = getNextModifiedRecord;
    pInfo->platform->nextRecordFunc = getNextRecord;
    pInfo->platform->openDatabaseFunc =  initAndOpenDatabase;
    return FALSE;
}
/*---------------------------------------------------------------------------*/
static int32 
readInt32(uint8 *p)
{
    return (((int32)p[0]) << 24)
        + (((int32)p[1]) << 16)
        + (((int32)p[2]) << 8)
        + ((int32)p[3]);
}
/*---------------------------------------------------------------------------*/
static 
int16 readInt16(uint8 * p)
{
    return (((int16)p[0]) << 8) + ((int16)p[1]);
}
/*---------------------------------------------------------------------------*/
static void 
readAndUseDeviceInfoDatabase(AGDeviceInfo * devInfo,
                             uint8 *dev_db_info_buffer,
                             uint32 dev_db_info_buffer_size)
{
    int database_id = 0;
    long result;

    if (verbose) 
        printf("Entering readAndUseDeviceInfoDatabase\n");

    /*  Open the device info database on the device.  It may or may not
        be present.  If it isn't, that's ok -- just return and don't
        change anything.
    */
    /* 11/24/00 - Changed name of the database that the color info is stored 
     * in. This should clear up some bug reports from people with color 
     * devices. 
     */
    result = dlp_OpenDB(sd, DEFAULT_CARD_NUM, dlpOpenRead,  
                        "AvGoDeviceInfo", &database_id);
    
    if (result < 0) {
        if (verbose) 
            printf("Unable to open MBlnDevice Info\n");

    } else {

        int attr = 0;
        int cat  = 0;
        recordid_t id;
        int rc;

        rc = dlp_ReadRecordByIndex(sd, database_id, 0, 
                                   (void *)dev_db_info_buffer, 
                                   &id, &dev_db_info_buffer_size, 
                                   &attr, &cat);
    

        if (rc >= 0) {
            uint8 *p = dev_db_info_buffer;
            int16 dev_db_info_version = readInt16(p);
            p+=sizeof(int16);
            devInfo->colorDepth = readInt32(p);
            p+=sizeof(int32);
            devInfo->screenWidth = readInt32(p);
            p+=sizeof(int32);
            devInfo->screenHeight = readInt32(p);
            p+=sizeof(int32);

            if (devInfo->serialNumber) 
                free(devInfo->serialNumber);

            /*  Next line is safe because we know p isn't null
                at this point and we know the creator of the
                record wrote at least a zero here.
            */
            devInfo->serialNumber = strdup((char*)p);

            if (verbose)
                printf("MBlnDeviceInfo sez: colorDepth = %d,"
                       " serial number is %s\n", 
                       devInfo->colorDepth, devInfo->serialNumber);

        }
        
        dlp_CloseDB(sd, database_id);
    }
}
/*---------------------------------------------------------------------------*/
static int 
readDeviceInfo(PalmSyncInfo * pInfo)
{
    AGDeviceInfo * devInfo;
    struct SysInfo sysInfo;
    struct CardInfo cardInfo;
    char osverstring[24];
    int err;
    int majorVersion, minorVersion, bugfixVersion, build, state;
    
    /* Start with clean slate. */
    devInfo = pInfo->deviceInfo;

    err = dlp_ReadSysInfo(sd, &sysInfo);
    if (err < 0) {
        fprintf(stderr, "dlp_ReadSysInfo failed with err %d\n", err);
        return -1;
    }

    cardInfo.card = DEFAULT_CARD_NUM;
    err = dlp_ReadStorageInfo(sd, DEFAULT_CARD_NUM, &cardInfo);
    if (err < 0) {
        fprintf(stderr, "dlp_ReadStorageInfo failed with err %d\n", err);
        return -1;
    }

    majorVersion = (((sysInfo.romVersion >> 28) & 0xf) * 10) + 
        ((sysInfo.romVersion >> 24) & 0xf);
    minorVersion = ((sysInfo.romVersion >> 20) & 0xf);
    bugfixVersion = ((sysInfo.romVersion >> 16) & 0xf);
    state = ((sysInfo.romVersion >> 12) & 0xf);
    build = (((sysInfo.romVersion >> 8) & 0xf) * 10) + 
        (((sysInfo.romVersion >> 4) & 0xf) * 10)+ (sysInfo.romVersion  & 0xf);
    

    snprintf(osverstring, 24, "%d.%d", majorVersion, minorVersion);

    if (verbose) {
        printf("OS Version = %s\n", osverstring);
    }

    devInfo->availableBytes   = cardInfo.ramFree;
    devInfo->serialNumber     = strdup("");
    devInfo->osName           = strdup("PALM_OS");
    devInfo->osVersion        = strdup(osverstring);
    devInfo->screenWidth      = 150;
    devInfo->screenHeight     = 150;

    if((majorVersion > 3) || (majorVersion == 3 && minorVersion >= 5))
        devInfo->colorDepth = 8;
    else {
        if(majorVersion > 2)
            devInfo->colorDepth = 2;
        else
            devInfo->colorDepth = 1;
    }

    if (verbose) {
        printf("Setting colordepth: devInfo->colorDepth = %d\n",
               devInfo->colorDepth);
    }

    readAndUseDeviceInfoDatabase(devInfo,
                                 pInfo->pilot_buffer,
                                 pInfo->pilot_buffer_size);

    /* Override the color depth if the user wants low res images. */
    if (lowres) {
        if (verbose) {
            printf("Overriding colordepth: devInfo->colorDepth = 1\n");
        }        
        devInfo->colorDepth = 1;
    }


    return 0;
}
/*---------------------------------------------------------------------------*/
static AGBool 
getPalmDatabaseCreationInfo(AGDBConfig *db, uint32 *creator, uint32 *flags, 
                            uint32 *type)
{
    AGBufferReader * r = NULL;
    
    if (verbose)
        printf("GetPalmDatabaseCreationInfo()\n");

    if (NULL == db)
        return FALSE;

    if (0 == db->platformDataLength || NULL == db->platformData)
        return FALSE;

    r = AGBufferReaderNew((uint8*)db->platformData);
    if (NULL == r)
        return FALSE;
    
    AGPalmReadDBConfigPlatformData((AGReader*)r, creator, type, flags);
    AGBufferReaderFree(r);
    return TRUE;
}

/*---------------------------------------------------------------------------*/
static int createDatabase(AGDBConfig *db)
{
    int dbhandle;
    long creator;
    int flags;
    int cardNo = DEFAULT_CARD_NUM;
    long type;
    int version = 0;
    int err;
    
    if (!db)
        return 0;

    if (verbose)
        printf("createDatabase\n");
    
    getPalmDatabaseCreationInfo(db, (uint32*)&creator, 
                                (uint32*)&flags, (uint32*)&type);

    if ((err = dlp_CreateDB(sd,  creator,  type, cardNo, flags,  
                            version, db->dbname, &dbhandle)) < 0) {
        if (verbose)
            printf("createDatabase: dlp_CreateDB failed err = %d\n", err);
        dbhandle = 0;
    }
    
    return dbhandle;
}

/*---------------------------------------------------------------------------*/
static long openDatabase(PalmSyncInfo *pInfo, char *dbname, AGBool create)
{

    long result;
    
    if (!dbname || !pInfo) {
        if (verbose)
            printf("\n");
        return -1;
    }
    
    if (verbose) {
        printf("... opening '%s' ...", dbname);
    }

    pInfo->currentDb =
        AGServerConfigGetDBConfigNamed(pInfo->currentServerConfig, dbname);
    
    result = dlp_OpenDB(sd, DEFAULT_CARD_NUM, dlpOpenRead|dlpOpenWrite,  
                        dbname, &pInfo->pilot_rHandle);
    
    if ((result < 0) && create)
        pInfo->pilot_rHandle = createDatabase(pInfo->currentDb);

    if (pInfo->pilot_rHandle) {
        uint32 creator, flags, type;
        if (getPalmDatabaseCreationInfo(pInfo->currentDb, &creator, &flags, 
                                        &type) && (flags & 0x01) != 0) {
            pInfo->currentDBIsResourceType = TRUE;
        } else
            pInfo->currentDBIsResourceType = FALSE;
        if (verbose)
            printf("successfully.\n");
    } else {
        if (verbose)
            printf("unsuccessfully.\n");
        pInfo->currentDBIsResourceType = FALSE;
        pInfo->currentDb = NULL;
    }

    return result;
}

/*---------------------------------------------------------------------------*/
static long closeDatabase(PalmSyncInfo * pInfo)
{
    int result;

    result = dlp_CloseDB(sd, pInfo->pilot_rHandle);
    pInfo->pilot_rHandle = 0;
    pInfo->currentDb = NULL;
    pInfo->currentDBIsResourceType = FALSE;
    return result;
}

/*---------------------------------------------------------------------------*/
static void deleteDatabase(char *dbname)
{
    if (verbose)
        printf("deleteDatabase(%s)\n", dbname);
    dlp_DeleteDB(sd, DEFAULT_CARD_NUM, dbname);
}
/*---------------------------------------------------------------------------*/
static void clearMods(long dbHandle)
{
    if (verbose)
        printf("clearMods()\n");
    dlp_CleanUpDatabase(sd, dbHandle);
    dlp_ResetSyncFlags(sd, dbHandle);

}
#define DEVICE_USERCONFIG_DB_NAME "MBlnUserConfig"
/*---------------------------------------------------------------------------*/
static long openUserConfigDatabase(int *threeone)
{
    long result;
    int userConfigDBHandle = 0;
    
    *threeone = 0;
    result = dlp_OpenDB(sd, DEFAULT_CARD_NUM, dlpOpenRead|dlpOpenWrite,  
                        DEVICE_PROFILE_DB_NAME, 
                        &userConfigDBHandle);

    if (result < 0) {
        
        if (verbose) {
            printf("Failed to locate MBlnProfile database. Lets look for"
                   " a MBlnUserConfig database\n");
        }
        /* OK Now lets look for a 3.1 client database */
        
        result = dlp_OpenDB(sd, DEFAULT_CARD_NUM, dlpOpenRead|dlpOpenWrite,  
                            DEVICE_USERCONFIG_DB_NAME, 
                            &userConfigDBHandle);
        if (result < 0) {
            
            result = dlp_CreateDB(sd, DEVICE_PROFILE_DB_CREATOR, 
                                  DEVICE_PROFILE_DB_TYPE, DEFAULT_CARD_NUM,
                                  0, 0, DEVICE_PROFILE_DB_NAME, 
                                  &userConfigDBHandle);
            if (result < 0) {
                fprintf(stderr, "Unable to create user Config Databage\n");
                userConfigDBHandle = 0;
            }        
        } else {
            if (verbose) {
                printf("Found a MBlnUserConfig, this must be MobileLink"
                       "3.1 or older\n");
            }
            *threeone = 1;
        }

    }
    return userConfigDBHandle;
}
#define BUFFERSIZE 0xFFFF
/*---------------------------------------------------------------------------*/
static int32 
readDeviceUserConfig32(int userConfigDBHandle, AGUserConfig **deviceUserConfig)
{
    recordid_t id;
    int bufferSize = BUFFERSIZE;
    int attr = 0;
    int cat  = 0;
    int rc;
    uint8 buffer[BUFFERSIZE];
    AGBufferReader * r = NULL;

    rc = dlp_ReadRecordByIndex(sd, userConfigDBHandle, 0, (void *)buffer, 
                               &id, &bufferSize, &attr, &cat);
    
    if (rc < 0) {
        if (verbose)
            printf("readDeviceUserConfig: dlp_ReadRecordByIndex , err = %d\n",
                   rc);
        return 0;
    }
    
    r = AGBufferReaderNew(buffer);
    if (r) {
        *deviceUserConfig = AGUserConfigNew();
        AGUserConfigReadData(*deviceUserConfig, (AGReader*)r);
        AGBufferReaderFree(r);
        return id;
    } else
        return 0;
}
#define BUFFERSIZE 0xFFFF
/*---------------------------------------------------------------------------*/
static int32 readDeviceUserConfig31(int userConfigDBHandle,
                                    AGUserConfig **deviceUserConfig)
{
    recordid_t id;
    int bufferSize = BUFFERSIZE;
    int attr = 0;
    int cat  = 0;
    int rc;
    uint8 buffer[BUFFERSIZE];
    AGBufferReader * r = NULL;

    rc = dlp_ReadRecordByIndex(sd, userConfigDBHandle, 0, (void *)buffer, 
                               &id, &bufferSize, &attr, &cat);
    
    if (rc < 0) {
        if (verbose)
            printf("readDeviceUserConfig: dlp_ReadRecordByIndex , err = %d\n",
                   rc);
        return 0;
    }
    
    r = AGBufferReaderNew(buffer);
    if (r) {
        *deviceUserConfig = AGUserConfigNew();
        MAL31ReadUserData(*deviceUserConfig, (AGReader*)r);
        AGBufferReaderFree(r);
        return id;
    } else
        return 0;
}
/*---------------------------------------------------------------------------*/
static int32 readDeviceUserConfig(int userConfigDBHandle,
                                  AGUserConfig **deviceUserConfig,
                                  int threeone)
{ 
    int32 ret;
    if (threeone) 
        ret = readDeviceUserConfig31(userConfigDBHandle, deviceUserConfig);
    else 
        ret = readDeviceUserConfig32(userConfigDBHandle, deviceUserConfig);
    return ret;
}
/*---------------------------------------------------------------------------*/
static void writeDeviceUserConfig(int userConfigDBHandle,
                                  AGUserConfig * deviceUserConfig,
                                  recordid_t *recID, int threeone)
{

    recordid_t id;
    int bufferSize = BUFFERSIZE;
    int attr = 0;
    int cat  = 0;
    uint8 buffer[BUFFERSIZE];
    AGBufferWriter * w = NULL;
    w = AGBufferWriterNew(0);
    if (w) {
        long result;
        
        if (threeone) {
            MAL31WriteUserData(deviceUserConfig, (AGWriter*)w);
        } else {
            AGUserConfigWriteData(deviceUserConfig, (AGWriter*)w);
        }

        result = dlp_ReadRecordByIndex(sd, userConfigDBHandle, 0, (void *)buffer, 
                                   &id, &bufferSize, &attr, &cat);
        
        if (result < 0)
            id = 0;

        result =  dlp_WriteRecord(sd, userConfigDBHandle, 0,
                                  id, 0, (void *)AGBufferWriterGetBuffer(w), 
                                  AGBufferWriterGetBufferSize(w), 
                                  &id);
        AGBufferWriterFree(w);

    }
}

/*---------------------------------------------------------------------------*/
static AGUserConfig *getUserConfig(uint32 * pilotID)
{
    
    /* The device is the truth. There is no way to set these values on 
       the desktop */
    AGUserConfig * deviceUserConfig = NULL;
    int userConfigDBHandle = 0;

    /* Get device's record. */
    userConfigDBHandle = openUserConfigDatabase(&threeone);

    if (userConfigDBHandle) {
        
        
        /* Retrieve device's idea of current userConfig. */
        *pilotID = readDeviceUserConfig(userConfigDBHandle,
                                        &deviceUserConfig, 
                                        threeone);
        
        /* Done with database for now, so close it. */
        dlp_CloseDB(sd, userConfigDBHandle);
        

    }
    return deviceUserConfig;
}

/*---------------------------------------------------------------------------*/
static void storeDeviceUserConfig(AGUserConfig *userConfig, recordid_t id)
{
    int threeone;
    int userConfigDBHandle = openUserConfigDatabase(&threeone);
    if (0 != userConfigDBHandle) {
        writeDeviceUserConfig(userConfigDBHandle,
                              userConfig, &id, threeone);
        dlp_CloseDB(sd, userConfigDBHandle);
    }
}

/*---------------------------------------------------------------------------*/
static void doStartServer(PalmSyncInfo * pInfo,
                          AGServerConfig *sc,
                          int32 *errCode)
{
    pInfo->currentServerConfig = sc;
    if(pInfo->commandProcessor) {
        AGCommandProcessorFree(pInfo->commandProcessor);
        pInfo->commandProcessor = NULL;
    }
    pInfo->commandProcessor = AGCommandProcessorNew(sc);
    pInfo->platform->performCommandOut = pInfo->commandProcessor;
    pInfo->platform->performCommandFunc = 
                    AGCommandProcessorGetPerformFunc(pInfo->commandProcessor);

    pInfo->commandProcessor->commands.out = pInfo;

    pInfo->commandProcessor->commands.performTaskFunc = cmdTASK;
    pInfo->commandProcessor->commands.performItemFunc = cmdITEM;
    pInfo->commandProcessor->commands.performDeleteDatabaseFunc 
        = cmdDELETEDATABASE;
    pInfo->commandProcessor->commands.performOpenDatabaseFunc 
        = cmdOPENDATABASE;
    pInfo->commandProcessor->commands.performCloseDatabaseFunc 
        = cmdCLOSEDATABASE;
    pInfo->commandProcessor->commands.performClearModsFunc = cmdCLEARMODS;
    pInfo->commandProcessor->commands.performGoodbyeFunc = cmdGOODBYE;
    pInfo->commandProcessor->commands.performRecordFunc = cmdRECORD;
    
}

/*---------------------------------------------------------------------------*/
static void 
doEndServer(PalmSyncInfo * pInfo, int32 *errCode)
{
    pInfo->currentServerConfig = NULL;
    if(pInfo->commandProcessor) {
        AGCommandProcessorFree(pInfo->commandProcessor);
        pInfo->commandProcessor = NULL;
    }
}
/*---------------------------------------------------------------------------*/
static int32 
cmdTASK(void *out, int32 *returnErrorCode, char *currentTask, 
        AGBool bufferable)
{
    if (currentTask)
        printf("%s\n", currentTask);
    return AGCLIENT_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int32 
cmdITEM(void *out, int32 *returnErrorCode, int32 currentItemNumber,
        int32 totalItemCount, char *currentItem)
{
    printf(".");
    fflush(stdout);
    if (currentItemNumber == totalItemCount)
        printf("\n");
    
    return AGCLIENT_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int32 
cmdDELETEDATABASE(void *out, int32 *returnErrorCode, char *dbname)
{
    if (verbose)
        printf("doCmdAG_DELETEDATABASE_CMD()\n");

    if (NULL != dbname) {
        if (verbose)
            printf("... trying to delete database %s from device\n", 
                  dbname);
        deleteDatabase(dbname);
    }
    return AGCLIENT_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int32 
cmdOPENDATABASE(void *out, int32 *returnErrorCode, char *dbname)
{
    PalmSyncInfo *pInfo = (PalmSyncInfo *)out;
    if (verbose)
        printf("doCmdAG_OPENDATABASE_CMD(%s)\n", dbname);

    if (NULL != dbname) {
        long result;
        /* We assign a result code here only for debugging.  It's ok for an
        open to fail here. */
        result = openDatabase(pInfo, dbname, TRUE);
    }
    return AGCLIENT_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int32 
cmdCLOSEDATABASE(void *out, int32 *returnErrorCode)
{
    PalmSyncInfo *pInfo = (PalmSyncInfo *)out;
    if (verbose)
        printf("doCmdAG_CLOSEDATABASE_CMD()\n");
    closeDatabase(pInfo);
    return AGCLIENT_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int32 
cmdCLEARMODS(void *out, int32 *returnErrorCode)
{
    PalmSyncInfo *pInfo = (PalmSyncInfo *)out;
    if (verbose)
        printf("doCmdAG_CLEARMODS_CMD()\n");
    clearMods(pInfo->pilot_rHandle);
    return AGCLIENT_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int32 
cmdGOODBYE(void *out, int32 *returnErrorCode, AGSyncStatus syncStatus,
           int32 errorCode, char *errorMessage)
{
    if (verbose)
        printf("doCmdAG_GOODBYE_CMD()\n");

    if (errorMessage)
        printf("%s\n", errorMessage);
    
    return AGCLIENT_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int 
getIndexFromPlatformData(uint8 *platformData)
{
    int16 recIndex;
    AGBufferReader reader;

    if (!platformData)
        return 0;

    AGBufferReaderInit(&reader, platformData);
    AGPalmReadRecordPlatformData((AGReader *)&reader, &recIndex);
    AGBufferReaderFinalize(&reader);
    return (int)recIndex;
}
/*---------------------------------------------------------------------------*/
static int32 
cmdRECORD(void *out, int32 *returnErrorCode, int32 *newUID,
          int32 uid, AGRecordStatus mod, int32 recordDataLength,
          void *recordData, int32 platformDataLength, void *platformData)
{
    PalmSyncInfo *pInfo = (PalmSyncInfo *)out;
    
    if (verbose)
        printf("doCmdAG_RECORD_CMD()\n");

    if(mod == AG_RECORD_NEW_TEMPORARY_UID) {
        uid = 0; 
    }

    if (AG_RECORD_DELETED == mod) {

        dlp_DeleteRecord(sd, pInfo->pilot_rHandle, 0, uid);

    } else if (recordDataLength <= 0x0000ffff) {

        if (pInfo->currentDBIsResourceType) {
            
            dlp_WriteRecord(sd, pInfo->pilot_rHandle, dlpDBFlagResource,
                            uid, 0, (void *)recordData,
                            recordDataLength, (recordid_t *)newUID);
            if (verbose)
                printf("doCmdAG_RECORD_CMD()\n");

        } else {

            dlp_WriteRecord(sd, pInfo->pilot_rHandle, 0,
                            uid, 0, (void *)recordData,
                            recordDataLength, (recordid_t *)newUID);
        }

    }
    return AGCLIENT_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int32 
initAndOpenDatabase(void *_pInfo, AGDBConfig *db, int32 *errCode)
{
    long result;
    PalmSyncInfo * pInfo = (PalmSyncInfo *)_pInfo;

    if (NULL == db->dbname) {
        *errCode = AGCLIENT_OPEN_ERR;
        return AGCLIENT_ERR;
    }

    result = openDatabase(pInfo, db->dbname, FALSE);
    if (result < 0) {
        if (result == dlpErrNotFound)
            *errCode = AGCLIENT_OPEN_ERR;
        else
            *errCode = AGCLIENT_UNKNOWN_ERR;
        return AGCLIENT_ERR;
    }

    pInfo->pilot_RecIndex = 0;
    pInfo->record = AGRecordNew(0, AG_RECORD_UNMODIFIED, 0, 0, 0, 0);
    if (!pInfo->record) {
        *errCode = AGCLIENT_OPEN_ERR;
        return AGCLIENT_ERR;
    }

    return AGCLIENT_IDLE;
}
/*---------------------------------------------------------------------------*/
static int32 
leaveGetRecord(PalmSyncInfo * pInfo, int32 result)
{
    if (pInfo->record) {

        /* Set recordData to NULL so that AGRecordFree doesn't try
        to free it (we own that buffer and will release it when the
        program ends). */
        pInfo->record->recordData = NULL;

        AGRecordFree(pInfo->record);
        pInfo->record = NULL;
    }
    return result;
}
/*---------------------------------------------------------------------------*/
static int32 
getRecordBase(PalmSyncInfo * pInfo, AGBool modonly,  AGRecord **record, 
              int32 *errCode)
{
    int32 result;
    int att = 0;
    int cat = 0;
    int size = pInfo->pilot_buffer_size;
    int idx   = pInfo->pilot_RecIndex++;

    result = (modonly) ?
        dlp_ReadNextModifiedRec (sd, pInfo->pilot_rHandle, pInfo->pilot_buffer,
                                 &pInfo->id, &idx,
                                 &size, &att, &cat)
        :
        dlp_ReadRecordByIndex(sd, pInfo->pilot_rHandle, idx,
                              pInfo->pilot_buffer, &pInfo->id,
                              &size, &att, &cat);
    
    if (result < 0) {
        closeDatabase(pInfo);
        if (result == dlpErrNotFound) {
            if (verbose)
                printf("(successfully reached end of records ...)\n");
            return leaveGetRecord(pInfo, AGCLIENT_IDLE);
        }
        else {
            *errCode = AGCLIENT_UNKNOWN_ERR;
            return leaveGetRecord(pInfo, AGCLIENT_ERR);
        }
    }
    pInfo->record = AGRecordInit(pInfo->record, pInfo->id,
                                 AGPalmPilotAttribsToMALMod((uint8)att),
                                 size, pInfo->pilot_buffer, 0, NULL);

    *record = pInfo->record;
    return AGCLIENT_CONTINUE;
}
/*---------------------------------------------------------------------------*/
static int32 
getNextModifiedRecord(void *out, AGRecord **record, int32 *errCode)
{
    if (verbose)
        printf("GetNextModifiedRecord()\n");

    return getRecordBase((PalmSyncInfo *)out, TRUE, record, errCode);
}
/*---------------------------------------------------------------------------*/
static int32 
getNextRecord(void *out, AGRecord **record, int32 *errCode)
{
    if (verbose)
        printf("GetNextRecord()\n");

    return getRecordBase((PalmSyncInfo *)out, FALSE, record, errCode);
}
/*---------------------------------------------------------------------------*/
static AGBool 
doClientProcessorLoop(PalmSyncInfo * pInfo, AGNetCtx *ctx)
{
    int32 dummyError;
    int32 cpResult;
    int32 syncCount;
    int32 i, n;
    AGBool cancelled = FALSE;
    AGLocationConfig *lc = NULL;
    int migrate = 0;

    n = AGUserConfigCount(pInfo->userConfig);

    /* Lets check for 3.1 to 3.x migration here */
    if (n == 1) {
        AGServerConfig * sc =
            AGUserConfigGetServerByIndex(pInfo->userConfig, 0);
        
        /* Yup, no server name */
        if (!sc->serverName) {
            /* Lets look for the 3.1 config database */
            long result;
            int userConfigDBHandle = 0;
            result = dlp_OpenDB(sd, DEFAULT_CARD_NUM, 
                                dlpOpenRead|dlpOpenWrite,  
                                DEVICE_USERCONFIG_DB_NAME, 
                                &userConfigDBHandle);
            if (result > 0) {
                char response[2];
                printf("It looks like you recently upgraded your client"
                       ". Would you\nlike to migrate your old settings"
                       "?[y/n] ");
                if ((fgets(response, 2, stdin) != 0)
                    && ((response[0] == 'y') || (response[0] == 'Y'))) {
           
                    threeone = 1;
                    /* Retrieve device's idea of current userConfig. */
                    result = readDeviceUserConfig(userConfigDBHandle,
                                                  &pInfo->userConfig, 
                                                  threeone);

                    /* This will cause us to write out the 3.1 data 
                       as 3.2 when we finish syncing */
                    threeone = 0;
                    
                    /* Done with database for now, so close it. */
                    dlp_CloseDB(sd, userConfigDBHandle);
                    

                    n = AGUserConfigCount(pInfo->userConfig);
                    
                    /* set a flag to delete the old database */
                    migrate = 1;

                }
            }
        }
    }
                    
    for (i = 0; i < n; ++i) {

        AGServerConfig * sc =
            AGUserConfigGetServerByIndex(pInfo->userConfig, i);
        
        if (cancelled)
            continue;

        if (NULL == sc)
            continue;

        if (sc->disabled)
            continue;

        if (NULL == sc->serverName || sc->serverPort <= 0) 
            continue;

        syncCount = 0;
        doStartServer(pInfo, sc, &dummyError);
 
        do {
            AGCommandProcessorStart(pInfo->commandProcessor);
            
            pInfo->deviceInfo = AGDeviceInfoNew();
            /* If this fails, we're totally dead.  Exit with fail code. */
            if(!pInfo->deviceInfo)
                return FALSE;
                
            readDeviceInfo(pInfo);

            if (httpProxy && httpProxyPort) {
                if (verbose) {
                    printf("Setting proxy to %s and port to %d\n",
                           httpProxy, httpProxyPort);
                }   
                lc = AGLocationConfigNew();
                lc->HTTPUseProxy = 1;
                lc->HTTPName = httpProxy;
                lc->HTTPPort = httpProxyPort;
                if (proxyUsername && proxyPassword) {
                    if (verbose) {
                        printf("Setting proxy user to %s and password to %s\n",
                               proxyUsername, proxyPassword);
                    }   
                    lc->HTTPUseAuthentication = 1;
                    lc->HTTPUsername = proxyUsername;
                    lc->HTTPPassword = proxyPassword;
                }
            }

            if (socksProxy && socksProxyPort) {
                if (verbose) {
                    printf("Setting socks proxy to %s and port to %d\n",
                           socksProxy, socksProxyPort);
                }   
                if (!lc)
                    lc = AGLocationConfigNew();
                lc->SOCKSUseProxy = 1;
                lc->SOCKSName = socksProxy;
                lc->SOCKSPort = socksProxyPort;
            }
            
            pInfo->clientProcessor =
                AGClientProcessorNew(pInfo->currentServerConfig,
                                     pInfo->deviceInfo, 
                                     lc,
                                     pInfo->platform, 
                                     TRUE,
                                     ctx);
                                        
            /* If this fails, we're totally dead.  Exit with fail code. */
            if (NULL == pInfo->clientProcessor) {
                AGDeviceInfoFree(pInfo->deviceInfo);
                return FALSE;
            }

            AGClientProcessorSetBufferServerCommands(pInfo->clientProcessor,
                                                     FALSE);
            
            AGClientProcessorSync(pInfo->clientProcessor);


            cpResult = AGCLIENT_CONTINUE;
            while (AGCLIENT_CONTINUE == cpResult) {
                cpResult = AGClientProcessorProcess(pInfo->clientProcessor);
                
                if (AGCLIENT_CONTINUE == cpResult && pInfo->quit) {
                    cancelled = TRUE;
                    cpResult = AGCLIENT_IDLE;
                }
                if (dlp_OpenConduit(sd)<0) {
                    fprintf(stderr, 
                            "Exiting on cancel, data not retrieved.\n");
                    exit(1);
                }
                
            }


            if(cpResult == AGCLIENT_ERR) {

                char *msg = AGGetMsg(pInfo->clientProcessor->errStringId);
                if (msg)
                    fprintf(stderr,"%s\n", msg);
                else
                    fprintf(stderr,"Unknown error\n");
                    
            }

            AGClientProcessorFree(pInfo->clientProcessor);
            AGDeviceInfoFree(pInfo->deviceInfo);

        } while (!cancelled
                 && AGCommandProcessorShouldSyncAgain(pInfo->commandProcessor) 
                 && syncCount++ < MAX_SYNCS);
        doEndServer(pInfo, &dummyError);

        /* It's possible to cancel in the middle of an open database,
           so if that happened, close it. */
        if (pInfo->pilot_rHandle)
            closeDatabase(pInfo);

        /* If we migrated from 3.1 to 3.2 delete the 3.1 database */
        if (migrate) 
            dlp_DeleteDB (sd, DEFAULT_CARD_NUM, DEVICE_USERCONFIG_DB_NAME);
        
    }

    return TRUE; /* success */
}
/*---------------------------------------------------------------------------*/
void 
Disconnect(void)
{
    if(sd == 0)
        return;
    
    dlp_EndOfSync(sd, 0);
    pi_close(sd);
    sd = 0;
}
/*---------------------------------------------------------------------------*/
int 
main(int argc, char *argv[])
{
    int cont = 1;
    int fd; 
    PalmSyncInfo *pInfo;
    uint32 pilotID;
    AGNetCtx *ctx;
    
    processCommandLine(argc, argv);
    
    setbuf (stdout, 0);
#ifndef _WIN32
    if( daemon_mode ) {
        /* We would like to close stdin and redirect stdout */
        /* to stderr, which is reopened on /dev/console */
        fd = open("/dev/console", O_WRONLY); dup2(fd, 1); dup2(fd, 2);
        
        /* Now fork of the shell, and run in the background */
        if( fork() )
            exit(0);
    }
#endif
    
    for(;cont;) {
      WaitingIs:
        pInfo = syncInfoNew();
        if (NULL == pInfo)
            return -1;
        
        Connect(pInfo);
        
        if ( dlp_OpenConduit(sd) < 0) {
            if(! daemon_mode ) {
                fprintf(stderr, "Exiting on cancel\n");
                exit(1);
            } else {
                printf("> back to waiting...\n");
                
                Disconnect(); 
                syncInfoFree(pInfo);
                goto WaitingIs;
            }
        }

        if (!loadSecLib(&ctx)) {
            ctx = (AGNetCtx *)malloc(sizeof(AGNetCtx));
            AGNetInit(ctx);
        }
        
        
        
        if (setupPlatformCalls(pInfo))
            return -1;
        
        pInfo->userConfig = getUserConfig(&pilotID);
        
        doClientProcessorLoop(pInfo, ctx);
        
        storeDeviceUserConfig(pInfo->userConfig, pilotID);
        
        if (secnetclose)
            (*secnetclose)(ctx);
        else
            AGNetClose(ctx);
        
        Disconnect(); 
        syncInfoFree(pInfo);
        free(ctx);
        
        if(! daemon_mode )
            cont = 0;
    }
    
    return 0;
}
/*---------------------------------------------------------------------------*/
RETSIGTYPE 
SigHandler(int signal)
{
  fprintf(stderr, "Abort on signal (%d)!", signal);
  Disconnect();
  exit(3);
}
/*---------------------------------------------------------------------------*/
static void 
Connect(PalmSyncInfo *pi) 
{
    struct pi_sockaddr addr;
    int ret;
    
    if (sd != 0)
        return;

#ifndef WIN32
    signal(SIGHUP, SigHandler);
    signal(SIGINT, SigHandler);
    signal(SIGSEGV, SigHandler);
#endif
    
    if (!(sd = pi_socket(PI_AF_SLP, PI_SOCK_STREAM, PI_PF_PADP))) {
        perror("pi_socket");
        exit(1);
    }
    
    addr.pi_family = PI_AF_SLP;
    strcpy(addr.pi_device, device);
    
    ret = pi_bind(sd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1) {
        fprintf(stderr, "Unable to bind to port '%s'.\n", device);
        exit(1);
    }
    
    printf("Waiting for connection on %s (press the HotSync button now)...\n"
           , device);
    
    ret = pi_listen(sd,1);
    if(ret == -1) {
        perror("pi_listen");
        exit(1);
    }
    
    sd = pi_accept(sd,0,0);
    if(sd == -1) {
        if( daemon_mode )
            return;
        else {
            perror("pi_accept");
            exit(1);
        }
    }
    
    if( verbose )
      puts("Connected");
}
/*----------------------------------------------------------------------------*/
static void 
Help(void)
{
      printf("Usage: %s [OPTIONS]\n\n", progname);
#ifdef HAVE_GETOPTLONG
      printf("  -v, --version        Version.\n");
      printf("  -p, --proxyaddress   Proxy address.\n");
      printf("  -r, --proxyport      Proxy port.\n");
      printf("  -u, --proxyname      Proxy username.\n");
      printf("  -d, --proxypasswd    Proxy password.\n");
      printf("  -s, --socksproxy     Socks proxy.\n");
      printf("  -o, --socksport      Socks port.\n");
      printf("  -g, --debug          Print out a lot of debug stuff.\n");
      printf("  -l, --lowres         Low resolution images.\n");
      printf("  -D, --daemon         Work as a daemon (disconnect from shell).\n");
      printf("  -h, --help           Print this help.\n\n");
#else
      printf("  -v,     Version.\n");
      printf("  -p,     Proxy address.\n");
      printf("  -r,     Proxy port.\n");
      printf("  -u,     Proxy username.\n");
      printf("  -d,     Proxy password.\n");
      printf("  -s,     Socks proxy.\n");
      printf("  -o,     Socks port.\n");
      printf("  -g,     Print out a lot of debug stuff.\n");
      printf("  -l,     Low Resoultion Images.\n");
      printf("  -D,     Work as a daemon (disconnect from shell).\n");
      printf("  -h,     Print this help.\n\n");
#endif      
      printf("The serial port to connect to may be specified by the PILOTPORT");
      printf("\nenvironment variable. If not specified it will default to \n");
      printf("/dev/pilot \n");
      printf("\n");
      printf("The baud rate to connect with may be specified by the PILOTRATE\n");
      printf("environment variable. If not specified, it will default to 9600.\n");
      printf("Please use caution setting it to higher values, as several types\n");
      printf("of workstations have problems with higher rates.\n");
      printf("\n");
      exit(0);
}
/*----------------------------------------------------------------------------*/
static int 
processCommandLine(int argc, char *argv[])
{
    char *str;
    int c, optind;
    
    str = getenv("PILOTPORT");
    if (str != NULL)
        device = str;
    
    progname = argv[0];
    do {
#ifdef HAVE_GETOPTLONG
        c = getopt_long( argc, argv, "v:p:r:u:d:s:o:gDhvl",
                         long_options, &optind);
#else
        c = getopt( argc, argv, "p:r:u:d:s:o:gDhvl");
#endif
        
        switch( c ) {
        case 'p': case 1:
            httpProxy = optarg;
            break;
        case 'r': case 2:
            httpProxyPort = atoi(optarg);
            break;
        case 'u': case 3:
            proxyUsername = optarg;
            break;
        case 'd': case 4:
            proxyPassword = optarg;
            break;
        case 's': case 5:
            socksProxy = optarg;
            break;
        case 'o': case 6:
            socksProxyPort = atoi(optarg);
            break;
        case 'g': case 7:
            verbose = 1;
            break;
        case 'D': case 8:
            verbose = 0;
            daemon_mode = 1;
            break;
        case 'h': case 9:
            Help();
            break;
        case 'v': case 10:
            printf("%s\n", VERSION_STRING);
            exit(0);
        case 'l':
        case 11:
            lowres = 1;
            break;
        }
        
    } while (c != EOF);

    return 0;
}
