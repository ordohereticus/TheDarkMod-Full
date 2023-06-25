/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3965 $
 * $Date: 2010-06-22 09:04:10 -0400 (Tue, 22 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <boost/shared_ptr.hpp>

/**
 * greebo: An object representing a single HttpRequest, holding 
 * the result (string) and status information.
 *
 * Use the Perform() method to execute the request.
 */
class CHttpRequest
{
public:
	CHttpRequest();
};
typedef boost::shared_ptr<CHttpRequest> CHttpRequestPtr;

#endif /* _HTTP_REQUEST_H_ */
