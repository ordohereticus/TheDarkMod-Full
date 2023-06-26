/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4650 $
 * $Date: 2011-03-04 13:18:20 -0500 (Fri, 04 Mar 2011) $
 * $Author: stgatilov $
 *
 ***************************************************************************/

#pragma once

namespace tdm
{

namespace updater
{

/**
 * The various update steps, in order.
 * During an update it's possible that some 
 * of these steps are skipped.
 */
enum UpdateStep
{
	Init,				// Startup, not doing anything yet
	CleanupPreviousSession,
	UpdateMirrors,
	DownloadCrcs,
	DownloadVersionInfo,
	CompareLocalFilesToNewest,
	DetermineLocalVersion,
	DownloadNewUpdater,
	DownloadDifferentialUpdate,
	PerformDifferentialUpdate,
	DownloadFullUpdate,
	PostUpdateCleanup,
	Done,
	RestartUpdater,
};

} // namespace

} // namespace
