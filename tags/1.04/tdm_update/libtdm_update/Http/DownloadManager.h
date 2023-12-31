/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include <map>
#include "Download.h"

namespace tdm
{

/**
 * The class handling the actual mission downloads.
 */
class DownloadManager
{
public:
	class DownloadVisitor
	{
	public:
		// Called per download object, with the assigned ID
		// The visitor should not change any downloads of the manager class
		virtual void Visit(int id, const DownloadPtr& download) = 0;
	};

private:
	// Ongoing downloads
	typedef std::map<int, DownloadPtr> Downloads;
	Downloads _downloads;

	int _nextAvailableId;

	bool _allDownloadsDone;

public:
	DownloadManager();

	void ProcessDownloads();

	void ClearDownloads();

	int AddDownload(const DownloadPtr& download);
	void RemoveDownload(int id);

	DownloadPtr GetDownload(int id);

	// Return the current in-progress download
	DownloadPtr GetCurrentDownload();

	// Return the ID of the current in-progress download
	int GetCurrentDownloadId();

	// Returns true if there is a download already in progress
	bool DownloadInProgress();

	// Returns true if there is a download in progress or waiting for start
	bool HasPendingDownloads();

	// Returns true if one or more downloads have failed status
	bool HasFailedDownloads();

	// Iterate over all registered downloads
	void ForeachDownload(DownloadVisitor& visitor);
};
typedef boost::shared_ptr<DownloadManager> DownloadManagerPtr;

}
