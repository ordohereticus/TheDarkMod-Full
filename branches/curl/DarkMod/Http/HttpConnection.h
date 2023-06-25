/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3965 $
 * $Date: 2010-06-22 09:04:10 -0400 (Tue, 22 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _HTTP_CONNECTION_H_
#define _HTTP_CONNECTION_H_

#include <boost/shared_ptr.hpp>

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
};
typedef boost::shared_ptr<CHttpConnection> CHttpConnectionPtr;

#endif /* _HTTP_CONNECTION_H_ */
