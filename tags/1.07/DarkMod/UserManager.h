/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4988 $
 * $Date: 2011-10-07 11:45:07 -0400 (Fri, 07 Oct 2011) $
 * $Author: grayman $
 *
 ***************************************************************************/

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
