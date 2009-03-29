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

#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <string.h>

#ifdef DEBUG
    #define DEBUG_PRINT(...) printf(__VA_ARGS__);
    #define DEBUG_WPRINT(...) wprintf(__VA_ARGS__);
    #define DEBUG_ENTER(class_name) if (strlen(#class_name) > 0) printf("[>] %s::%s()\n", #class_name, __FUNCTION__); else printf("[>] %s()\n", __FUNCTION__);
    #define DEBUG_LEAVE(class_name) if (strlen(#class_name) > 0) printf("[<] %s::%s()\n\n", #class_name, __FUNCTION__); else printf("[<] %s()\n\n", __FUNCTION__);
    #define DEBUG_ENTER_PRETTY() printf("[>] %s\n", __PRETTY_FUNCTION__);
    #define DEBUG_LEAVE_PRETTY() printf("[<] %s\n\n", __PRETTY_FUNCTION__);
#else
    #define DEBUG_PRINT(...)
    #define DEBUG_WPRINT(...)
    #define DEBUG_ENTER(class_name)
    #define DEBUG_LEAVE(class_name)
    #define DEBUG_ENTER_PRETTY()
    #define DEBUG_LEAVE_PRETTY()
#endif

#endif
