/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4723 $
 * $Date: 2011-03-25 00:49:57 -0400 (Fri, 25 Mar 2011) $
 * $Author: greebo $
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
