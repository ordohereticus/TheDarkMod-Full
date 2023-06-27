/*************************************************************************
 *
 * PROJECT: The Dark Mod - Launcher
 * $Source$
 * $Revision: 5050 $
 * $Date: 2011-11-27 13:39:58 -0500 (Sun, 27 Nov 2011) $
 * $Author: greebo $
 *
 *************************************************************************/

#ifndef _TRACELOG_H_
#define _TRACELOG_H_

#include <fstream>
#include <string>

/** 
 * greebo: Public service class to write to logfile and/or console.
 */
class TraceLog
{
	// The file stream which will be filled with bytes
	std::ofstream _logStream;

public:
	TraceLog() :
		_logStream("tdmlauncher.log")
	{}

	~TraceLog();

	// Write to the logfile
	static void Write(const std::string& str);
	static void WriteLine(const std::string& str);

	// Contains the static singleton
	static TraceLog& Instance();
};

#endif /* TRACELOG */ 
