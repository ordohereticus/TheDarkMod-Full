/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4886 $
 * $Date: 2011-06-14 03:35:25 -0400 (Tue, 14 Jun 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "File.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/stat.h>
#endif

namespace tdm
{

void File::MarkAsExecutable(const fs::path& path)
{
#ifndef WIN32
	TraceLog::WriteLine(LOG_VERBOSE, "Marking file as executable: " + path.file_string());

	struct stat mask;
	stat(path.file_string().c_str(), &mask);

	mask.st_mode |= S_IXUSR|S_IXGRP|S_IXOTH;

	if (chmod(path.file_string().c_str(), mask.st_mode) == -1)
	{
		TraceLog::Error("Could not mark file as executable: " + path.file_string());
	}
#endif
}

}
