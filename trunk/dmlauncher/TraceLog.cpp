/*************************************************************************
 *
 * PROJECT: The Dark Mod - Launcher
 * $Source$
 * $Revision: 5050 $
 * $Date: 2011-11-27 13:39:58 -0500 (Sun, 27 Nov 2011) $
 * $Author: greebo $
 *
 *************************************************************************/

#include "TraceLog.h"

#include <iostream>

TraceLog::~TraceLog()
{
	_logStream << "Closing logfile." << std::endl;

	_logStream.flush();
	_logStream.close();
}

void TraceLog::Write(const std::string& str)
{
	Instance()._logStream << str;
	std::cout << str;
}

void TraceLog::WriteLine(const std::string& str)
{
	Instance()._logStream << str << std::endl;
	std::cout << str << std::endl;
}

TraceLog& TraceLog::Instance() 
{
	static TraceLog _instance;
	return _instance;
}
