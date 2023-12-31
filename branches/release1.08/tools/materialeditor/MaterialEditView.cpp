/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5171 $ (Revision of last commit) 
 $Date: 2012-01-07 03:08:06 -0500 (Sat, 07 Jan 2012) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/
#include "precompiled_engine.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: MaterialEditView.cpp 5171 2012-01-07 08:08:06Z greebo $");

#include "MaterialEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EDIT_HEIGHT 25

#define EDIT_TAB_CONTROL	0x2006
#define NAME_CONTROL		0x2007

IMPLEMENT_DYNCREATE(MaterialEditView, CFormView)

BEGIN_MESSAGE_MAP(MaterialEditView, CFormView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_NOTIFY(TCN_SELCHANGE, EDIT_TAB_CONTROL, OnTcnSelChange)
	ON_NOTIFY(EN_CHANGE, IDC_MATERIALEDITOR_EDIT_TEXT, OnEnChangeEdit)
END_MESSAGE_MAP()

/**
* Constructor for MaterialEditView.
*/
MaterialEditView::MaterialEditView()
: CFormView(MaterialEditView::IDD) {
	
	initHack = false;
	sourceInit = false;
	sourceChanged = false;
}

/**
* Destructor for MaterialEditView.
*/
MaterialEditView::~MaterialEditView() {
}

/**
* Called when the selected material has changed.
* @param pMaterial The newly selected material.
*/
void MaterialEditView::MV_OnMaterialSelectionChange(MaterialDoc* pMaterial) {

	//Apply any text changes that have been made
	ApplyMaterialSource();

	if(pMaterial) {
		m_nameEdit.SetWindowText(pMaterial->name);
		m_textView.SetReadOnly(false);

		//If the edit tab is selected then get the source
		int sel = m_tabs.GetCurSel();
		if (sel == 1) {
			GetMaterialSource();
		}

		currentMaterialName = pMaterial->name;
	} else {
		m_nameEdit.SetWindowText("");

		GetMaterialSource();
		m_textView.SetReadOnly(true);

		currentMaterialName = "";
	}
}

void MaterialEditView::MV_OnMaterialNameChanged(MaterialDoc* pMaterial, const char* oldName) {
	if(!currentMaterialName.Icmp(oldName)) {
		currentMaterialName = pMaterial->name;
	}
}

/**
* Returns the current source text in the source edit control.
*/
idStr MaterialEditView::GetSourceText() {
	idStr text;
	m_textView.GetText(text);

	text.Replace( "\n", "" );
	text.Replace( "\r", "\r\n" );
	text.Replace( "\v", "\r\n" );
	text.StripLeading( "\r\n" );
	text.Insert( "\r\n\r\n", 0 );
	text.StripTrailing( "\r\n" );

	return text;
}

/**
* Gets the source of the current document and populates the
* source edit control.
*/
void MaterialEditView::GetMaterialSource() {

	//Clear it
	sourceInit = true;
	m_textView.SetText("");
	sourceInit = false;

	if(materialDocManager) {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		if(material) {
			idStr text = material->GetEditSourceText();

			// clean up new-line crapola
			text.Replace( "\r", "" );
			text.Replace( "\n", "\r" );
			text.Replace( "\v", "\r" );
			text.StripLeading( '\r' );
			text.Append( "\r" );

			sourceInit = true;
			m_textView.SetText(text);
			sourceInit = false;
		}
	}	
}

/**
* Takes the source out of the edit control and applies it
* to the material.
*/
void MaterialEditView::ApplyMaterialSource() {

	if(!sourceChanged)
		return;

	MaterialDoc* material = materialDocManager->CreateMaterialDoc(currentMaterialName);

	if(material) {
		idStr text = GetSourceText();
		material->ApplySourceModify(text);
	}

	sourceChanged = false;
}

/**
* Transfers data to and from the controls in the console.
*/
void MaterialEditView::DoDataExchange(CDataExchange* pDX) {

	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MATERIALEDITOR_EDIT_TEXT, m_textView);
}

/**
* Called by the MFC framework when the view is being created.
*/
void MaterialEditView::OnInitialUpdate() {
	
	CFormView::OnInitialUpdate();

	if(!initHack) {
		initHack = true;
		m_textView.Init();
		m_textView.LoadKeyWordsFromFile( "editors/material.def" );
		m_textView.ShowWindow(SW_HIDE);

		m_textView.SetText("");
		m_textView.SetReadOnly(true);
	}
}

/**
* Called by the MFC framework when the view is being created.
*/
int MaterialEditView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_nameEdit.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_READONLY, CRect(0,0,0,0), this, NAME_CONTROL);

	m_editSplitter.CreateStatic(this, 1, 2);

	if(!m_editSplitter.CreateView(0, 0, RUNTIME_CLASS(StageView), CSize(200, 200), NULL)) {
		TRACE0("Failed to create stage property pane\n");
		return -1;
	}

	if(!m_editSplitter.CreateView(0, 1, RUNTIME_CLASS(MaterialPropTreeView), CSize(500, 200), NULL)) {
		TRACE0("Failed to create property pane\n");
		return -1;
	}

	m_nameEdit.SetFont(materialEditorFont);

	m_stageView = (StageView*)m_editSplitter.GetPane(0, 0);
	m_materialPropertyView = (MaterialPropTreeView*)m_editSplitter.GetPane(0, 1);

	m_tabs.Create(TCS_BOTTOM | TCS_FLATBUTTONS | WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, EDIT_TAB_CONTROL);
	m_tabs.InsertItem(0, "Properties");
	m_tabs.InsertItem(1, "Text");

	m_tabs.SetFont(materialEditorFont);

	return 0;
}

/**
* Windows message called when the window is resized.
*/
void MaterialEditView::OnSize(UINT nType, int cx, int cy) {
	CFormView::OnSize(nType, cx, cy);

	CRect tabRect;
	m_tabs.GetItemRect(0, tabRect);

	int tabHeight = tabRect.Height()+5;

	//Hardcode the edit window height
	if(m_nameEdit.GetSafeHwnd()) {
		m_nameEdit.MoveWindow(1,1, cx-2, 20);
	}

	if(m_tabs.GetSafeHwnd()) {
		m_tabs.MoveWindow(0, cy-tabHeight, cx, tabHeight);
	}

	if(m_editSplitter.GetSafeHwnd()) {
		m_editSplitter.MoveWindow(1, 22, cx-2, cy-tabHeight-22);
	}

	if(m_textView.GetSafeHwnd()) {
		m_textView.MoveWindow(1, 22, cx-2, cy-tabHeight-22);
	}
}

/**
* Called when the user changes the properties/text tab selection. This methods shows and hides 
* the appropriate windows.
*/
void MaterialEditView::OnTcnSelChange(NMHDR *pNMHDR, LRESULT *pResult) {

	int sel = m_tabs.GetCurSel();

	switch(sel) {
		case 0:
			m_editSplitter.ShowWindow(SW_SHOW);
			m_textView.ShowWindow(SW_HIDE);

			ApplyMaterialSource();

			m_stageView->RefreshStageList();
			
			break;
		case 1:
			m_editSplitter.ShowWindow(SW_HIDE);
			m_textView.ShowWindow(SW_SHOW);

			GetMaterialSource();
			m_textView.SetReadOnly(false);
	}
}

/**
* Called when the user changes text in the edit control
*/
void MaterialEditView::OnEnChangeEdit( NMHDR *pNMHDR, LRESULT *pResult ) {
	if(materialDocManager && !sourceInit) {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		if(material && !material->IsSourceModified()) {
			sourceChanged = true;
			material->SourceModify(this);
		}
	}
}







