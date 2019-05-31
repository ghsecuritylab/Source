/*
 * Copyright 2003, 2004 Porchdog Software. All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without modification,
 *	are permitted provided that the following conditions are met:
 *
 *		1. Redistributions of source code must retain the above copyright notice,
 *		   this list of conditions and the following disclaimer.   
 *		2. Redistributions in binary form must reproduce the above copyright notice,
 *		   this list of conditions and the following disclaimer in the documentation
 *		   and/or other materials provided with the distribution.
 *
 *	THIS SOFTWARE IS PROVIDED BY PORCHDOG SOFTWARE ``AS IS'' AND ANY
 *	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *	IN NO EVENT SHALL THE HOWL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *	OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *	OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	The views and conclusions contained in the software and documentation are those
 *	of the authors and should not be interpreted as representing official policies,
 *	either expressed or implied, of Porchdog Software.
 */
/*
 * This code is based on Source Tree 
 * Written by Tim Tabor
 * Copyright(c) 2001 Tim Tabor
 */

#include "stdafx.h"
#include "IHowlExplorerBar.h"
#include <MsHTML.h>
#include <UrlMon.h>
#include <WinInet.h>
#include <stdio.h>
#include "CHowlExplorerBar.h"
#include "About.h"
#include "FTP.h"
#include <salt/debug.h>
#include <discovery/discovery.h>
#include <discovery/text_record.h>

#define DISCOVERY_DEFAULT_DOMAIN	"local."
#define DISCOVERY_HTTP_SERVICE	"_http._tcp."
#define DISCOVERY_FTP_SERVICE		"_ftp._tcp."

#pragma comment(lib, "urlmon.lib")

CHowlExplorerBar::CHowlExplorerBar()
:
	m_bHasFocus(false),
	m_browsing(SW_FALSE),
	m_resolving(SW_FALSE),
	m_discovery_error_id(0),
	m_http_tree_id(0),
	m_ftp_tree_id(0),
	m_services(NULL)
{
	ZeroMemory(&m_dbi, sizeof m_dbi);

	if ((m_discovery_status = sw_discovery_init(&m_discovery)) == SW_OKAY)
	{
		enum { WHITE = RGB(0,0,0) };
		m_image_list.Create(16, 16, ILC_COLORDDB, 7, 0);
		CBitmap bitmap;
		bitmap.LoadBitmap(IDB_LOGO);
		m_image_list.Add(bitmap);
	}

	load();
}


CHowlExplorerBar::~CHowlExplorerBar()
{
	unload();

	if (IsWindow())
	{
		if (m_http_tree_id != NULL)
		{
			DeleteItem(m_http_tree_id);
		}
	
		if (m_ftp_tree_id != NULL)
		{
			DeleteItem(m_ftp_tree_id);
		}

		if (m_discovery_error_id != NULL)
		{
			DeleteItem(m_discovery_error_id);
		}
	}

	sw_discovery_fina(m_discovery);
}


HRESULT WINAPI CHowlExplorerBar::UpdateRegistry(BOOL bRegister)
{
	// Bug workaround.  See Q247705.
	RegDeleteKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Discardable\\PostSetup\\Component Categories\\{00021494-0000-0000-C000-000000000046}\\Enum"));

	return _Module.UpdateRegistryFromResource(IDR_HOWL_EXPLORER_BAR, bRegister);
}

STDMETHODIMP CHowlExplorerBar::GetClassID(LPCLSID pClassID)
{
	ATLTRACE(_T("CHowlExplorerBar::GetClassID\n"));

	ATLASSERT(pClassID);
	if (!pClassID) return E_POINTER;

	*pClassID = GetObjectCLSID();
	return S_OK;
}

STDMETHODIMP CHowlExplorerBar::IsDirty()
{
	ATLTRACE(_T("CHowlExplorerBar::IsDirty\n"));

	return S_FALSE;
}

STDMETHODIMP CHowlExplorerBar::Load(LPSTREAM pStream)
{
	ATLTRACE(_T("CHowlExplorerBar::Load\n"));

	return S_OK;
}

