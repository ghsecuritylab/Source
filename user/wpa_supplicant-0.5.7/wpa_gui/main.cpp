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
#ifdef CONFIG_NATIVE_WINDOWS
#include <winsock.h>
#endif /* CONFIG_NATIVE_WINDOWS */
#include <qapplication.h>
#include "wpagui.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    WpaGui w;
    int ret;

#ifdef CONFIG_NATIVE_WINDOWS
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
	printf("Could not find a usable WinSock.dll\n");
	return -1;
    }
#endif /* CONFIG_NATIVE_WINDOWS */

    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    ret = a.exec();

#ifdef CONFIG_NATIVE_WINDOWS
    WSACleanup();
#endif /* CONFIG_NATIVE_WINDOWS */

    return ret;
}
