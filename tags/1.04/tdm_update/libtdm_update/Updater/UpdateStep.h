/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4578 $
 * $Date: 2011-02-11 03:38:53 -0500 (Fri, 11 Feb 2011) $
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
