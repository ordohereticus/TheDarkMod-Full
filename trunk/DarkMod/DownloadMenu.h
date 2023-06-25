/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 4407 $
 * $Date: 2011-01-10 21:39:10 -0500 (Mon, 10 Jan 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _DOWNLOAD_MENU_H_
#define	_DOWNLOAD_MENU_H_

#include <map>
#include <boost/shared_ptr.hpp>

// Handles mainmenu that displays list of downloadable missions
class CDownloadMenu
{
private:
	// The index of the first displayed mission
	int _availListTop;
	int _selectedListTop;

	idList<int> _selectedMissions;

	// A mapping "selected mission id" => "download id"
	typedef std::map<int, int> ActiveDownloads;
	ActiveDownloads _downloads;

public:
	CDownloadMenu();

	// handles main menu commands
	void HandleCommands(const idStr& cmd, idUserInterface* gui);

	// updates the GUI variables
	void UpdateGUI(idUserInterface* gui);

private:
	void StartDownload(idUserInterface* gui);

	void UpdateDownloadProgress(idUserInterface* gui);

	void ShowDownloadResult(idUserInterface* gui);

	void UpdateMissionDetails(idUserInterface* gui);

	void PerformScreenshotStep(idUserInterface* gui, int step);

	void UpdateNextScreenshotData(idUserInterface* gui, int nextScreenshotNum);
	void UpdateScreenshotItemVisibility(idUserInterface* gui);
};
typedef boost::shared_ptr<CDownloadMenu> CDownloadMenuPtr;

#endif	/* _DOWNLOAD_MENU_H_ */
