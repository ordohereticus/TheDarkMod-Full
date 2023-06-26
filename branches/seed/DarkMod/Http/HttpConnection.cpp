/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4448 $
 * $Date: 2011-01-19 00:29:47 -0500 (Wed, 19 Jan 2011) $
 * $Author: stgatilov $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: HttpConnection.cpp 4448 2011-01-19 05:29:47Z stgatilov $", init_version);

#include "HttpConnection.h"
#include "HttpRequest.h"

#ifdef WIN32
#pragma warning(disable: 4800) // stgatilov: suppress "forcing value to bool" warning in WinSDK
#include <winsock2.h> // greebo: need to include winsock2 before curl/curl.h
#include <Ws2tcpip.h>
#include <Wspiapi.h>
#endif

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

bool CHttpConnection::HasProxy()
{
	return idStr::Length(cv_tdm_proxy.GetString()) > 0;
}

idStr CHttpConnection::GetProxyHost()
{
	return cv_tdm_proxy.GetString();
}

idStr CHttpConnection::GetProxyUsername()
{
	return cv_tdm_proxy_user.GetString();
}

idStr CHttpConnection::GetProxyPassword()
{
	return cv_tdm_proxy_pass.GetString();
}

CHttpRequestPtr CHttpConnection::CreateRequest(const std::string& url)
{
	return CHttpRequestPtr(new CHttpRequest(*this, url));
}

CHttpRequestPtr CHttpConnection::CreateRequest(const std::string& url, const std::string& destFilename)
{
	return CHttpRequestPtr(new CHttpRequest(*this, url, destFilename));
}
