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

#ifndef WATCHDOG_H
#define WATCHDOG_H

#if (defined(CONFIG_DEFAULTS_ASUS_EAN66))

#define RESET_BTN_GPIO_IRQ	25
#ifdef WPS_BTN_SUPPORT
#define WPS_BTN_GPIO_IRQ	26
#endif

#elif (defined(CONFIG_DEFAULTS_ASUS_RPN12))

#define RESET_BTN_GPIO_IRQ	42
#ifdef WPS_BTN_SUPPORT
#define WPS_BTN_GPIO_IRQ	38
#endif

#elif (defined(CONFIG_DEFAULTS_ASUS_RPN14) || defined(CONFIG_DEFAULTS_ASUS_RPN53) || defined(CONFIG_DEFAULTS_ASUS_RPAC52))

#define TOUCH_SENSOR_GPIO_IRQ	24
#ifdef WPS_BTN_SUPPORT
#define WPS_BTN_GPIO_IRQ	25
#endif
#define RESET_BTN_GPIO_IRQ	26
#define AUDIO_JACK_GPIO_IRQ	27

#elif (defined(CONFIG_DEFAULTS_ASUS_RPAC56))
//TBD.
#define RESET_BTN_GPIO_IRQ	21
#ifdef WPS_BTN_SUPPORT
#define WPS_BTN_GPIO_IRQ	18 // 63? TBD...
#endif

#elif (defined(CONFIG_DEFAULTS_ASUS_WMPN12))

#define RESET_BTN_GPIO_IRQ	26
#define VOL_KNOB_A_GPIO_IRQ	41
#define VOL_KNOB_B_GPIO_IRQ	40
#define VOL_KNOB_M_GPIO_IRQ	42
#ifdef WPS_BTN_SUPPORT
#define WPS_BTN_GPIO_IRQ	VOL_KNOB_M_GPIO_IRQ
#endif

#else
#error Invalid Product!!
#endif

#endif
