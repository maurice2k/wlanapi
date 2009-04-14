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

#include "wlanapi.h"

/**
 * Constructor
 */
wlanapi::wlanapi() {

    get_adapter_list_func = NULL;
    get_ap_list_func = NULL;

    #ifdef WIN32

        // Wireless Zero Configuration API (WZC API)
        wzc_library = NULL;
        WZCEnumInterfaces = NULL;
        WZCQueryInterface = NULL;
        WZCRefreshInterface = NULL;

        // Native Wifi API (WLAN API)
        wlan_library = NULL;
        wlan_handle = NULL;
        WlanOpenHandle = NULL;
        WlanEnumInterfaces = NULL;
        WlanGetNetworkBssList = NULL;
        WlanCloseHandle = NULL;
        WlanFreeMemory = NULL;

        WlanGetAvailableNetworkList = NULL;

    #else

        #ifdef DBUS
            dbus_connection = NULL;
            _dbus_nm_proxy = NULL;
        #endif

    #endif

}

/**
 * Destructor
 */
wlanapi::~wlanapi() {

    DEBUG_ENTER(wlanapi)

    #ifdef WIN32

    if (wlan_handle != NULL) {
        WlanCloseHandle(wlan_handle, NULL);
        wlan_handle = NULL;
        DEBUG_PRINT("  Closed WLAN API handle\n");
    }

    #else

        #ifdef DBUS
        _dbus_shutdown();
        #endif

    #endif

    DEBUG_LEAVE(wlanapi)

}

/**
 * Returns list of adapters
 *
 * @return ADAPTER_LIST&
 * @throws wlanapi_exception
 */
const ADAPTER_LIST& wlanapi::get_adapter_list() {

    DEBUG_ENTER(wlanapi)

    if (get_adapter_list_func == NULL) {

        #ifdef WIN32

        try {
            _wzc_get_adapter_list();
        } catch (std::exception &e){

            DEBUG_PRINT("WZC failed: %s\n", e.what());

            try {
                _wlan_get_adapter_list();
            } catch (std::exception &e){

                DEBUG_PRINT("WLAN failed: %s\n", e.what());

                try {
                    _ndis_get_adapter_list();
                } catch (std::exception &e){
                    DEBUG_PRINT("NDIS failed: %s\n", e.what());
                    throw wlanapi_exception("Unable to find a valid method for retrieving a list of network adapters");
                }

            }

        }

        #else

            #ifdef DBUS

            try {
                _dbus_get_adapter_list();
            } catch (std::exception &e) {
                DEBUG_PRINT("DBUS failed: %s\n", e.what());
                throw wlanapi_exception("Unable to find a valid method for retrieving a list of network adapters");
            }
            #endif // DBUS

        #endif

    } else {
        (this->*get_adapter_list_func)();
    }

    DEBUG_LEAVE(wlanapi);

    return adapter_list;

}

/**
 * Returns list of access points for a given adapter. If no adapter is
 * given all adapters are queried for their access points.
 *
 * Please note that the list may contain multiple references to one
 * access point.
 *
 * @param  ADAPTER_NAME *adapter_name [OPTIONAL]
 * @return AP_LIST&
 * @throws wlanapi_exception
 */
