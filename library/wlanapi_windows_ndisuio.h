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

#ifndef __WLANAPI_WINDOWS_NDISUIO_H_
#define __WLANAPI_WINDOWS_NDISUIO_H_

#define OID_802_11_BSSID 0x0D010101
#define OID_802_11_SSID 0x0D010102
#define OID_802_11_NETWORK_TYPES_SUPPORTED 0x0D010203
#define OID_802_11_NETWORK_TYPE_IN_USE 0x0D010204
#define OID_802_11_TX_POWER_LEVEL 0x0D010205
#define OID_802_11_RSSI 0x0D010206
#define OID_802_11_RSSI_TRIGGER 0x0D010207
#define OID_802_11_INFRASTRUCTURE_MODE 0x0D010108
#define OID_802_11_FRAGMENTATION_THRESHOLD 0x0D010209
#define OID_802_11_RTS_THRESHOLD 0x0D01020A
#define OID_802_11_NUMBER_OF_ANTENNAS 0x0D01020B
#define OID_802_11_RX_ANTENNA_SELECTED 0x0D01020C
#define OID_802_11_TX_ANTENNA_SELECTED 0x0D01020D
#define OID_802_11_SUPPORTED_RATES 0x0D01020E
#define OID_802_11_DESIRED_RATES 0x0D010210
#define OID_802_11_CONFIGURATION 0x0D010211
#define OID_802_11_STATISTICS 0x0D020212
#define OID_802_11_ADD_WEP 0x0D010113
#define OID_802_11_REMOVE_WEP 0x0D010114
#define OID_802_11_DISASSOCIATE 0x0D010115
#define OID_802_11_POWER_MODE 0x0D010216
#define OID_802_11_BSSID_LIST 0x0D010217
#define OID_802_11_AUTHENTICATION_MODE 0x0D010118
#define OID_802_11_PRIVACY_FILTER 0x0D010119
#define OID_802_11_BSSID_LIST_SCAN 0x0D01011A
#define OID_802_11_WEP_STATUS 0x0D01011B
#define OID_802_11_RELOAD_DEFAULTS 0x0D01011C

//#define IOCTL_NDISUIO_QUERY_OID_VALUE 0x120804

#define _NDIS_CONTROL_CODE(request, method) \
      CTL_CODE(FILE_DEVICE_PHYSICAL_NETCARD, request, method, FILE_ANY_ACCESS)

#define IOCTL_NDIS_QUERY_GLOBAL_STATS \
    _NDIS_CONTROL_CODE(0, METHOD_OUT_DIRECT)


#define FSCTL_NDISUIO_BASE      FILE_DEVICE_NETWORK

#define _NDISUIO_CTL_CODE(_Function, _Method, _Access) \
    CTL_CODE(FSCTL_NDISUIO_BASE, _Function, _Method, _Access)

// FILE_READ_ACCESS | FILE_WRITE_ACCESS vs. FILE_ANY_ACCESS

