/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3731 $
 * $Date: 2009-10-29 03:50:38 -0400 (Thu, 29 Oct 2009) $
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

	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);

private:

	idList< idEntityPtr<idActor> >			m_users;



};

#endif /* USER_MANAGER_H */
