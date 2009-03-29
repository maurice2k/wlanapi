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

#ifndef __WLANAPI_WINDOWS_WZC_H_
#define __WLANAPI_WINDOWS_WZC_H_

#define INTF_DESCR      (0x00010000)
#define INTF_BSSIDLIST  (0x04000000)
#define INTF_LIST_SCAN  (0x08000000)
//#define INTF_BSSIDLIST 0xFFFFFFFF

// WZCEnumInterfaces

typedef struct {
  LPWSTR wszGuid;
} INTF_KEY_ENTRY, *PINTF_KEY_ENTRY;

typedef struct {
  DWORD dwNumIntfs;
  PINTF_KEY_ENTRY pIntfs;
} INTFS_KEY_TABLE, *PINTFS_KEY_TABLE;

typedef DWORD (WINAPI *WZCEnumInterfacesFunction)(LPWSTR pSrvAddr, PINTFS_KEY_TABLE pIntfs);

// WZCQueryInterface

typedef struct {
  DWORD   dwDataLen;
  LPBYTE  pData;
} RAW_DATA, *PRAW_DATA;

typedef struct {
  LPWSTR wszGuid;
  LPWSTR wszDescr;
  ULONG ulMediaState;
  ULONG ulMediaType;
  ULONG ulPhysicalMediaType;
  INT nInfraMode;
  INT nAuthMode;
  INT nWepStatus;
  ULONG padding1[2];  // 16 chars on Windows XP SP3 or SP2 with WLAN Hotfix installed, 8 chars otherwise
  DWORD dwCtlFlags;
  DWORD dwCapabilities;
  RAW_DATA rdSSID;
  RAW_DATA rdBSSID;
  RAW_DATA rdBSSIDList;
  RAW_DATA rdStSSIDList;
  RAW_DATA rdCtrlData;
  BOOL bInitialized;
  ULONG padding2[64];  // for security reason ...
} INTF_ENTRY, *PINTF_ENTRY;

typedef DWORD (WINAPI *WZCQueryInterfaceFunction)(LPWSTR pSrvAddr, DWORD dwInFlags, PINTF_ENTRY pIntf, LPDWORD pdwOutFlags);


// This structure is not quite the same as the WinCE equivalent.
typedef struct _NDIS_WLAN_BSSID {
  UCHAR padding1[4];
  ULONG Length;
  UCHAR padding2[4];
  NDIS_802_11_MAC_ADDRESS MacAddress;
  UCHAR Reserved[2];
  NDIS_802_11_SSID Ssid;
  ULONG Privacy;
  NDIS_802_11_RSSI Rssi;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;

typedef struct _NDIS_802_11_BSSID_LIST {
  ULONG NumberOfItems;
  NDIS_WLAN_BSSID Bssid[1];
} NDIS_802_11_BSSID_LIST, *PNDIS_802_11_BSSID_LIST;



typedef DWORD (WINAPI *WZCRefreshInterfaceFunction)(
    LPWSTR pSrvAddr,
    DWORD dwInFlags,
    PINTF_ENTRY pIntf,
    LPDWORD pdwOutFlags
);

#endif
