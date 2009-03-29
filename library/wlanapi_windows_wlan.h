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
 *
 *
 * The following information has been taken from MSDN.
 */

#ifndef __WLANAPI_WINDOWS_WLAN_H_
#define __WLANAPI_WINDOWS_WLAN_H_

typedef enum _WLAN_INTERFACE_STATE {
  wlan_interface_state_not_ready               = 0,
  wlan_interface_state_connected               = 1,
  wlan_interface_state_ad_hoc_network_formed   = 2,
  wlan_interface_state_disconnecting           = 3,
  wlan_interface_state_disconnected            = 4,
  wlan_interface_state_associating             = 5,
  wlan_interface_state_discovering             = 6,
  wlan_interface_state_authenticating          = 7
} WLAN_INTERFACE_STATE, *PWLAN_INTERFACE_STATE;

typedef struct _WLAN_INTERFACE_INFO {
  GUID InterfaceGuid;
  WCHAR strInterfaceDescription[256];
  WLAN_INTERFACE_STATE isState;
} WLAN_INTERFACE_INFO, *PWLAN_INTERFACE_INFO;

typedef struct _WLAN_INTERFACE_INFO_LIST {
  DWORD dwNumberOfItems;
  DWORD dwIndex;
  WLAN_INTERFACE_INFO InterfaceInfo[];
} WLAN_INTERFACE_INFO_LIST, *PWLAN_INTERFACE_INFO_LIST;

typedef UCHAR DOT11_MAC_ADDRESS[6];

typedef DOT11_MAC_ADDRESS* PDOT11_MAC_ADDRESS;

typedef enum _DOT11_BSS_TYPE {
  dot11_BSS_type_infrastructure   = 1,
  dot11_BSS_type_independent      = 2,
  dot11_BSS_type_any              = 3
} DOT11_BSS_TYPE, *PDOT11_BSS_TYPE;


#define DOT11_SSID_MAX_LENGTH 32

typedef struct _DOT11_SSID {
  ULONG uSSIDLength;
  UCHAR ucSSID[DOT11_SSID_MAX_LENGTH];
} DOT11_SSID, *PDOT11_SSID;

typedef enum _DOT11_PHY_TYPE {
  dot11_phy_type_unknown,
  dot11_phy_type_any,
  dot11_phy_type_fhss,
  dot11_phy_type_dsss,
  dot11_phy_type_irbaseband,
  dot11_phy_type_ofdm,
  dot11_phy_type_hrdsss,
  dot11_phy_type_erp,
  dot11_phy_type_ht,
  dot11_phy_type_IHV_start,
  dot11_phy_type_IHV_end
} DOT11_PHY_TYPE, *PDOT11_PHY_TYPE;

#define DOT11_RATE_SET_MAX_LENGTH 126

typedef struct _WLAN_RATE_SET {
  ULONG uRateSetLength;
  USHORT usRateSet[DOT11_RATE_SET_MAX_LENGTH];
} WLAN_RATE_SET, *PWLAN_RATE_SET;

typedef struct _WLAN_BSS_ENTRY {
  DOT11_SSID dot11Ssid;
  ULONG uPhyId;
  DOT11_MAC_ADDRESS dot11Bssid;
  DOT11_BSS_TYPE dot11BssType;
  DOT11_PHY_TYPE dot11BssPhyType;
  LONG lRssi;
  ULONG uLinkQuality;
  BOOLEAN bInRegDomain;
  USHORT usBeaconPeriod;
  ULONGLONG ullTimestamp;
  ULONGLONG ullHostTimestamp;
  USHORT usCapabilityInformation;
  ULONG ulChCenterFrequency;
  WLAN_RATE_SET wlanRateSet;
  ULONG ulIeOffset;
  ULONG ulIeSize;
} WLAN_BSS_ENTRY, *PWLAN_BSS_ENTRY;

 typedef struct _WLAN_BSS_LIST {
  DWORD dwTotalSize;
  DWORD dwNumberOfItems;
  WLAN_BSS_ENTRY wlanBssEntries[1];
} WLAN_BSS_LIST, *PWLAN_BSS_LIST;


/******************/

typedef DWORD WLAN_REASON_CODE, *PWLAN_REASON_CODE;

