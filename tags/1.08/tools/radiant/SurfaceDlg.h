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
#if !defined(AFX_SURFACEDLG_H__D84E0C22_9EEA_11D1_B570_00AA00A410FC__INCLUDED_)
#define AFX_SURFACEDLG_H__D84E0C22_9EEA_11D1_B570_00AA00A410FC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SurfaceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSurfaceDlg dialog

class CSurfaceDlg : public CDialog
{
	bool m_bPatchMode;
	CWnd *focusControl;

	// Construction
public:
	CSurfaceDlg(CWnd* pParent = NULL);   // standard constructor
  void SetTexMods();

// Dialog Data
	//{{AFX_DATA(CSurfaceDlg)
	enum { IDD = IDD_SURFACE };
	CEdit	m_wndRotateEdit;
	CEdit	m_wndVert;
	CEdit	m_wndHorz;
	CSliderCtrl	m_wndVerticalSubdivisions;
	CSliderCtrl	m_wndHorzSubdivisions;
	CSpinButtonCtrl	m_wndWidth;
	CSpinButtonCtrl	m_wndHeight;
	CSpinButtonCtrl	m_wndVShift;
	CSpinButtonCtrl	m_wndVScale;
	CSpinButtonCtrl	m_wndRotate;
	CSpinButtonCtrl	m_wndHShift;
	CSpinButtonCtrl	m_wndHScale;
	int		m_nHorz;
	int		m_nVert;
	float	m_horzScale;
	float	m_horzShift;
	float	m_rotate;
	float	m_vertScale;
	float	m_vertShift;
	CString	m_strMaterial;
	BOOL	m_subdivide;
	float	m_fHeight;
	float	m_fWidth;
	BOOL	m_absolute;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSurfaceDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:

  void UpdateSpinners(int nScrollCode, int nPos, CScrollBar* pBar);
  void UpdateSpinners(bool bUp, int nID);
	// Generated message map functions
	//{{AFX_MSG(CSurfaceDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnApply();
	virtual void OnOK();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	afx_msg void OnBtnCancel();
	afx_msg void OnBtnColor();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBtnPatchdetails();
	afx_msg void OnBtnPatchnatural();
	afx_msg void OnBtnPatchreset();
	afx_msg void OnBtnAxial();
	afx_msg void OnBtnBrushfit();
	afx_msg void OnBtnFacefit();
	afx_msg void OnCheckSubdivide();
	afx_msg void OnChangeEditHorz();
	afx_msg void OnChangeEditVert();
	afx_msg void OnSetfocusHscale();
	afx_msg void OnKillfocusHscale();
	afx_msg void OnKillfocusVscale();
	afx_msg void OnSetfocusVscale();
	afx_msg void OnKillfocusEditWidth();
	afx_msg void OnSetfocusEditWidth();
	afx_msg void OnKillfocusEditHeight();
	afx_msg void OnSetfocusEditHeight();
	afx_msg void OnBtnFlipx();
	afx_msg void OnBtnFlipy();
	afx_msg void OnKillfocusRotate();
	afx_msg void OnSetfocusRotate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SURFACEDLG_H__D84E0C22_9EEA_11D1_B570_00AA00A410FC__INCLUDED_)