STDMETHODIMP CHowlExplorerBar::Save(LPSTREAM pStream, BOOL fClearDirty)
{
	ATLTRACE(_T("CHowlExplorerBar::Save\n"));

	return S_OK;
}

STDMETHODIMP CHowlExplorerBar::GetSizeMax(ULARGE_INTEGER *pul)
{
	ATLTRACE(_T("CHowlExplorerBar::GetSizeMax\n"));

	if (pul == 0) return E_POINTER;
	ZeroMemory(pul, sizeof *pul);

	return E_NOTIMPL;
}

STDMETHODIMP CHowlExplorerBar::SetSite(IUnknown* pUnkSite)
{
	ATLTRACE("CHowlExplorerBar::SetSite\n");

	AdviseWebBrowser(false);
	m_spInputObjectSite.Release();
	if (m_spWB2 != 0) m_spWB2.Release();

	if (!pUnkSite) return S_OK;

	HR(pUnkSite->QueryInterface(IID_IInputObjectSite, (void**) &m_spInputObjectSite));

	CComQIPtr<IOleWindow> spOleWindow(pUnkSite);
	if (!spOleWindow) return E_FAIL;	// don't rely on hr

	spOleWindow->GetWindow(&m_wndSite.m_hWnd);
	if (!m_wndSite.IsWindow()) return E_FAIL;

	Create(m_wndSite, rcDefault);	// will be resized
	if (!IsWindow()) return E_FAIL;

	CComPtr<IServiceProvider> spSP;
	pUnkSite->QueryInterface(&spSP);
	if (spSP != 0)
	{
		CComPtr<IServiceProvider> spSP2;
		spSP->QueryService(SID_STopLevelBrowser, &spSP2);
		if (spSP2 != 0)
		{
			spSP2->QueryService(SID_SWebBrowserApp, &m_spWB2);
			if (m_spWB2 != 0)
			{
				AdviseWebBrowser(true);
				/* DisplayDocumentTree(); */
			}
		}
	}


	return S_OK;
}

STDMETHODIMP CHowlExplorerBar::GetSite(REFIID riid, LPVOID* ppvReturn)
{
	ATLTRACE(_T("CHowlExplorerBar::GetSite\n"));

	if (ppvReturn == 0) return E_POINTER;

	*ppvReturn = 0;

	ATLASSERT(m_spInputObjectSite);
	if (m_spInputObjectSite == 0) return E_FAIL;

	m_spInputObjectSite->QueryInterface(riid, ppvReturn);
	ATLASSERT(*ppvReturn);
	if (*ppvReturn == 0) return E_FAIL;

	return S_OK;
}

STDMETHODIMP CHowlExplorerBar::GetWindow(HWND *phWnd)
{
	ATLTRACE(_T("CHowlExplorerBar::GetWindow\n"));

	if (!phWnd) return E_POINTER;
	*phWnd = 0;

	ATLASSERT(IsWindow());
	if (!IsWindow()) return E_FAIL;

	*phWnd = m_hWnd;
	return S_OK;
}


STDMETHODIMP CHowlExplorerBar::ContextSensitiveHelp(BOOL fEnterMode)
{
	ATLTRACE(_T("CHowlExplorerBar::ContextSensitiveHelp\n"));

	return E_NOTIMPL;
}

//
// Sink events only when we're visible
//
STDMETHODIMP CHowlExplorerBar::ShowDW(BOOL fShow)
{
	ATLTRACE(_T("CHowlExplorerBar::ShowDW\n"));

	ATLASSERT(IsWindow());
	if (!IsWindow()) return E_FAIL;

	SetImageList((HIMAGELIST) m_image_list, TVSIL_NORMAL);

	if (m_discovery_status == SW_OKAY)
	{
		if (m_http_tree_id == 0)
		{
			m_http_tree_id	= InsertItem(L"Websites Near Me", 0, 0, NULL, TVI_LAST);
		}

		if (m_ftp_tree_id == 0)
		{
			m_ftp_tree_id	= InsertItem(L"FTP Sites Near Me", 0, 0, NULL, TVI_LAST);
		}
	}
	else if (m_discovery_error_id == 0)
	{
		m_discovery_error_id = InsertItem(L"Howl Service Not Available", TVI_ROOT, TVI_LAST);
		
	}

	if (fShow)
	{
		ShowWindow(SW_SHOW);
		AdviseWebBrowser(true);
	}
	else if (!fShow)
	{
		ShowWindow(SW_HIDE);
		AdviseWebBrowser(false);
	}

	return S_OK;
}

