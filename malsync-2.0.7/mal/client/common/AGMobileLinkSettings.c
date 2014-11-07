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

/* Owner:  miket */

#ifdef _WIN32 /**** Entire file is NOOP if not on Windows. *****/

#include <AGMobileLinkSettings.h>
#include <AGUtil.h>
#include <windows.h>

AGMobileLink * AGMobileLinkInit(AGMobileLink * mobileLink)
{
    bzero(mobileLink, sizeof(AGMobileLink));

    return mobileLink;
}

AGMobileLink * AGMobileLinkInitWithDefaults(AGMobileLink * mobileLink)
{
    AGMobileLinkInit(mobileLink);

    mobileLink->useWizards = TRUE;
    
    return mobileLink;
}

AGMobileLink * AGMobileLinkNew(void)
{
    AGMobileLink * result;

    result = (AGMobileLink *)malloc(sizeof(AGMobileLink));
    return (NULL == result) ? result : AGMobileLinkInit(result);
}

void AGMobileLinkFinalize(AGMobileLink * mobileLink)
{
    bzero(mobileLink, sizeof(AGMobileLink));
}

void AGMobileLinkFree(AGMobileLink * mobileLink)
{
    AGMobileLinkFinalize(mobileLink);
    free(mobileLink);
}

void openMobileLinkRegistryKey(HKEY * key, REGSAM access)
{
    *key = NULL;

    RegOpenKeyEx(HKEY_CURRENT_USER,
        kMALConfig,
        0,
        access,
        key);
}
/*---------------------------------------------------------------------------*/
char *AGMobileLinkGetSecurityDLLLocation()
{

    DWORD cbSize = 0;
    DWORD type;
    HKEY key = NULL;
    char *ret = NULL;
    LONG rc;

    openMobileLinkRegistryKey(&key, KEY_READ);

    if (key) {
        
        /* Get size of buffer needed to hold name */
        rc = RegQueryValueEx(key, kMALConfig_Security_DLL_Location,
                             0, &type, NULL, &cbSize);

        if (rc == ERROR_SUCCESS) {
                /* Allocate buffer to hold name. */
            ret = (char*)malloc(cbSize + 1);
            if (ret) {        
                cbSize = cbSize + 1;
                rc = RegQueryValueEx(key,  kMALConfig_Security_DLL_Location,
                                     0, &type, (LPBYTE)ret, &cbSize);
                if (rc != ERROR_SUCCESS) {
                    free(ret);
                    ret = NULL;
                }
            }
       }
       
        RegCloseKey(key);
    }
    
    return ret;

}
AGMobileLink * AGMobileLinkNewAndReadFromRegistry(void)
{
    AGMobileLink * result;
    DWORD cbSize;
    DWORD type;

    cbSize = sizeof(AGMobileLink);
    result = (AGMobileLink *)malloc(cbSize);
    if (NULL != result) {

        HKEY key = NULL;

        bzero(result, cbSize);

        openMobileLinkRegistryKey(&key, KEY_READ);

        if (NULL != key) {

            LONG regResult;

            regResult = RegQueryValueEx(key,
                kMALConfig_Use_Wizards,
                0,
                &type, 
                (uint8*)result,
                &cbSize);
            
            RegCloseKey(key);

            if (ERROR_SUCCESS == regResult)
                return result;
        }

        AGMobileLinkInitWithDefaults(result);

    }
    return result;
}

static HKEY createRegistryKeyForWriting(void)
{
    HKEY key = NULL;
    DWORD disposition;

    openMobileLinkRegistryKey(&key, KEY_WRITE);
    if (NULL != key)
        return key;
    
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
        kMALConfig,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &key,
        &disposition))
        return key;

    return NULL;
}

void AGMobileLinkWriteToRegistry(AGMobileLink * mobileLink)
{
    HKEY key = NULL;

    key = createRegistryKeyForWriting();

    if (NULL == key)
        return;

    RegSetValueEx(key,
        kMALConfig_Use_Wizards,
        0,
        REG_BINARY,
        (char*)mobileLink,
        sizeof(AGMobileLink));
    
    RegCloseKey(key);

}

AGBool AGMobileLinkGetUseWizards(void)
{
    AGBool result;
    AGMobileLink * mobileLink;

    mobileLink = AGMobileLinkNewAndReadFromRegistry();

    if (NULL != mobileLink) {
        result = mobileLink->useWizards;
        AGMobileLinkFree(mobileLink);
        return result;
    }

    /* Registry problem; return our default. */
    return TRUE;
}

void AGMobileLinkSetUseWizards(AGBool use)
{
    AGMobileLink * mobileLink;

    mobileLink = AGMobileLinkNewAndReadFromRegistry();

    if (NULL != mobileLink) {
        mobileLink->useWizards = use;
        AGMobileLinkWriteToRegistry(mobileLink);
        AGMobileLinkFree(mobileLink);
    }
}

/* ----------------------------------------------------------------------------
*/
void AGMobileLinkDeviceEntryFree(AGDeviceEntry * dle)
{
    if (NULL != dle->deviceName)
        free(dle->deviceName);
    if (NULL != dle->prefsPath)
        free(dle->prefsPath);
    free(dle);
}

/* ----------------------------------------------------------------------------
*/
AGDeviceEntry * AGMobileLinkDeviceEntryNew(char * deviceName,
                                           AGDeviceType deviceType,
                                           char * prefsPath)
{
    AGDeviceEntry * dle;

    dle = (AGDeviceEntry *)malloc(sizeof(AGDeviceEntry));
    if (NULL == dle)
        return NULL;

    bzero(dle, sizeof(AGDeviceEntry));

    dle->deviceName = deviceName;
    dle->deviceType = deviceType;
    dle->prefsPath = prefsPath;

    return dle;
}

