/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4039 $
 * $Date: 2010-07-11 00:41:50 -0400 (Sun, 11 Jul 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

#include "../Http/HttpRequest.h"
#include <boost/shared_ptr.hpp>

/**
 * An object representing a single download
 */
class CDownload
{
private:
	// The target URL we're downloading from
	idStr _url;

	// destination file
	idStr _destFilename;

public:
	enum Status
	{
		NOT_STARTED_YET,
		IN_PROGRESS,
		FAILED,
		SUCCESS,
	};

private:
	Status _status;

	// The corresponding HTTP request
	CHttpRequestPtr _request;

public:
	CDownload(const idStr& url, const idStr& destFilename);

	~CDownload();

	// Start the download
	void Start();

	// Cancel the download. If finished, this does nothing.
	void Stop();
	
	// The current status of this download
	Status GetStatus();
};
typedef boost::shared_ptr<CDownload> CDownloadPtr;

#endif /* _DOWNLOAD_H_ */
