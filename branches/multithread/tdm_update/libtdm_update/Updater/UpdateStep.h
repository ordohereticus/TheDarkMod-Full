/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
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
