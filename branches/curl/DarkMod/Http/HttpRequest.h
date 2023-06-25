/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3973 $
 * $Date: 2010-06-22 21:38:58 -0400 (Tue, 22 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <boost/shared_ptr.hpp>

class CHttpConnection;

typedef void CURL;

/**
 * greebo: An object representing a single HttpRequest, holding 
 * the result (string) and status information.
 *
 * Use the Perform() method to execute the request.
 */
class CHttpRequest
{
public:
	enum Status
	{
		NOT_PERFORMED_YET = -1,
		OK = 0,	// successful
		IN_PROGRESS,
		FAILED
	};

private:
	// The connection we're working with
	CHttpConnection& _conn;

	// The URL we're supposed to query
	std::string _url;

	std::vector<char> _buffer;

	// The curl handle
	CURL* _handle;

	// The current state
	Status _status;

public:
	CHttpRequest(CHttpConnection& conn, const std::string& url);

	// Callback for CURL
	static size_t WriteMemoryCallback(void* ptr, size_t size, size_t nmemb, CHttpRequest* self);

	Status GetStatus();

	// Perform the request
	void Perform();

	// Returns the result string
	std::string GetResultString();
};
typedef boost::shared_ptr<CHttpRequest> CHttpRequestPtr;

#endif /* _HTTP_REQUEST_H_ */
