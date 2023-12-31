/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5122 $ (Revision of last commit) 
 $Date: 2011-12-11 14:47:31 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#ifndef __AFX_STDAFX_H__
#define __AFX_STDAFX_H__

//  include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently

//#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

void InitAfx( void );

// tool tips
typedef struct toolTip_s {
	int id;
	char *tip;
} toolTip_t;

int DefaultOnToolHitTest( const toolTip_t *toolTips, const CDialog *dialog, CPoint point, TOOLINFO* pTI );
BOOL DefaultOnToolTipNotify( const toolTip_t *toolTips, UINT id, NMHDR *pNMHDR, LRESULT *pResult );

// edit control
bool EditControlEnterHit( CEdit *edit );
float EditVerifyFloat( CEdit *edit, bool allowNegative = true );
float EditSpinFloat( CEdit *edit, bool up );

// combo box
int SetSafeComboBoxSelection( CComboBox *combo, const char *string, int skip );
int GetSafeComboBoxSelection( CComboBox *combo, CString &string, int skip );
int UnsetSafeComboBoxSelection( CComboBox *combo, CString &string );

#endif /* !__AFX_STDAFX_H__ */
