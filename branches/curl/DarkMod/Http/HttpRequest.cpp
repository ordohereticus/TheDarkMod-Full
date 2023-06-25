/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3965 $
 * $Date: 2010-06-22 09:04:10 -0400 (Tue, 22 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: HttpRequest.cpp 3965 2010-06-22 13:04:10Z greebo $", init_version);

#include "HttpRequest.h"

#include <winsock2.h> // greebo: need to include winsock2 before curl/curl.h
#include <curl/curl.h>

struct MemoryStruct
{
	char *memory;
	size_t size;
};
 
static void* myrealloc(void *ptr, size_t size)
{
	/* There might be a realloc() out there that doesn't like reallocing
	 NULL pointers, so we take care of it here */ 
	if (ptr)
		return realloc(ptr, size);
	else
		return malloc(size);
}
 
static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	mem->memory = static_cast<char*>(myrealloc(mem->memory, mem->size + realsize + 1));

	if (mem->memory)
	{
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	}

	return realsize;
}

CHttpRequest::CHttpRequest()
{
}
