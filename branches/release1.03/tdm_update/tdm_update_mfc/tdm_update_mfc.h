/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4344 $
 * $Date: 2010-11-28 00:02:54 -0500 (Sun, 28 Nov 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/


// tdm_update_mfc.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// UpdaterApplication:
// See tdm_update_mfc.cpp for the implementation of this class
//
class UpdaterApplication : public CWinAppEx
{
public:
	UpdaterApplication();

public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern UpdaterApplication theApp;