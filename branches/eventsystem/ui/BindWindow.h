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
#ifndef __BINDWINDOW_H
#define __BINDWINDOW_H

class idUserInterfaceLocal;
class idBindWindow : public idWindow {
public:
	idBindWindow(idUserInterfaceLocal *gui);
	idBindWindow(idDeviceContext *d, idUserInterfaceLocal *gui);
	virtual ~idBindWindow();

	virtual const char *HandleEvent(const sysEvent_t *event, bool *updateVisuals);
	virtual void PostParse();
	virtual void Draw(int time, float x, float y);
	virtual size_t Allocated(){return idWindow::Allocated();};
// 
//  
	virtual idWinVar *GetWinVarByName(const char *_name, bool winLookup = false, drawWin_t** owner = NULL );
// 
	virtual void Activate( bool activate, idStr &act );
	
private:
	void CommonInit();
	idWinStr bindName;
	bool waitingOnKey;
};

#endif // __BINDWINDOW_H
