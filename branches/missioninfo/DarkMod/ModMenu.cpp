/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3930 $
 * $Date: 2010-06-10 03:47:44 -0400 (Thu, 10 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/
// Copyright (C) 2004 Id Software, Inc.
//

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ModMenu.cpp 3930 2010-06-10 07:47:44Z greebo $", init_version);

#include "ModMenu.h"
#include "../DarkMod/shop.h"
#include "../DarkMod/MissionData.h"
#include "../DarkMod/declxdata.h"
#include "boost/filesystem.hpp"
#include "../DarkMod/ZipLoader/ZipLoader.h"
#include "../DarkMod/Missions/MissionManager.h"

#include <string>
#ifdef _WINDOWS
#include <process.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

CModMenu::CModMenu() :
	_modTop(0)
{}

void CModMenu::Init()
{
	SearchForNewMods();
	BuildModList();
	InitCurrentMod();
	InitStartingMap();
}

void CModMenu::Clear()
{
	_modsAvailable.Clear();
	_curModName.Empty();
	_startingMap.Empty();
}

void CModMenu::Save(idSaveGame* savefile) const
{
	// Nothing to save yet
}

void CModMenu::Restore(idRestoreGame* savefile)
{
	// Nothing to restore yet
}

namespace fs = boost::filesystem;

#if 0
namespace {

	/**
	 * Private helper function reading the contents of the given file
	 * into a string.
	 */
	idStr readFile(const fs::path& fileName)
	{
		idStr returnValue;

		FILE* file = fopen(fileName.file_string().c_str(), "r");

		if (file != NULL)
		{
			// Get the filesize
			fseek(file, 0, SEEK_END);
			long len = ftell(file);
			fseek(file, 0, SEEK_SET);

			// Allocate a new buffer and read the contents
			char* buf = new char[len+1];
			fread(buf, len, 1, file);

			// NULL-terminate the string
			buf[len] = 0;

			// Copy the result into the idStr
			returnValue = buf;

			delete[] buf;

			fclose(file);
		}

		return returnValue;
	}

} // namespace
#endif

// Handle mainmenu commands
void CModMenu::HandleCommands(const char *menuCommand, idUserInterface *gui)
{
	idStr cmd = menuCommand;

	if (cmd == "refreshMissionList")
	{
		gameLocal.m_MissionManager->ReloadMissionList();

		// Update the GUI state
		UpdateGUI(gui);

		int numNewMissions = gameLocal.m_MissionManager->GetNumNewMissions();

		if (numNewMissions > 0)
		{
			if (numNewMissions > 1)
			{
				gui->SetStateString("newFoundMissionsText", va("%d new missions are available", numNewMissions));
			}
			else
			{
				gui->SetStateString("newFoundMissionsText", va("A new mission is available", numNewMissions));
			}

			gui->SetStateString("newFoundMissionsList", gameLocal.m_MissionManager->GetNewFoundMissionsText());
			gui->HandleNamedEvent("OnNewMissionsFound");

			gameLocal.m_MissionManager->ClearNewMissionList();
		}
	}
	else if (cmd == "loadModNotes")
	{
		// Get selected mod
		int modIndex = gui->GetStateInt("modSelected", "0") + _modTop;

		CMissionInfoPtr info = gameLocal.m_MissionManager->GetMissionInfo(modIndex);

		// Load the readme.txt contents, if available
		gui->SetStateString("ModNotesText", info != NULL ? info->GetMissionNotes() : "");
	}
	else if (cmd == "onMissionSelected")
	{
		UpdateSelectedMod(gui);
	}
	else if (cmd == "eraseSelectedModFromDisk")
	{
		int modIndex = gui->GetStateInt("modSelected", "0") + _modTop;

		CMissionInfoPtr info = gameLocal.m_MissionManager->GetMissionInfo(modIndex);

		if (info != NULL)
		{
			gameLocal.m_MissionManager->EraseModFolder(info->modName);
		}

		gui->HandleNamedEvent("OnSelectedMissionErasedFromDisk");

		// Update the selected mission
		UpdateSelectedMod(gui);
	}
	else if (cmd == "update")
	{
		gameLocal.Error("Deprecated update method called by main menu.");
	}
	else if (cmd == "modsNextPage")
	{
		// Scroll down a page
		_modTop += gui->GetStateInt("modsPerPage", "10");

		if (_modTop > _modsAvailable.Num())
		{
			_modTop = 0;
		}

		UpdateGUI(gui);
	}
	else if (cmd == "modsPrevPage")
	{
		// Scroll up a page
		_modTop -= gui->GetStateInt("modsPerPage", "10");

		if (_modTop < 0)
		{
			_modTop = 0;
		}

		UpdateGUI(gui);
	}
	else if (cmd == "darkmodLoad")
	{
		// Get selected mod
		int modIndex = gui->GetStateInt("modSelected", "0") + _modTop;

		InstallMod(modIndex, gui);
	}
	else if (cmd == "darkmodRestart")
	{
		// Get selected mod
		RestartGame();
	}
	else if (cmd == "briefing_show")
	{
		// Display the briefing text
		_briefingPage = 1;
		DisplayBriefingPage(gui);
	}
	else if (cmd == "briefing_scroll_down_request")
	{
		// Display the next page of briefing text
		_briefingPage++;
		DisplayBriefingPage(gui);
	}
	else if (cmd == "briefing_scroll_up_request")
	{
		// Display the previous page of briefing text
		_briefingPage--;
		DisplayBriefingPage(gui);
	}
	else if (cmd == "uninstallMod")
	{
		UninstallMod(gui);
	}
}

void CModMenu::UpdateSelectedMod(idUserInterface* gui)
{
	// Get selected mod
	int modIndex = gui->GetStateInt("modSelected", "0") + _modTop;

	CMissionInfoPtr info = gameLocal.m_MissionManager->GetMissionInfo(modIndex);

	if (info != NULL)
	{
		bool missionIsCurrentlyInstalled = _curModName == info->modName;
		
		// Don't display the install button if the mod is already installed
		gui->SetStateBool("installModButtonVisible", !missionIsCurrentlyInstalled);
		gui->SetStateBool("hasModNoteButton", info->HasMissionNotes());

		// Set the mod size info
		std::size_t missionSize = info->GetMissionFolderSize();
		idStr missionSizeStr = info->GetMissionFolderSizeString();
		gui->SetStateString("selectedModSize", missionSize > 0 ? missionSizeStr : "-");

		gui->SetStateBool("eraseSelectedModButtonVisible", missionSize > 0 && !missionIsCurrentlyInstalled);

		idStr eraseMissionText = va("You're about to delete the contents of the mission folder from your disk, including savegames and screenshots:"
			"\n\n%s\n\nNote that the downloaded mission PK4 in your darkmod/fms/ folder will not be "
			"affected by this operation, you're still able to re-install the mission.", info->GetMissionFolderPath().c_str());
		gui->SetStateString("eraseMissionText", eraseMissionText);

		gui->SetStateString("selectedModCompleted", info->GetMissionCompletedString());
		gui->SetStateString("selectedModLastPlayDate", info->GetKeyValue("last_play_date", "-"));
	}
	else
	{
		gui->SetStateBool("installModButtonVisible", false);
		gui->SetStateString("selectedModSize", "0 Bytes");
		gui->SetStateBool("eraseSelectedModButtonVisible", false);
		gui->SetStateBool("hasModNoteButton", false);
	}
}

// Displays the current page of briefing text
void CModMenu::DisplayBriefingPage(idUserInterface *gui)
{
	// look up the briefing xdata, which is in "maps/<map name>/mission_briefing"
	idStr briefingData = idStr("maps/") + cv_tdm_mapName.GetString() + "/mission_briefing";

	gameLocal.Printf("DisplayBriefingPage: briefingData is %s\n", briefingData.c_str());

	// Load the XData declaration
	const tdmDeclXData* xd = static_cast<const tdmDeclXData*>(
		declManager->FindType(DECL_XDATA, briefingData, false)
	);

	const char* briefing = "";
	bool scrollDown = false;
	bool scrollUp = false;

	if (xd != NULL)
	{
		gameLocal.Printf("DisplayBriefingPage: xdata found.\n");

		// get page count from xdata
		int numPages = xd->m_data.GetInt("num_pages");

		gameLocal.Printf("DisplayBriefingPage: numPages is %d\n", numPages);

		// ensure current page is between 1 and page count, inclusive
		_briefingPage = idMath::ClampInt(1, numPages, _briefingPage);

		// load up page text
		idStr page = va("page%d_body", _briefingPage);

		gameLocal.Printf("DisplayBriefingPage: current page is %d", _briefingPage);

		briefing = xd->m_data.GetString(page);

		// set scroll button visibility
		scrollDown = numPages > _briefingPage;
		scrollUp = _briefingPage > 1;
	}
	else
	{
		gameLocal.Warning("DisplayBriefingPage: Could not find briefing xdata: %s", briefingData.c_str());
	}

	// update GUI
	gui->SetStateString("BriefingText", briefing);
	gui->SetStateBool("ScrollDownVisible", scrollDown);
	gui->SetStateBool("ScrollUpVisible", scrollUp);
}

void CModMenu::UpdateGUI(idUserInterface* gui)
{
	int modsPerPage = gui->GetStateInt("modsPerPage", "10");

	// Display the name of each FM
	for (int modIndex = 0; modIndex < modsPerPage; ++modIndex)
	{
		idStr guiName = va("mod%d_name", modIndex);
		idStr guiDesc = va("mod%d_desc", modIndex);
		idStr guiAuthor = va("mod%d_author", modIndex);
		idStr guiImage = va("mod%d_image", modIndex);
		idStr guiAvailable = va("modAvail%d", modIndex);
		idStr guiCompleted = va("modCompleted%d", modIndex);

		int available = 0;

		int missionIndex = _modTop + modIndex;
		int numMissions = gameLocal.m_MissionManager->GetNumMissions();

		CMissionInfoPtr info;

		// Retrieve the mission info
		if (_modTop + modIndex < numMissions)
		{
			info = gameLocal.m_MissionManager->GetMissionInfo(missionIndex);
		}

		gui->SetStateInt(guiAvailable,	info != NULL ? 1 : 0);
		gui->SetStateString(guiName,	info != NULL ? info->displayName : "");
		gui->SetStateString(guiDesc,	info != NULL ? info->description : "");
		gui->SetStateString(guiAuthor,	info != NULL ? info->author : "");
		gui->SetStateString(guiImage,	info != NULL ? info->image : "");
		gui->SetStateBool(guiCompleted,	info != NULL ? info->MissionCompleted() : false);
	}

	gui->SetStateBool("isModsScrollUpVisible", _modTop != 0);
	gui->SetStateBool("isModsScrollDownVisible", _modTop + modsPerPage < _modsAvailable.Num());

	// Update the currently installed mod
	ModInfo curModInfo;

	if (!_curModName.IsEmpty())
	{
		curModInfo = GetModInfo(_curModName);
		gui->SetStateBool("hasCurrentMod", true);
	}
	else
	{
		curModInfo.title = "<No Mission Installed>";
		gui->SetStateBool("hasCurrentMod", false);
	}

	gui->SetStateString("currentModName", curModInfo.title);
	gui->SetStateString("currentModDesc", curModInfo.desc);
}

/*idStr CModMenu::GetModNotes(int modIndex)
{
	// Sanity check
	if (modIndex < 0 || modIndex >= _modsAvailable.Num())
	{
		return "";
	}

	idStr fmPath = cv_tdm_fm_path.GetString() + _modsAvailable[modIndex] + "/";

	// Check for the readme.txt file
	idStr notesFileName = fmPath + cv_tdm_fm_notes_file.GetString();

	char* buffer = NULL;

	if (fileSystem->ReadFile(notesFileName, reinterpret_cast<void**>(&buffer)) == -1)
	{
		// File not found
		return "";
	}

	idStr modNotes(buffer);
	fileSystem->FreeFile(buffer);

	return modNotes;
}*/

CModMenu::ModInfo CModMenu::GetModInfo(int modIndex)
{
	// Sanity check
	if (modIndex < 0 || modIndex >= _modsAvailable.Num())
	{
		return ModInfo();
	}

	const idStr& modDirName = _modsAvailable[modIndex];

	idStr fmPath = cv_tdm_fm_path.GetString() + modDirName + "/";

	// Check for the darkmod.txt file
	idStr descFileName = fmPath + cv_tdm_fm_desc_file.GetString();

	char* buffer = NULL;

	if (fileSystem->ReadFile(descFileName, reinterpret_cast<void**>(&buffer)) == -1)
	{
		// File not found
		return ModInfo();
	}

	idStr modFileContent(buffer);
	fileSystem->FreeFile(buffer);

	if (modFileContent.IsEmpty())
	{
		// Failed to find info
		return ModInfo();
	}

	ModInfo info;
	info.pathToFMPackage = fmPath;

	if (!modFileContent.IsEmpty())
	{
		int titlePos = modFileContent.Find("Title:", false);
		int descPos = modFileContent.Find("Description:", false);
		int authorPos = modFileContent.Find("Author:", false);
		int versionPos = modFileContent.Find("Required TDM Version:", false);

		int len = modFileContent.Length();

		if (titlePos >= 0)
		{
			info.title = idStr(modFileContent, titlePos, (descPos != -1) ? descPos : len);
			info.title.StripLeadingOnce("Title:");
			info.title.StripLeading(" ");
			info.title.StripLeading("\t");
			info.title.StripTrailingWhitespace();
		}

		if (descPos >= 0)
		{
			info.desc = idStr(modFileContent, descPos, (authorPos != -1) ? authorPos : len);
			info.desc.StripLeadingOnce("Description:");
			info.desc.StripLeading(" ");
			info.desc.StripLeading("\t");
			info.desc.StripTrailingWhitespace();
		}

		if (authorPos >= 0)
		{
			info.author = idStr(modFileContent, authorPos, (versionPos != -1) ? versionPos : len);
			info.author.StripLeadingOnce("Author:");
			info.author.StripLeading(" ");
			info.author.StripLeading("\t");
			info.author.StripTrailingWhitespace();
		}

		if (versionPos >= 0)
		{
			info.requiredVersionStr = idStr(modFileContent, versionPos, len);
			info.requiredVersionStr.StripLeadingOnce("Required TDM Version:");
			info.requiredVersionStr.StripLeading(" ");
			info.requiredVersionStr.StripLeading("\t");
			info.requiredVersionStr.StripTrailingWhitespace();

			// Parse version
			int dotPos = info.requiredVersionStr.Find('.');

			if (dotPos != -1)
			{
				info.requiredMajor = atoi(info.requiredVersionStr.Mid(0, dotPos));
				info.requiredMinor = atoi(info.requiredVersionStr.Mid(dotPos + 1, info.requiredVersionStr.Length() - dotPos));
			}
		}

		// Check for mod image
		if (fileSystem->ReadFile(info.pathToFMPackage + cv_tdm_fm_splashimage_file.GetString(), NULL) != -1)
		{
			idStr splashImageName = cv_tdm_fm_splashimage_file.GetString();
			splashImageName.StripFileExtension();

			info.image = info.pathToFMPackage + splashImageName;
		}
	}

	return info;
}

CModMenu::ModInfo CModMenu::GetModInfo(const idStr& modDirName)
{
	int index = _modsAvailable.FindIndex(modDirName);

	return (index != -1) ? GetModInfo(index) : ModInfo();
}

CModMenu::MoveList CModMenu::SearchForNewMods(const idStr& extension)
{
	idStr fmPath = cv_tdm_fm_path.GetString();
	idFileList* pk4files = fileSystem->ListFiles(fmPath, extension, false, true);

	MoveList moveList;

	fs::path darkmodPath = GetDarkmodPath();

	// Iterate over all found PK4s and check if they're valid
	for (int i = 0; i < pk4files->GetNumFiles(); ++i)
	{
		fs::path pk4path = darkmodPath / pk4files->GetFile(i);

		DM_LOG(LC_MAINMENU, LT_INFO)LOGSTRING("Found %s in FM root folder: %s\r", extension.c_str(), pk4path.file_string().c_str());

		// Does the PK4 file contain a proper description file?
		CZipFilePtr pk4file = CZipLoader::Instance().OpenFile(pk4path.file_string().c_str());

		if (pk4file == NULL)
		{
			DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Could not open PK4 in root folder: %s\r", pk4path.file_string().c_str());
			continue; // failed to open zip file
		}

		if (!pk4file->ContainsFile(cv_tdm_fm_desc_file.GetString()))
		{
			DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Ignoring PK4 file, no 'darkmod.txt' found inside archive: %s\r", pk4path.file_string().c_str());
			continue; // no darkmod.txt
		}

		// Deduce the mod folder name based on the PK4 name
		idStr modName = pk4path.leaf().c_str();
		modName.StripPath();
		modName.StripFileExtension();
		modName.ToLower();

		if (modName.IsEmpty()) continue; // error?

		// Clean modName string from any weird characters
		for (int i = 0; i < modName.Length(); ++i)
		{
			if (idStr::CharIsAlpha(modName[i]) || idStr::CharIsNumeric(modName[i])) continue;

			modName[i] = '_'; // replace non-ASCII keys with underscores
		}

		// Assemble the mod folder, e.g. c:/games/doom3/darkmod/fms/outpost
		fs::path modFolder = darkmodPath / cv_tdm_fm_path.GetString() / modName.c_str();

		// Create the fm folder, if necessary
		if (!fs::exists(modFolder))
		{
			DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Mod folder doesn't exist for PK4, creating: %s\r", modFolder.file_string().c_str());
			try
			{
				fs::create_directory(modFolder);
			}
			catch (fs::basic_filesystem_error<fs::path>& e)
			{
				DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Exception while creating folder for PK4: %s\r", e.what());
			}
		}

		// Move the PK4 to that folder
		fs::path targetPath = modFolder / (modName + ".pk4").c_str();

		// Remember to move this file as soon as we're done here
		moveList.push_back(MoveList::value_type(pk4path, targetPath));
	}

	fileSystem->FreeFileList(pk4files);

	return moveList;
}

void CModMenu::SearchForNewMods()
{
	// List all PK4s in the fms/ directory
	MoveList moveList = SearchForNewMods(".pk4");
	MoveList zipMoveList = SearchForNewMods(".zip");

	// Merge the zips into the pk4 list
	moveList.merge(zipMoveList);

	// greebo: Now that the file list has been freed, the D3 engine no longer holds locks on those files
	// and we can start moving them into their respective locations
	for (MoveList::const_iterator i = moveList.begin(); i != moveList.end(); ++i)
	{
		DoRemoveFile(i->second);

		try
		{
			fs::rename(i->first, i->second);
			DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Moved %s to %s\r", i->first.file_string().c_str(), i->second.file_string().c_str());
		}
		catch (fs::basic_filesystem_error<fs::path>& e)
		{
			DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Exception while moving file: %s\r", e.what());
		}

		fs::path targetPath = i->second;
		targetPath.remove_leaf();

		// Remove any darkmod.txt, splashimage etc. when copying a new PK4. It may contain updated versions of those.
		DoRemoveFile(targetPath / cv_tdm_fm_desc_file.GetString());
		DoRemoveFile(targetPath / cv_tdm_fm_startingmap_file.GetString());
		DoRemoveFile(targetPath / cv_tdm_fm_splashimage_file.GetString());
		DoRemoveFile(targetPath / cv_tdm_fm_notes_file.GetString());
	}
}

void CModMenu::BuildModList()
{
	// Clear the list first
	_modsAvailable.Clear();

	// List all folders in the fms/ directory
	idStr fmPath = cv_tdm_fm_path.GetString();
	idFileList* fmDirectories = fileSystem->ListFiles(fmPath, "/", false);

	for (int i = 0; i < fmDirectories->GetNumFiles(); ++i)
	{
		// The name of the FM directory below fms/
		idStr fmDir = fmDirectories->GetFile(i);

		DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Looking for %s file in %s.\r", cv_tdm_fm_desc_file.GetString(), (fmPath + fmDir).c_str());

		// Check for an uncompressed darkmod.txt file
		idStr descFileName = fmPath + fmDir + "/" + cv_tdm_fm_desc_file.GetString();

		if (fileSystem->ReadFile(descFileName, NULL) != -1)
		{
			// File exists, add this as available mod
			_modsAvailable.Alloc() = fmDir;
			continue;
		}

		// no "darkmod.txt" file found, check in the PK4 files
		DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("%s file not found, looking for PK4s.\r", descFileName.c_str());

		// Check for PK4s in that folder (and all subdirectories)
		idFileList* pk4files = fileSystem->ListFilesTree(fmPath + fmDir, ".pk4", false);

		DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("%d PK4 files found in %s.\r", pk4files->GetNumFiles(), (fmPath + fmDir).c_str());

		for (int j = 0; j < pk4files->GetNumFiles(); ++j)
		{
			fs::path pk4path = GetDarkmodPath() / pk4files->GetFile(j);

			CZipFilePtr pk4file = CZipLoader::Instance().OpenFile(pk4path.file_string().c_str());

			if (pk4file == NULL)
			{
				DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Could not open PK4: %s\r", pk4path.file_string().c_str());
				continue; // failed to open zip file
			}

			if (pk4file->ContainsFile(cv_tdm_fm_desc_file.GetString()))
			{
				// Hurrah, we've found the darkmod.txt file, extract the contents
				// and attempt to save to folder
				_modsAvailable.Alloc() = fmDir;

				fs::path darkmodPath = GetDarkmodPath();
				fs::path fmPath = darkmodPath / cv_tdm_fm_path.GetString() / fmDir.c_str();
				fs::path destPath = fmPath / cv_tdm_fm_desc_file.GetString();

				pk4file->ExtractFileTo(cv_tdm_fm_desc_file.GetString(), destPath.string().c_str());

				// Check for the other meta-files as well
				if (pk4file->ContainsFile(cv_tdm_fm_startingmap_file.GetString()))
				{
					destPath = fmPath / cv_tdm_fm_startingmap_file.GetString();
					pk4file->ExtractFileTo(cv_tdm_fm_startingmap_file.GetString(), destPath.string().c_str());
				}

				if (pk4file->ContainsFile(cv_tdm_fm_splashimage_file.GetString()))
				{
					destPath = fmPath / cv_tdm_fm_splashimage_file.GetString();
					pk4file->ExtractFileTo(cv_tdm_fm_splashimage_file.GetString(), destPath.string().c_str());
				}

				if (pk4file->ContainsFile(cv_tdm_fm_notes_file.GetString()))
				{
					destPath = fmPath / cv_tdm_fm_notes_file.GetString();
					pk4file->ExtractFileTo(cv_tdm_fm_notes_file.GetString(), destPath.string().c_str());
				}
			}
		}

		fileSystem->FreeFileList(pk4files);
	}

	fileSystem->FreeFileList(fmDirectories);

	gameLocal.Printf("Found %d mods in the FM folder.\n", _modsAvailable.Num());

	// Sort the mod list alphabetically
	SortModList();
}

// Compare functor to sort mods by title
int CModMenu::ModSortCompare(const int* a, const int* b)
{
	// Get the first mod title
	ModInfo aInfo = gameLocal.m_ModMenu->GetModInfo(*a);
	ModInfo bInfo = gameLocal.m_ModMenu->GetModInfo(*b);

	return aInfo.title.Icmp(bInfo.title);
}

void CModMenu::SortModList()
{
	// greebo: idStrList has a specialised algorithm, preventing me
	// from using a custom sort algorithm, hence this ugly thing here
	idList<int> indexList;

	indexList.SetNum(_modsAvailable.Num());
	for (int i = 0; i < _modsAvailable.Num(); ++i)
	{
		indexList[i] = i;
	}

	indexList.Sort( CModMenu::ModSortCompare );

	idStrList temp = _modsAvailable;

	for (int i = 0; i < indexList.Num(); ++i)
	{
		_modsAvailable[i] = temp[indexList[i]];
	}
}

void CModMenu::InitCurrentMod()
{
	idStr gameBase = cvarSystem->GetCVarString("fs_game_base");

	// We only have a mod if game_base is set correctly, otherwise we're in "darkmod".
	_curModName = (!gameBase.IsEmpty()) ? cvarSystem->GetCVarString("fs_game") : "";
}

void CModMenu::InitStartingMap()
{
	_startingMap.Empty();

	if (_curModName.IsEmpty())
	{
		return;
	}

	// Find out which is the starting map of the current mod
	fs::path doomPath(fileSystem->RelativePathToOSPath("", "fs_savepath"));
	doomPath /= "..";

	fs::path startingMapPath(cv_tdm_fm_path.GetString());
	startingMapPath = startingMapPath / _curModName.c_str() / cv_tdm_fm_startingmap_file.GetString();

	char* buffer = NULL;

	if (fileSystem->ReadFile(startingMapPath.string().c_str(), reinterpret_cast<void**>(&buffer)) != -1)
	{
		// We have a startingmap
		_startingMap = buffer;
		fileSystem->FreeFile(reinterpret_cast<void*>(buffer));

		cv_tdm_mapName.SetString(_startingMap);
	}
	else
	{
		gameLocal.Warning("No '%s' file for the current mod: %s", startingMapPath.string().c_str(), _curModName.c_str());
	}
}

fs::path CModMenu::GetDarkmodPath()
{
	return fs::path(g_Global.GetDarkmodPath());
}

bool CModMenu::DoCopyFile(const fs::path& source, const fs::path& dest, bool overwrite)
{
	if (overwrite)
	{
		try
		{
			// According to docs, remove() doesn't throw if file doesn't exist
			fs::remove(dest);
			DM_LOG(LC_MAINMENU, LT_INFO)LOGSTRING("Destination file %s already exists, has been removed before copying.\r", dest.string().c_str());
		}
		catch (fs::basic_filesystem_error<fs::path>& e)
		{
			// Don't care about removal error
			DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Caught exception while removing destination file %s: %s\r", dest.string().c_str(), e.what());
		}
	}

	// Copy the source file to the destination
	try
	{
		fs::copy_file(source, dest);
		DM_LOG(LC_MAINMENU, LT_INFO)LOGSTRING("File successfully copied to %s.\r", dest.string().c_str());

		return true;
	}
	catch (fs::basic_filesystem_error<fs::path>& e)
	{
		DM_LOG(LC_MAINMENU, LT_ERROR)LOGSTRING("Exception while coyping file from %s to %s: %s\r", 
			source.string().c_str(), dest.string().c_str(), e.what());

		return false;
	}
}

bool CModMenu::DoRemoveFile(const fs::path& fileToRemove)
{
	try
	{
		fs::remove(fileToRemove);
		DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Removed file in %s\r", fileToRemove.file_string().c_str());

		return true;
	}
	catch (fs::basic_filesystem_error<fs::path>& e)
	{
		DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Exception while removing file: %s\r", e.what());
		return false;
	}
}

void CModMenu::InstallMod(int modIndex, idUserInterface* gui)
{
	// Path to the parent directory
	fs::path parentPath(fileSystem->RelativePathToOSPath("", "fs_savepath"));
	parentPath = parentPath.remove_leaf().remove_leaf();

	// Sanity-check
	if (modIndex < 0 || modIndex >= _modsAvailable.Num())
	{
		return;
	}

	const idStr& modDirName = _modsAvailable[modIndex];

	ModInfo info = GetModInfo(modIndex);

	// Check the required TDM version of this FM
	if (info.requiredMajor > TDM_VERSION_MAJOR || info.requiredMinor > TDM_VERSION_MINOR)
	{
		gui->SetStateString("requiredVersionCheckFailText", 
			va("Cannot install this mission, as it requires\n%s v%d.%02d.\n\nYou are running %s v%d.%02d. Please run the tdm_update application to update your installation.",
			GAME_VERSION, info.requiredMajor, info.requiredMinor, GAME_VERSION, TDM_VERSION_MAJOR, TDM_VERSION_MINOR));
		gui->HandleNamedEvent("OnRequiredVersionCheckFail");
		return;
	}

	// Issue the named command to the GUI
	gui->SetStateString("modInstallProgressText", "Installing Mission Package\n\n" + info.title);
	gui->HandleNamedEvent("OnModInstallationStart");

	// Ensure that the target folder exists
	fs::path targetFolder = parentPath / modDirName.c_str();

	if (!fs::create_directory(targetFolder))
	{
		// Directory exists, not a problem, but log this
		DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("FM targetFolder already exists: %s\r", targetFolder.string().c_str());
	}

	// Path to the darkmod directory
	fs::path darkmodPath = GetDarkmodPath();

	// Copy all PK4s from the FM folder (and all subdirectories)
	idFileList* pk4Files = fileSystem->ListFilesTree(info.pathToFMPackage, ".pk4", false);

	for (int i = 0; i < pk4Files->GetNumFiles(); ++i)
	{
		// Source file (full OS path)
		fs::path pk4fileOsPath = GetDarkmodPath() / pk4Files->GetFile(i);

		// Target location
		fs::path targetFile = targetFolder / pk4fileOsPath.leaf();

		DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Copying file %s to %s\r", pk4fileOsPath.string().c_str(), targetFile.string().c_str());

		// Use boost::filesystem instead of id's (comments state that copying large files can be problematic)
		//fileeSystem->CopyFile(pk4fileOsPath, targetFile.string().c_str());

		// Copy the PK4 file and make sure any target file with the same name is removed beforehand
		DoCopyFile(pk4fileOsPath, targetFile, true);
	}

	fileSystem->FreeFileList(pk4Files);

	// Path to file that holds the current FM name
	fs::path currentFMPath(darkmodPath / cv_tdm_fm_current_file.GetString());

	DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Trying to save current FM name to %s\r", currentFMPath.file_string().c_str());

	// Save the name of the new mod
	FILE* currentFM = fopen(currentFMPath.file_string().c_str(), "w+");

	if (currentFM != NULL)
	{
		fputs(modDirName, currentFM);
		fclose(currentFM);
	}
	else
	{
		DM_LOG(LC_MAINMENU, LT_ERROR)LOGSTRING("Could not save current FM name to %s\r", currentFMPath.file_string().c_str());
	}

	DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Successfully saved current FM name to %s\r", currentFMPath.file_string().c_str());

	// Assemble the path to the FM's DoomConfig.cfg
	fs::path doomConfigPath = targetFolder / "DoomConfig.cfg";
	
	// Check if we should synchronise DoomConfig.cfg files
	if (cv_tdm_fm_sync_config_files.GetBool())
	{
		// Yes, sync DoomConfig.cfg

		// Always copy the DoomConfig.cfg from darkmod/ to the new mod/
		// Remove any DoomConfig.cfg that might exist there beforehand
		DoCopyFile(darkmodPath / "DoomConfig.cfg", doomConfigPath, true);
	}
	else
	{
		// No, don't sync DoomConfig.cfg, but at least copy a basic one over there if it doesn't exist
		if (!fs::exists(doomConfigPath))
		{
			DM_LOG(LC_MAINMENU, LT_INFO)LOGSTRING("DoomConfig.cfg not found in FM folder, copy over from darkmod.\r");

			DoCopyFile(darkmodPath / "DoomConfig.cfg", doomConfigPath);
		}
	}

	// Check if the config.spec file already exists in the mod folder
	fs::path configSpecPath = targetFolder / "config.spec";
	if (!fs::exists(configSpecPath))
	{
		// Copy the config.spec file from darkmod/ to the new mod/
		DoCopyFile(darkmodPath / "config.spec", configSpecPath);
	}

	gui->HandleNamedEvent("OnModInstallationFinished");
}

void CModMenu::UninstallMod(idUserInterface* gui)
{
	// To uninstall the current FM, just clear the FM name in currentfm.txt	

	// Path to the darkmod directory
	fs::path darkmodPath = GetDarkmodPath();

	// Path to file that holds the current FM name
	fs::path currentFMPath(darkmodPath / cv_tdm_fm_current_file.GetString());

	DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Trying to clear current FM name in %s\r", currentFMPath.file_string().c_str());

	if (DoRemoveFile(currentFMPath))
	{
		DM_LOG(LC_MAINMENU, LT_INFO)LOGSTRING("Current FM file removed: %s.\r", currentFMPath.string().c_str());
	}
	else
	{
		// Log removal error
		DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Caught exception while removing current FM file %s.\r", currentFMPath.string().c_str());
	}

	gui->HandleNamedEvent("OnModUninstallFinished");
}

void CModMenu::RestartGame()
{
	// Path to the parent directory
	fs::path parentPath(fileSystem->RelativePathToOSPath("", "fs_savepath"));
	parentPath = parentPath.remove_leaf().remove_leaf();

	// Path to the darkmod directory
	fs::path darkmodPath = GetDarkmodPath();

	// path to tdmlauncher
#ifdef _WINDOWS
	fs::path launcherExe(darkmodPath / "tdmlauncher.exe");
#else
	fs::path launcherExe(darkmodPath / "tdmlauncher.linux");
#endif

	if (!fs::exists(launcherExe))
	{
		gameLocal.Error("Could not find tdmlauncher!");
		return;
	}

	fs::path enginePath;

#ifdef _WINDOWS
	// Get the command line of the current process
	idStr cmdLine = GetCommandLine();

	int d3Pos = cmdLine.Find("DOOM3.exe", false);
	cmdLine = cmdLine.Mid(0, d3Pos + 9);
	cmdLine.StripLeadingOnce("\"");
	cmdLine.StripLeading(" ");
	cmdLine.StripLeading("\t");

	enginePath = cmdLine.c_str();
#else
	// TDM launcher needs to know where the engine is located, pass this as first argument
	char exepath[PATH_MAX] = {0};
	readlink("/proc/self/exe", exepath, sizeof(exepath));

	enginePath = fs::path(exepath);
#endif

	// command line to spawn tdmlauncher
	idStr commandLine(launcherExe.file_string().c_str());

	idStr engineArgument(enginePath.string().c_str());

	// greebo: Optional delay between restarts to fix sound system release issues in some Linux systems
	idStr additionalDelay = "";
	if (cv_tdm_fm_restart_delay.GetInteger() > 0)
	{
		additionalDelay = " --delay=";
		additionalDelay += cv_tdm_fm_restart_delay.GetString();
	}

#ifdef _WINDOWS
	// Create a tdmlauncher process, setting the working directory to the doom directory
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;

	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));

	siStartupInfo.cb = sizeof(siStartupInfo);

	CreateProcess(NULL, (LPSTR) (commandLine + " " + engineArgument + additionalDelay).c_str(), NULL, NULL,  false, 0, NULL,
		parentPath.file_string().c_str(), &siStartupInfo, &piProcessInfo);
#else
	// start tdmlauncher
	DM_LOG(LC_MAINMENU, LT_DEBUG)LOGSTRING("Starting tdmlauncher %s with argument %s\r", commandLine.c_str(), engineArgument.c_str());
	if (execlp(commandLine.c_str(), commandLine.c_str(), engineArgument.c_str(), "pause", NULL) == -1)
	{
		int errnum = errno;
		gameLocal.Error("execlp failed with error code %d: %s", errnum, strerror(errnum));
	}

	_exit(EXIT_FAILURE);
#endif

	// Issue the quit command to the game
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, "quit" );
}
