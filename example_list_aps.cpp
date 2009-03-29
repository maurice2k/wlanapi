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

#include <vector>
#include <string>
#include <stdexcept>

#include "library/wlanapi.h"

using namespace std;

int main() {

	wlanapi *wi = new wlanapi();

    try {

        const ADAPTER_LIST &adapter_list = wi->get_adapter_list();
        const AP_LIST &ap_list = wi->get_ap_list(NULL);

        for (AP_LIST::const_iterator it = ap_list.begin(); it < ap_list.end(); ++it) {

            printf("ap name:         %s\n", it->name);
            printf("ap mac address:  %02x-%02x-%02x-%02x-%02x-%02x\n", it->mac_address.u[0],
                                                                       it->mac_address.u[1],
                                                                       it->mac_address.u[2],
                                                                       it->mac_address.u[3],
                                                                       it->mac_address.u[4],
                                                                       it->mac_address.u[5]);
            printf("ap rssi:         %i\n", it->rssi);
            printf("\n");

        }

    } catch (std::exception &e) {
        printf("[X] %s\n", e.what());
    }

    printf("Press enter to exit.");
    scanf("...");

    delete wi;

}
