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
 * descriptor_test.cpp
 *
 *  Test suite program for C++ bindings
 */

#include <iostream>
#include <iomanip>
#include "usbpp.h"

using namespace std;

int main(void)
{

	USB::Busses buslist; 

	cout << "bus/device  idVendor/idProduct" << endl;

	//  buslist.init();

	USB::Bus *bus;
	list<USB::Bus *>::const_iterator biter;
	USB::Device *device;
	list<USB::Device *>::const_iterator diter;
	int i, j, k, l;

	for (biter = buslist.begin(); biter != buslist.end(); biter++) {
		bus = *biter;

		for (diter = bus->begin(); diter != bus->end(); diter++) {
			device = *diter;

			cout << bus->directoryName() << "/" 
				 << device->fileName() << "     "
				 << ios::uppercase << hex << setw(4) << setfill('0')
				 << device->idVendor() << "/"
				 << ios::uppercase << hex << setw(4) << setfill('0')
				 << device->idProduct() << endl;
			if (device->Vendor() != "") {
				cout << "- Manufacturer : " << device->Vendor() << endl;
			} else {
				cout << "- Unable to fetch manufacturer string" << endl;
			}
			if (device->Product() != "") {
				cout << "- Product      : " << device->Product() << endl;
			} else {
				cout << "- Unable to fetch product string" << endl;
			}
			if (device->SerialNumber() != "") {
				cout << "- Serial Number: " << device->SerialNumber() << endl;
			}

			USB::Configuration *this_Configuration;
			this_Configuration = device->firstConfiguration();
			for (i=0; i < device->numConfigurations(); i++) {
				this_Configuration->dumpDescriptor();
				USB::Interface *this_Interface;
				this_Interface = this_Configuration->firstInterface();
				for (j=0; j < this_Configuration->numInterfaces(); j++) {
					USB::AltSetting *this_AltSetting;
					this_AltSetting = this_Interface->firstAltSetting();
					for (k=0; k < this_Interface->numAltSettings(); k++) {
						this_AltSetting->dumpDescriptor();
						USB::Endpoint *this_Endpoint;
						this_Endpoint = this_AltSetting->firstEndpoint();
						for (l=0; l < this_AltSetting->numEndpoints(); l++) {
							this_Endpoint->dumpDescriptor();
							this_Endpoint = this_AltSetting->nextEndpoint();
						}
						this_AltSetting = this_Interface->nextAltSetting();
					}
					this_Interface = this_Configuration->nextInterface();
				}
				this_Configuration = device->nextConfiguration();
			}
		}
	}

	return 0;
}
