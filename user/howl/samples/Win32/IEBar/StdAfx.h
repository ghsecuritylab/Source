#if !defined(AFX_STDAFX_H__875FEB65_F910_40E1_B98E_1912C72E62D6__INCLUDED_)
#define AFX_STDAFX_H__875FEB65_F910_40E1_B98E_1912C72E62D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define WINVER 0x0400	// <-- necessary for NT4 compatibility

#define _ATL_APARTMENT_THREADED

#define _WTL_FORWARD_DECLARE_CSTRING

#include <atlbase.h>
#include <atlapp.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlcrack.h>

#define HR(_ex) { HRESULT _hr = _ex; if (FAILED(_hr)) return _hr; }
#define LENGTHOF(a) (sizeof(a) == sizeof(&*a)) ? 0 : (sizeof(a) / sizeof(*a))


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__875FEB65_F910_40E1_B98E_1912C72E62D6__INCLUDED)
