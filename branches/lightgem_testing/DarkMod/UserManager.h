/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4767 $
 * $Date: 2011-04-10 11:28:50 -0400 (Sun, 10 Apr 2011) $
 * $Author: stgatilov $
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