const AP_LIST& wlanapi::get_ap_list(ADAPTER_NAME *adapter_name) {

    DEBUG_ENTER(wlanapi);

    if (get_ap_list_func == NULL) {
        throw wlanapi_exception("You have to call get_adapter_list() first!");
    }

#ifdef WIN32
    if (adapter_name == NULL || (adapter_name != NULL && wcslen(adapter_name) == 0)) {
#else
    if (adapter_name == NULL || (adapter_name != NULL && strlen(adapter_name) == 0)) {
#endif

        for (ADAPTER_LIST::iterator it = adapter_list.begin(); it < adapter_list.end(); ++it) {
            DEBUG_PRINT("  Handling adapter %s...\n", (char *)it->name);
            (this->*get_ap_list_func)(it->name);
        }

    } else {
        (this->*get_ap_list_func)(adapter_name);
    }

    DEBUG_LEAVE(wlanapi);

    return ap_list;

}

#ifdef WIN32

/**
 * Fetches list of adapters using NDIS User-Mode I/O driver (NDISUIO 5.1)
 * http://msdn.microsoft.com/en-us/library/ms892537.aspx
 *
 * Works with Windows XP >= SP1
 *
 * @return void
 * @throws wlanapi_exception
 */
void wlanapi::_ndis_get_adapter_list() {

    DWORD dwBytes;
    BOOL result;
    HANDLE ndisuio;
    ADAPTER_INFO adapter_info;

    DEBUG_ENTER(wlanapi)

    ndisuio = CreateFile(
        "\\\\.\\\\Ndisuio",
        GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        INVALID_HANDLE_VALUE
    );

    if (ndisuio == INVALID_HANDLE_VALUE) {
        throw wlanapi_exception("NDIS: Failed to open connection to NDISUIO with error code: %d", (int)GetLastError());
    }

    result = DeviceIoControl(
        ndisuio,
        IOCTL_NDISUIO_BIND_WAIT,
        NULL,
        0,
        NULL,
        0,
        &dwBytes,
        NULL
    );

    if (!result) {
        throw wlanapi_exception("NDIS: IOCTL_NDISUIO_BIND_WAIT failed with error code: %d", (int)GetLastError());
    }

    PNDISUIO_QUERY_BINDING pQueryBinding = (PNDISUIO_QUERY_BINDING)malloc(sizeof(NDISUIO_QUERY_BINDING) + 1024);
    //boost::scoped_array<NDISUIO_QUERY_BINDING> pQueryBinding((PNDISUIO_QUERY_BINDING)new char[sizeof(NDISUIO_QUERY_BINDING) + 1024]);

    adapter_list.clear();

    int i = 0;
    while (1) {

        memset(pQueryBinding, 0, sizeof(NDISUIO_QUERY_BINDING));
        pQueryBinding->BindingIndex = i;

        dwBytes = 0;

        result = DeviceIoControl(
            ndisuio,
            IOCTL_NDISUIO_QUERY_BINDING,
            pQueryBinding,
            sizeof(NDISUIO_QUERY_BINDING),
            pQueryBinding,
            sizeof(NDISUIO_QUERY_BINDING) + 1024,
            &dwBytes,
            NULL
        );

        if (!result) {
            break;
        }

        WCHAR wtmp[ADAPTER_NAME_LENGTH + ADAPTER_DESCRIPTION_LENGTH];

        if (pQueryBinding->DeviceNameLength < sizeof(wtmp)) {

            memset(wtmp, 0, sizeof(wtmp));
            memcpy(wtmp, (PUCHAR)pQueryBinding + pQueryBinding->DeviceNameOffset, pQueryBinding->DeviceNameLength);

            DEBUG_WPRINT(L"  pQueryBinding->DeviceNameLength: %u\n", pQueryBinding->DeviceNameLength);
            DEBUG_WPRINT(L"  pQueryBinding->DeviceName: %s\n", wtmp);

            int length = wcslen(wtmp);
            if (length > 0 && length < ADAPTER_NAME_LENGTH) {

                memset(&adapter_info, 0, sizeof(adapter_info));

                WCHAR *pos = wcsstr(wtmp, L"\\DEVICE\\");
                if (pos != NULL) {
                    wcscpy(adapter_info.name, pos + 8);
                } else {
                    wcscpy(adapter_info.name, wtmp);
                }

                DEBUG_WPRINT(L"  adapter_info.name: %s\n", adapter_info.name);

                if (pQueryBinding->DeviceDescrLength < sizeof(wtmp)) {

                    memset(wtmp, 0, sizeof(wtmp));
                    memcpy(wtmp, (PUCHAR)pQueryBinding + pQueryBinding->DeviceDescrOffset, pQueryBinding->DeviceDescrLength);

                    DEBUG_WPRINT(L"  pQueryBinding->DeviceDescrLength: %u\n", pQueryBinding->DeviceDescrLength);
                    DEBUG_WPRINT(L"  pQueryBinding->DeviceDescr: %s\n", wtmp);

                    length = wcslen(wtmp);
                    if (length > 0 && length < ADAPTER_DESCRIPTION_LENGTH) {

                        wcscpy(adapter_info.description, wtmp);
                        DEBUG_WPRINT(L"  adapter_info.description: %s\n", adapter_info.description);

                    }

                }

                adapter_list.push_back(adapter_info);

            }

        }

        i++;

        DEBUG_PRINT("\n");

    }

    free(pQueryBinding);
    get_ap_list_func = &wlanapi::_ndis_get_ap_list;
    get_adapter_list_func = &wlanapi::_ndis_get_adapter_list;

    DEBUG_LEAVE(wlanapi)

}

/**
 * Fetches list of access points using NDIS User-Mode I/O driver (NDISUIO 5.1)
 * http://msdn.microsoft.com/en-us/library/ms892537.aspx
 *
 * Works with Windows XP >= SP1
 *
 * @return void
 * @throws wlanapi_exception
 */
void wlanapi::_ndis_get_ap_list(ADAPTER_NAME *adapter_name) {

    BOOL result;
    HANDLE ndisuio;
    DWORD dwOIDCode;
    DWORD dwBytes;
    AP_INFO ap_info;

    DEBUG_ENTER(wlanapi);

    DEBUG_WPRINT(L"  Querying adapter '%s'...\n", adapter_name);

    char device[ADAPTER_NAME_LENGTH + 4];
    memset(device, 0, sizeof(device));
    strcpy(device, "\\\\.\\");
    wcstombs(device + 4, adapter_name, ADAPTER_NAME_LENGTH - 1);

    ndisuio = CreateFile(
        device,
        GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        INVALID_HANDLE_VALUE
    );

    if (ndisuio == INVALID_HANDLE_VALUE) {
        throw wlanapi_exception("NDIS: Failed to open connection to NDISUIO with error code: %d", (int)GetLastError());
    }

    DEBUG_PRINT("  Connection to NDISUIO has been opened.\n");

    dwOIDCode = OID_802_11_BSSID_LIST_SCAN;
    dwBytes = 0;
    result = DeviceIoControl(
        ndisuio,
        IOCTL_NDIS_QUERY_GLOBAL_STATS,
        &dwOIDCode,
        sizeof(dwOIDCode),
        NULL,
        0,
        &dwBytes,
        NULL
    );

    //printf("result: %d\n", result);
    //printf("dwBytes: %d\n", dwBytes);

    PNDIS_802_11_BSSID_LIST_EX pList = (PNDIS_802_11_BSSID_LIST_EX)malloc(sizeof(NDIS_802_11_BSSID_LIST_EX) * 50);
    //boost::scoped_array<NDIS_802_11_BSSID_LIST_EX> pList((PNDIS_802_11_BSSID_LIST_EX)new char[sizeof(NDIS_802_11_BSSID_LIST_EX) * 50]);

    memset(pList, 0, sizeof(NDIS_802_11_BSSID_LIST_EX) * 50);

    dwOIDCode = OID_802_11_BSSID_LIST;
    dwBytes = 0;
    result = DeviceIoControl(
        ndisuio,
        IOCTL_NDIS_QUERY_GLOBAL_STATS,
        &dwOIDCode,
        sizeof(dwOIDCode),
        pList,
        sizeof(NDIS_802_11_BSSID_LIST_EX) * 50,
        &dwBytes,
        NULL
    );

    //printf("result: %d\n", result);
    //printf("dwBytes: %d\n", dwBytes);

    DEBUG_PRINT("  pList->NumberOfItems: %ld\n\n", pList->NumberOfItems);

    PNDIS_WLAN_BSSID_EX pBssid = (PNDIS_WLAN_BSSID_EX)&pList->Bssid[0];

    const unsigned char *buffer_end = reinterpret_cast<unsigned char*>(pBssid) + dwBytes;

    for (DWORD i = 0; i < pList->NumberOfItems; ++i) {

        memset(&ap_info, 0, sizeof(ap_info));

        if (pBssid->Length < sizeof(NDIS_WLAN_BSSID) || (reinterpret_cast<unsigned char*>(pBssid) + pBssid->Length > buffer_end)) {
            DEBUG_PRINT("  Bssid structure looks odd. Break!\n");
            break;
        }

        DEBUG_PRINT("  == [ %ld ] ==================================================\n", i + 1);
        DEBUG_PRINT("  pBssid->Length: %ld\n", pBssid->Length);
        DEBUG_PRINT("  pBssid->Ssid.Ssid: %s\n", pBssid->Ssid.Ssid);
        DEBUG_PRINT("  pBssid->MacAddress: %02X-%02X-%02X-%02X-%02X-%02X\n", pBssid->MacAddress[0], pBssid->MacAddress[1], pBssid->MacAddress[2], pBssid->MacAddress[3], pBssid->MacAddress[4], pBssid->MacAddress[5]);
        DEBUG_PRINT("  pBssid->Rssi: %ld\n", pBssid->Rssi);
        DEBUG_PRINT("\n");

        memcpy(&ap_info.mac_address, pBssid->MacAddress, sizeof(ap_info.mac_address));
        strncpy(ap_info.name, (AP_NAME *)pBssid->Ssid.Ssid, AP_NAME_LENGTH - 1);
        ap_info.rssi = pBssid->Rssi;

        ap_list.push_back(ap_info);

        pBssid = (PNDIS_WLAN_BSSID_EX)((PUCHAR)pBssid + pBssid->Length);

    }

    free(pList);
    CloseHandle(ndisuio);

    DEBUG_LEAVE(wlanapi);

}

/**
 * Fetches list of adapters using Wireless Zero Configuration service (WZCSVC)
 * http://msdn.microsoft.com/en-us/library/ms706593(VS.85).aspx
 *
 * Works with Windows XP >= SP2
 *
 * @return void
 * @throws wlanapi_exception
 */
void wlanapi::_wzc_get_adapter_list() {

    ADAPTER_INFO adapter_info;

    DEBUG_ENTER(wlanapi)

    if (wzc_library == NULL) {
        wzc_library = LoadLibrary("wzcsapi");
    }

    if (wzc_library == NULL) {
        throw wlanapi_exception("WZC: Unable to load wzcsapi!\n");
    }

    DEBUG_PRINT("  Successfully loaded library 'wzcsapi'\n");

    if (WZCEnumInterfaces == NULL) {

        WZCEnumInterfaces = (WZCEnumInterfacesFunction)GetProcAddress(wzc_library, "WZCEnumInterfaces");

        if (WZCEnumInterfaces == NULL) {
            throw wlanapi_exception("WZC: Unable to find process address of function 'WZCEnumInterfaces' (error code: %d)\n", (int)GetLastError());
        }

    }

    // Get the list of interfaces. WZCEnumInterfaces allocates INTFS_KEY_TABLE::pIntfs.
    INTFS_KEY_TABLE interface_list;
    memset(&interface_list, 0, sizeof(INTFS_KEY_TABLE));

    DWORD result = WZCEnumInterfaces(NULL, &interface_list);
    if (result != ERROR_SUCCESS) {
        throw wlanapi_exception("WZC: Interface enumeration failed; WCZ service is probably not running\n");
    }

    DEBUG_PRINT("  Number of devices: %lu\n", interface_list.dwNumIntfs);

    if (WZCQueryInterface == NULL) {

        WZCQueryInterface = (WZCQueryInterfaceFunction)GetProcAddress(wzc_library, "WZCQueryInterface");

        if (WZCQueryInterface == NULL) {

            // cleanup
            LocalFree(interface_list.pIntfs);

            throw wlanapi_exception("WZC: Unable to find process address of function 'WZCQueryInterface' (error code: %d)\n", (int)GetLastError());

        }

    }

    if (WZCRefreshInterface == NULL) {

        WZCRefreshInterface = (WZCRefreshInterfaceFunction)GetProcAddress(wzc_library, "WZCRefreshInterface");

        if (WZCRefreshInterface == NULL) {

            // cleanup
            LocalFree(interface_list.pIntfs);

            throw wlanapi_exception("WZC: Unable to find process address of function 'WZCRefreshInterface' (error code: %d)\n", (int)GetLastError());

        }

    }

    adapter_list.clear();

    for (int i = 0; i < static_cast<int>(interface_list.dwNumIntfs); ++i) {

        INTF_ENTRY interface_data;
        memset(&interface_data, 0, sizeof(INTF_ENTRY));

        interface_data.wszGuid = interface_list.pIntfs[i].wszGuid;
        DWORD dwOutFlags = 1;

        DWORD result = WZCQueryInterface(NULL, INTF_DESCR, &interface_data, &dwOutFlags);

        if (result != ERROR_SUCCESS) {

            // cleanup
            LocalFree(interface_list.pIntfs);

            throw wlanapi_exception("WZC: Interface query failed; WCZ service is probably not running\n");

        }

        int length = wcslen(interface_list.pIntfs[i].wszGuid);
        if (length > 0 && length < ADAPTER_NAME_LENGTH) {

            memset(&adapter_info, 0, sizeof(adapter_info));

            wcscpy(adapter_info.name, interface_list.pIntfs[i].wszGuid);
            DEBUG_WPRINT(L"  interface_list.pIntfs[%i].wszGuid: %s\n", i, interface_list.pIntfs[i].wszGuid);
            DEBUG_WPRINT(L"  adapter_info.name: %s\n", adapter_info.name);

            length = wcslen(interface_data.wszDescr);
            if (length > 0 && length < ADAPTER_DESCRIPTION_LENGTH) {
                wcscpy(adapter_info.description, interface_data.wszDescr);
                DEBUG_WPRINT(L"  interface_data.wszDescr: %s\n", interface_data.wszDescr);
                DEBUG_WPRINT(L"  adapter_info.description: %s\n", adapter_info.description);
            }

            adapter_list.push_back(adapter_info);

        }

        DEBUG_PRINT("\n");

    }

    LocalFree(interface_list.pIntfs);

    get_ap_list_func = &wlanapi::_wzc_get_ap_list;
    get_adapter_list_func = &wlanapi::_wzc_get_adapter_list;

    DEBUG_LEAVE(wlanapi)

}

/**
 * Fetches list of access points using Wireless Zero Configuration service (WZCSVC)
 * http://msdn.microsoft.com/en-us/library/ms706593(VS.85).aspx
 *
 * Works with Windows XP >= SP2
 *
 * @return void
 * @throws wlanapi_exception
 */
void wlanapi::_wzc_get_ap_list(ADAPTER_NAME *adapter_name) {

    DWORD result;
    AP_INFO ap_info;

    DEBUG_ENTER(wlanapi);

    DEBUG_WPRINT(L"  Querying adapter '%s'...\n", adapter_name);

    INTF_ENTRY interface_data;
    memset(&interface_data, 0, sizeof(INTF_ENTRY));
    interface_data.wszGuid = adapter_name;
    DWORD dwOutFlags = 1;

    result = WZCQueryInterface(NULL, INTF_BSSIDLIST | INTF_LIST_SCAN, &interface_data, &dwOutFlags);

    if (result != ERROR_SUCCESS) {
        throw wlanapi_exception("WZC: Interface query failed; WCZ service is probably not running\n");
    }

    if ((dwOutFlags & INTF_BSSIDLIST) != INTF_BSSIDLIST) {
        throw wlanapi_exception("WZC: Interface query consistency failure: incorrect flags\n");
    }

    if (interface_data.rdBSSIDList.dwDataLen == 0 || interface_data.rdBSSIDList.dwDataLen < sizeof(NDIS_802_11_BSSID_LIST)) {

        int data_until_padding = (UCHAR*)&interface_data.padding1 - (UCHAR*)&interface_data;

        // this is a hack to support Windows XP SP2 with WLAN Hotfix and SP3
        memmove((UCHAR*)&interface_data + data_until_padding, (UCHAR*)&interface_data + data_until_padding + 8, sizeof(interface_data) - data_until_padding - 8);

        if (interface_data.rdBSSIDList.dwDataLen == 0 || interface_data.rdBSSIDList.dwDataLen < sizeof(NDIS_802_11_BSSID_LIST)) {
            // cleanup
            LocalFree(interface_data.rdBSSIDList.pData);
            throw wlanapi_exception("WZC: Interface query consistency failure: no data or incorrect data length (length: %ld)\n", interface_data.rdBSSIDList.dwDataLen);
        }

    }

    DEBUG_PRINT("  dwOutFlags & INTF_BSSIDLIST == INTF_BSSIDLIST: %d\n", (dwOutFlags & INTF_BSSIDLIST) == INTF_BSSIDLIST);
    DEBUG_PRINT("  interface_data.rdBSSIDList.dwDataLen: %ld\n", interface_data.rdBSSIDList.dwDataLen);

    const PNDIS_802_11_BSSID_LIST pList = reinterpret_cast<NDIS_802_11_BSSID_LIST*>(interface_data.rdBSSIDList.pData);
    DEBUG_PRINT("  pList->NumberOfItems: %ld\n\n", pList->NumberOfItems);

    PNDIS_WLAN_BSSID pBssid = reinterpret_cast<PNDIS_WLAN_BSSID>(&pList->Bssid[0]);
    const unsigned char *buffer_end = reinterpret_cast<unsigned char*>(pBssid) + interface_data.rdBSSIDList.dwDataLen;

    for (DWORD i = 0; i < pList->NumberOfItems; ++i) {

        if (pBssid->Length < sizeof(NDIS_WLAN_BSSID) || (reinterpret_cast<unsigned char*>(pBssid) + pBssid->Length > buffer_end)) {
            // cleanup
            LocalFree(interface_data.rdBSSIDList.pData);
            throw wlanapi_exception("WZC: Bssid structure looks odd. Break!\n");
        }

        memset(&ap_info, 0, sizeof(ap_info));

        DEBUG_PRINT("  == [ %ld ] ==================================================\n", i + 1);
        DEBUG_PRINT("  pBssid->Length: %ld\n", pBssid->Length);
        DEBUG_PRINT("  pBssid->Ssid.Ssid: %s\n", pBssid->Ssid.Ssid);
        DEBUG_PRINT("  pBssid->MacAddress: %02X-%02X-%02X-%02X-%02X-%02X\n", pBssid->MacAddress[0], pBssid->MacAddress[1], pBssid->MacAddress[2], pBssid->MacAddress[3], pBssid->MacAddress[4], pBssid->MacAddress[5]);
        DEBUG_PRINT("  pBssid->Rssi: %ld\n", pBssid->Rssi);
        DEBUG_PRINT("\n");

        memcpy(&ap_info.mac_address, pBssid->MacAddress, sizeof(ap_info.mac_address));
        strncpy(ap_info.name, (AP_NAME *)pBssid->Ssid.Ssid, AP_NAME_LENGTH - 1);
        ap_info.rssi = pBssid->Rssi;

        ap_list.push_back(ap_info);

        pBssid = reinterpret_cast<PNDIS_WLAN_BSSID>(reinterpret_cast<unsigned char*>(pBssid) + pBssid->Length);

    }

    LocalFree(interface_data.rdBSSIDList.pData);

    DEBUG_LEAVE(wlanapi);

}

/**
 * Fetches list of adapters using Native Wifi (WLAN) API
 * http://msdn.microsoft.com/en-us/library/ms706556(VS.85).aspx
 *
 * Works with Windows XP >= SP3 and Windows Vista
 *
 * @return void
 * @throws wlanapi_exception
 */
void wlanapi::_wlan_get_adapter_list() {

    ADAPTER_INFO adapter_info;
    DWORD result;

    DEBUG_ENTER(wlanapi)

    if (wlan_library == NULL) {
        wlan_library = LoadLibrary("wlanapi");
    }

    if (wlan_library == NULL) {
        throw wlanapi_exception("WLAN: Unable to load wlanapi!\n");
    }

    DEBUG_PRINT("  Successfully loaded library 'wlanapi'\n");

    if (WlanOpenHandle == NULL) {

        WlanOpenHandle = (WlanOpenHandleFunction)GetProcAddress(wlan_library, "WlanOpenHandle");

        if (WlanOpenHandle == NULL) {
            throw wlanapi_exception("WLAN: Unable to find process address of function 'WlanOpenHandle' (error code: %d)\n", (int)GetLastError());
        }

    }

    if (WlanCloseHandle == NULL) {

        WlanCloseHandle = (WlanCloseHandleFunction)GetProcAddress(wlan_library, "WlanCloseHandle");

        if (WlanCloseHandle == NULL) {
            throw wlanapi_exception("WLAN: Unable to find process address of function 'WlanCloseHandle' (error code: %d)\n", (int)GetLastError());
        }

    }

    if (WlanEnumInterfaces == NULL) {

        WlanEnumInterfaces = (WlanEnumInterfacesFunction)GetProcAddress(wlan_library, "WlanEnumInterfaces");

        if (WlanEnumInterfaces == NULL) {
            throw wlanapi_exception("WLAN: Unable to find process address of function 'WlanEnumInterfaces' (error code: %d)\n", (int)GetLastError());
        }

    }

    if (WlanGetNetworkBssList == NULL) {

        WlanGetNetworkBssList = (WlanGetNetworkBssListFunction)GetProcAddress(wlan_library, "WlanGetNetworkBssList");

        if (WlanGetNetworkBssList == NULL) {
            throw wlanapi_exception("WLAN: Unable to find process address of function 'WlanGetNetworkBssList' (error code: %d)\n", (int)GetLastError());
        }

    }

    if (WlanFreeMemory == NULL) {

        WlanFreeMemory = (WlanFreeMemoryFunction)GetProcAddress(wlan_library, "WlanFreeMemory");

        if (WlanFreeMemory == NULL) {
            throw wlanapi_exception("WLAN: Unable to find process address of function 'WlanFreeMemory' (error code: %d)\n", (int)GetLastError());
        }

    }

    if (WlanGetAvailableNetworkList == NULL) {

        WlanGetAvailableNetworkList = (WlanGetAvailableNetworkListFunction)GetProcAddress(wlan_library, "WlanGetAvailableNetworkList");

        if (WlanGetAvailableNetworkList == NULL) {
            throw wlanapi_exception("WLAN: Unable to find process address of function 'WlanGetAvailableNetworkList' (error code: %d)\n", (int)GetLastError());
        }

    }


    DWORD negotiated_version = 0;

    if (wlan_handle == NULL) {

        result = WlanOpenHandle(1, NULL, &negotiated_version, &wlan_handle);

        if (result != ERROR_SUCCESS || wlan_handle == NULL) {
            throw wlanapi_exception("WLAN: Error creating WLAN API handle (error code: %d)\n", (int)GetLastError());
        }

    }

    DEBUG_PRINT("  Negotiated WLAN API version: %ld\n", negotiated_version);

    // get list of adapters

    WLAN_INTERFACE_INFO_LIST *interface_list;

    result = WlanEnumInterfaces(wlan_handle, NULL, &interface_list);
    DEBUG_PRINT("  interface_list.dwNumberOfItems: %ld\n", interface_list->dwNumberOfItems);

    adapter_list.clear();

    // iterate through list of adapters

    for (unsigned int i = 0; i < interface_list->dwNumberOfItems; ++i) {

        memset(&adapter_info, 0, sizeof(adapter_info));

        DEBUG_PRINT("  interface_list->InterfaceInfo[%d].InterfaceGuid: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n", i,
            interface_list->InterfaceInfo[i].InterfaceGuid.Data1,
            interface_list->InterfaceInfo[i].InterfaceGuid.Data2,
            interface_list->InterfaceInfo[i].InterfaceGuid.Data3,
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[0],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[1],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[2],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[3],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[4],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[5],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[6],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[7]
        );

        swprintf(adapter_info.name, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            interface_list->InterfaceInfo[i].InterfaceGuid.Data1,
            interface_list->InterfaceInfo[i].InterfaceGuid.Data2,
            interface_list->InterfaceInfo[i].InterfaceGuid.Data3,
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[0],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[1],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[2],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[3],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[4],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[5],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[6],
            interface_list->InterfaceInfo[i].InterfaceGuid.Data4[7]
        );

        DEBUG_WPRINT(L"  adapter_info.name: %s\n", adapter_info.name);

        DEBUG_WPRINT(L"  interface_list->InterfaceInfo[%d].strInterfaceDescription: %s\n", i, interface_list->InterfaceInfo[i].strInterfaceDescription);

        wcsncpy(adapter_info.description, interface_list->InterfaceInfo[i].strInterfaceDescription, ADAPTER_DESCRIPTION_LENGTH - 1);
        DEBUG_WPRINT(L"  adapter_info.description %s\n", adapter_info.description);

        adapter_list.push_back(adapter_info);

    }

    WlanFreeMemory(interface_list);

    get_ap_list_func = &wlanapi::_wlan_get_ap_list;
    get_adapter_list_func = &wlanapi::_wlan_get_adapter_list;

    DEBUG_LEAVE(wlanapi)

}

/**
 * Fetches list of acess points using Native Wifi (WLAN) API
 * http://msdn.microsoft.com/en-us/library/ms706556(VS.85).aspx
 *
 * Works with Windows XP >= SP3 and Windows Vista
 *
 * @return void
 * @throws wlanapi_exception
 */
void wlanapi::_wlan_get_ap_list(ADAPTER_NAME *adapter_name) {

    DWORD result;
    AP_INFO ap_info;

    DEBUG_ENTER(wlanapi);

    DEBUG_WPRINT(L"  Querying adapter '%s'...\n", adapter_name);

    if (adapter_name == NULL || wcslen(adapter_name) != 38) {
        throw wlanapi_exception("WLAN: No adapter name given or length mismatch (should be 38 wchars long)");
    }

    adapter_name[37] = L'\0';
    adapter_name++;

    GUID guid;
    swscanf(adapter_name, L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        &guid.Data1,
        &guid.Data2,
        &guid.Data3,
        &guid.Data4[0],
        &guid.Data4[1],
        &guid.Data4[2],
        &guid.Data4[3],
        &guid.Data4[4],
        &guid.Data4[5],
        &guid.Data4[6],
        &guid.Data4[7]
    );

    WLAN_BSS_LIST *bss_list;

    result = WlanGetNetworkBssList(
        wlan_handle,
        &guid,
        NULL,  // return all available BSSes
        dot11_BSS_type_any,
        false,
        NULL,
        &bss_list
    );

    if (result != 0x32) {

        // Looks like we're running on Windows Vista...
        //
        // When called from Windows XP (SP3 or SP2 with WLAN Hotfix) this function always
        // returns 0x32, which means it is simply NOT implemented.
        //
        // For more information habe a look at:
        // http://www.wlanapi.info/wiki/windows_xp#WindowsXPServicePack2withWLANAPIHotfix

        if (result != ERROR_SUCCESS) {
            throw wlanapi_exception("WLAN: Unable to get BSS list using WlanGetNetworkBssList (return code: %ld)\n", result);
        }

        DEBUG_PRINT("  bss_list->dwNumberOfItems: %d\n", bss_list->dwNumberOfItems);

        for (DWORD i = 0; i < bss_list->dwNumberOfItems; ++i) {

            memset(&ap_info, 0, sizeof(ap_info));

            DEBUG_PRINT("  == [ %d ] ==================================================\n", i + 1);
            DEBUG_PRINT("  bss_list->wlanBssEntries[%d].dot11Ssid.ucSSID: %s\n", i, bss_list->wlanBssEntries[i].dot11Ssid.ucSSID);
            DEBUG_PRINT("  bss_list->wlanBssEntries[%d].dot11Bssid: %02X-%02X-%02X-%02X-%02X-%02X\n", i, bss_list->wlanBssEntries[i].dot11Bssid[0], bss_list->wlanBssEntries[i].dot11Bssid[1], bss_list->wlanBssEntries[i].dot11Bssid[2], bss_list->wlanBssEntries[i].dot11Bssid[3], bss_list->wlanBssEntries[i].dot11Bssid[4], bss_list->wlanBssEntries[i].dot11Bssid[5]);
            DEBUG_PRINT("  bss_list->wlanBssEntries[%d].lRssi: %d\n", i, (signed)bss_list->wlanBssEntries[i].lRssi);

            DEBUG_PRINT("\n");

            strncpy(ap_info.name, (AP_NAME *)bss_list->wlanBssEntries[i].dot11Ssid.ucSSID, AP_NAME_LENGTH - 1);
            memcpy(&ap_info.mac_address, bss_list->wlanBssEntries[i].dot11Bssid, sizeof(ap_info.mac_address));
            ap_info.rssi = (signed)bss_list->wlanBssEntries[i].lRssi;

            ap_list.push_back(ap_info);

        }

        WlanFreeMemory(bss_list);

    } else {

        // It seems we're running on Windows XP (SP3 or SP2 with WLAN Hotfix), as the
        // function WlanGetNetworkBssList returned 0x32, which means it is NOT implemented.
        //
        // In this case we're using WlanGetAvailableNetworkList instead, which unfortunately
        // gives as a little bit less accurately signal strength values and no MAC address.
        //
        // For more information habe a look at:
        // http://www.wlanapi.info/wiki/windows_xp#WindowsXPServicePack2withWLANAPIHotfix

        PWLAN_AVAILABLE_NETWORK_LIST network_list;

        result = WlanGetAvailableNetworkList(
            wlan_handle,
            &guid,
            0x00000001,
            NULL,
            &network_list
        );

        if (result != ERROR_SUCCESS) {
            throw wlanapi_exception("WLAN: Unable to get BSS list using WlanGetAvailableNetworkList (return code: %ld)\n", result);
        }

        DEBUG_PRINT("  sizeof(WLAN_AVAILABLE_NETWORK): %d\n", sizeof(WLAN_AVAILABLE_NETWORK));

        DEBUG_PRINT("  network_list->dwNumberOfItems: %ld\n", network_list->dwNumberOfItems);
        PWLAN_AVAILABLE_NETWORK network = &network_list->Network[0];

        for (DWORD i = 0; i < network_list->dwNumberOfItems; ++i) {

            if (network->dwFlags != 0 || network->strProfileName[0] != 0) {

                // Only show adapter entries which do not have a profile and
                // which are currently not connected to avoid duplicates:
                // http://msdn.microsoft.com/en-us/library/ms707403(VS.85).aspx

                network++;
                continue;
            }

            memset(&ap_info, 0, sizeof(ap_info));

            DEBUG_PRINT("  == [ %ld ] ==================================================\n", i + 1);
            DEBUG_PRINT("  network->dot11Ssid.ucSSID: %s\n", network->dot11Ssid.ucSSID);
            DEBUG_WPRINT(L"  network->strProfileName: %s\n", network->strProfileName);
            DEBUG_PRINT("  network->wlanSignalQuality: %lu\n", network->wlanSignalQuality);
            DEBUG_PRINT("  network->uNumberOfBssids: %lu\n", network->uNumberOfBssids);
            DEBUG_PRINT("  network->dwFlags: %lu\n", network->dwFlags);

            strncpy(ap_info.name, (AP_NAME *)network->dot11Ssid.ucSSID, AP_NAME_LENGTH - 1);

            // We do only have wlanSignalQuality here, so we have to convert it
            // to an RSSI value using linear interpolation:
            // http://msdn.microsoft.com/en-us/library/ms707403(VS.85).aspx
            ap_info.rssi = -100 + network->wlanSignalQuality / 2;

            ap_list.push_back(ap_info);

            network++;

        }

        WlanFreeMemory(network_list);

    }

    DEBUG_LEAVE(wlanapi);

}

#else

#ifdef DBUS
/**
 * Opens D-BUS connection and creates proxy for NetworkManager
 *
 * @return void
 * @throws wlanapi_exception
 */
void wlanapi::_dbus_initiate() {

    DEBUG_ENTER(wlanapi);

    GError *error = NULL;
    char error_msg[256] = {0};

    if (dbus_connection != NULL) {
        return;
    }

    g_type_init();
    DEBUG_PRINT("  Initialized type system\n");

    // connect to the system bus
    dbus_connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_connection == NULL) {
        strncpy(error_msg, error->message, 255);
        error_msg[255] = '\0';
        g_error_free(error);
        throw wlanapi_exception("D-BUS: Failed to open connection: %s", error_msg);
    }

    DEBUG_PRINT("  Successfully opened SYSTEM D-BUS\n");

    // create a proxy object for the remote interface "org.freedesktop.NetworkManager" on the system bus
    _dbus_nm_proxy = dbus_g_proxy_new_for_name(
        dbus_connection,
        "org.freedesktop.NetworkManager",
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.NetworkManager"
    );

    if (_dbus_nm_proxy != NULL) {
        DEBUG_PRINT("  Created proxy for NetworkManager\n");
    } else {
        throw wlanapi_exception("D-BUS: Failed to create proxy for NetworkManager");
    }

    DEBUG_LEAVE(wlanapi);

}

