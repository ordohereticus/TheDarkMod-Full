/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
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
