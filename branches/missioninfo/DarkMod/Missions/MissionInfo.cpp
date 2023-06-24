/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3919 $
 * $Date: 2010-06-08 03:06:50 -0400 (Tue, 08 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MissionInfo.cpp 3919 2010-06-08 07:06:50Z greebo $", init_version);

#include "MissionInfo.h"
#include "MissionInfoDecl.h"

std::size_t CMissionInfo::GetModFolderSize()
{
	return 0;
}

idStr CMissionInfo::GetKeyValue(const char* key)
{
	assert(_decl != NULL);

	return _decl->data.GetString(key);
}

void CMissionInfo::SetKeyValue(const char* key, const char* value)
{
	assert(_decl != NULL);

	_declDirty = true;

	_decl->data.Set(key, value);
}

void CMissionInfo::Save()
{
	// Don't do unnecessary work
	if (!_declDirty) return;

	// Generate new declaration body text
	_decl->SetText("\"Test\" \"Body\"");
	_decl->ReplaceSourceFileText();
}
