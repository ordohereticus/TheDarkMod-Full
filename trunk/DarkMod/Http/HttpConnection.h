/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4024 $
 * $Date: 2010-07-07 23:57:03 -0400 (Wed, 07 Jul 2010) $
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
 */
class CHttpConnection
{
public:
	CHttpConnection();

	~CHttpConnection();

	bool HasProxy();

	idStr GetProxyHost();
	idStr GetProxyUsername();
	idStr GetProxyPassword();

	/**
	 * Constructs a new HTTP request using the given URL
	 */ 
	CHttpRequestPtr CreateRequest(const std::string& url);
};
typedef boost::shared_ptr<CHttpConnection> CHttpConnectionPtr;

#endif /* _HTTP_CONNECTION_H_ */
