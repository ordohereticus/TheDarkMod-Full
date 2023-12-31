/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "DownloadManager.h"

#include <iostream>

namespace tdm
{

DownloadManager::DownloadManager() :
	_nextAvailableId(1),
	_allDownloadsDone(true)
{}

int DownloadManager::AddDownload(const DownloadPtr& download)
{
	int id = _nextAvailableId++;

	_downloads[id] = download;

	_allDownloadsDone = false;

	return id;
}

DownloadPtr DownloadManager::GetDownload(int id)
{
	Downloads::iterator found = _downloads.find(id);

	return (found != _downloads.end()) ? found->second : DownloadPtr();
}

void DownloadManager::ClearDownloads()
{
	_downloads.clear();
}

bool DownloadManager::DownloadInProgress()
{
	for (Downloads::const_iterator i = _downloads.begin(); i != _downloads.end(); ++i)
	{
		if (i->second->GetStatus() == Download::IN_PROGRESS)
		{
			return true;
		}
	}

	return false;
}

bool DownloadManager::HasPendingDownloads()
{
	for (Downloads::const_iterator i = _downloads.begin(); i != _downloads.end(); ++i)
	{
		if (i->second->GetStatus() == Download::IN_PROGRESS ||
			i->second->GetStatus() == Download::NOT_STARTED_YET)
		{
			return true;
		}
	}

	return false;
}

void DownloadManager::RemoveDownload(int id)
{
	Downloads::iterator found = _downloads.find(id);

	if (found != _downloads.end()) 
	{
		_downloads.erase(found);
	}
}

int DownloadManager::GetCurrentDownloadId()
{
	for (Downloads::const_iterator i = _downloads.begin(); i != _downloads.end(); ++i)
	{
		if (i->second->GetStatus() == Download::IN_PROGRESS)
		{
			return i->first;
		}
	}

	return -1;
}

DownloadPtr DownloadManager::GetCurrentDownload()
{
	return GetDownload(GetCurrentDownloadId());
}

void DownloadManager::ProcessDownloads()
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
		if (i->second->GetStatus() == Download::NOT_STARTED_YET)
		{
			i->second->Start();
			return;
		}
	}

	// No download left to handle
	_allDownloadsDone = true;
}

bool DownloadManager::HasFailedDownloads()
{
	for (Downloads::const_iterator i = _downloads.begin(); i != _downloads.end(); ++i)
	{
		if (i->second->GetStatus() == Download::FAILED)
		{
			return true;
		}
	}

	return false;
}

void DownloadManager::ForeachDownload(DownloadVisitor& visitor)
{
	for (Downloads::const_iterator i = _downloads.begin(); i != _downloads.end(); ++i)
	{
		visitor.Visit(i->first, i->second);
	}
}

}
