/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5092 $
 * $Date: 2011-12-06 12:30:27 -0500 (Tue, 06 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: DownloadManager.cpp 5092 2011-12-06 17:30:27Z greebo $", init_version);

#include "DownloadManager.h"

CDownloadManager::CDownloadManager() :
	_nextAvailableId(1),
	_allDownloadsDone(true)
{}

int CDownloadManager::AddDownload(const CDownloadPtr& download)
{
	int id = _nextAvailableId++;

	_downloads[id] = download;

	_allDownloadsDone = false;

	return id;
}

CDownloadPtr CDownloadManager::GetDownload(int id)
{
	Downloads::iterator found = _downloads.find(id);

	return (found != _downloads.end()) ? found->second : CDownloadPtr();
}

void CDownloadManager::ClearDownloads()
{
	_downloads.clear();
}

bool CDownloadManager::DownloadInProgress()
{
	for (Downloads::const_iterator i = _downloads.begin(); i != _downloads.end(); ++i)
	{
		if (i->second->GetStatus() == CDownload::IN_PROGRESS)
		{
			return true;
		}
	}

	return false;
}

void CDownloadManager::RemoveDownload(int id)
{
	Downloads::iterator found = _downloads.find(id);

	if (found != _downloads.end()) 
	{
		_downloads.erase(found);
	}
}

void CDownloadManager::ProcessDownloads()
{
	if (_allDownloadsDone || _downloads.empty()) 
	{
		return; // nothing to do
	}

	if (DownloadInProgress())
	{
		return; // download still in progress
	}

	// No download in progress, pick a new from the queue
	for (Downloads::const_iterator i = _downloads.begin(); i != _downloads.end(); ++i)
	{
		if (i->second->GetStatus() == CDownload::NOT_STARTED_YET)
		{
			DM_LOG(LC_MAINMENU, LT_INFO)LOGSTRING("Starting download: %i", i->first);

			i->second->Start();

			// Check if this download has a related one, if yes, launch both at once
			int relatedId = i->second->GetRelatedDownloadId();

			if (relatedId != -1)
			{
				CDownloadPtr related = GetDownload(relatedId);

				if (related)
				{
					DM_LOG(LC_MAINMENU, LT_INFO)LOGSTRING("Starting related download: %i", relatedId);

					related->Start();
				}
			}

			return;
		}
	}

	// No download left to handle
	_allDownloadsDone = true;
}
