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

#ifndef __WLANAPI_EXCEPTION_H_
#define __WLANAPI_EXCEPTION_H_

#include <stdexcept>
#include <stdarg.h>


class wlanapi_exception : public std::runtime_error {
    public:
        wlanapi_exception(const char* message, ...);
        virtual const char * what() const throw();
    protected:
        char msg[512];
    private:
};

#endif // __WLANAPI_EXCEPTION_H_
