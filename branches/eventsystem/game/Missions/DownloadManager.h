/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5121 $ (Revision of last commit) 
 $Date: 2011-12-11 14:12:26 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#ifndef _MISSION_DOWNLOAD_MANAGER_H_
#define _MISSION_DOWNLOAD_MANAGER_H_

#include <map>
#include "Download.h"

/**
 * The class handling the actual mission downloads.
 */
class CDownloadManager
{
private:
	// Ongoing downloads
	typedef std::map<int, CDownloadPtr> Downloads;
	Downloads _downloads;

	int _nextAvailableId;

	bool _allDownloadsDone;

public:
	CDownloadManager();

	void ProcessDownloads();

	void ClearDownloads();

	int AddDownload(const CDownloadPtr& download);
	void RemoveDownload(int id);

	CDownloadPtr GetDownload(int id);

	// Returns true if there is a download already in progress
	bool DownloadInProgress();
};
typedef boost::shared_ptr<CDownloadManager> CDownloadManagerPtr;

#endif /* _MISSION_DOWNLOAD_MANAGER_H_ */
