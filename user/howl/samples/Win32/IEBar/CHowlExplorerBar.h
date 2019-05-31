#ifndef __HOWL_EXPLORER_BAR_H
#define __HOWL_EXPLORER_BAR_H

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

#include "resource.h"
#include <ComDef.h>
#include <ShlObj.h>
#include <ShlGuid.h>
#include <ComDef.h>
#include <ShlGuid.h>
#include <ExDisp.h>
#include <ExDispid.h>
#include <salt/salt.h>
#include <discovery/discovery.h>


//	Note - this object is created when it's first shown,
//		persists for the life of the browser session.


class CHowlExplorerBar;	// fwd

typedef IDispEventImpl<1, CHowlExplorerBar, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 0> WB2Events;

/////////////////////////////////////////////////////////////////////////////
// CHowlExplorerBar

// {3588080F-0056-4d28-8519-F0479C73D40A}
DEFINE_GUID(CLSID_HowlExplorerBar, 0x3588080f, 0x56, 0x4d28, 0x85, 0x19, 0xf0, 0x47, 0x9c, 0x73, 0xd4, 0xa);

typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN  | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE> CTvTraits;

class ATL_NO_VTABLE CHowlExplorerBar
: 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CHowlExplorerBar, &CLSID_HowlExplorerBar>,
	public IPersistStream,
	public IObjectWithSite,
	public IDeskBand,
	public IInputObject,
	public IContextMenu,
	public CWindowImpl<CHowlExplorerBar, CTreeViewCtrl, CTvTraits>,
	public WB2Events
{
private:

	CComPtr<IInputObjectSite> m_spInputObjectSite;
	CComPtr<IWebBrowser2> m_spWB2;
	CWindow m_wndSite;
	DESKBANDINFO m_dbi;
	bool m_bHasFocus;

	enum {
		MINCX = 30,
		MINCY = 30
	};

public:

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHowlExplorerBar)
	COM_INTERFACE_ENTRY(IPersist)
	COM_INTERFACE_ENTRY(IPersistStream)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IDeskBand)
	COM_INTERFACE_ENTRY(IInputObject)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CHowlExplorerBar)
	IMPLEMENTED_CATEGORY(CATID_InfoBand)
END_CATEGORY_MAP()

BEGIN_SINK_MAP(CHowlExplorerBar)
	SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, BeforeNavigate2)
	SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, DocumentComplete)
END_SINK_MAP()

DECLARE_WND_SUPERCLASS(_T("Howl"), CTreeViewCtrl::GetWndClassName())

BEGIN_MSG_MAP_EX(CHowlExplorerBar)
	MSG_WM_SETFOCUS(OnSetFocus)
	MSG_WM_KILLFOCUS(OnKillFocus)
	MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
	COMMAND_ID_HANDLER_EX(ID_ABOUT, OnAbout)
END_MSG_MAP()

public:

	CHowlExplorerBar();
	~CHowlExplorerBar();

	static HRESULT WINAPI UpdateRegistry(BOOL bRegister);

// IPersistStream
	STDMETHOD(GetClassID)(LPCLSID pClassID);
	STDMETHOD(IsDirty)();
	STDMETHOD(Load)(LPSTREAM pStream);
	STDMETHOD(Save)(LPSTREAM pStream, BOOL fClearDirty);
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pul);

// IObjectWithSite
	STDMETHOD(SetSite)(IUnknown* pUnkSite);
	STDMETHOD(GetSite)(REFIID riid, LPVOID* ppvReturn);

// IDeskBand (inherits IOleWindow and IDockingWindow)
	STDMETHOD(GetWindow)(HWND* phWnd);
	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
	STDMETHOD(ShowDW)(BOOL fShow);
	STDMETHOD(CloseDW)(DWORD dwReserved);
	STDMETHOD(ResizeBorderDW)(LPCRECT, IUnknown*, BOOL);
	STDMETHOD(GetBandInfo)(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi);

