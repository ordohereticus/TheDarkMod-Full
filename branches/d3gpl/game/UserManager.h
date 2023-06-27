/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5121 $ (Revision of last commit) 
 $Date: 2011-12-11 14:12:26 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#ifndef USER_MANAGER_H
#define USER_MANAGER_H

class UserManager
{
	
public:

	int GetNumUsers();

	// Adds user to the list, sorted by alert level
	void AddUser(idActor*);

	void AppendUser(idActor* actor); // grayman #1327 - append to the list, don't care about alert level

	void RemoveUser(idActor*);

	idActor* GetMasterUser();

	idActor* GetUserAtIndex(const int index); // grayman #2345
	void InsertUserAtIndex(idActor* actor,const int index); // grayman #2345

	int GetIndex(idActor* user);			// grayman #2345
	void ResetMaster(CFrobDoor* frobDoor); // grayman #2706

	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);

private:

	idList< idEntityPtr<idActor> >			m_users;



};

#endif /* USER_MANAGER_H */
