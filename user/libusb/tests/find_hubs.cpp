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
 * find_hubs.cpp
 *
 *  Test suite program for C++ bindings
 */

#include <iostream>
#include <iomanip>
#include <usbpp.h>

using namespace std;

int main(void)
{

	USB::Busses buslist;
	USB::Device *device;
	list<USB::Device *> hubList;
	list<USB::Device *>::const_iterator iter;

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
	}

	return 0;
}
