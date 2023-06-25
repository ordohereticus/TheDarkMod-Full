/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4039 $
 * $Date: 2010-07-11 00:41:50 -0400 (Sun, 11 Jul 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: DownloadManager.cpp 4039 2010-07-11 04:41:50Z greebo $", init_version);

#include "DownloadManager.h"

void CDownloadManager::AddDownload(const CDownloadPtr& download)
{
	_downloads.push_back(download);

	// Go ahead and start the download (TODO)
	download->Start();
}

bool CDownloadManager::DownloadInProgress()
{
	return false;
}