STDMETHODIMP CHowlExplorerBar::CloseDW(DWORD dwReserved)
{
	ATLTRACE(_T("CHowlExplorerBar::CloseDW\n"));

	ATLASSERT(IsWindow());
	if (!IsWindow()) return E_FAIL;

	ShowDW(FALSE);
	DestroyWindow();

	return S_OK;
}

// never called for band objects
STDMETHODIMP CHowlExplorerBar::ResizeBorderDW(LPCRECT, IUnknown*, BOOL)
{
	ATLTRACE(_T("CHowlExplorerBar::ResizeBorderDW\n"));
	return E_NOTIMPL;
}

STDMETHODIMP CHowlExplorerBar::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi)
{
	ATLTRACE(_T("CHowlExplorerBar::GetBandInfo\n"));
	if (pdbi == 0) return E_INVALIDARG;

	if (pdbi->dwMask & DBIM_MINSIZE)
	{
		pdbi->ptMinSize.x = MINCX;
		pdbi->ptMinSize.y = MINCY;
	}
	if (pdbi->dwMask & DBIM_MAXSIZE)
	{
		pdbi->ptMaxSize.x = -1;
		pdbi->ptMaxSize.y = -1;
	}
	if (pdbi->dwMask & DBIM_INTEGRAL)
	{
		pdbi->ptIntegral.x = 1;
		pdbi->ptIntegral.y = 1;
	}
	if (pdbi->dwMask & DBIM_ACTUAL)
	{
		//!  what do other folks do?
		CWindow wndFrame = GetTopLevelWindow();
		CRect r;
		wndFrame.GetClientRect(&r);
		pdbi->ptActual.x = 0;
		pdbi->ptActual.y = r.Height() / 3;
	}
	if (pdbi->dwMask & DBIM_TITLE)
	{
		ocscpy(pdbi->wszTitle, L"Howl");
 	}
	if (pdbi->dwMask & DBIM_MODEFLAGS)
	{
		pdbi->dwModeFlags = DBIMF_VARIABLEHEIGHT;
	}
	if (pdbi->dwMask & DBIM_BKCOLOR)
	{
		// use default background color
		pdbi->dwMask &= ~DBIM_BKCOLOR;
	}
	return S_OK;

}

STDMETHODIMP CHowlExplorerBar::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
	ATLTRACE(_T("CHowlExplorerBar::UIActivateIO\n"));

	if (fActivate) SetFocus();
	return S_OK;
}

STDMETHODIMP CHowlExplorerBar::HasFocusIO()
{
//	ATLTRACE(_T("CHowlExplorerBar::HasFocusIO\n"));

	return m_bHasFocus ? S_OK : S_FALSE;
}

STDMETHODIMP CHowlExplorerBar::TranslateAcceleratorIO(LPMSG lpMsg)
{
//	ATLTRACE(_T("CHowlExplorerBar::TranslateAcceleratorIO\n"));

	if (lpMsg->message != WM_KEYDOWN) return S_FALSE;	// not handled

	//	nb: CTRL-Left,Right,Up,Down,PgUp,PgDown,Home,End: 
	//		Scroll without changing selection

	HRESULT hr;
	switch (lpMsg->wParam)
	{
	case VK_UP:			// Move selection to the previous non-hidden item.
	case VK_DOWN:		// Move selection to the next non-hidden item.
	case VK_HOME:		// Move selection to the first item.
	case VK_END:		// Move selection to the last item.
	case VK_LEFT:		// If expanded then collapse, otherwise move to parent.
	case VK_RIGHT:		// If collapsed then expand, otherwise move to first child.
	case VK_ADD:		// Expand.
	case VK_SUBTRACT:	// Collapse.
	case VK_MULTIPLY:	// Expand all.
	case VK_PRIOR:		// Move up GetVisibleCount items.
	case VK_NEXT:		// Move down GetVisibleCount items.
	case VK_BACK:		// Move to parent.
		SendMessage(lpMsg->message, lpMsg->wParam, lpMsg->lParam);
		hr = S_OK;
		break;
	default:
		hr = S_FALSE;	// not handled;
		break;
	}

	return hr;
}

