/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include <fstream>
#include <vector>
#include <boost/shared_ptr.hpp>

typedef void CURL;

namespace tdm
{

class HttpConnection;

/**
 * greebo: An object representing a single HttpRequest, holding 
 * the result (string) and status information.
 *
 * Use the Perform() method to execute the request.
 */
class HttpRequest
{
public:

	enum RequestStatus
	{
		NOT_PERFORMED_YET,
		OK,	// successful
		IN_PROGRESS,
		FAILED,
		ABORTED,
	};

private:
	// The connection we're working with
	HttpConnection& _conn;

	// The URL we're supposed to query
	std::string _url;

	std::vector<char> _buffer;

	// The curl handle
	CURL* _handle;

	// The current state
	RequestStatus _status;

	std::string _destFilename;

	std::ofstream _destStream;

	// True if we should cancel the download
	bool _cancelFlag;

	double _progress;
	double _downloadSpeed;

	std::size_t _downloadedBytes;

	std::string _errorMessage;

public:
	HttpRequest(HttpConnection& conn, const std::string& url);

	HttpRequest(HttpConnection& conn, const std::string& url, const std::string& destFilename); // TODO: Change to fs::path

	// Callbacks for CURL
	static size_t WriteMemoryCallback(void* ptr, size_t size, size_t nmemb, HttpRequest* self);
	static size_t WriteFileCallback(void* ptr, size_t size, size_t nmemb, HttpRequest* self);

	RequestStatus GetStatus();

	// Perform the request
	void Perform();

	void Cancel();

	// Between 0.0 and 1.0
	double GetProgressFraction();

	// in bytes/second
	double GetDownloadSpeed();

	// Number of bytes received
	std::size_t GetDownloadedBytes();

	// Returns the result string
	std::string GetResultString();

	// If GetStatus == FAILED, this holds the curl error
	std::string GetErrorMessage();

private:
	void InitRequest();

	void UpdateProgress();
};
typedef boost::shared_ptr<HttpRequest> HttpRequestPtr;

}
