/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
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
 ***************************************************************************
 */

#ifndef __RALINK_MT7628_ESW_H__
#define __RALINK_MT7628_ESW_H__

#define REG_ESW_PFC1			0x14
#define REG_ESW_PVIDC0			0x40
#define REG_ESW_PVIDC1			0x44
#define REG_ESW_PVIDC2			0x48
#define REG_ESW_PVIDC3			0x4c
#define REG_ESW_POA			0x80
#define REG_ESW_POC2			0x98
#define REG_ESW_SGC2			0xe4

#include "../../../autoconf.h"
#if (defined(CONFIG_DEFAULTS_ASUS_RPN12))
#define ESW_WAN_PORT 0
#else
#error Invalid Product!!
#endif

int esw_fd;

int switch_init(void);
void switch_fini(void);
int esw_reg_read(int offset, unsigned int *value);
int esw_reg_write(int offset, int value);

int GetPhyStatus(int port, int print_format);
int GetLinkStatus(int port);

#ifdef VLAN
void config_esw(int type);
void restore_esw(void);

// port0 ~ port4
enum partition_type
{
	WLLLL,
	LWLLL,
	LLWLL,
	LLLWL,
	LLLLW
};
#endif

#endif
