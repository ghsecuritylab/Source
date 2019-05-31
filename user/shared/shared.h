#ifndef __SHARED_H__
#define __SHARED_H__

#include "notify_rc.h"
#include "process.h"
#include "shutils.h"

#define _STR(n)	#n
#define STR(n)	_STR(n)

#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
#define IsExpressWayCli()	(re_expressway == 1 && n == 0) || (re_expressway == 2 && n == 1)
#define IsExpressWayApcli()	(re_expressway == 1 && n == 1) || (re_expressway == 2 && n == 0)
#endif

#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
#define IsMediaBridge()		re_mediabridge == 1
#endif

#endif
