/**
 * wlanapi library 0.4
 *
 * Copyright (C) 2008, 2009 Moritz Mertinkat <moritz@mertinkat.net>
 * All rights reserved.
 *
 * wlanapi library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wlanapi library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.*
 *
 * You should have received a copy of the GNU General Public License
 * along with wlanapi library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __WLANAPI_H_
#define __WLANAPI_H_

#include <vector>

#include "debug.h"
#include "wlanapi_exception.h"

#ifdef WIN32

    #include <windows.h>
    #include <winioctl.h>

    #define UNDER_CE

    #include "wlanapi_windows_ndisuio.h"
    #include "wlanapi_windows_wzc.h"
    #include "wlanapi_windows_wlan.h"

#else

    #include <stdio.h>
    #include <iostream>
    #include <stdlib.h>

    #ifdef DBUS
        #include <dbus/dbus.h>
        #include <dbus/dbus-glib.h>
    #endif

#endif

#define ADAPTER_NAME_LENGTH 256
#define ADAPTER_DESCRIPTION_LENGTH 256

#define AP_NAME_LENGTH 256

#ifdef WIN32
    typedef wchar_t ADAPTER_NAME;
    typedef wchar_t ADAPTER_DESCRIPTION;
    typedef char AP_NAME;
#else
    typedef char ADAPTER_NAME;
    typedef char ADAPTER_DESCRIPTION;
    typedef char AP_NAME;
#endif

typedef struct _ADAPTER_INFO {
    ADAPTER_NAME name[ADAPTER_NAME_LENGTH];
    ADAPTER_DESCRIPTION description[ADAPTER_DESCRIPTION_LENGTH];
} ADAPTER_INFO;

typedef std::vector<ADAPTER_INFO> ADAPTER_LIST;

typedef struct _MAC_ADDRESS {
    unsigned char u[6];
} MAC_ADDRESS;

typedef struct _AP_INFO {
    AP_NAME name[AP_NAME_LENGTH];
    signed int rssi;
    MAC_ADDRESS mac_address;
} AP_INFO;

typedef std::vector<AP_INFO> AP_LIST;

class wlanapi {
    public:
        const ADAPTER_LIST& get_adapter_list();
        const AP_LIST& get_ap_list(ADAPTER_NAME *adapter_name);
        wlanapi();
        virtual ~wlanapi();
    protected:
    private:

        #ifdef WIN32

            void _ndis_get_adapter_list();
            void _ndis_get_ap_list(ADAPTER_NAME *adapter_name);
            void _ndis_initiate();
            void _ndis_shutdown();

            void _wzc_get_adapter_list();
            void _wlan_get_adapter_list();

            void _wzc_get_ap_list(ADAPTER_NAME *adapter_name);
            void _wlan_get_ap_list(ADAPTER_NAME *adapter_name);

            HINSTANCE wzc_library;
            WZCEnumInterfacesFunction WZCEnumInterfaces;
            WZCQueryInterfaceFunction WZCQueryInterface;
            WZCRefreshInterfaceFunction WZCRefreshInterface;

            HINSTANCE wlan_library;
            HANDLE wlan_handle;
            WlanOpenHandleFunction WlanOpenHandle;
            WlanEnumInterfacesFunction WlanEnumInterfaces;
            WlanGetNetworkBssListFunction WlanGetNetworkBssList;
            WlanCloseHandleFunction WlanCloseHandle;
            WlanFreeMemoryFunction WlanFreeMemory;
            WlanGetAvailableNetworkListFunction WlanGetAvailableNetworkList;

        #else

            #ifdef DBUS
                void _dbus_get_adapter_list();
                void _dbus_get_ap_list(ADAPTER_NAME *adapter_name);
                void _dbus_initiate();
                void _dbus_shutdown();
                DBusGConnection *dbus_connection;
                DBusGProxy *_dbus_nm_proxy;
            #endif

        #endif

        ADAPTER_LIST adapter_list;
        AP_LIST ap_list;

        typedef void (wlanapi::*GET_ADAPTER_LIST_FUNC)();
        GET_ADAPTER_LIST_FUNC get_adapter_list_func;

        typedef void (wlanapi::*GET_AP_LIST_FUNC)(ADAPTER_NAME *adapter_name);
        GET_AP_LIST_FUNC get_ap_list_func;

};

#endif // __WLANAPI_H_
