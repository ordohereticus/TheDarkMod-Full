/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4344 $
 * $Date: 2010-11-28 00:02:54 -0500 (Sun, 28 Nov 2010) $
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
