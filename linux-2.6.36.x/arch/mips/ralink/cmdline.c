/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     cmdline parsing for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */

#include <linux/init.h>
#include <linux/string.h>

#include <asm/bootinfo.h>

#if CONFIG_RALINK_BAUDRATE == 57600
#define BAUDRATE "57600"
#elif CONFIG_RALINK_BAUDRATE == 115200
#define BAUDRATE "115200"
#else
#error Invalid baudrate
#endif

#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
#ifdef CONFIG_SYSFS
char rt2880_cmdline[]="console=ttyS1,"BAUDRATE"n8 root=/dev/mtdblock5";
#else
char rt2880_cmdline[]="console=ttyS1,"BAUDRATE"n8 root=1f05";
#endif
#elif defined (CONFIG_RT2880_ROOTFS_IN_RAM)
char rt2880_cmdline[]="console=ttyS1,"BAUDRATE"n8 root=/dev/ram0";
#else
#error "RT2880 Root File System not defined"
#endif

extern int prom_argc;
extern int *_prom_argv;

/*
 * YAMON (32-bit PROM) pass arguments and environment as 32-bit pointer.
 * This macro take care of sign extension.
 */
#define prom_argv(index) ((char *)(((int *)(int)_prom_argv)[(index)]))

extern char arcs_cmdline[COMMAND_LINE_SIZE];

char * __init prom_getcmdline(void)
{
	return &(arcs_cmdline[0]);
}

void  __init prom_init_cmdline(void)
{
	char *cp;
#ifdef CONFIG_UBOOT_CMDLINE
	int actr=1; /* Always ignore argv[0] */
#endif

	cp = &(arcs_cmdline[0]);

#ifdef CONFIG_UBOOT_CMDLINE
	while(actr < prom_argc) {
	    strcpy(cp, prom_argv(actr));
	    cp += strlen(prom_argv(actr));
	    *cp++ = ' ';
	    actr++;
	}
#else
	strcpy(cp, rt2880_cmdline);
	cp += strlen(rt2880_cmdline);
	*cp++ = ' ';
#endif

	if (cp != &(arcs_cmdline[0])) /* get rid of trailing space */
	    --cp;
	*cp = '\0';
	
}