/**
 * Closes D-BUS connection and releases proxy for NetworkManager
 *
 * @return void
 */
void wlanapi::_dbus_shutdown() {

    DEBUG_ENTER(wlanapi);

    if (dbus_connection != NULL) {

        dbus_g_connection_unref(dbus_connection);
        g_object_unref(_dbus_nm_proxy);

        dbus_connection = NULL;
        _dbus_nm_proxy = NULL;

    }

    DEBUG_LEAVE(wlanapi);

}

/**
 * Fetches adapter list using (K)NetworkManager over D-BUS
 *
 * This should work for most recent (as of 2008) Gnome and KDE desktop
 * systems. Tested under Ubuntu 7.x/8.x, Kubuntu 7.x/8.x and openSUSE 11.x
 *
 * @return void
 * @throws wlanapi_exception
 */
void wlanapi::_dbus_get_adapter_list() {

    DEBUG_ENTER(wlanapi);

    GError *error = NULL;
    char error_msg[256] = {0};
    DBusGProxy *proxy_props;
    GPtrArray *g_device_list = NULL;

    GValue g_device_type = {0};
    guint device_type = 0;

    GValue g_interface = {0};

    ADAPTER_INFO adapter_info;

    _dbus_initiate();

    // get list of devices

    int res = dbus_g_proxy_call(
        _dbus_nm_proxy,
        "GetDevices",
        &error,
        G_TYPE_INVALID,
        dbus_g_type_get_collection("GPtrArray", DBUS_TYPE_G_OBJECT_PATH),   &g_device_list,
        G_TYPE_INVALID
    );

    if (!res) {
        strcpy(error_msg, error->message);
        g_error_free(error);
        error = NULL;
        throw wlanapi_exception("D-BUS: Error calling org.freedesktop.NetworkManager.GetDevices method: %s", error_msg);
    }

    DEBUG_PRINT("  Successfully called org.freedesktop.NetworkManager.GetDevices method\n");

    // interate through list of devices and get some extra information

    for (uint i = 0; i < g_device_list->len; i++) {

        gchar *path = (gchar*)g_ptr_array_index(g_device_list, i);

        DEBUG_PRINT("\n  Device UDI: %s\n", path);

        proxy_props = dbus_g_proxy_new_from_proxy(
            _dbus_nm_proxy,
            DBUS_INTERFACE_PROPERTIES,
            path
        );

        int res = dbus_g_proxy_call(
            proxy_props,
            "Get",
            &error,
            G_TYPE_STRING,  "org.freedesktop.NetworkManager.Device",
            G_TYPE_STRING,  "DeviceType",
            G_TYPE_INVALID,
            G_TYPE_VALUE,   &g_device_type,
            G_TYPE_INVALID
        );

        if (!res) {

            strcpy(error_msg, error->message);
            g_error_free(error);
            error = NULL;

            // cleanup
            g_ptr_array_free(g_device_list, TRUE);
            g_object_unref(proxy_props);
            g_free(path);

            throw wlanapi_exception("D-BUS: Error calling org.freedesktop.DBus.Properties.Get method: %s", error_msg);

        }

        device_type = g_value_get_uint(&g_device_type);
        g_value_unset(&g_device_type);

        DEBUG_PRINT("  Device Type: %d (%s)\n", device_type, (device_type == 1 ? "wired" : (device_type == 2 ? "wireless" : "unknown")));

        if (device_type == 2) {

            // wireless device

            memset(&adapter_info, 0, sizeof(adapter_info));

            int res = dbus_g_proxy_call(
                proxy_props,
                "Get",
                &error,
                G_TYPE_STRING,  "org.freedesktop.NetworkManager.Device",
                G_TYPE_STRING,  "Interface",
                G_TYPE_INVALID,
                G_TYPE_VALUE,   &g_interface,
                G_TYPE_INVALID
            );

            if (!res) {

                strcpy(error_msg, error->message);
                g_error_free(error);
                error = NULL;

                // cleanup
                g_ptr_array_free(g_device_list, TRUE);
                g_object_unref(proxy_props);
                g_free(path);

                throw wlanapi_exception("D-BUS: Error calling org.freedesktop.DBus.Properties.Get method: %s", error_msg);

            }

            int length = strlen(path);
            if (length > 0 && length < ADAPTER_NAME_LENGTH) {

                strncpy(adapter_info.name, path, ADAPTER_NAME_LENGTH - 1);
                strncpy(adapter_info.description, g_value_get_string(&g_interface), ADAPTER_DESCRIPTION_LENGTH - 1);
                g_value_unset(&g_interface);

                DEBUG_PRINT("  adapter_info.name: %s\n", adapter_info.name);
                DEBUG_PRINT("  adapter_info.description: %s\n", adapter_info.description);

                adapter_list.push_back(adapter_info);

                get_ap_list_func = &wlanapi::_dbus_get_ap_list;
                get_adapter_list_func = &wlanapi::_dbus_get_adapter_list;

            }

        }

        g_object_unref(proxy_props);
        g_free(path);

    }

    g_ptr_array_free(g_device_list, TRUE);

    DEBUG_LEAVE(wlanapi);

}

