/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4044 $
 * $Date: 2010-07-11 10:14:08 -0400 (Sun, 11 Jul 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/
// Copyright (C) 2004 Id Software, Inc.
//

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: DownloadMenu.cpp 4044 2010-07-11 14:14:08Z greebo $", init_version);

#include "DownloadMenu.h"
#include "Missions/MissionManager.h"
#include "Missions/Download.h"
#include "Missions/DownloadManager.h"

CDownloadMenu::CDownloadMenu() :
	_availListTop(0),
	_selectedListTop(0)
{}

void CDownloadMenu::HandleCommands(const idStr& cmd, idUserInterface* gui)
{
	if (cmd == "mainmenu_heartbeat")
	{
		// Update download progress
		if (_selectedMissions.Num() > 0)
		{
			UpdateDownloadProgress(gui);
		}
	}
	else if (cmd == "refreshAvailableMissionList")
	{
		gui->HandleNamedEvent("onAvailableMissionsRefreshed"); // hide progress dialog in any case

		if (!cv_tdm_allow_http_access.GetBool() || gameLocal.m_HttpConnection == NULL)
		{
			// HTTP Access disallowed, display message
			gameLocal.Printf("HTTP requests disabled, cannot check for available missions.\n");

			GuiMessage msg;
			msg.type = GuiMessage::MSG_OK;
			msg.okCmd = "close_msg_box";
			msg.title = "Unable to contact Mission Archive";
			msg.message = "HTTP Requests have been disabled,\n cannot check for available missions.";

			gameLocal.AddMainMenuMessage(msg);

			return;
		}

		// Refresh list
		gameLocal.m_MissionManager->ReloadDownloadableMissions();

		UpdateGUI(gui);
	}
	else if (cmd == "onDownloadableMissionSelected")
	{
		int selectedMission = gui->GetStateInt("av_mission_selected");

		int missionIndex = selectedMission + _availListTop;

		// Update mission details
		const DownloadableMissionList& missions = gameLocal.m_MissionManager->GetDownloadableMissions();

		if (missionIndex > missions.Num()) return;

		gui->SetStateString("av_mission_title", missions[missionIndex].title);
		gui->SetStateString("av_mission_author", missions[missionIndex].author);
		gui->SetStateString("av_mission_release_date", missions[missionIndex].releaseDate);
		gui->SetStateString("av_mission_size", va("%0.1f MB", missions[missionIndex].sizeMB));

		gui->SetStateBool("av_mission_details_visible", true);

		gui->HandleNamedEvent("UpdateAvailableMissionColours");
	}
	else if (cmd == "onSelectMissionForDownload")
	{
		int selectedMission = gui->GetStateInt("av_mission_selected");
		int missionIndex = selectedMission + _availListTop;

		_selectedMissions.AddUnique(missionIndex);

		gui->SetStateInt("av_mission_selected", -1);
		gui->SetStateBool("av_mission_details_visible", false);

		UpdateGUI(gui);
	}
	else if (cmd == "onDeselectMissionForDownload")
	{
		int selectedItem = gui->GetStateInt("dl_mission_selected");
		int index = selectedItem + _selectedListTop;

		if (index > _selectedMissions.Num()) return;

		_selectedMissions.Remove(_selectedMissions[index]);

		UpdateGUI(gui);
	}
	else if (cmd == "onDownloadableMissionScrollUp")
	{
		int numMissionsPerPage = gui->GetStateInt("packagesPerPage", "5");
		_availListTop -= numMissionsPerPage;

		if (_availListTop < 0) _availListTop = 0;

		UpdateGUI(gui);
	}
	else if (cmd == "onDownloadableMissionScrollDown")
	{
		int numMissionsPerPage = gui->GetStateInt("packagesPerPage", "5");
		_availListTop += numMissionsPerPage;

		UpdateGUI(gui);
	}
	else if (cmd == "onSelectedMissionScrollUp")
	{
		int itemsPerPage = gui->GetStateInt("selectedPackagesPerPage", "5");
		_selectedListTop -= itemsPerPage;

		if (_selectedListTop < 0) _selectedListTop = 0;

		UpdateGUI(gui);
	}
	else if (cmd == "onSelectedMissionScrollDown")
	{
		int itemsPerPage = gui->GetStateInt("selectedPackagesPerPage", "5");
		_selectedListTop += itemsPerPage;

		UpdateGUI(gui);
	}
	else if (cmd == "onStartDownload")
	{
		StartDownload(gui);
	}
}

void CDownloadMenu::StartDownload(idUserInterface* gui)
{
	// Add a new download for each selected mission
	gui->SetStateBool("dl_button_visible", false);

	const DownloadableMissionList& missions = gameLocal.m_MissionManager->GetDownloadableMissions();

	for (int i = 0; i < _selectedMissions.Num(); ++i)
	{
		int missionIndex = _selectedMissions[i];

		if (missionIndex > missions.Num()) continue;

		const DownloadableMission& mission = missions[missionIndex];

		idStr url = mission.downloadLocations[0];
		idStr filename;
		url.ExtractFileName(filename);

		idStr targetPath = g_Global.GetDarkmodPath().c_str();
		targetPath += "/";
		targetPath += cv_tdm_fm_path.GetString();
		targetPath += filename;

		CDownloadPtr download(new CDownload(url, targetPath));

		int id = gameLocal.m_DownloadManager->AddDownload(download);

		// Store this ID
		_downloads[missionIndex] = id;
	}
}

void CDownloadMenu::UpdateGUI(idUserInterface* gui)
{
	const DownloadableMissionList& missions = gameLocal.m_MissionManager->GetDownloadableMissions();

	int numMissionsPerPage = gui->GetStateInt("packagesPerPage", "5");

	for (int i = 0; i < numMissionsPerPage; ++i)
	{
		// Apply page offset
		int missionIndex = i + _availListTop;
		bool missionExists = missionIndex < missions.Num();

		bool missionSelected = _selectedMissions.FindIndex(missionIndex) != -1;
		gui->SetStateBool(va("av_mission_avail_%d", i), missionExists && !missionSelected);
		gui->SetStateBool(va("av_mission_selected_%d", i), missionSelected);
		gui->SetStateString(va("av_mission_name_%d", i), missionExists ? missions[missionIndex].title : "");
	}

	gui->SetStateBool("av_mission_scroll_up_visible", _availListTop > 0);
	gui->SetStateBool("av_mission_scroll_down_visible", _availListTop + numMissionsPerPage < missions.Num());

	int numSelectedMissionsPerPage = gui->GetStateInt("selectedPackagesPerPage", "5");

	// Missions in the download queue
	for (int i = 0; i < numSelectedMissionsPerPage; ++i)
	{
		// Apply page offset
		int listIndex = i + _selectedListTop;
		bool listItemExists = listIndex < _selectedMissions.Num();

		// Get the referenced mission index, -1 ==> no mission
		int missionIndex = listItemExists ? _selectedMissions[listIndex] : -1;

		gui->SetStateBool(va("dl_mission_avail_%d", i), listItemExists);
		gui->SetStateString(va("dl_mission_name_%d", i), missionIndex != -1 ? missions[missionIndex].title : "");
	}

	gui->SetStateBool("dl_mission_scroll_up_visible", _selectedListTop > 0);
	gui->SetStateBool("dl_mission_scroll_down_visible", _selectedListTop + numSelectedMissionsPerPage < _selectedMissions.Num());

	gui->SetStateInt("dl_mission_count", _selectedMissions.Num());
	gui->SetStateBool("dl_button_available", _selectedMissions.Num() > 0); //  TODO: && noDownloadInProgress
	gui->SetStateBool("dl_button_visible", true);

	gui->HandleNamedEvent("UpdateAvailableMissionColours");
	gui->HandleNamedEvent("UpdateSelectedMissionColours");
}

void CDownloadMenu::UpdateDownloadProgress(idUserInterface* gui)
{
	int numSelectedMissionsPerPage = gui->GetStateInt("selectedPackagesPerPage", "5");

	// Missions in the download queue
	for (int i = 0; i < numSelectedMissionsPerPage; ++i)
	{
		// Apply page offset
		int listIndex = i + _selectedListTop;
		bool listItemExists = listIndex < _selectedMissions.Num();

		// Get the referenced mission index, -1 ==> no mission
		int missionIndex = listItemExists ? _selectedMissions[listIndex] : -1;

		if (!listItemExists || missionIndex == -1)
		{
			gui->SetStateString(va("dl_mission_progress_%d", i), "");
			continue;
		}

		// Find the download ID
		ActiveDownloads::const_iterator it = _downloads.find(missionIndex);

		if (it == _downloads.end())
		{
			gui->SetStateString(va("dl_mission_progress_%d", i), "queued ");
			continue;
		}
		
		CDownloadPtr download = gameLocal.m_DownloadManager->GetDownload(it->second);

		if (download == NULL) continue;

		switch (download->GetStatus())
		{
		case CDownload::NOT_STARTED_YET:
			gui->SetStateString(va("dl_mission_progress_%d", i), "queued ");
			break;
		case CDownload::FAILED:
			gui->SetStateString(va("dl_mission_progress_%d", i), "failed ");
			break;
		case CDownload::IN_PROGRESS:
			gui->SetStateString(va("dl_mission_progress_%d", i), va("%0.0f%s", download->GetProgressFraction()*100, "% "));
			break;
		case CDownload::SUCCESS:
			gui->SetStateString(va("dl_mission_progress_%d", i), "100% ");
			break;
		};
	}
}
