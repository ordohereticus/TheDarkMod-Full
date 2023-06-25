/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3966 $
 * $Date: 2010-06-22 09:42:32 -0400 (Tue, 22 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: HttpConnection.cpp 3966 2010-06-22 13:42:32Z greebo $", init_version);

#include "HttpConnection.h"
#include "HttpRequest.h"

#include <winsock2.h> // greebo: need to include winsock2 before curl/curl.h
#include <curl/curl.h>

CHttpConnection::CHttpConnection()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

CHttpConnection::~CHttpConnection()
{
	// Clean up cURL
	curl_global_cleanup();
}

CHttpRequestPtr CHttpConnection::CreateRequest(const std::string& url)
{
	return CHttpRequestPtr(new CHttpRequest(*this, url));
}
