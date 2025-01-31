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
 * find_mice.cpp
 *
 *  Test suite program for C++ bindings
 */

#include <iostream>
#include <iomanip>
#include <usbpp.h>

#define VENDOR_LOGITECH 0x046D

using namespace std;

int main(void)
{

	USB::Busses buslist;
	USB::Device *device;
	list<USB::Device *> miceFound;
	list<USB::Device *>::const_iterator iter;

	cout << "idVendor/idProduct/bcdDevice" << endl;

	USB::DeviceIDList mouseList;

	mouseList.push_back(USB::DeviceID(VENDOR_LOGITECH, 0xC00E)); // Wheel Mouse Optical
	mouseList.push_back(USB::DeviceID(VENDOR_LOGITECH, 0xC012)); // MouseMan Dual Optical
	mouseList.push_back(USB::DeviceID(VENDOR_LOGITECH, 0xC00F)); // MouseMan Traveler
	mouseList.push_back(USB::DeviceID(VENDOR_LOGITECH, 0xC024)); // MX300 Optical 
	mouseList.push_back(USB::DeviceID(VENDOR_LOGITECH, 0xC025)); // MX500 Optical 
	mouseList.push_back(USB::DeviceID(VENDOR_LOGITECH, 0xC503)); // Logitech Dual receiver
	mouseList.push_back(USB::DeviceID(VENDOR_LOGITECH, 0xC506)); // MX700 Optical Mouse
	mouseList.push_back(USB::DeviceID(VENDOR_LOGITECH, 0xC031)); // iFeel Mouse (silver)


	miceFound = buslist.match(mouseList);

	for (iter = miceFound.begin(); iter != miceFound.end(); iter++) {
		device = *iter;

		cout << hex << setw(4) << setfill('0')
			 << device->idVendor() << "  /  "
			 << hex << setw(4) << setfill('0')
			 << device->idProduct() << "  /  "
			 << hex << setw(4) << setfill('0')
			 << device->idRevision() << "       "
			 << endl;
	}

	return 0;
}
