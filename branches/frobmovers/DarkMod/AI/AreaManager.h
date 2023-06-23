/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AREA_MANAGER_H__
#define __AREA_MANAGER_H__

#include <map>

class idActor;

namespace ai
{

class AreaManager
{
private:
	typedef std::multimap<int, const idActor*> ForbiddenAreasMap;
	ForbiddenAreasMap _forbiddenAreas;

public:
	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);

	void AddForbiddenArea(int areanum, const idActor* actor);
	bool AreaIsForbidden(int areanum, const idActor* actor) const;
	void RemoveForbiddenArea(int areanum, const idActor* actor);

	void Clear();
};

} // namespace ai

#endif /* __AREA_MANAGER_H__ */