#define WLAN_MAX_PHY_TYPE_NUMBER 8

typedef ULONG WLAN_SIGNAL_QUALITY;

typedef enum DOT11_AUTH_ALGORITHM {
  DOT11_AUTH_ALGO_80211_OPEN,
  DOT11_AUTH_ALGO_80211_SHARED_KEY,
  DOT11_AUTH_ALGO_WPA,
  DOT11_AUTH_ALGO_WPA_PSK,
  DOT11_AUTH_ALGO_RSNA,
  DOT11_AUTH_ALGO_RSNA_PSK,
  DOT11_AUTH_ALGO_IHV_START,
  DOT11_AUTH_ALGO_IHV_END
} DOT11_AUTH_ALGORITHM, *PDOT11_AUTH_ALGORITHM;

typedef enum DOT11_CIPHER_ALGORITHM {
  DOT11_CIPHER_ALGO_NONE,
  DOT11_CIPHER_ALGO_WEP40,
  DOT11_CIPHER_ALGO_TKIP,
  DOT11_CIPHER_ALGO_CCMP,
  DOT11_CIPHER_ALGO_WEP104,
  DOT11_CIPHER_ALGO_WEP,
  DOT11_CIPHER_ALGO_IHV_START,
  DOT11_CIPHER_ALGO_IHV_END
} DOT11_CIPHER_ALGORITHM, *PDOT11_CIPHER_ALGORITHM;

typedef struct _WLAN_AVAILABLE_NETWORK {
  WCHAR strProfileName[256];
  DOT11_SSID dot11Ssid;
  DOT11_BSS_TYPE dot11BssType;
  ULONG uNumberOfBssids;
  BOOL bNetworkConnectable;
  WLAN_REASON_CODE wlanNotConnectableReason;
  ULONG uNumberOfPhyTypes;
  DOT11_PHY_TYPE dot11PhyTypes[WLAN_MAX_PHY_TYPE_NUMBER];
  BOOL bMorePhyTypes;
  WLAN_SIGNAL_QUALITY wlanSignalQuality;
  BOOL bSecurityEnabled;
  DOT11_AUTH_ALGORITHM dot11DefaultAuthAlgorithm;
  DOT11_CIPHER_ALGORITHM dot11DefaultCipherAlgorithm;
  DWORD dwFlags;
  DWORD dwReserved;
} WLAN_AVAILABLE_NETWORK, *PWLAN_AVAILABLE_NETWORK;

typedef struct _WLAN_AVAILABLE_NETWORK_LIST {
  DWORD dwNumberOfItems;
  DWORD dwIndex;
  WLAN_AVAILABLE_NETWORK Network[1];
} WLAN_AVAILABLE_NETWORK_LIST, *PWLAN_AVAILABLE_NETWORK_LIST;

typedef DWORD (WINAPI *WlanGetAvailableNetworkListFunction)(
    HANDLE hClientHandle,
    const GUID *pInterfaceGuid,
    DWORD dwFlags,
    PVOID pReserved,
    PWLAN_AVAILABLE_NETWORK_LIST *ppAvailableNetworkList
);


/******************/

// Native Wifi Functions

typedef DWORD (WINAPI *WlanOpenHandleFunction)(
    DWORD dwClientVersion,
    PVOID pReserved,
    PDWORD pdwNegotiatedVersion,
    PHANDLE phClientHandle
);

typedef DWORD (WINAPI *WlanEnumInterfacesFunction)(
    HANDLE hClientHandle,
    PVOID pReserved,
    PWLAN_INTERFACE_INFO_LIST *ppInterfaceList
);

typedef DWORD (WINAPI *WlanGetNetworkBssListFunction)(
    HANDLE hClientHandle,
    const GUID *pInterfaceGuid,
    const  PDOT11_SSID pDot11Ssid,
    DOT11_BSS_TYPE dot11BssType,
    BOOL bSecurityEnabled,
    PVOID pReserved,
    PWLAN_BSS_LIST *ppWlanBssList
);

typedef DWORD (WINAPI *WlanCloseHandleFunction)(
    HANDLE hClientHandle,
    PVOID pReserved
);

typedef VOID (WINAPI *WlanFreeMemoryFunction)(
  PVOID pMemory
);

#endif
