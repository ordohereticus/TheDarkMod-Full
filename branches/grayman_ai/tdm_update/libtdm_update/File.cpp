/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod Updater (http://www.thedarkmod.com/)
 
 $Revision: 5122 $ (Revision of last commit) 
 $Date: 2011-12-11 14:47:31 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

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