#define IOCTL_NDISUIO_QUERY_OID_VALUE \
    _NDISUIO_CTL_CODE(0x201, METHOD_BUFFERED, \
                      FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISUIO_SET_OID_VALUE \
    _NDISUIO_CTL_CODE(0x205, METHOD_BUFFERED, \
                      FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISUIO_QUERY_BINDING \
    _NDISUIO_CTL_CODE(0x203, METHOD_BUFFERED, \
                      FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISUIO_BIND_WAIT \
    _NDISUIO_CTL_CODE(0x204, METHOD_BUFFERED, \
                      FILE_READ_ACCESS | FILE_WRITE_ACCESS)


// http://msdn.microsoft.com/en-us/library/aa910075.aspx (NDIS_802_11_MAC_ADDRESS, Windows CE)
typedef UCHAR NDIS_802_11_MAC_ADDRESS[6];

// http://msdn.microsoft.com/en-us/library/aa931485.aspx (NDIS_802_11_SSID, Windows CE)
typedef struct _NDIS_802_11_SSID {
  ULONG SsidLength;
  UCHAR Ssid [32];
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;

// http://msdn.microsoft.com/en-us/library/aa929300.aspx (NDIS_802_11_RSSI, Windows CE)
typedef LONG NDIS_802_11_RSSI;

// http://msdn.microsoft.com/en-us/library/aa932076.aspx (NDIS_802_11_NETWORK_TYPE, Windows CE)
typedef enum _NDIS_802_11_NETWORK_TYPE {
  Ndis802_11FH,
  Ndis802_11DS,
  Ndis802_11NetworkTypeMax,
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;

// http://msdn.microsoft.com/en-us/library/aa447880.aspx (NDIS_802_11_CONFIGURATION_FH, Windows CE)
typedef struct _NDIS_802_11_CONFIGURATION_FH {
 ULONG Length;
 ULONG HopPattern;
 ULONG HopSet;
 ULONG DwellTime;
} NDIS_802_11_CONFIGURATION_FH, *PNDIS_802_11_CONFIGURATION_FH;

// http://msdn.microsoft.com/en-us/library/aa930570.aspx (NDIS_802_11_CONFIGURATION, Windows CE)
typedef struct _NDIS_802_11_CONFIGURATION
{
  ULONG  Length;
  ULONG  BeaconPeriod;
  ULONG  ATIMWindow;
  ULONG  DSConfig;
  NDIS_802_11_CONFIGURATION_FH  FHConfig;
} NDIS_802_11_CONFIGURATION, *PNDIS_802_11_CONFIGURATION;

// http://msdn.microsoft.com/en-us/library/aa931142.aspx (NDIS_802_11_NETWORK_INFRASTRUCTURE, Windows CE)
typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE {
  Ndis802_11IBSS,
  Ndis802_11Infrastructure,
  Ndis802_11AutoUnknown,
  Ndis802_11InfrastructureMax,
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;

// http://msdn.microsoft.com/en-us/library/ms799391.aspx (only described here -- no struct, Windows XP)
typedef UCHAR NDIS_802_11_RATES_EX[16];

// http://msdn.microsoft.com/en-us/library/ms799391.aspx (OID_802_11_BSSID_LIST, Windows XP)
typedef struct _NDIS_WLAN_BSSID_EX
{
  ULONG  Length;
  NDIS_802_11_MAC_ADDRESS  MacAddress;
  UCHAR  Reserved[2];
  NDIS_802_11_SSID  Ssid;
  ULONG  Privacy;
  NDIS_802_11_RSSI  Rssi;
  NDIS_802_11_NETWORK_TYPE  NetworkTypeInUse;
  NDIS_802_11_CONFIGURATION  Configuration;
  NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
  NDIS_802_11_RATES_EX  SupportedRates;
  ULONG  IELength;
  UCHAR  IEs[1];
} NDIS_WLAN_BSSID_EX, *PNDIS_WLAN_BSSID_EX;

// http://msdn.microsoft.com/en-us/library/ms799391.aspx (OID_802_11_BSSID_LIST, Windows XP)
typedef struct _NDIS_802_11_BSSID_LIST_EX
{
  ULONG  NumberOfItems;
  NDIS_WLAN_BSSID_EX  Bssid[1];
} NDIS_802_11_BSSID_LIST_EX, *PNDIS_802_11_BSSID_LIST_EX;


typedef ULONG NDIS_OID, *PNDIS_OID;

typedef struct _NDISUIO_QUERY_BINDING {
  ULONG BindingIndex;
  ULONG DeviceNameOffset;
  ULONG DeviceNameLength;
  ULONG DeviceDescrOffset;
  ULONG DeviceDescrLength;
} NDISUIO_QUERY_BINDING, *PNDISUIO_QUERY_BINDING;

typedef struct _NDISUIO_SET_OID {
  NDIS_OID Oid;
  #ifdef UNDER_CE
  PTCHAR ptcDeviceName;
  #endif
  UCHAR Data[sizeof(ULONG)];
} NDISUIO_SET_OID, *PNDISUIO_SET_OID;

#endif
