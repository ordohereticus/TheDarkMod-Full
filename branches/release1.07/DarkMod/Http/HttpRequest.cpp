/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5016 $
 * $Date: 2011-10-30 03:12:32 -0400 (Sun, 30 Oct 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: HttpRequest.cpp 5016 2011-10-30 07:12:32Z greebo $", init_version);

#include "HttpRequest.h"
#include "HttpConnection.h"

#ifdef WIN32
#pragma warning(disable: 4800) // stgatilov: suppress "forcing value to bool" warning in WinSDK
#include <winsock2.h> // greebo: need to include winsock2 before curl/curl.h
#include <Ws2tcpip.h>
#include <Wspiapi.h>
#endif

#include <curl/curl.h>

CHttpRequest::CHttpRequest(CHttpConnection& conn, const std::string& url) :
	_conn(conn),
	_url(url),
	_handle(NULL),
	_status(NOT_PERFORMED_YET),
	_cancelFlag(false),
	_progress(0)
{}

CHttpRequest::CHttpRequest(CHttpConnection& conn, const std::string& url, const std::string& destFilename) :
	_conn(conn),
	_url(url),
	_handle(NULL),
	_status(NOT_PERFORMED_YET),
	_destFilename(destFilename),
	_cancelFlag(false),
	_progress(0)
{}

void CHttpRequest::InitRequest()
{
	// Init the curl session
	_handle = curl_easy_init();

	// specify URL to get
	curl_easy_setopt(_handle, CURLOPT_URL, _url.c_str());

	// Connect the callback
	if (!_destFilename.empty())
	{
		curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, CHttpRequest::WriteFileCallback);
	}
	else
	{
		curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, CHttpRequest::WriteMemoryCallback);
	}

	// We pass ourselves as user data pointer to the callback function
	curl_easy_setopt(_handle, CURLOPT_WRITEDATA, this);

	// Set agent
	idStr agent = "The Dark Mod Agent/";
	agent += va("%d.%02d", TDM_VERSION_MAJOR, TDM_VERSION_MINOR);

#ifdef WIN32
	agent += " Win32";
#elif defined(__linux__)
	agent += " Linux";
#elif defined(MACOS_X)
	agent += " MacOSX";
#endif

	curl_easy_setopt(_handle, CURLOPT_USERAGENT, agent.c_str());

	// Get the proxy from the HttpConnection class
	if (_conn.HasProxy())
	{
		curl_easy_setopt(_handle, CURLOPT_PROXY, _conn.GetProxyHost().c_str());
		curl_easy_setopt(_handle, CURLOPT_PROXYUSERPWD, (_conn.GetProxyUsername() + ":" + _conn.GetProxyPassword()).c_str());
	}
}

void CHttpRequest::Perform()
{
	InitRequest();

	_progress = 0;
	_status = IN_PROGRESS;

	// Check target file
	if (!_destFilename.empty())
	{
		_destStream.open(_destFilename.c_str(), std::ofstream::out|std::ofstream::binary);
	}

	CURLcode result = curl_easy_perform(_handle);

	if (!_destFilename.empty())
	{
		_destStream.flush();
		_destStream.close();
	}

	if (_cancelFlag)
	{
		_status = ABORTED;
	}
	else
	{
		switch (result)
		{
		case CURLE_OK:
			_status = OK;
			_progress = 1.0;
			break;
		default:
			_status = FAILED;
		};
	}

	curl_easy_cleanup(_handle);

	_handle = NULL;
}

void CHttpRequest::Cancel()
{
	// The memory callback will catch this flag
	_cancelFlag = true;
}

CHttpRequest::RequestStatus CHttpRequest::GetStatus()
{
	return _status;
}

double CHttpRequest::GetProgressFraction()
{
	return _progress;
}

std::string CHttpRequest::GetResultString()
{
	return _buffer.empty() ? "" : std::string(&_buffer.front());
}

XmlDocumentPtr CHttpRequest::GetResultXml()
{
	XmlDocumentPtr doc(new pugi::xml_document);
	
	doc->load(GetResultString().c_str());

	return doc;
}

void CHttpRequest::UpdateProgress()
{
	double size;
	double downloaded;
	CURLcode result = curl_easy_getinfo(_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);

	if (result != CURLE_OK) 
	{
		_progress = 0;
		return;
	}

	result = curl_easy_getinfo(_handle, CURLINFO_SIZE_DOWNLOAD, &downloaded);

	if (result != CURLE_OK) 
	{
		_progress = 0;
		return;
	}

	_progress = downloaded / size;

	if (_progress > 1.0)
	{
		_progress = 1.0;
	}
}

size_t CHttpRequest::WriteMemoryCallback(void* ptr, size_t size, size_t nmemb, CHttpRequest* self)
{
	if (self->_cancelFlag)
	{
		return 0; // cancel the process
	}

	// Needed size
	std::size_t bytesToCopy = size * nmemb;

	std::vector<char>& buf = self->_buffer; // shortcut 

	std::size_t appendPosition = buf.size() > 0 ? buf.size() - 2 : 0;

	// The first allocation should request one extra byte for the trailing \0
	self->_buffer.resize(buf.size() > 0 ? buf.size() + bytesToCopy : bytesToCopy + 1);

	// Push the bytes
	memcpy(&(buf[appendPosition]), ptr, bytesToCopy);

	// Append trailing \0 if possible
	if (buf.size() > 0)
	{
		buf[buf.size() - 1] = 0;
	}

	self->UpdateProgress();

	return static_cast<size_t>(bytesToCopy);
}

size_t CHttpRequest::WriteFileCallback(void* ptr, size_t size, size_t nmemb, CHttpRequest* self)
{
	if (self->_cancelFlag)
	{
		return 0; // cancel the process
	}

	// Needed size
	std::size_t bytesToCopy = size * nmemb;

	self->_destStream.write(static_cast<const char*>(ptr), bytesToCopy);

	self->UpdateProgress();

	return static_cast<size_t>(bytesToCopy);
}