// IInputObject
    STDMETHOD(UIActivateIO)(BOOL fActivate, LPMSG lpMsg);
    STDMETHOD(HasFocusIO)();
    STDMETHOD(TranslateAcceleratorIO)(LPMSG lpMsg);

	// IContextMenuImpl
	STDMETHOD(QueryContextMenu)(HMENU hContextMenu, UINT iContextMenuFirstItem, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	STDMETHOD(GetCommandString)(UINT idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax);
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);

// DWebBrowserEvents2
	void _stdcall BeforeNavigate2(IDispatch* pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, VARIANT_BOOL* Cancel);
	void _stdcall DocumentComplete(IDispatch* pDisp, VARIANT* URL);

// CWindowImpl
	LRESULT OnSetFocus(HWND /*hWnd*/);
	LRESULT OnKillFocus(HWND /*hWnd*/);
	LRESULT OnLButtonDblClk(UINT /*nFlags*/, CPoint point);
	LRESULT OnViewSource(UINT /*nNotifyCode*/, int /*nID*/, HWND /*hWndCtl*/);
	LRESULT OnCopyURL(UINT /*nNotifyCode*/, int /*nID*/, HWND /*hWndCtl*/);
	LRESULT OnAbout(UINT /*nNotifyCode*/, int, HWND);

private:
	void AdviseWebBrowser(bool bAdvise);

	void DisplayDocumentTree();
	void AddDocument(HTREEITEM htiParent, IHTMLDocument2* pDoc2);
	void AddFrames(HTREEITEM htiDoc, IHTMLDocument2* pDoc2);
	void AddScripts(HTREEITEM htiDoc, IHTMLDocument2* pDoc2, LPCWSTR pwzBaseUrl);
	void AddStyleSheets(HTREEITEM htiDoc, IHTMLDocument2* pDoc2, LPCWSTR pwzBaseUrl);

	void ResolveBaseUrl(IHTMLDocument2* pDoc2, BSTR* pbstrBaseUrl);
	bool GetBaseElemHref(IHTMLDocument2* pDoc2, BSTR* pbstrHref);

	HRESULT GetHTMLDoc2FromWB2(IWebBrowser2* pWB2, IHTMLDocument2** ppDoc2);
	bool GetURLFromHTI(HTREEITEM hti, CString& strUrl);


	static sw_result
	browse_reply(
            sw_discovery                 discovery,
            sw_discovery_browse_id       id,
            sw_discovery_browse_status   status,
				sw_uint32							interface_index,
            sw_const_string               name,
            sw_const_string               type,
            sw_const_string               domain,
            sw_opaque                     extra);


	static sw_result
	resolve_reply(
            sw_discovery                 discovery,
            sw_discovery_resolve_id      id,
				sw_uint32							interface_index,
            sw_const_string               name,
            sw_const_string               type,
            sw_const_string               domain,
            sw_ipv4_address					address,
            sw_port                       port,
            sw_const_string               text_record_string,
				sw_octets							text_record,
				sw_uint32								text_record_len,
            sw_opaque                     extra);

	
	struct ServiceInfo;


	struct ServiceInfo
	{
		HTREEITEM						m_tree_id;
		char							*	m_internal_name;
		CString							m_visible_name;
		char							*	m_type;
		sw_uint16						m_port;
		CString							m_textRecord;
		ServiceInfo					*	m_next;
	};


	ServiceInfo*
	lookup_service(
				sw_const_string	name,
				sw_const_string	type);


	ServiceInfo*
	add_service(
				sw_const_string	name,
				sw_const_string	type);

	
	void
	remove_service(
				ServiceInfo	*		service);


	void
	load();

	void
	unload();


	void
	browse(
				sw_bool enable);


	void
	resolve(
				ServiceInfo	*	sinfo);

	CImageList						m_image_list;
	HTREEITEM						m_discovery_error_id;
	HTREEITEM						m_http_tree_id;
	HTREEITEM						m_ftp_tree_id;
	ServiceInfo					*	m_services;
	sw_discovery					m_discovery;
	sw_result						m_discovery_status;
	sw_bool							m_browsing;
	sw_discovery_browse_id		m_http_id;
	sw_discovery_browse_id		m_ftp_id;
	sw_bool							m_resolving;
	sw_discovery_resolve_id		m_resolve_id;
	sw_salt							m_salt;
};


#endif
