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

#ifndef __DIALOGDECLBROWSER_H__
#define __DIALOGDECLBROWSER_H__

#pragma once

// DialogDeclBrowser dialog

class DialogDeclBrowser : public CDialog {

	DECLARE_DYNAMIC(DialogDeclBrowser)

public:
						DialogDeclBrowser( CWnd* pParent = NULL );   // standard constructor
	virtual				~DialogDeclBrowser();

	void				ReloadDeclarations( void );
	bool				CompareDecl( HTREEITEM item, const char *name ) const;

	//{{AFX_VIRTUAL(DialogDeclBrowser)
	virtual BOOL		OnInitDialog();
	virtual void		DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(DialogDeclBrowser)
	afx_msg BOOL		OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnSetFocus( CWnd *pOldWnd );
	afx_msg void		OnDestroy();
	afx_msg void		OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void		OnMove( int x, int y );
	afx_msg void		OnSize( UINT nType, int cx, int cy );
	afx_msg void		OnSizing( UINT nSide, LPRECT lpRect );
	afx_msg void		OnTreeSelChanged( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void		OnTreeDblclk( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedFind();
	afx_msg void		OnBnClickedEdit();
	afx_msg void		OnBnClickedNew();
	afx_msg void		OnBnClickedReload();
	afx_msg void		OnBnClickedOk();
	afx_msg void		OnBnClickedCancel();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:

	//{{AFX_DATA(DialogDeclBrowser)
	enum				{ IDD = IDD_DIALOG_DECLBROWSER };
	CStatusBarCtrl		statusBar;
	CPathTreeCtrl		declTree;
	CStatic				findNameStatic;
	CStatic				findTextStatic;
	CEdit				findNameEdit;
	CEdit				findTextEdit;
	CButton				findButton;
	CButton				editButton;
	CButton				newButton;
	CButton				reloadButton;
	CButton				cancelButton;
	//}}AFX_DATA

	static toolTip_t	toolTips[];

	CRect				initialRect;
	CPathTreeCtrl		baseDeclTree;
	int					numListedDecls;
	idStr				findNameString;
	idStr				findTextString;

	TCHAR *				m_pchTip;
	WCHAR *				m_pwchTip;

private:
	void				AddDeclTypeToTree( declType_t type, const char *root, CPathTreeCtrl &tree );
	void				AddScriptsToTree( CPathTreeCtrl &tree );
	void				AddGUIsToTree( CPathTreeCtrl &tree );
	void				InitBaseDeclTree( void );

	void				GetDeclName( HTREEITEM item, idStr &typeName, idStr &declName ) const;
	const idDecl *		GetDeclFromTreeItem( HTREEITEM item ) const;
	const idDecl *		GetSelectedDecl( void ) const;
	void				EditSelected( void ) const;
};

#endif /* !__DIALOGDECLBROWSER_H__ */
