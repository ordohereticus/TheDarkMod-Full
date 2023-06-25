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

static bool init_version = FileVersionList("$Id: Download.cpp 4039 2010-07-11 04:41:50Z greebo $", init_version);

#include "Download.h"
#include "../Http/HttpConnection.h"

CDownload::CDownload(const idStr& url, const idStr& destFilename) :
	_url(url),
	_destFilename(destFilename),
	_status(NOT_STARTED_YET)
{}

CDownload::~CDownload()
{
	Stop();
}

CDownload::Status CDownload::GetStatus()
{
	return _status;
}

void CDownload::Start()
{
	_request = gameLocal.m_HttpConnection->CreateRequest(_url.c_str(), _destFilename.c_str());

	_request->Perform();

	if (_request->GetStatus() != CHttpRequest::OK)
	{
		gameLocal.Printf("Connection Error.\n");
		_status = FAILED;
	}
	else
	{
		_status = SUCCESS;
	}
}

void CDownload::Stop()
{
	if (_status == IN_PROGRESS)
	{
		// TODO
	}
}
