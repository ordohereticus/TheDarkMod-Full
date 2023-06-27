/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4039 $
 * $Date: 2010-07-11 00:41:50 -0400 (Sun, 11 Jul 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _HTTP_CONNECTION_H_
#define _HTTP_CONNECTION_H_

#include <boost/shared_ptr.hpp>

class CHttpRequest;
typedef boost::shared_ptr<CHttpRequest> CHttpRequestPtr;

/**
 * greebo: An object representing a single HttpConnection, holding 
 * proxy settings and providing error handling.
 *
 * Use the CreateRequest() method to generate a new request object.
 *
 * TDM provides a single http connection object via the gameLocal class:
 *
 * gameLocal.m_HttpConnection->CreateRequest("http://www.thedarkmod.com");
 *
 * Note: the m_HttpConnection object can be NULL if HTTP requests have been
 * disabled by the user.
 */
class CHttpConnection
{
	friend class idGameLocal;

private:
	CHttpConnection();

public:
	~CHttpConnection();

	bool HasProxy();

	idStr GetProxyHost();
	idStr GetProxyUsername();
	idStr GetProxyPassword();

	/**
	 * Constructs a new HTTP request using the given URL (optional: filename)
	 */ 
	CHttpRequestPtr CreateRequest(const std::string& url);
	CHttpRequestPtr CreateRequest(const std::string& url, const std::string& destFilename);
};
typedef boost::shared_ptr<CHttpConnection> CHttpConnectionPtr;

#endif /* _HTTP_CONNECTION_H_ */
