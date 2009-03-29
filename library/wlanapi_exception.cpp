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

#include "wlanapi_exception.h"

wlanapi_exception::wlanapi_exception(const char* message, ...) : std::runtime_error::runtime_error(message) {

    va_list arguments;

    va_start(arguments, message);
    vsnprintf(msg, sizeof(msg), message, arguments);
    va_end(arguments);

}

const char *wlanapi_exception::what() const throw() {
    return msg;
}
