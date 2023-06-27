/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4636 $
 * $Date: 2011-02-26 13:03:16 -0500 (Sat, 26 Feb 2011) $
 * $Author: greebo $
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
