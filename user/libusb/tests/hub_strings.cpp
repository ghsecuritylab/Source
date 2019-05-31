/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
// -*- C++;indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-
/*
 * hub_strings.cpp
 *
 *  Test suite program for C++ bindings
 */

#include <iostream>
#include <iomanip>
#include <usbpp.h>
#include <errno.h>

using namespace std;

int main(void)
{
	USB::Busses buslist;
	USB::Device *device;
	list<USB::Device *> hubList;
	list<USB::Device *>::const_iterator iter;
	string manufString, prodString, serialString1, serialString2;
	int retval;

	cout << "Class/SubClass/Protocol" << endl;

	hubList = buslist.match(0x9);

	for (iter = hubList.begin(); iter != hubList.end(); iter++) {
		device = *iter;

		cout << hex << setw(2) << setfill('0')
			 << int(device->devClass()) << "      " 
			 << hex << setw(2) << setfill('0')
			 << int(device->devSubClass()) << "      "
			 << hex << setw(2) << setfill('0')
			 << int(device->devProtocol()) << endl;
		retval = device->string(manufString, 3);
		if ( 0 < retval ) {
			cout << "3: " << manufString << endl;
		} else {
			if (-EPIPE != retval) { // we ignore EPIPE, because some hubs don't have strings
				cout << "fetching string 3 failed: " << usb_strerror() << endl;
				return EXIT_FAILURE;
			}
		}
		retval = device->string(prodString, 2);
		if ( 0 < retval ) {
			cout << "2: " << prodString << endl;
		} else {
			if (-EPIPE != retval) { // we ignore EPIPE, because some hubs don't have strings
				cout << "fetching string 2 failed: " << usb_strerror() << endl;
				return EXIT_FAILURE;
			}
		}

		retval = device->string(serialString1, 1);
		if ( 0 < retval ) {
			cout << "1a: " << serialString1 << endl;
		} else {
			if (-EPIPE != retval) { // we ignore EPIPE, because some hubs don't have strings
				cout << "fetching string 1a failed: " << usb_strerror() << endl;
				return EXIT_FAILURE;
			}
		}

		retval = device->string(serialString2, 1, 0x0409);
		if ( 0 < retval ) {
			cout << "1b: " << serialString2 << endl;
			if (serialString2 != serialString1) {
				cout << "String fetch with explicit language ID produced different result" << endl;
			}
		} else {
			if (-EPIPE != retval) { // we ignore EPIPE, because some hubs don't have strings
				cout << "fetching string 1b failed: " << usb_strerror() << endl;
				return EXIT_FAILURE;
			}
		}

		cout << endl;
	}

	return EXIT_SUCCESS;
}
