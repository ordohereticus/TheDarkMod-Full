/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3928 $
 * $Date: 2010-06-10 02:41:38 -0400 (Thu, 10 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MissionInfoDecl.cpp 3928 2010-06-10 06:41:38Z greebo $", init_version);

#include "MissionInfoDecl.h"

const char* const CMissionInfoDecl::TYPE_NAME = "tdm_missioninfo";

bool CMissionInfoDecl::Parse(idLexer& src)
{
	idToken		key;
	idToken		value;

	if (!src.ReadToken(&key))
	{
		src.Warning("Unclosed mission info declaration.");
		return false;
	}

	if (key.type != TT_PUNCTUATION || key.subtype != P_BRACEOPEN)
	{
		src.Warning("Cannot find opening brace in file %s:%d.", src.GetFileName(), src.GetLineNum());
		return false;
	}

	while (true)
	{
		// If there's an EOF, this is an error.
		if (!src.ReadToken(&key))
		{
			src.Warning("Unclosed mission info declaration.");
			return false;
		}

		// Quit upon encountering the closing brace.
		if (key.type == TT_PUNCTUATION && key.subtype == P_BRACECLOSE)
		{
			break;
		}
		else if (key.type == TT_STRING)
		{
			// Found a string, this must be a key
			if (!src.ReadToken(&value))
			{
				src.Warning("Unexpected EOF in key/value pair.");
				return false;
			}

			if (value.type == TT_STRING)
			{
				// Save the key:value pair.
				data.Set(key.c_str(), value.c_str());
			}
			else
			{
				src.Warning("Invalid value: %s", value.c_str());
				continue;
			}
		}
		else 
		{
			src.Warning("Unrecognized token: %s", key.c_str());
			continue;
		}
	}

	return true;
}

void CMissionInfoDecl::Update(const idStr& name)
{
	_bodyText = TYPE_NAME;
	_bodyText += " " + name;
	_bodyText += "\n{\n";

	// Dump the keyvalues
	for (int i = 0; i < data.GetNumKeyVals(); ++i)
	{
		const idKeyValue* kv = data.GetKeyVal(i);

		_bodyText += "\t\"" + kv->GetKey() + "\"";
		_bodyText += "\t\"" + kv->GetValue() + "\"\n";
	}

	_bodyText += "}\n\n";
}

void CMissionInfoDecl::SaveToFile(idFile* file)
{
	file->Write(_bodyText.c_str(), _bodyText.Length());
}
