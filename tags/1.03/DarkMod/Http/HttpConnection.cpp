/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4039 $
 * $Date: 2010-07-11 00:41:50 -0400 (Sun, 11 Jul 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: HttpConnection.cpp 4039 2010-07-11 04:41:50Z greebo $", init_version);

#include "HttpConnection.h"
#include "HttpRequest.h"

#ifdef WIN32
#include <winsock2.h> // greebo: need to include winsock2 before curl/curl.h
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