STDMETHODIMP  CHowlExplorerBar::QueryContextMenu(HMENU hShellContextMenu, UINT iContextMenuFirstItem, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	ATLTRACE(_T("CHowlExplorerBar::QueryContextMenu\n"));

	CPoint pt = GetMessagePos();
	ScreenToClient(&pt);
	UINT nHtFlags = 0;
	HTREEITEM hti = HitTest(pt, &nHtFlags);
	
	if (hti != 0) SelectItem(hti);

	hti = GetSelectedItem();

	CString strUrl;
	bool bHasUrl = (hti != 0) && GetURLFromHTI(hti, strUrl);

	CMenu menubar;
	menubar.LoadMenu(IDR_CONTEXT_MENU);
	CMenuHandle menu = menubar.GetSubMenu(0);

	CMenuHandle shellmenu(hShellContextMenu);

	UINT iShellItem = iContextMenuFirstItem;	//! remove plus one
	UINT idShellCmd = idCmdFirst;

	int n = menu.GetMenuItemCount();
	for (int i=0; i<n; ++i)
	{
		CMenuItemInfo mii;
		TCHAR sz[128] = {0};
		mii.fMask = MIIM_TYPE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.cch = LENGTHOF(sz);
		mii.dwTypeData = sz;
		menu.GetMenuItemInfo(i, TRUE, &mii);
		
		if (!bHasUrl && (mii.wID != ID_ABOUT))
		{
			mii.fMask |= MIIM_STATE;
			mii.fState = MF_GRAYED;
		}
		mii.wID = idShellCmd++;

		shellmenu.InsertMenuItem(iShellItem++, TRUE, &mii);
	}

	return n;
}

// Not called for explorer bars
STDMETHODIMP CHowlExplorerBar::GetCommandString(UINT idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
	ATLTRACE(_T("CHowlExplorerBar::GetCommandString\n"));

	return E_NOTIMPL;
}

//	The shell sends either strings or indexes
//	IE never sends strings
//	The indexes are into an array of my commands
//	So the verb for the first command I added to the menu is always 0x0000
//	Here - because I don't have any submenus - 
//		I can treat the 'verb' as an menu item position	
STDMETHODIMP CHowlExplorerBar::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
	ATLTRACE(_T("CHowlExplorerBar::InvokeCommand\n"));

	// IE doesn't send string commands
	if (HIWORD(lpici->lpVerb) != 0) return 0;

	CMenu menubar;
	menubar.LoadMenu(IDR_CONTEXT_MENU);
	CMenuHandle menu = menubar.GetSubMenu(0);
	UINT nMyCmdID = menu.GetMenuItemID((UINT)lpici->lpVerb);

	CAbout dlg;

	dlg.DoModal();

	return S_OK;
}

void _stdcall CHowlExplorerBar::BeforeNavigate2(IDispatch* pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, VARIANT_BOOL* Cancel)
{
	if (!m_spWB2.IsEqualObject(pDisp)) return;

	if (URL == 0 || URL->vt != VT_BSTR || URL->bstrVal == 0) return;

	if (m_spWB2.IsEqualObject(pDisp))
	{
		CString strUrl(URL->bstrVal);
		if (strUrl.Left(4) == _T("http")
			|| strUrl.Left(4) == _T("file"))
		{
			//DeleteItem(0);
		}
	}
}

