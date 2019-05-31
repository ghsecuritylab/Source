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
#include "FTP.h"
//#include <salt/debug.h>


LRESULT
CFTP::OnInitDialog(
					UINT		uMsg,
					WPARAM	wParam,
					LPARAM	lParam,
					BOOL	&	bHandled)
{
	CenterWindow(GetParent());

	BSTR txt = ::SysAllocString(m_ftp_server_unicode);

	SetDlgItemText(IDC_FTP_SERVER, txt);

	m_anon_login = SW_FALSE;

	return 1;  // Let the system set the focus
}


LRESULT
CFTP::OnAnonLogin(
					WORD		wNotifyCode,
					WORD		wID,
					HWND		hWndCtl,
					BOOL	&	bHandled)
{
	CButton button = GetDlgItem(IDC_FTP_ANON_LOGIN);
	CComboBox username = GetDlgItem(IDC_FTP_USERNAME);
	CEdit		 password = GetDlgItem(IDC_FTP_PASSWORD);

	switch (button.GetCheck())
	{
		case 0:
		{
			username.EnableWindow(1);
			password.EnableWindow(1);
			m_anon_login = SW_FALSE;
		}
		break;

		case 1:
		{
			username.EnableWindow(0);
			password.EnableWindow(0);
			m_anon_login = SW_TRUE;
		}
		break;
	}

	return 1;
}


LRESULT
CFTP::OnLogOn(
					WORD		wNotifyCode,
					WORD		wID,
					HWND		hWndCtl,
					BOOL	&	bHandled)
{
	wchar_t buf[256];

	if (GetDlgItemText(IDC_FTP_USERNAME, buf, 256))
	{
		WideCharToMultiByte(CP_ACP, 0, buf, -1, m_username, 256, NULL, NULL);
	}

	if (GetDlgItemText(IDC_FTP_PASSWORD, buf, 256))
	{
		WideCharToMultiByte(CP_ACP, 0, buf, -1, m_password, 256, NULL, NULL);
	}

	EndDialog(IDLOGON);

	return 0;
}


LRESULT
CFTP::OnCancel(
					WORD		wNotifyCode,
					WORD		wID,
					HWND		hWndCtl,
					BOOL	&	bHandled)
{
	EndDialog(IDCANCELLOGON);
	return 0;
}


LRESULT
CFTP::GetURL(
					HWND					window,
					sw_ipv4_address	ftp_server,
					char				*	url)
{
	CButton button = GetDlgItem(IDC_FTP_ANON_LOGIN);
	LRESULT ret;

	//
	// initialize
	//
	m_username[0] = '\0';
	m_password[0] = '\0';

	//
	// stash this away
	//
	sw_ipv4_address_name(ftp_server, m_ftp_server, 16);
	m_ftp_server_unicode = m_ftp_server;

	switch (DoModal(window))
	{
		case IDLOGON:
		{
			if (m_anon_login)
			{
				sprintf(url, "ftp://anonymous:anonymous@%s", m_ftp_server);
			}
			else
			{
				sprintf(url, "ftp://%s:%s@%s", m_username, m_password, m_ftp_server);
			}
			
			ret = 1;
		}
		break;

		default:
		{
			ret = 0;
		}
	}

	return ret;
}