AGDeviceEntry * AGMobileLinkGetCurrentDevice(void)
{
    HKEY key = NULL;
    DWORD cbSize;
    DWORD reg_val_type;
    char * deviceName = NULL;
    char * prefsPath = NULL;
    AGDeviceType deviceType = (AGDeviceType)-1;

    openMobileLinkRegistryKey(&key, KEY_READ);

    if (NULL != key) {

        LONG regResult;

        /* Read in device type. */
        cbSize = sizeof(AGDeviceType);
        regResult = RegQueryValueEx(key,
            kMALConfig_Current_Device_Type,
            0,
            &reg_val_type, 
            (LPBYTE)&deviceType,
            &cbSize);

        if (AG_NO_DEVICE_TYPE != deviceType) {

            /* Get size of buffer needed to hold device name. */
            regResult = RegQueryValueEx(key,
                (AG_PALM_DEVICE_TYPE == deviceType)
                ? kMALConfig_Current_Palm_Device_Name
                : kMALConfig_Current_CE_Device_Name,
                0,
                &reg_val_type, 
                NULL,
                &cbSize);

            if (ERROR_SUCCESS == regResult) {

                /* Allocate buffer to hold device name. */
                deviceName = (char*)malloc(cbSize + 1);
                if (NULL != deviceName) {
        
                    /* Read in actual device name. */
                    cbSize = cbSize + 1;
                    regResult = RegQueryValueEx(key,
                        (AG_PALM_DEVICE_TYPE == deviceType)
                        ? kMALConfig_Current_Palm_Device_Name
                        : kMALConfig_Current_CE_Device_Name,
                        0,
                        &reg_val_type, 
                        (LPBYTE)deviceName,
                        &cbSize);

                    if (ERROR_SUCCESS != regResult) {

                        free(deviceName);
                        deviceName = NULL;

                    }

                }

            }
        
            /* Get size of buffer needed to hold device path. */
            regResult = RegQueryValueEx(key,
                (AG_PALM_DEVICE_TYPE == deviceType)
                ? kMALConfig_Current_Palm_Device_Path
                : kMALConfig_Current_CE_Device_Path,
                0,
                &reg_val_type, 
                NULL,
                &cbSize);

            if (ERROR_SUCCESS == regResult) {

                /* Allocate buffer to hold device path. */
                prefsPath = (char*)malloc(cbSize + 1);

                if (NULL != prefsPath) {
        
                    /* Read in actual device path. */
                    cbSize = cbSize + 1;
                    regResult = RegQueryValueEx(key,
                        (AG_PALM_DEVICE_TYPE == deviceType)
                        ? kMALConfig_Current_Palm_Device_Path
                        : kMALConfig_Current_CE_Device_Path,
                        0,
                        &reg_val_type, 
                        (LPBYTE)prefsPath,
                        &cbSize);

                    if (ERROR_SUCCESS != regResult) {

                        free(prefsPath);
                        prefsPath = NULL;

                    }

                }

            }

        }

        RegCloseKey(key);

    }

    if (NULL != deviceName && NULL != prefsPath && deviceType >= 0)
        return AGMobileLinkDeviceEntryNew(deviceName, deviceType, prefsPath);
    else
        return NULL;

}

void AGMobileLinkSetCurrentDevice(AGDeviceEntry * deviceEntry)
{
    HKEY key = NULL;
    AGDeviceEntry * deviceEntrySafe = NULL;
    AGDeviceEntry deNone;

    if (NULL != deviceEntry) {
        deviceEntrySafe = deviceEntry;
        memset(&deNone, 0, sizeof(AGDeviceEntry));
    }
    else {

        deviceEntrySafe = &deNone;
        deNone.deviceName = strdup("");
        deNone.deviceType = AG_NO_DEVICE_TYPE;
        deNone.prefsPath = strdup("");

    }

    openMobileLinkRegistryKey(&key, KEY_WRITE);

    if (NULL != key) {

        LONG regResult;

        regResult = RegSetValueEx(key,
            kMALConfig_Current_Device_Type,
            0,
            REG_DWORD, 
            (LPBYTE)&deviceEntrySafe->deviceType,
            sizeof(DWORD));

        if (AG_NO_DEVICE_TYPE != deviceEntrySafe->deviceType) {

            regResult = RegSetValueEx(key,
                (AG_PALM_DEVICE_TYPE == deviceEntrySafe->deviceType)
                ? kMALConfig_Current_Palm_Device_Name
                : kMALConfig_Current_CE_Device_Name,
                0,
                REG_SZ, 
                deviceEntrySafe->deviceName,
                (NULL != deviceEntrySafe->deviceName)
                ? strlen(deviceEntrySafe->deviceName) + 1 : 0);

            regResult = RegSetValueEx(key,
                (AG_PALM_DEVICE_TYPE == deviceEntrySafe->deviceType)
                ? kMALConfig_Current_Palm_Device_Path
                : kMALConfig_Current_CE_Device_Path,
                0,
                REG_SZ, 
                deviceEntrySafe->prefsPath,
                (NULL != deviceEntrySafe->prefsPath)
                ? strlen(deviceEntrySafe->prefsPath) + 1 : 0);

        }

        RegCloseKey(key);

    }

    if (NULL != deNone.deviceName)
        free(deNone.deviceName);
    if (NULL != deNone.prefsPath)
        free(deNone.prefsPath);

}

#endif /* _WIN32 */