void _stdcall CHowlExplorerBar::DocumentComplete(IDispatch* pDisp, VARIANT* URL)
{
	if (!m_spWB2.IsEqualObject(pDisp)) return;

	/* DisplayDocumentTree(); */
}


LRESULT CHowlExplorerBar::OnSetFocus(HWND /*hWnd*/)
{
	ATLTRACE("CHowlExplorerBar::OnSetFocus\n");
	SetMsgHandled(FALSE);
	m_bHasFocus = true;
	m_spInputObjectSite->OnFocusChangeIS(static_cast<IDockingWindow*>(this), TRUE);
	return 0;
}

LRESULT CHowlExplorerBar::OnKillFocus(HWND /*hWnd*/)
{
	ATLTRACE("CHowlExplorerBar::OnKillFocus\n");
	SetMsgHandled(FALSE);
	m_bHasFocus = false;
	m_spInputObjectSite->OnFocusChangeIS(static_cast<IDockingWindow*>(this), FALSE);
	return 0;
}


LRESULT CHowlExplorerBar::OnAbout(UINT /*nNotifyCode*/, int /*nID*/, HWND /*hWndCtl*/)
{
	CAbout dlg;
	dlg.DoModal(m_hWnd);

	return 0;
}

void CHowlExplorerBar::AdviseWebBrowser(bool bAdvise)
{
	if (m_spWB2 == 0) return;

	bool bIsAdvised = m_dwEventCookie != 0xFEFEFEFE;

	if (bAdvise && !bIsAdvised) DispEventAdvise(m_spWB2);
	else if (!bAdvise && bIsAdvised) DispEventUnadvise(m_spWB2);
}


HRESULT CHowlExplorerBar::GetHTMLDoc2FromWB2(IWebBrowser2* pWB2, IHTMLDocument2** ppDoc2)
{
	ATLASSERT(pWB2 != 0);
	ATLASSERT(ppDoc2 != 0 && *ppDoc2 == 0);

	CComPtr<IDispatch> spDisp;
	HRESULT hr = pWB2->get_Document(&spDisp);
	if (FAILED(hr)) return hr;


	hr = spDisp->QueryInterface(ppDoc2);

	return hr;
}

bool CHowlExplorerBar::GetURLFromHTI(HTREEITEM hti, CString& strUrl)
{
	GetItemText(hti, strUrl);

	int iSlash = strUrl.Find(_T("//"));
	if (iSlash == -1)
	{
		strUrl.Empty();
		return false;
	}

	return true;
}


void
CHowlExplorerBar::load()
{
	//
	// browse http services
	//
	sw_discovery_browse(m_discovery, 0, DISCOVERY_HTTP_SERVICE, NULL, (sw_discovery_browse_reply) browse_reply, this, &m_http_id);

	//
	// browse ftp services
	//
	sw_discovery_browse(m_discovery, 0, DISCOVERY_FTP_SERVICE, NULL, (sw_discovery_browse_reply) browse_reply, this, &m_ftp_id);
}


void
CHowlExplorerBar::unload()
{
	sw_discovery_stop_browse(m_discovery, m_http_id);
	sw_discovery_stop_browse(m_discovery, m_ftp_id);

	resolve(NULL);

	while (m_services)
	{
		ServiceInfo * sinfo;

		sinfo = m_services;
		m_services = sinfo->m_next;

		if (IsWindow())
		{
			DeleteItem(sinfo->m_tree_id);
		}

		delete sinfo->m_internal_name;
		delete sinfo->m_type;
		delete sinfo;
	}
}


void
CHowlExplorerBar::resolve(
									ServiceInfo	*	sinfo)
{
	if (m_resolving)
	{
		sw_discovery_stop_resolve(m_discovery, m_resolve_id);
		m_resolving = SW_FALSE;
	}

	if (sinfo)
	{
		if (sw_discovery_resolve(m_discovery, sinfo->m_internal_name, sinfo->m_type, DISCOVERY_DEFAULT_DOMAIN, this, (sw_discovery_resolve_reply) resolve_reply, sinfo, &m_resolve_id))
		{
			m_resolving = SW_TRUE;
		}
	}
}


