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
#if !defined(AFX_PROPERTYLIST_H__74205380_1B56_11D4_BC48_00105AA2186F__INCLUDED_)
#define AFX_PROPERTYLIST_H__74205380_1B56_11D4_BC48_00105AA2186F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyList.h : header file
//

#define PIT_COMBO		0	//PIT = property item type
#define PIT_EDIT		1
#define PIT_COLOR		2
#define PIT_FONT		3
#define PIT_FILE		4
#define PIT_SCRIPT		5
#define PIT_MODEL		6
#define PIT_SOUND		7
#define PIT_GUI			8
#define PIT_MATERIAL	9
#define PIT_VAR			10

#define IDC_PROPCMBBOX   712
#define IDC_PROPEDITBOX  713
#define IDC_PROPBTNCTRL  714


/////////////////////////////////////////////////////////////////////////////
//CPropertyList Items
class CPropertyItem
{
// Attributes
public:
	CString m_propName;
	CString m_curValue;
	int m_nItemType;
	CString m_cmbItems;
	int data;

public:
	CPropertyItem(CString propName, CString curValue,
				  int nItemType, CString cmbItems)
	{
		m_propName = propName;
		m_curValue = curValue;
		m_nItemType = nItemType;
		m_cmbItems = cmbItems;
		data = -1;
	}
	void SetData(int d) {
		data = d;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CPropertyList window

class CPropertyList : public CListBox
{
// Construction
public:
	CPropertyList();

// Attributes
public:

// Operations
public:
	int AddItem(CString txt);
	int AddPropItem(CPropertyItem* pItem);
	void ResetContent();
	CEdit *GetEditBox() {
		return &m_editBox;
	}
	void SetUpdateInspectors(bool b) {
		updateInspectors = b;
	}
	void SetDivider( int div ) {
		m_nDivider = div;
	}
	afx_msg void OnKillfocusEditBox();
	afx_msg void OnChangeEditBox();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyList)
	public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnSelchange();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropertyList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	//}}AFX_MSG
	afx_msg void OnKillfocusCmbBox();
	afx_msg void OnSelchangeCmbBox();
	afx_msg void OnButton();

	DECLARE_MESSAGE_MAP()

	void InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo);
	void DisplayButton(CRect region);

	CComboBox m_cmbBox;
	CEdit m_editBox;
	CButton m_btnCtrl;
	CFont m_SSerif8Font;
	
	int m_curSel,m_prevSel;
	int m_nDivider;
	int m_nDivTop;
	int m_nDivBtm;
	int m_nOldDivX;
	int m_nLastBox;
	BOOL m_bTracking;
	BOOL m_bDivIsSet;
	HCURSOR m_hCursorArrow;
	HCURSOR m_hCursorSize;
	CPropertyItem *measureItem;
	bool updateInspectors;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYLIST_H__74205380_1B56_11D4_BC48_00105AA2186F__INCLUDED_)
