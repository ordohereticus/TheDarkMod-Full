/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4344 $
 * $Date: 2010-11-28 00:02:54 -0500 (Sun, 28 Nov 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include "Updater/Updater.h"
#include "Util.h"

namespace tdm
{

class FileProgressHandler :
	public updater::Updater::FileOperationProgress
{
private:
	updater::CurFileInfo _info;

	UpdaterDialog& _dialog;

public:
	FileProgressHandler(UpdaterDialog& dialog) :
		_dialog(dialog)
	{}

	void OnProgress(const updater::CurFileInfo& info)
	{
		_dialog.SetProgress(info.progressFraction);
		_dialog.SetProgressText(info.file.string());
	}

	void OnFinish()
	{
		_dialog.SetProgress(1.0);
	}
};

}