LRESULT
CHowlExplorerBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);	// dbl-clicking should open items w/ children

	HTREEITEM hti = GetSelectedItem();

	if (hti == 0)
	{
		return 0;
	}

	if (m_discovery_error_id != NULL)
	{
		return 0;	
	}

	if ((hti == m_http_tree_id) || (hti == m_ftp_tree_id))
	{
		unload();
		load();
	}
	else
	{
		ServiceInfo	* sinfo;
	
		for (sinfo = m_services; sinfo; sinfo = sinfo->m_next)
		{
			if (sinfo->m_tree_id == hti)
			{
				resolve(sinfo);
				break;
			}
		}
	}

	return 0;
}


sw_result
CHowlExplorerBar::browse_reply(
            					sw_discovery                 discovery,
            					sw_discovery_browse_id       id,
            					sw_discovery_browse_status   status,
									sw_uint32							interface_index,
            					sw_const_string               name,
            					sw_const_string               type,
            					sw_const_string               domain,
            					sw_opaque                     extra)
{
	CHowlExplorerBar * self = (CHowlExplorerBar*) extra;

	switch (status)
	{
		case SW_DISCOVERY_BROWSE_ADD_DEFAULT_DOMAIN:
		case SW_DISCOVERY_BROWSE_ADD_DOMAIN:
		{
		}
		break;

		case SW_DISCOVERY_BROWSE_ADD_SERVICE:
		{
			ServiceInfo	*	sinfo;

			//
			// make sure we don't know about this service
			//
			sinfo = self->lookup_service(name, type);

			if (sinfo != NULL)
			{
				goto exit;
			}

			//
			// okay, now we know that it's a service we haven't seen before
			//
			sinfo					= self->add_service(name, type);

			if (strcmp(type, (const char*) DISCOVERY_HTTP_SERVICE) == 0)
			{
				sinfo->m_tree_id	= self->InsertItem(sinfo->m_visible_name, 0, 0, self->m_http_tree_id, TVI_LAST);
				self->SortChildren(self->m_http_tree_id);
			}
			else if (strcmp(type, (const char*) DISCOVERY_FTP_SERVICE) == 0)
			{
				sinfo->m_tree_id	= self->InsertItem(sinfo->m_visible_name, 0, 0, self->m_ftp_tree_id, TVI_LAST);
				self->SortChildren(self->m_ftp_tree_id);
			}

			self->SelectItem(self->m_http_tree_id);
			self->SelectItem(self->m_ftp_tree_id);
			self->SelectItem(NULL);
		}
		break;

		case SW_DISCOVERY_BROWSE_REMOVE_DOMAIN:
		{
		}
		break;

		case SW_DISCOVERY_BROWSE_REMOVE_SERVICE:
		{
			ServiceInfo	*	sinfo;

			//
			// make sure we don't know about this service
			//
			sinfo = self->lookup_service(name, type);

			if (sinfo)
			{
				self->remove_service(sinfo);
			}

		}
		break;
	}

exit:

   return SW_OKAY;
}