/**
 * Fetches list of access points using (K)NetworkManager over D-BUS
 *
 * This should work for most recent (as of 2008) Gnome and KDE desktop
 * systems. Tested under Ubuntu 7.x/8.x, Kubuntu 7.x/8.x and openSUSE 11.x
 *
 * @return void
 * @throws wlanapi_exception
 */
void wlanapi::_dbus_get_ap_list(ADAPTER_NAME *adapter_name) {

    DEBUG_ENTER(wlanapi);

    GError *error = NULL;
    char error_msg[256] = {0};
    DBusGProxy *proxy_props;
    DBusGProxy *proxy_nm_dw;
    GPtrArray *g_ap_list = NULL;

    GValue g_value = {0};
    guchar strength = 0;
    GArray *ssid = NULL;
    const gchar *hwaddress = NULL;

    AP_INFO ap_info;

    // create a proxy object for the remote interface "org.freedesktop.NetworkManager" on the system bus

    proxy_nm_dw = dbus_g_proxy_new_from_proxy(
        _dbus_nm_proxy,
        "org.freedesktop.NetworkManager.Device.Wireless",
        adapter_name
    );

    if (proxy_nm_dw != NULL) {
        DEBUG_PRINT("  Created proxy for NetworkManager.Device.Wireless\n");
    } else {
        throw wlanapi_exception("D-BUS: Failed to create proxy for NetworkManager");
    }

    // get list of access points

    int res = dbus_g_proxy_call(
        proxy_nm_dw,
        "GetAccessPoints",
        &error,
        G_TYPE_INVALID,
        dbus_g_type_get_collection("GPtrArray", DBUS_TYPE_G_OBJECT_PATH),   &g_ap_list,
        G_TYPE_INVALID
    );

    if (!res) {

        strcpy(error_msg, error->message);
        g_error_free(error);
        error = NULL;

        // cleanup
        g_object_unref(proxy_nm_dw);

        throw wlanapi_exception("D-BUS: Error calling org.freedesktop.NetworkManager.GetAccessPoints method: %s", error_msg);

    }

    DEBUG_PRINT("  Successfully called org.freedesktop.NetworkManager.GetAccessPoints method\n");

    // interate through list of access points

    for (uint i = 0; i < g_ap_list->len; i++) {

        gchar *path = (gchar*)g_ptr_array_index(g_ap_list, i);

        DEBUG_PRINT("\n  Access point UDI: %s\n", path);

        proxy_props = dbus_g_proxy_new_from_proxy(
            _dbus_nm_proxy,
            DBUS_INTERFACE_PROPERTIES,
            path
        );

        memset(&ap_info, 0, sizeof(ap_info));

        // retrieve signal strength

        error = NULL;
        int res = dbus_g_proxy_call(
            proxy_props,
            "Get",
            &error,
            G_TYPE_STRING,  "org.freedesktop.NetworkManager.AccessPoint",
            G_TYPE_STRING,  "Strength",
            G_TYPE_INVALID,
            G_TYPE_VALUE,   &g_value,
            G_TYPE_INVALID
        );

        if (!res) {

            strncpy(error_msg, error->message, 255);
            error_msg[255] = '\0';
            g_error_free(error);

            // cleanup
            g_ptr_array_free(g_ap_list, TRUE);
            g_object_unref(proxy_props);
            g_free(path);

            throw wlanapi_exception("D-BUS: Error calling org.freedesktop.DBus.Properties.Get method: %s", error_msg);

        }

        strength = g_value_get_uchar(&g_value);
        g_value_unset(&g_value);

        // Strength is expressed as a percentage and thus we have to convert
        // it to a RSSI value using linear interpolation.
        ap_info.rssi = -100 + strength / 2;

        DEBUG_PRINT("  Strength: %i\n", strength);

        // retrieve SSID

        error = NULL;
        res = dbus_g_proxy_call(
            proxy_props,
            "Get",
            &error,
            G_TYPE_STRING,  "org.freedesktop.NetworkManager.AccessPoint",
            G_TYPE_STRING,  "Ssid",
            G_TYPE_INVALID,
            G_TYPE_VALUE,   &g_value,
            G_TYPE_INVALID
        );

        if (!res) {

            strncpy(error_msg, error->message, 255);
            error_msg[255] = '\0';
            g_error_free(error);

            // cleanup
            g_ptr_array_free(g_ap_list, TRUE);
            g_object_unref(proxy_props);
            g_free(path);

            throw wlanapi_exception("D-BUS: Error calling org.freedesktop.DBus.Properties.Get method: %s", error_msg);

        }

        ssid = (GArray *)g_value_get_boxed(&g_value);

        int length = MAX(0, MIN(ssid->len, AP_NAME_LENGTH - 1));

        strncpy(ap_info.name, (AP_NAME *)ssid->data, length);
        ap_info.name[length] = '\0';
        g_value_unset(&g_value);

        DEBUG_PRINT("  Ssid: %s\n", ap_info.name);

        // retrieve mac address

        error = NULL;
        res = dbus_g_proxy_call(
            proxy_props,
            "Get",
            &error,
            G_TYPE_STRING,  "org.freedesktop.NetworkManager.AccessPoint",
            G_TYPE_STRING,  "HwAddress",
            G_TYPE_INVALID,
            G_TYPE_VALUE,   &g_value,
            G_TYPE_INVALID
        );

        if (!res) {

            strncpy(error_msg, error->message, 255);
            error_msg[255] = '\0';
            g_error_free(error);

            // cleanup
            g_ptr_array_free(g_ap_list, TRUE);
            g_object_unref(proxy_props);
            g_free(path);

            throw wlanapi_exception("D-BUS: Error calling org.freedesktop.DBus.Properties.Get method: %s", error_msg);

        }

        hwaddress = g_value_get_string(&g_value);
        DEBUG_PRINT("  Mac address1: %s\n", (char *)hwaddress);

        int int_list[6];

        sscanf(hwaddress, "%x:%x:%x:%x:%x:%x",
            &int_list[0],
            &int_list[1],
            &int_list[2],
            &int_list[3],
            &int_list[4],
            &int_list[5]
        );

        for (int i = 0; i < 6; ++i) {
            ap_info.mac_address.u[i] = int_list[i];
        }

        g_value_unset(&g_value);

        // push ap item to list

        ap_list.push_back(ap_info);

        g_object_unref(proxy_props);
        g_free(path);

    }

    g_ptr_array_free(g_ap_list, TRUE);

    DEBUG_LEAVE(wlanapi);

}

#endif // DBUS

#endif

