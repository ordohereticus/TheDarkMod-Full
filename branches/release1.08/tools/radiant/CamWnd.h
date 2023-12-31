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
#if !defined(AFX_CAMWND_H__44B4BA03_781B_11D1_B53C_00AA00A410FC__INCLUDED_)
#define AFX_CAMWND_H__44B4BA03_781B_11D1_B53C_00AA00A410FC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

typedef enum
{
	cd_wire,
	cd_solid,
	cd_texture,
	cd_light,
	cd_blend
} camera_draw_mode;

typedef struct
{
	int			width, height;

	idVec3		origin;
	idAngles	angles;

	camera_draw_mode	draw_mode;

	idVec3		color;			// background 

	idVec3		forward, right, up;	// move matrix
	idVec3		vup, vpn, vright;	// view matrix
} camera_t;


/////////////////////////////////////////////////////////////////////////////
// CCamWnd window
class CXYWnd;

class CCamWnd : public CWnd
{
  DECLARE_DYNCREATE(CCamWnd);
// Construction
public:
	CCamWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCamWnd)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void ShiftTexture_BrushPrimit(face_t *f, int x, int y);
	CXYWnd* m_pXYFriend;
	void SetXYFriend(CXYWnd* pWnd);
	virtual ~CCamWnd();
	camera_t& Camera(){return m_Camera;};
	void Cam_MouseControl(float dtime);
	void Cam_ChangeFloor(bool up);
	void BuildRendererState();
	void ToggleRenderMode();
	void ToggleRebuildMode();
	void ToggleEntityMode();
	void ToggleSelectMode();
	void ToggleAnimationMode();
	void ToggleSoundMode();
	void SetProjectionMatrix();
	void UpdateCameraView();

	void BuildEntityRenderState( entity_t *ent, bool update );
	bool GetRenderMode() {
		return renderMode;
	}
	bool GetRebuildMode() {
		return rebuildMode;
	}
	bool GetEntityMode() {
		return entityMode;
	}
	bool GetAnimationMode() {
		return animationMode;
	}
	bool GetSelectMode() {
		return selectMode;
	}
	bool GetSoundMode() {
		return soundMode;
	}


	bool UpdateRenderEntities();
	void MarkWorldDirty();

	void SetView( const idVec3 &origin, const idAngles &angles ) {
		m_Camera.origin = origin;
		m_Camera.angles = angles;
	}

protected:
	void Cam_Init();
	void Cam_BuildMatrix();
	void Cam_PositionDrag();
	void Cam_MouseLook();
	void Cam_MouseDown(int x, int y, int buttons);
	void Cam_MouseUp (int x, int y, int buttons);
	void Cam_MouseMoved (int x, int y, int buttons);
	void InitCull();
	bool CullBrush (brush_t *b, bool cubicOnly);
	void Cam_Draw();
	void Cam_Render();

	// game renderer interaction
	qhandle_t	worldModelDef;
	idRenderModel	*worldModel;		// createRawModel of the brush and patch geometry
	bool	worldDirty;
	bool	renderMode;
	bool	rebuildMode;
	bool	entityMode;
	bool	selectMode;
	bool	animationMode;
	bool	soundMode;
	void	FreeRendererState();
	void	UpdateCaption();
	bool	BuildBrushRenderData(brush_t *brush);
	void	DrawEntityData();


	camera_t m_Camera;
	int	m_nCambuttonstate;
	CPoint m_ptButton;
	CPoint m_ptCursor;
	CPoint m_ptLastCursor;
	face_t* m_pSide_select;
	idVec3 m_vCull1;
	idVec3 m_vCull2;
	int m_nCullv1[3];
	int m_nCullv2[3];
	bool m_bClipMode;
	idVec3 saveOrg;
	idAngles saveAng;
	bool saveValid;

	// Generated message map functions
protected:
	void OriginalMouseDown(UINT nFlags, CPoint point);
	void OriginalMouseUp(UINT nFlags, CPoint point);
	//{{AFX_MSG(CCamWnd)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMWND_H__44B4BA03_781B_11D1_B53C_00AA00A410FC__INCLUDED_)