sw_result
CHowlExplorerBar::resolve_reply(
            					sw_discovery                 discovery,
            					sw_discovery_resolve_id      id,
									sw_uint32							interface_index,
            					sw_const_string               name,
            					sw_const_string               type,
            					sw_const_string               domain,
									sw_const_string					hostname,
            					sw_ipv4_address               address,
            					sw_port                       port,
									sw_octets							text_record,
									sw_uint32								text_record_len,
            					sw_opaque                     extra)
{
	char				name_buf[16];
	char				buffer[256];
	CString			url;
	ServiceInfo	*	sinfo = (ServiceInfo*) extra;
	CComVariant		cv;

	sw_discovery_stop_resolve(discovery, id);

	sw_ipv4_address_name(address, name_buf, 16);

	char										key[255];
	char										val[255];
	sw_text_record_string_iterator	it;
	
	sw_text_record_string_iterator_init(&it, text_record_string);

	if (strcmp(type, DISCOVERY_HTTP_SERVICE) == 0)
	{
		sprintf(buffer, "http://%s:%d", name_buf, port);

		url = buffer;

		while (sw_text_record_string_iterator_next(it, key, val) == SW_OKAY)
		{
			if (sw_strcmp(key, "path") == 0)
			{
				url += val;
				break;
			}
		}
	}
	else if (strcmp(type, DISCOVERY_FTP_SERVICE) == 0)
	{
		bool	foundUser = false;
		bool	foundPass = false;
		char	u[255];
		char	p[255];

		while (sw_text_record_string_iterator_next(it, key, val) == SW_OKAY)
		{
			if (sw_strcmp(key, "u") == 0)
			{
				foundUser = true;
				strcpy(u, val);
			}
			else if (sw_strcmp(key, "p") == 0)
			{
				foundPass = true;
				strcpy(p, val);
			}
		}

		if (foundUser && foundPass)
		{
			sprintf(buffer, "ftp://%s:%s@%s", u, p, name_buf);

			url = buffer;
		}
		else
		{
			CFTP dlg;

			//
			// user might hit cancel button
			//
			if (!dlg.GetURL(self->m_hWnd, address, buffer))
			{
				sw_text_record_string_iterator_fina(it);
				return SW_OKAY;
			}

			url = buffer;
		}
	}

	sw_text_record_string_iterator_fina(it);
	
	self->m_spWB2->Navigate(CComBSTR(url), &cv, &cv, &cv, &cv);

   return SW_OKAY;
}


CHowlExplorerBar::ServiceInfo*
CHowlExplorerBar::lookup_service(
												sw_const_string	name,
												sw_const_string	type)
{
	ServiceInfo	*	sinfo;

	for (sinfo = m_services; sinfo; sinfo = sinfo->m_next)
	{
		if ((strcmp(sinfo->m_internal_name, name) == 0) &&
		    (strcmp(sinfo->m_type, type) == 0))
		{
			return sinfo;
		}
	}

	return NULL;
}


CHowlExplorerBar::ServiceInfo*
CHowlExplorerBar::add_service(
												sw_const_string	name,
												sw_const_string	type)
{
	ServiceInfo *	sinfo;
	wchar_t		*	wide_buf;
	int				sz;

	sinfo = new ServiceInfo;

	//
	// store as UTS-8
	//
	sinfo->m_internal_name	=	new char[strlen(name) + 1];
	strcpy(sinfo->m_internal_name, name);

	//
	// now convert it to something Windows likes
	//
	sz = MultiByteToWideChar(CP_UTF8, 0, name, strlen(name) + 1, NULL, 0);
	wide_buf = new wchar_t[sz];
	MultiByteToWideChar(CP_UTF8, 0, name, strlen(name) + 1, wide_buf, sz);

	sinfo->m_visible_name	=	wide_buf;
	delete [] wide_buf;
	sinfo->m_type				=	new char[strlen(type) + 1];
	strcpy(sinfo->m_type, type);

	sinfo->m_next	=	m_services;
	m_services 		=	sinfo;
	
	return sinfo;
}


void
CHowlExplorerBar::remove_service(
												ServiceInfo	*	sinfo)
{
	ServiceInfo * inserted;

	/*
	 * check for head of list
	 */
	if (m_services == sinfo)
	{
		DeleteItem(sinfo->m_tree_id);
		m_services = sinfo->m_next;
		
		delete sinfo->m_internal_name;
		delete sinfo->m_type;
		delete sinfo;
	}
	else
	{
		for (inserted = m_services; (inserted && (inserted->m_next != sinfo)); inserted = inserted->m_next);

		if (inserted)
		{
			inserted->m_next = sinfo->m_next;

			DeleteItem(sinfo->m_tree_id);
			delete sinfo->m_internal_name;
			delete sinfo->m_type;
			delete sinfo;
		}
	}
}
