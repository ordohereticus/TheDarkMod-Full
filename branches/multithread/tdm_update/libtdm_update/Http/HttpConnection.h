/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace tdm
{

class HttpRequest;
typedef boost::shared_ptr<HttpRequest> HttpRequestPtr;

/**
 * greebo: An object representing a single HttpConnection, holding 
 * proxy settings and providing error handling.
 *
 * Use the CreateRequest() method to generate a new request object.
 */
class HttpConnection
{
private:
	std::string _proxyHost;
	std::string _proxyUser;
	std::string _proxyPass;

	// a thread-safe counter, measuring the download bandwidth used
	std::size_t _bytesDownloaded;

	// The mutex for managing access to the counter above
	boost::mutex _bytesDownloadedMutex;

public:
	HttpConnection();
	~HttpConnection();

	bool HasProxy();

	std::string GetProxyHost();
	std::string GetProxyUsername();
	std::string GetProxyPassword();

	void SetProxyHost(const std::string& host);
	void SetProxyUsername(const std::string& user);
	void SetProxyPassword(const std::string& pass);

	// Add downlodded bytes to the counter
	void AddBytesDownloaded(std::size_t bytes);

	// Returns the total number of bytes downloaded through this connection
	std::size_t GetBytesDownloaded() const;

	/**
	 * Constructs a new HTTP request using the given URL (optional: filename)
	 */ 
	HttpRequestPtr CreateRequest(const std::string& url);
	HttpRequestPtr CreateRequest(const std::string& url, const std::string& destFilename);
};
typedef boost::shared_ptr<HttpConnection> HttpConnectionPtr;

}
