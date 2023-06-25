/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4344 $
 * $Date: 2010-11-28 00:02:54 -0500 (Sun, 28 Nov 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "TraceLog.h"

namespace tdm
{

void TraceLog::Register(const ILogWriterPtr& logWriter)
{
	_writers.insert(logWriter);
}

void TraceLog::Unregister(const ILogWriterPtr& logWriter)
{
	_writers.erase(logWriter);
}

void TraceLog::Write(LogClass lc, const std::string& output)
{
	TraceLog& log = Instance();

	for (LogWriters::const_iterator i = log._writers.begin(); i != log._writers.end(); ++i)
	{
		(*i)->WriteLog(lc, output);
	}
}

void TraceLog::WriteLine(LogClass lc, const std::string& output)
{
	TraceLog& log = Instance();

	std::string outputWithNewLine = output + "\n";

	for (LogWriters::const_iterator i = log._writers.begin(); i != log._writers.end(); ++i)
	{
		(*i)->WriteLog(lc, outputWithNewLine);
	}
}

TraceLog& TraceLog::Instance()
{
	static TraceLog _instance;
	return _instance;
}

} // namespace
