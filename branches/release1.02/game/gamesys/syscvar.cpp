/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3891 $
 * $Date: 2010-04-28 13:11:55 -0400 (Wed, 28 Apr 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: syscvar.cpp 3891 2010-04-28 17:11:55Z greebo $", init_version);

#include "../game_local.h"

#if defined( _DEBUG )
	#define	BUILD_DEBUG	"-debug"
#else
	#define	BUILD_DEBUG "-release"
#endif

/*

All game cvars should be defined here.

*/

const char *si_gameTypeArgs[]		= { "singleplayer", "deathmatch", "Tourney", "Team DM", "Last Man", NULL };
const char *si_readyArgs[]			= { "Not Ready", "Ready", NULL }; 
const char *si_spectateArgs[]		= { "Play", "Spectate", NULL };

const char *ui_skinArgs[]			= { "skins/characters/player/marine_mp", "skins/characters/player/marine_mp_red", "skins/characters/player/marine_mp_blue", "skins/characters/player/marine_mp_green", "skins/characters/player/marine_mp_yellow", NULL };
const char *ui_teamArgs[]			= { "Red", "Blue", NULL }; 

struct gameVersion_s {
	gameVersion_s( void ) { sprintf( string, "%s.%d%s %s %s %s", ENGINE_VERSION, BUILD_NUMBER, BUILD_DEBUG, BUILD_STRING, __DATE__, __TIME__ ); }
	char	string[256];
} gameVersion;

idCVar g_version(					"g_version",				gameVersion.string,	CVAR_GAME | CVAR_ROM, "game version" );

/**
* DarkMod Cvars - see text description in declaration below for descriptions
**/
idCVar cv_player_spawnclass(		"tdm_player_spawnclass",	"atdm:player_thief",	CVAR_GAME, "The player's classname." );

idCVar cv_ai_sndvol(				"tdm_ai_sndvol",			"0.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Modifier to the volume of suspcious sounds that AI's hear.  Defaults to 0.0 dB" );
idCVar cv_ai_bark_show(				"tdm_ai_showbark",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "Displays the current sound when the AI starts barking" );
idCVar cv_ai_sight_prob(			"tdm_ai_sight_prob",		"0.7",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Modifies the AI's chance of seeing you.  Small changes may have a large effect." );
idCVar cv_ai_sight_mag(				"tdm_ai_sight_mag",			"1.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Modifies the amount of visual alert that gets added on when the sight probability check succeeds and the AI do see you (default 1.0)." );
idCVar cv_ai_sightmaxdist(			"tdm_ai_sightmax",			"60.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The distance (in meters) above which an AI will not see you even with a fullbright lightgem.  Defaults to 60m.  Effects visibility in a complicated way." );
idCVar cv_ai_sightmindist(			"tdm_ai_sightmin",			"11.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The distance (in meters) below which an AI has a 100% chance of seeing you with a fullbright lightgem.  Defaults to 11m (~36 ft).  Effects visibility in a complicated way." );
idCVar cv_ai_tactalert(				"tdm_ai_tact",				"20.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The default tactile alert if an AI bumps an enemy or an enemy bumps an AI.  Default value is 20 alert units (pretty much full alert)." );
idCVar cv_ai_bumpobject_impulse(	"tdm_ai_bumpobject_impulse", "1500",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Impulse applied when an AI bumps into an object with its AF when animating.  Different from the kick force it applies to an object blocking its path." );
idCVar cv_ai_debug(					"tdm_ai_debug",				"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL,  "If set to true, AI alert events will be sent to the console for debugging purposes." );
idCVar cv_ai_fov_show (				"tdm_ai_showfov",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, a debug graphic showing the field of vision of the AI will be drawn.  Blue cone represents the vertical FOV, orange cone represents the horizontal.");
idCVar cv_ai_ko_show (				"tdm_ai_showko",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, a debug graphic showing the knockout region of the AI will be drawn.  The green cone shows the vertical admittance angle, the red cone shows the horizontal angle.");
idCVar cv_ai_animstate_show (		"tdm_ai_showanimstate",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, debug text showing the name of the AI's current animation state will be shown.");
idCVar cv_ai_task_show (			"tdm_ai_showtasks",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, debug text showing the name of the AI's current tasks will be shown.");
idCVar cv_ai_alertlevel_show (		"tdm_ai_showalert",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, debug text showing the AI's current total alert units is shown (Note: This is not the alert state, use tdm_ai_showtasks for that).");
idCVar cv_ai_dest_show (			"tdm_ai_showdest",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, an arrow is drawn from every AI to its intended pathing destination.");
idCVar cv_ai_goalpos_show (			"tdm_ai_showgoalpos",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, the current goalpos (seekpos) is drawn in the world (!= move destination).");
idCVar cv_ai_aasarea_show (			"tdm_ai_showAASarea",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, the current AAS area of the AI is drawn in the world.");
idCVar cv_ai_debug_blocked (		"tdm_ai_debug_blocked",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, debug drawings of the movement subsystem are shown.");
idCVar cv_ai_door_show(				"tdm_ai_showdoor",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, the desired positions for opening and closing the door are shown.");
idCVar cv_ai_elevator_show(			"tdm_ai_showelevator",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to true, the debug drawings for AI handling elevators are shown.");

idCVar cv_ai_sight_thresh	(		"tdm_ai_sight_thresh",		"1.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This is the minimum light per-AI-frame gem generated visual stimulus amount required for an AI to be able to see the player or another entity directly when searching.");
idCVar cv_ai_sight_scale	(		"tdm_ai_sight_scale",		"1000.0",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This is the distance that is multiplied by the lightQuotient from the LAS and visual acuity of the AI scaled from 0 to 1, that indicates how far away the AI can be and see a location.");
idCVar cv_ai_show_enemy_visibility ("tdm_ai_show_enemy_visibility",	"0",		CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to 1, the visibility of the AI's enemy is drawn (red = obscured or hidden in darkness, green = the opposite).");
idCVar cv_ai_show_conversationstate("tdm_ai_show_conversationstate", "0",		CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to 1, the AI will draw debug output with regard to their conversation state.");

idCVar cv_ai_acuity_L3	(			"tdm_ai_acuity_L3",			"1.1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This is the amount by which the acutities of an AI is multiplied if the AI is at alert level 3 but not yet 4");
idCVar cv_ai_acuity_L4	(			"tdm_ai_acuity_L4",			"1.3",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This is the amount by which the acutities of an AI is multiplied if the AI is at alert level 4 but not yet 5");
idCVar cv_ai_acuity_L5	(			"tdm_ai_acuity_L5",			"1.5",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This is the amount by which the acutities of an AI is multiplied if the AI is at alert level 5");
idCVar cv_ai_acuity_susp(			"tdm_ai_acuity_susp",		"1.2",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This is the amount by which the acutities of an AI is multiplied if the AI is suspicious due to evidence. It is in addition to the other modifiers.");

idCVar cv_sndprop_disable(			"tdm_sndprop_disable",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL,  "If set to true, sound propagation will not be calculated." );
idCVar cv_spr_debug(				"tdm_spr_debug",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL,  "If set to true, sound propagation debugging information will be sent to the console, and the log information will become more detailed." );
idCVar cv_spr_show(					"tdm_showsprop",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL,  "If set to true, sound propagation paths to nearby AI will be shown as lines. The volume of the sound heard by the AI and the alert increase will be displayed." );
idCVar cv_spr_radius_show(			"tdm_showsprop_radius",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL,  "If set to true, sound ranges are drawn." );

idCVar cv_ko_show(					"tdm_showko",				"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL,  "If set to true, knockout zones will be shown for debugging." );
idCVar cv_ai_search_show (			"tdm_ai_search_show",		"0.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "If >= 1.0, this is the number of milliseconds for which a graphic showing search activity targets will be shown. If < 1.0 then the graphics will not be drawn. For debugging.");
idCVar cv_ai_visdist_show (			"tdm_ai_visdist_show",		"0.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "If >= 1.0, this is the number of milliseconds for which a graphic showing vision distance test results.  Green arrows indicate within range, Red indicate out of range, purple indicate gap between range and target. For debugging.");

idCVar cv_ai_opt_disable (			"tdm_ai_opt_disable",		"1",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AIs not in the Potentially Visible Set will be completely disabled if they have neverdormant set to 0.");
idCVar cv_ai_opt_noanims (			"tdm_ai_opt_noanims",		"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AIs will not animate.");
idCVar cv_ai_opt_novisualscan (			"tdm_ai_opt_novisualscan",	"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AIs not in the Potentially Visible Set will not look for enemies, not even enemy AIs.");
idCVar cv_ai_opt_forceopt (			"tdm_ai_opt_forceopt",		"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AIs will always be treated as being outside the PVS for the purposes of the other tdm_ai_opt_* spawnargs." );
idCVar cv_ai_opt_nothink (			"tdm_ai_opt_nothink",		"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AI will not perform their regular thinking routine (including Mind)." );
idCVar cv_ai_opt_interleavethinkmindist (			"tdm_ai_opt_interleavethinkmindist",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "If true (nonzero), the AI will start interleaved thinking if the distance to the player is greater than the set value." );
idCVar cv_ai_opt_interleavethinkmaxdist (			"tdm_ai_opt_interleavethinkmaxdist",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "If true (nonzero), this is the distance where interleave frame will reach its maximum value." );
idCVar cv_ai_opt_interleavethinkskippvscheck (		"tdm_ai_opt_interleavethinkskipPVS",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If true (nonzero), the player PVS check for interleaved thinking will be skipped, so that the AI can also do interleaved thinking while in view." );
idCVar cv_ai_opt_interleavethinkframes (			"tdm_ai_opt_interleavethinkframes",			"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "If true (nonzero), this is the maximum interleaved thinking frame number." );
idCVar cv_ai_opt_update_enemypos_interleave (		"tdm_ai_opt_update_enemypos_interleave",	"48",			CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "Time to pass between enemy position updates. Set this to 0 for updates each frame." );

idCVar cv_ai_opt_nomind (			"tdm_ai_opt_nomind",		"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AI has its Mind thinking routines disabled." );
idCVar cv_ai_opt_novisualstim (			"tdm_ai_opt_novisualstim",		"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AI will not process any incoming visual stimuli." );
idCVar cv_ai_opt_nolipsync (			"tdm_ai_opt_nolipsync",	"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AI will not play lipsync animations." );
idCVar cv_ai_opt_nopresent (			"tdm_ai_opt_nopresent",	"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AI will not be presented." );
idCVar cv_ai_opt_noobstacleavoidance (			"tdm_ai_opt_noobstacleavoidance",	"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), AI will not check for obstacles." );

idCVar cv_ai_debug_anims (			"tdm_ai_debug_anims",	"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), show debug info about AI anims in the console and log file." );

idCVar cv_show_health (          "tdm_show_health",   "0",            CVAR_ARCHIVE | CVAR_GAME | CVAR_BOOL, "If true (nonzero), show health of entities for debugging." );

idCVar cv_ai_show_aasfuncobstacle_state("tdm_ai_show_aasfuncobstacle_state",	"0", CVAR_ARCHIVE | CVAR_GAME | CVAR_BOOL, "If true (nonzero), idFuncAASObstacles will show their state at spawn time and during changes." );

idCVar cv_interaction_vfp_type("tdm_interaction_vfp_type",	"1", CVAR_ARCHIVE | CVAR_BOOL, "If 1, the enhanced interaction shaders are used. Set to 0 to use the default D3 one." );
idCVar cv_tdm_widescreenmode("tdm_wideScreenMode",	"0", CVAR_ARCHIVE | CVAR_INTEGER, "The widescreen mode selected in the main menu (for internal use)." );

idCVar cv_tdm_show_trainer_messages("tdm_show_trainer_messages", "1", CVAR_BOOL | CVAR_ARCHIVE, "Whether TDM trainer maps should display pop-ups with helpful gameplay information." );

idCVar cv_tdm_default_relations_def( "tdm_default_relations_def", "atdm:team_relations_default", CVAR_GAME | CVAR_ARCHIVE, "The name of the entityDef holding the TDM default team relationships." );
idCVar cv_tdm_mapName( "tdm_mapName", "", CVAR_GUI, "(internal) The map which is about to be started by the GUI." );
idCVar cv_tdm_fm_path( "tdm_fm_path", "fms/", CVAR_GUI, "(internal) The path where the fan mission packages are stored.");
idCVar cv_tdm_fm_desc_file( "tdm_fm_desc_file", "darkmod.txt", CVAR_GUI, "(internal) The description file name of FM packages.");
idCVar cv_tdm_fm_notes_file( "tdm_fm_notes_file", "readme.txt", CVAR_GUI, "(internal) The readme file name of FM packages.");
idCVar cv_tdm_fm_current_file( "tdm_fm_current_file", "currentfm.txt", CVAR_GUI, "(internal) The file where the name of the currently installed FM is stored.");
idCVar cv_tdm_fm_startingmap_file( "tdm_fm_startingmap_file", "startingmap.txt", CVAR_GUI, "(internal) The file where the name of the starting map of an FM is stored.");
idCVar cv_tdm_fm_splashimage_file( "tdm_fm_splashimage_file", "install_splash.tga", CVAR_GUI, "(internal) The file to be used as splash screen for the installation GUI.");
idCVar cv_tdm_fm_sync_config_files("tdm_fm_sync_config_files", "1", CVAR_BOOL | CVAR_ARCHIVE, "If true (1) the FM loader will sync DoomConfig.cfg files from the darkmod folder to the FM directory and back.");
idCVar cv_tdm_fm_restart_delay("tdm_fm_restart_delay",	"0", CVAR_ARCHIVE | CVAR_INTEGER, "If non-zero, this is the timespan in milliseconds to wait between D3 restarts (to let the previous D3 process fully release all resources)." );

idCVar cv_debug_aastype(			"tdm_debug_aastype",		"aas32",		CVAR_GAME | CVAR_ARCHIVE, "Sets the AAS type used for visualisation with impulse 27");

idCVar cv_las_showtraces(			"tdm_las_showtraces",		"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), traces from light origin to testpoints used for visibility testiung are drawn." );

idCVar cv_show_gameplay_time(		"tdm_show_gameplaytime",	"0",			CVAR_GAME | CVAR_BOOL, "If true (nonzero), the gameplay time is shown in the player HUD." );

idCVar cv_tdm_difficulty(			"tdm_difficulty",	"-1",					CVAR_GAME | CVAR_INTEGER, "Set this to 0, 1 or 2 to override the difficulty setting of any map (for testing purposes). Set this back to -1 to disable the setting (which is the default). This setting isn't saved between session." );

idCVar cv_sr_disable (				"tdm_sr_disable",           "0",           CVAR_GAME | CVAR_BOOL, "Set to 1 to disable all stim/response processing." );
idCVar cv_sr_show(					"tdm_show_stimresponse",    "0",           CVAR_GAME | CVAR_INTEGER, "Set to 1 to show all successful stims, set to 2 to show all including failed ones." );

idCVar cv_debug_mainmenu(			"tdm_debug_mainmenu",      "0",            CVAR_BOOL, "Set to 1 to enable main menu GUI debugging in the console." );
idCVar cv_mainmenu_confirmquit(		"tdm_mainmenu_confirmquit",      "1", CVAR_ARCHIVE | CVAR_BOOL, "Set to 0 to disable the 'Quit Game' confirmation dialog when exiting the game." );

idCVar cv_force_savegame_load(		"tdm_force_savegame_load", "0",   CVAR_BOOL|CVAR_ARCHIVE, "Set to 1 to skip code revision check on savegame load." );

/**
* Dark Mod player movement
* Use multipliers instead of setting a speed for each
**/
idCVar cv_pm_runmod(				"pm_runmod",			"2.12",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The multiplier used to obtain run speed from pm_walkspeed." );
idCVar cv_pm_run_backmod(			"pm_run_backmod",		"0.7",			CVAR_GAME | CVAR_FLOAT				,	"The multiplier applied to existing run speed when the player is running backwards." );
idCVar cv_pm_creepmod(				"pm_creepmod",			"0.44",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The multiplier used to obtain creep speed from pm_walkspeed." );
idCVar cv_pm_crouchmod(				"pm_crouchmod",			"0.54",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The multiplier used to obtain crouch speed from walk speed." );
idCVar cv_pm_max_swimspeed_mod(		"pm_max_swimspeed_mod",	"1.4",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Maximum speed of the player when moving in >= waist deep water, relative to player walkspeed." );
idCVar cv_pm_pushmod(				"pm_pushmod",			"0.15",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Alters the impulse applied when the player runs into an object. Fractional modifier that multiplies the default D3 impulse. ONLY APPLIES TO OBJECTS BEING KICKED. Default is 0.15" );
idCVar cv_pm_push_maximpulse(		"pm_push_maximpulse",	"300",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This is the maximum impulse that is allowed to be propagated by the player to moveables just by kicking them. Only applies for 'lightweight' moveables below playermass*pm_push_heavy_threshold. Default is 300 units*kg per second." );
idCVar cv_pm_push_start_delay(		"pm_push_start_delay",	"1000",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Defines the delay in msecs before heavy things get pushed by the player. Default is 1000 msecs." );
idCVar cv_pm_push_accel_time(		"pm_push_accel_time",	"1000",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Defines the acceleration time in msecs when the player is starting to push things. After this time has passed, the pushed object has reached its maximum possible velocity. Default is 1000 msecs." );
idCVar cv_pm_push_heavy_threshold(	"pm_push_heavy_threshold",	"0.15",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Defines the fraction of the player mass, above which pushable things are considered as 'heavy'. Default is 0.75." );
idCVar cv_pm_push_max_mass(			"pm_push_max_mass",		"200",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Defines the maximum mass in kg a moveable can have to be pushable at all. Default is 200 kg.");

idCVar cv_pm_weightmod(				"pm_weightmod",			"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Gets multiplied to the force applied to objects below the player model. Defaults to 1." );

idCVar cv_pm_mantle_reach(			"pm_mantle_reach",		"0.5",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Horizontal reach of mantle ability, as fraction of player height.  Default is 0.5" );
idCVar cv_pm_mantle_height(			"pm_mantle_height",		"0.2",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Vertical reach of mantle ability, as fraction of player height.  Default is 0.2" );
idCVar cv_pm_mantle_minflatness(		"pm_mantle_minflatness",		"0.707",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Cannot mantle on top of surfaces whose angle's cosine is smaller than this value. e.g. >1.0 means nothing can be mantled; 1.0 means only perfectly flat floors (0 degrees) can be mantled on top of; ~0.707 means no surfaces steeper than 45 degrees can be mantled on top of; 0.5 means no surfaces steeper than 60 degrees can be mantled on top of; a large negative value (like -10) means all surfaces can be mantled regardless of steepness. Default is 0.707." );


/**
* Dark Mod Jumping 
**/
idCVar cv_tdm_walk_jump_vel(			"tdm_walk_jump_vel",			"2.2",			CVAR_FLOAT, "Jump velocity multiplier when walking" );
idCVar cv_tdm_run_jump_vel(				"tdm_run_jump_vel",				"2.5",			CVAR_FLOAT, "Jump velocity multiplier when running" );
idCVar cv_tdm_crouch_jump_vel(			"tdm_crouch_jump_vel",			"0.5",			CVAR_FLOAT, "Jump velocity multiplier when crouching" );
idCVar cv_tdm_min_vel_jump(				"tdm_min_vel_jump",				"0.0",			CVAR_FLOAT, "The minimum speed before cv_tdm_run_jump_vel is used" );
idCVar cv_tdm_fwd_jump_vel(				"tdm_fwd_jump_vel",				"50.0",			CVAR_FLOAT, "Forward vector multiple for jumping" );
idCVar cv_tdm_backwards_jump_modifier(	"tdm_back_jump_factor",			"0.5",			CVAR_FLOAT, "Backwards multiplier for jumping, relative to forward jumping. A factor of 1 means that the player gains as much speed through backwards jumping as through forward jumping." );
idCVar cv_tdm_jump_relaxation_time(		"tdm_jump_relaxation_time",		"4",			CVAR_FLOAT, "Time in seconds needed to regain full jump strength again." );

idCVar cv_tdm_footfalls_movetype_specific( "tdm_footfall_sounds_movetype_specific", "1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "Set to 1 to use move-type dependent foot fall sounds." );

// Dark Mod crouching
idCVar cv_tdm_crouch_toggle(			"tdm_toggle_crouch",			"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "Set to 1 to make crouching toggleable." );
idCVar cv_tdm_crouch_toggle_hold_time(	"tdm_crouch_toggle_hold_time",	"400",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The time to hold crouch while on a rope/ladder for starting to slide down." );
idCVar cv_tdm_reattach_delay(			"tdm_reattach_delay",			"100",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Delay (in ms) for reattaching to ropes/ladders after detaching using crouch." );


/**
* Dark Mod Leaning
**/
idCVar cv_pm_lean_angle(			"pm_lean_angle",		"15.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The tilt angle that the player can lean to at a full lean, in degrees." );
idCVar cv_pm_lean_time(				"pm_lean_time",			"400.0",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Time it takes to get to a full lean, in milliseconds." );
idCVar cv_pm_lean_height(			"pm_lean_height",		"0.4",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Height of the fulcrum about which the player leans, as a fraction of the player's eye height." );
idCVar cv_pm_lean_stretch(			"pm_lean_stretch",		"2",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The amount the player stretches out at a full lean, written as a fraction of the player's un-stretched height." );
idCVar cv_pm_lean_forward_angle(	"pm_lean_forward_angle",	"2",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Forward lean: The tilt angle that the player can lean to at a full lean, in degrees." );
idCVar cv_pm_lean_forward_time(		"pm_lean_forward_time",		"400.0",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Forward lean: Time it takes to get to a full lean, in milliseconds." );
idCVar cv_pm_lean_forward_height(	"pm_lean_forward_height",	"0.4",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Forward lean: Height of the fulcrum about which the player leans, as a fraction of the player's eye height." );
idCVar cv_pm_lean_forward_stretch(	"pm_lean_forward_stretch",	"15",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Forward lean: The amount the player stretches out at a full lean, written as a fraction of the player's un-stretched height." );
idCVar cv_pm_lean_to_valid_increments( "pm_lean_to_valid_increments", "25",		CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "Integer number of increments used for testing when leaning back to a valid position after a leaned clipping problem.  The higher the number, the smoother the un-lean will feel, but the higher the computation time." );
idCVar cv_pm_lean_door_increments(	"pm_lean_door_increments", "10",			CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "Integer number of increments used for testing when extending a lean through a door acoustically." );
idCVar cv_pm_lean_door_max(			"pm_lean_door_max",		"40",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Max distance of door that may be listened through at normal incidence, in doom units." );
idCVar cv_pm_lean_door_bounds_exp(	"pm_lean_door_bounds_exp", "8.0",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Amount to expand the camera view bounds by when testing if the player is still pressed against a door for listening purposes (default 8.0)."); 
idCVar cv_pm_lean_toggle(			"pm_lean_toggle",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "Set to 1 to make leaning toggleable." );

/**
* Dark Mod Frobbing
* Frob expansion radius for easier frobbing, time it takes for frob highlight to fade in and out
**/
idCVar cv_frob_distance_default(	"tdm_frob_distance_default",	"63",	CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "Default frob distance for all items in doom units.  For developer use only, will break game if set wrong!  Default is 63.");
idCVar cv_frob_width(				"tdm_frob_width",		"10.0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "When frobbing, a cube of this dimension is created at the point the frob hit, and things within are frob candidates.  Makes frobbing easier but can go thru solid objects if set too high.  Default is 10.");
idCVar cv_frob_fadetime(			"tdm_frob_fadetime",	"100",		CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "Time it takes for frob highlight effect to fade in and out." );
idCVar cv_frob_debug_bounds(		"tdm_frob_debug_bounds", "0",		CVAR_GAME | CVAR_BOOL,					"Set to 1 to see a visualization of the bounds that are used to check for frobable items within them." );
idCVar cv_frob_weapon_selects_weapon(	"tdm_frob_weapon_selects_weapon", "0",		CVAR_GAME | CVAR_BOOL,	"Set to 1 to have weapons automatically selected when the respective item is picked up." );
idCVar cv_frob_debug_hud(	"tdm_frob_debug_hud", "0",		CVAR_GAME | CVAR_BOOL,	"Set to 1 to show some frobbing info." );

/**
* Dark Mod Misc. Control Options
**/
idCVar cv_weapon_next_on_empty(		"tdm_weapon_next_on_empty",	"0",	CVAR_GAME | CVAR_BOOL, "If set to 1, when you run out of a particular type of arrows, you will auto-switch to the next available weapon.  (This can get you in trouble if it switches to a fire arrow)." );

/**
* Physics
**/
idCVar cv_collision_damage_scale_vert(	"tdm_collision_damage_scale_vert", "1",	CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This globally scales the damage AI take from vertical collisions/decelerations. This multiplies delta-velocity squared." );
idCVar cv_collision_damage_scale_horiz(	"tdm_collision_damage_scale_horiz", "0.5",	CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This globally scales the damage AI take from horizontal collisions/decelerations. This multiplies delta-velocity squared." );
idCVar cv_drag_limit_force(				"tdm_drag_limit_force", "1", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "Cheat: Set to 0 to disable finite acceleration while grabbing objects." );
idCVar cv_drag_force_max(				"tdm_drag_force_max", "100000", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Maximum force the player can apply to a dragged object [kg * doom units / second^2]" );
idCVar cv_drag_stuck_dist(				"tdm_drag_stuck_dist", "38.0", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Distance from the grab point at which object is determined to be 'stuck' and possibly auto-dropped." );
idCVar cv_drag_damping(					"tdm_drag_damping", "0.0", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Damping applied to objects being grabbed by the player" );
idCVar cv_drag_damping_AF(				"tdm_drag_damping_af", "0.4", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Damping applied to ragdolls being grabbed by the player" );
idCVar cv_drag_AF_ground_timer(			"tdm_drag_af_ground_timer", "800", CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "Time in milliseconds that it takes to ramp up to full vertical velocity after a ground-restricted body has come back to ground contact." );
idCVar cv_drag_AF_free(					"tdm_drag_af_free", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "This is a cheat that allows lifting all AI bodies completely off the ground when dragging them.  Useful for mappers who want to set up ragdolls ingame." );
idCVar cv_drag_jump_masslimit(			"tdm_drag_jump_masslimit", "20", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "When the player is holding something above this mass, jumping becomes impossible." );
idCVar cv_drag_encumber_minmass(		"tdm_drag_encumber_minmass", "10", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Minimum carried mass that starts to effect movement speed." );
idCVar cv_drag_encumber_maxmass(		"tdm_drag_encumber_maxmass", "55", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Carried mass at which movement speed clamps to the lowest value." );
idCVar cv_drag_encumber_max(			"tdm_drag_encumber_max", "0.4", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Maximum encumbrance when carrying heavy things (expressed as a fraction of maximum unencumbered movement speed)." );
idCVar cv_dragged_item_highlight(		"tdm_dragged_item_highlight", "1", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "Set this to 1 (=TRUE) if the grabbed item should always be highlighted." );
idCVar cv_drag_debug(					"tdm_drag_debug", "0", CVAR_GAME | CVAR_BOOL, "Shows debug arrows for desired velocity and contact plane normals when moving objects with the grabber." );

idCVar cv_melee_debug(					"tdm_melee_debug", "0", CVAR_GAME | CVAR_BOOL, "Enable to show debug melee combat graphics." );
idCVar cv_melee_state_debug(			"tdm_melee_debug_state", "0", CVAR_GAME | CVAR_BOOL, "Enable to display debug text representing AI melee status." );
// ishtvan: Renamed these with a _ after the name to force their new default settings to take effect in release 1.01
idCVar cv_melee_mouse_thresh(			"tdm_melee_mouse_thresh_", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "In manual control mode, sets the mouse movement threshold needed to decide on which attack/parry to make" );
idCVar cv_melee_mouse_decision_time(	"tdm_melee_mouse_decision_time_", "100", CVAR_GAME | CVAR_INTEGER, "In manual control mode, holding the button for this many milliseconds, an attack/parry decision is made based on mouse motion, even if it is below threshold mouse motion." );
idCVar cv_melee_mouse_dead_time(		"tdm_melee_mouse_dead_time_", "120", CVAR_GAME | CVAR_INTEGER, "Controls how long after the button press that mouse motion is damped by the fraction in tdm_mouse_slowview.  In milliseconds." );
idCVar cv_melee_mouse_slowview(			"tdm_melee_mouse_slowview_", "0.0", CVAR_GAME | CVAR_FLOAT, "In manual control mode, when attack/parry button is held down, the view turn rate is decreased to this fraction while the mouse motion is determining which attack/parry to make." );
idCVar cv_melee_invert_attack(			"tdm_melee_invert_attack", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to 1, mouse motions are inverted when controlling which attack to make." ); 
idCVar cv_melee_invert_parry(			"tdm_melee_invert_parry", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to 1, mouse motions are inverted when controlling which parry to make." ); 
idCVar cv_melee_auto_parry(				"tdm_melee_auto_parry", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "If set to 1, melee parries are chosen automatically to match the enemy's attack (manual parry mode is much harder)." );
idCVar cv_melee_forbid_auto_parry(		"tdm_melee_forbid_auto_parry", "0", CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "If set to 1, auto parry is forced to be off.  Only manual allowed.  Works in conjunction with the main menu, do not adjust by hand." );
idCVar cv_melee_max_particles(			"tdm_melee_max_particles", "10", CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "Max number of particles a single melee swing can generate (eye candy setting)." );
idCVar cv_melee_difficulty(				"tdm_melee_difficulty", "normal", CVAR_GAME | CVAR_ARCHIVE, "Melee difficulty as set by the menu (Do not adjust ingame, cheater!!)" );

idCVar cv_phys_show_momentum(			"tdm_phys_show_momentum", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "Set this to 1 (=TRUE) to draw the linear impulse of (some) entities." );

/**
* DarkMod Item Manipulation
* Throw_min and throw_max are the min and max impulses applied to items thrown
**/
idCVar cv_throw_min(				"tdm_throw_min",			"600",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Minimum impulse applied to a thrown object." );
idCVar cv_throw_max(				"tdm_throw_max",			"3500",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Maximum impulse applied to a thrown object." );
idCVar cv_throw_time(				"tdm_throw_time",			"1200",			CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "When throwing an object, time it takes to charge up to the max throw force in milliseconds." );
idCVar cv_throw_max_vel(			"tdm_throw_max_vel",		"900",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Velocity of thrown objects is clamped to this value (in doomunits / second).  Needed to balance throwing of very light objects." );

idCVar cv_bounce_sound_max_vel(		"tdm_bounce_sound_max_vel",	"400",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "At this velocity moveable collision sounds reach their maximum volume." );
idCVar cv_bounce_sound_min_vel(		"tdm_bounce_sound_min_vel",	"80",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "This is the minimum velocity at which moveable collision sounds can be heard at all." );


idCVar cv_reverse_grab_control(		"tdm_grabber_reverse_control",	"0",	CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "Set to 1 to reverse the direction when using next/prev weapon to increase/decrease the distance of the grabbed item." );

/**
* DarkMod Inventory
**/
idCVar cv_tdm_inv_gui_file(	"tdm_inv_hud_file", "guis/tdm_inv.gui",	CVAR_GAME, "The name of the gui file that represents the hud for the inventory.");
idCVar cv_tdm_inv_loot_item_def("tdm_inv_loot_item_def", "atdm:inv_loot_info_item", CVAR_GAME, "The name of the entityDef that defines the player's inventory loot item.");

idCVar cv_tdm_obj_gui_file(	"tdm_obj_hud_file", "guis/tdm_objectives.gui",	CVAR_GAME, "The name of the gui file that defines the in-game objectives.");

idCVar cv_tdm_hud_opacity(	"tdm_hud_opacity", "0.7",	CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT,	"The opacity of the HUD GUIs. [0..1]", 0, 1 );
idCVar cv_tdm_hud_hide_lightgem(	"tdm_hud_hide_lightgem", "0",	CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT,	"If set to true (=1), the lightgem will be hidden.", 0, 1 );
idCVar cv_tdm_inv_hud_pickupmessages(	"tdm_inv_hud_pickupmessages", "1",	CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT,	"If set to 1, the HUD is displaying the item the player has just picked up.");
idCVar cv_tdm_inv_loot_sound("tdm_inv_loot_sound", "frob_loot",	CVAR_GAME | CVAR_ARCHIVE, "The name of the sound that is to be played when loot has been acquired.");
idCVar cv_tdm_inv_use_on_frob("tdm_inv_use_on_frob", "0",	CVAR_GAME | CVAR_ARCHIVE, "When set to '1' currently selected inventory items will be used on frob.");
idCVar cv_tdm_door_control("tdm_door_control", "0",			CVAR_GAME | CVAR_ARCHIVE, "Acivates experimental door control.  When active, hold down frob and move mouse to fine-control a door.");
idCVar cv_tdm_door_control_sensitivity( "tdm_door_control_sensitivity", "0.01", CVAR_GAME, "Sets fine door control mouse sensitivity." );
idCVar cv_tdm_inv_use_visual_feedback("tdm_inv_use_visual_feedback", "1",	CVAR_GAME | CVAR_ARCHIVE, "When set to '1' the HUD is giving visual feedback when the currently selected item is used on the highlighted one.");


idCVar cv_tdm_rope_pull_force_factor("tdm_rope_pull_force_factor", "140", CVAR_GAME | CVAR_ARCHIVE, "The factor by which the pulling force when jumping on a rope gets multiplied.");

idCVar cv_tdm_underwater_blur("tdm_underwater_blur", "3", CVAR_GAME | CVAR_ARCHIVE, "The strength of the blur effect when the player is underwater.");

/**
* DarkMod movement volumes.  Walking volume is zero dB, other volumes are added to that
**/
idCVar cv_pm_stepvol_walk(		"pm_stepvol_walk",	"0",					CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Audible volume modifier for walking footsteps (should be 0.0)" );
idCVar cv_pm_stepvol_run(		"pm_stepvol_run",	"8",					CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Audible modifier for running footsteps." );
idCVar cv_pm_stepvol_creep(		"pm_stepvol_creep",	"-5",					CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Audible modifier for creeping footsteps." );

idCVar cv_pm_stepvol_crouch_walk(	"pm_stepvol_crouch_walk",	"-2",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Audible volume modifier for crouch walking footsteps" );
idCVar cv_pm_stepvol_crouch_run(	"pm_stepvol_crouch_run",	"4",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Audible volume modifier for crouch running footsteps" );
idCVar cv_pm_stepvol_crouch_creep(	"pm_stepvol_crouch_creep",	"-7",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Audible volume modifier for crouch creeping footsteps" );
idCVar cv_pm_min_stepsound_interval("pm_min_stepsound_interval",	"200",		CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "The minimum time in msec which has to pass before the next player footstep sound is allowed to be played." );
idCVar cv_pm_rope_snd_rep_dist(		"pm_rope_snd_rep_dist",		"32",		CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "When climbing a rope, this sets the vertical distance in doomunits between repeats of the rope climbing sound (default 32 [du])." );
idCVar cv_pm_rope_velocity_letgo(	"pm_rope_velocity_letgo",	"100",		CVAR_GAME | CVAR_FLOAT, "Maximum allowed velocity of the rope before the player lets go." );
idCVar cv_pm_rope_swing_impulse(	"pm_rope_swing_impulse",	"360000",	CVAR_GAME | CVAR_FLOAT, "Impulse applied to rope when the player 'kicks their legs' to swing on the rope. (Warning: Setting too high can damage player)" );
idCVar cv_pm_rope_swing_reptime(	"pm_rope_swing_reptime",	"500",		CVAR_GAME | CVAR_INTEGER, "How often you can kick to swing on the rope, in milliseconds." );
idCVar cv_pm_rope_swing_kickdist(	"pm_rope_swing_kickdist",	"36",		CVAR_GAME | CVAR_FLOAT, "When kicking on a rope, check this far ahead of the player's anchor point in the kick direction, to see if they are kicking off of something." );
idCVar cv_pm_water_downwards_velocity(	"pm_water_downwards_velocity",	"-4",		CVAR_GAME | CVAR_FLOAT, "The factor which the gravity vector gets scaled with to calculate the standard downwards velocity in water volumes. Negative values will let the player float upwards." );
idCVar cv_pm_water_z_friction(	"pm_water_z_friction",	"0.995",		CVAR_GAME | CVAR_FLOAT , "When the player is underwater and has really small z-velocities, this factor gets applied each frame, so that the player stops floating upwards when reaching the surface." );
idCVar cv_pm_show_waterlevel(	"pm_show_waterlevel",	"0",		CVAR_GAME | CVAR_BOOL , "Shows the waterlevel of the player." );
idCVar cv_pm_climb_distance(	"pm_climb_distance",	"10",		CVAR_GAME | CVAR_FLOAT , "How far away the edge of the player clipbox is from climbed surfaces (e.g. ladders)" );

/**
 * Darkmod lightgem variables. These are only for debuggingpurpose to tweak the lightgem
 * in a release version they should be disabled.
 */
idCVar cv_lg_distance("tdm_lg_distance",	"0",		CVAR_GAME | CVAR_FLOAT,	"Sets the distance for camera of the lightgem testmodel." );
idCVar cv_lg_xoffs("tdm_lg_xoffs",		"0",		CVAR_GAME | CVAR_FLOAT,	"Sets the x adjustment value for the camera on the testmodel" );
idCVar cv_lg_yoffs("tdm_lg_yoffs",		"0",		CVAR_GAME | CVAR_FLOAT,	"Sets the y adjustment value for the camera on the testmodel" );
idCVar cv_lg_zoffs("tdm_lg_zoffs",		"17",		CVAR_GAME | CVAR_FLOAT,	"Sets the z adjustment value for the camera on the testmodel" );
idCVar cv_lg_oxoffs("tdm_lg_oxoffs",		"0",		CVAR_GAME | CVAR_FLOAT,	"Sets the x adjustment value for the testmodels object position" );
idCVar cv_lg_oyoffs("tdm_lg_oyoffs",		"0",		CVAR_GAME | CVAR_FLOAT,	"Sets the y adjustment value for the testmodels object position" );
idCVar cv_lg_ozoffs("tdm_lg_ozoffs",		"-20",		CVAR_GAME | CVAR_FLOAT,	"Sets the z adjustment value for the testmodels object position" );
idCVar cv_lg_fov("tdm_lg_fov",			"70",		CVAR_GAME | CVAR_INTEGER,	"Sets the y value for the field of view on the lightgem testmodel." );
idCVar cv_lg_interleave("tdm_lg_interleave",	"1",	CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE,		"If set to 0 no lightgem processing is done. Any other values determines how often the lightgem should be processed.\n1 (default) means to process every frame." );
idCVar cv_lg_hud("tdm_lg_hud",			"0",		CVAR_GAME | CVAR_INTEGER,	"Shows the rendersnaphost n = <1..6> of the lightgem on-screen. If 0 none is shown." );
idCVar cv_lg_weak("tdm_lg_weak",			"0",		CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE,		"Switches to the weaker algorithm, but may be faster." );
idCVar cv_lg_player("tdm_lg_player",		"0",		CVAR_GAME | CVAR_BOOL,		"Shows the lightem testmodel in the gamescreen if set to 1." );
idCVar cv_lg_renderpasses("tdm_lg_renderpasses",		"2",	CVAR_GAME | CVAR_INTEGER,	"Set number of renderpasses used for the lightgem calculation (1..2)" );
idCVar cv_lg_debug("tdm_lg_debug",		"0",		CVAR_GAME | CVAR_BOOL,	"switch on debug prints." );
idCVar cv_lg_model("tdm_lg_model",		"models/darkmod/misc/system/lightgem.lwo",	CVAR_GAME | CVAR_ARCHIVE,	"Set the lightgem model file. Map has to be restarted to take effect." );
idCVar cv_lg_adjust("tdm_lg_adjust",		"0",		CVAR_GAME | CVAR_FLOAT,	"Adds a constant value to the lightgem." );
idCVar cv_lg_split("tdm_lg_split",		"0",		CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE,	"Lightgem is always fully calculated (no splitting between interleaves). Warning! This can cause particle flickering if set to 1." );
idCVar cv_lg_path("tdm_lg_path",		"",	CVAR_GAME,	"Dump the rendersnapshot to the filepath specified here." );
idCVar cv_lg_crouch_modifier("tdm_lg_crouch_modifier",	"-3",	CVAR_GAME | CVAR_INTEGER,	"The value the lightgem is adjusted by when the player is crouching." );
idCVar cv_lg_image_width("tdm_lg_image_width",	"64",	CVAR_GAME | CVAR_INTEGER,		"Defines the pixel width of the lightem rendering. WARNING!!! Increasing this severly affects performance!" );
idCVar cv_lg_screen_width("tdm_lg_screen_width",	"640",	CVAR_GAME | CVAR_INTEGER,		"Defines the screen width of the lightem rendering. WARNING!!! Increasing this severly affects performance!" );
idCVar cv_lg_screen_height("tdm_lg_screen_height",	"480",	CVAR_GAME | CVAR_INTEGER,		"Defines the screen height of the lightem rendering. WARNING!!! Increasing this severly affects performance!" );
idCVar cv_lg_velocity_mod_min_velocity("tdm_lg_velocity_mod_min_velocity", "0", CVAR_GAME | CVAR_FLOAT, "The minimum velocity the player must be at to make the lightgem level increase.");
idCVar cv_lg_velocity_mod_max_velocity("tdm_lg_velocity_mod_max_velocity", "300", CVAR_GAME | CVAR_FLOAT, "The maximum player speed taken into account for the lightgem.");
idCVar cv_lg_velocity_mod_amount("tdm_lg_velocity_mod_amount", "1", CVAR_GAME | CVAR_FLOAT, "The maximum light level increase factor due to player velocity (this will be multiplied when the player velocity is >= tdm_lg_player_velocity_mod_max).");

idCVar cv_empty_model("tdm_empty_model", "models/darkmod/misc/system/empty.lwo", CVAR_GAME | CVAR_ARCHIVE, "The empty model referenced by the 'waitForRender' script event.");

// Proxy CVARs
idCVar cv_tdm_s_doorDistanceAdd("tdm_s_doorDistanceAdd", "450", CVAR_GAME | CVAR_ARCHIVE, "This value will overwrite s_doorDistanceAdd at game startup.");
idCVar cv_tdm_gui_smallFontLimit("tdm_gui_smallFontLimit", "0.15", CVAR_GAME | CVAR_ARCHIVE, "This value will overwrite gui_smallFontLimit at game startup.");
idCVar cv_tdm_gui_mediumFontLimit("tdm_gui_mediumFontLimit", "0.30", CVAR_GAME | CVAR_ARCHIVE, "This value will overwrite gui_mediumFontLimit at game startup.");
idCVar cv_tdm_s_maxSoundsPerShader("tdm_s_maxSoundsPerShader", "0", CVAR_GAME | CVAR_ARCHIVE, "This value will overwrite s_maxSoundsPerShader at game startup (unless this set to -1).");

/**
 * Variables needed for lockpicking.
 */
idCVar cv_lp_pin_base_count("tdm_lp_base_count",	"5",	CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "Base number of clicks per pin. This number will be added to the pinpattern." );
idCVar cv_lp_sample_delay("tdm_lp_sample_delay",	"10",	CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "Time in ms added to each pin sample to create a small pause between each pinsample." );
idCVar cv_lp_pick_timeout("tdm_lp_pick_timeout",	"500",	CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "Timeout that defines the maximum reaction time before the pin is to be considered unpicked and started over." );
idCVar cv_lp_max_pick_attempts("tdm_lp_autopick_attempts",	"3",	CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "How many pick attemps before it automatically gets picked? 0 = unlimited (you must solve it yourself)" );
idCVar cv_lp_auto_pick("tdm_lp_auto_pick",	"0",	CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "Determines if auto picking is enabled (see tdm_lp_max_pick_attempts)" );
idCVar cv_lp_randomize("tdm_lp_randomize",	"1",	CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "If set to 1, the jiggling, while lockpicking, will be randomized, otherwise it is linear." );
idCVar cv_lp_pawlow("tdm_lp_pawlow",	"0",	CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "If set to 1 the sweetspot sound will play at the end of the pattern instead of at the beginning. Thus making it into a reaction game." );
idCVar cv_lp_debug_hud("tdm_lp_debug_hud",	"0",	CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "If set to 1 the lockpicking debug HUD is shown during lockpicking." );

idCVar cv_door_auto_open_on_unlock("tdm_door_auto_open_on_unlock",	"1",	CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "If set to 1 doors and chests will start to open after being unlocked." );

idCVar cv_dm_distance("tdm_distance",		"",	CVAR_GAME,	"Shows the distance from the player to the entity" );

/**
 * Ambient light method variable
 */
idCVar cv_ambient_method("tdm_ambient_method",	"0",	CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "Method used for ambient light rendering.\n\n0 = Ambient Light method (uses the ambient light for the ambient brightness) \n1 = Texture Brightness method (uses texture brightness instead of light. This is faster but looks worse)" );

/**
 * Volume of speakers with s_music set
 */
idCVar cv_music_volume("tdm_music_volume",	"1.0",	CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "Volume in dB for speakers with s_music set. Goes from -40 to 0." );

// Tels:
idCVar cv_voice_player_volume("tdm_voice_player_volume",	 "1.0",	CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "Volume in dB for the player voice speaker. Goes from 0 .. 60." );
idCVar cv_voice_from_off_volume("tdm_voice_from_off_volume", "1.0",	CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "Volume in dB for the voice-over speaker. Goes from 0 .. 60." );

idCVar cv_moveable_collision("tdm_show_moveable_collision",	"0",	CVAR_GAME | CVAR_BOOL, "If set to 1, shows the velocity at which the moveable collides and the volume of the resulting sound." );

idCVar cv_darkradiant_rcfserver_enable("darkradiant_rcfserver_enable",	"0",	CVAR_BOOL | CVAR_ARCHIVE, "If set to 1 the DarkRadiant RCF Server will be enabled during the next startup." );

/**
* End DarkMod cvars
**/

// noset vars
idCVar gamename(					"gamename",					GAME_VERSION,	CVAR_GAME | CVAR_SERVERINFO | CVAR_ROM, "" );
idCVar gamedate(					"gamedate",					__DATE__,		CVAR_GAME | CVAR_ROM, "" );

// server info
idCVar si_name(						"si_name",					"DOOM Server",	CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE, "name of the server" );
idCVar si_gameType(					"si_gameType",		si_gameTypeArgs[ 0 ],	CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE, "game type - singleplayer, deathmatch, Tourney, Team DM or Last Man", si_gameTypeArgs, idCmdSystem::ArgCompletion_String<si_gameTypeArgs> );
idCVar si_map(						"si_map",					"game/mp/d3dm1",CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE, "map to be played next on server", idCmdSystem::ArgCompletion_MapName );
idCVar si_maxPlayers(				"si_maxPlayers",			"4",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_INTEGER, "max number of players allowed on the server", 1, 4 );
idCVar si_fragLimit(				"si_fragLimit",				"10",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_INTEGER, "frag limit", 1, MP_PLAYER_MAXFRAGS );
idCVar si_timeLimit(				"si_timeLimit",				"10",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_INTEGER, "time limit in minutes", 0, 60 );
idCVar si_teamDamage(				"si_teamDamage",			"0",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_BOOL, "enable team damage" );
idCVar si_warmup(					"si_warmup",				"0",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_BOOL, "do pre-game warmup" );
idCVar si_usePass(					"si_usePass",				"0",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_BOOL, "enable client password checking" );
idCVar si_pure(						"si_pure",					"1",			CVAR_GAME | CVAR_SERVERINFO | CVAR_BOOL, "server is pure and does not allow modified data" );
idCVar si_spectators(				"si_spectators",			"1",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_BOOL, "allow spectators or require all clients to play" );
idCVar si_serverURL(				"si_serverURL",				"",				CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE, "where to reach the server admins and get information about the server" );


// user info
idCVar ui_name(						"ui_name",					"Player",		CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE, "player name" );
idCVar ui_skin(						"ui_skin",				ui_skinArgs[ 0 ],	CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE, "player skin", ui_skinArgs, idCmdSystem::ArgCompletion_String<ui_skinArgs> );
idCVar ui_team(						"ui_team",				ui_teamArgs[ 0 ],	CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE, "player team", ui_teamArgs, idCmdSystem::ArgCompletion_String<ui_teamArgs> ); 
idCVar ui_autoSwitch(				"ui_autoSwitch",			"1",			CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE | CVAR_BOOL, "auto switch weapon" );
idCVar ui_autoReload(				"ui_autoReload",			"1",			CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE | CVAR_BOOL, "auto reload weapon" );
idCVar ui_showGun(					"ui_showGun",				"1",			CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE | CVAR_BOOL, "show gun" );
idCVar ui_ready(					"ui_ready",				si_readyArgs[ 0 ],	CVAR_GAME | CVAR_USERINFO, "player is ready to start playing", idCmdSystem::ArgCompletion_String<si_readyArgs> );
idCVar ui_spectate(					"ui_spectate",		si_spectateArgs[ 0 ],	CVAR_GAME | CVAR_USERINFO, "play or spectate", idCmdSystem::ArgCompletion_String<si_spectateArgs> );
idCVar ui_chat(						"ui_chat",					"0",			CVAR_GAME | CVAR_USERINFO | CVAR_BOOL | CVAR_ROM | CVAR_CHEAT, "player is chatting" );

// change anytime vars
idCVar developer(					"developer",				"0",			CVAR_GAME | CVAR_BOOL, "" );

idCVar r_aspectRatio( 				"r_aspectRatio",			"0",			CVAR_RENDERER | CVAR_INTEGER | CVAR_ARCHIVE, "aspect ratio of view:\n0 = 4:3\n1 = 16:9\n2 = 16:10", 0, 2 );

idCVar g_cinematic(					"g_cinematic",				"1",			CVAR_GAME | CVAR_BOOL, "skips updating entities that aren't marked 'cinematic' '1' during cinematics" );
idCVar g_cinematicMaxSkipTime(		"g_cinematicMaxSkipTime",	"600",			CVAR_GAME | CVAR_FLOAT, "# of seconds to allow game to run when skipping cinematic.  prevents lock-up when cinematic doesn't end.", 0, 3600 );

idCVar g_muzzleFlash(				"g_muzzleFlash",			"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show muzzle flashes" );
idCVar g_projectileLights(			"g_projectileLights",		"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show dynamic lights on projectiles" );
idCVar g_bloodEffects(				"g_bloodEffects",			"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show blood splats, sprays and gibs" );
idCVar g_doubleVision(				"g_doubleVision",			"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show double vision when taking damage" );
idCVar g_monsters(					"g_monsters",				"1",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_decals(					"g_decals",					"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show decals such as bullet holes" );
idCVar g_knockback(					"g_knockback",				"1000",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar g_skill(						"g_skill",					"1",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar g_nightmare(					"g_nightmare",				"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "if nightmare mode is allowed" );
idCVar g_gravity(					"g_gravity",		DEFAULT_GRAVITY_STRING, CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_skipFX(					"g_skipFX",					"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_skipParticles(				"g_skipParticles",			"0",			CVAR_GAME | CVAR_BOOL, "" );

idCVar g_disasm(					"g_disasm",					"0",			CVAR_GAME | CVAR_BOOL, "disassemble script into base/script/disasm.txt on the local drive when script is compiled" );
idCVar g_debugBounds(				"g_debugBounds",			"0",			CVAR_GAME | CVAR_BOOL, "checks for models with bounds > 2048" );
idCVar g_debugAnim(					"g_debugAnim",				"-1",			CVAR_GAME | CVAR_INTEGER, "displays information on which animations are playing on the specified entity number.  set to -1 to disable." );
idCVar g_debugMove(					"g_debugMove",				"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugDamage(				"g_debugDamage",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugWeapon(				"g_debugWeapon",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugScript(				"g_debugScript",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugMover(				"g_debugMover",				"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugTriggers(				"g_debugTriggers",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugCinematic(			"g_debugCinematic",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_stopTime(					"g_stopTime",				"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_damageScale(				"g_damageScale",			"1",			CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "scale final damage on player by this factor" );
idCVar g_armorProtection(			"g_armorProtection",		"0.3",			CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "armor takes this percentage of damage" );
idCVar g_armorProtectionMP(			"g_armorProtectionMP",		"0.6",			CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "armor takes this percentage of damage in mp" );
idCVar g_useDynamicProtection(		"g_useDynamicProtection",	"0",			CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "scale damage and armor dynamically to keep the player alive more often" );
idCVar g_healthTakeTime(			"g_healthTakeTime",			"5",			CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "how often to take health in nightmare mode" );
idCVar g_healthTakeAmt(				"g_healthTakeAmt",			"5",			CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "how much health to take in nightmare mode" );
idCVar g_healthTakeLimit(			"g_healthTakeLimit",		"25",			CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "how low can health get taken in nightmare mode" );



idCVar g_showPVS(					"g_showPVS",				"0",			CVAR_GAME | CVAR_INTEGER, "", 0, 2 );
idCVar g_showTargets(				"g_showTargets",			"0",			CVAR_GAME | CVAR_BOOL, "draws entities and thier targets.  hidden entities are drawn grey." );
idCVar g_showTriggers(				"g_showTriggers",			"0",			CVAR_GAME | CVAR_BOOL, "draws trigger entities (orange) and thier targets (green).  disabled triggers are drawn grey." );
idCVar g_showCollisionWorld(		"g_showCollisionWorld",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_showCollisionModels(		"g_showCollisionModels",	"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_showCollisionTraces(		"g_showCollisionTraces",	"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_maxShowDistance(			"g_maxShowDistance",		"128",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_showEntityInfo(			"g_showEntityInfo",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_showviewpos(				"g_showviewpos",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_showcamerainfo(			"g_showcamerainfo",			"0",			CVAR_GAME | CVAR_ARCHIVE, "displays the current frame # for the camera when playing cinematics" );
idCVar g_showTestModelFrame(		"g_showTestModelFrame",		"0",			CVAR_GAME | CVAR_BOOL, "displays the current animation and frame # for testmodels" );
idCVar g_showActiveEntities(		"g_showActiveEntities",		"0",			CVAR_GAME | CVAR_BOOL, "draws boxes around thinking entities.  dormant entities (outside of pvs) are drawn yellow.  non-dormant are green." );
idCVar g_showEnemies(				"g_showEnemies",			"0",			CVAR_GAME | CVAR_BOOL, "draws boxes around monsters that have targeted the the player" );

idCVar g_frametime(					"g_frametime",				"0",			CVAR_GAME | CVAR_BOOL, "displays timing information for each game frame" );

// TDM: greebo: Use this to stretch the hardcoded 16 msec each frame takes. This can be used to let the game run ultra-slow.
idCVar g_timeModifier(				"g_timeModifier",			"1",			CVAR_GAME | CVAR_FLOAT, "Use this to stretch the hardcoded 16 msec each frame takes. This can be used to let the game run ultra-slow." );
idCVar g_timeentities(				"g_timeEntities",			"0",			CVAR_GAME | CVAR_FLOAT, "when non-zero, shows entities whose think functions exceeded the # of milliseconds specified" );


idCVar g_enablePortalSky(			"g_enablePortalSky",		"1",			CVAR_GAME | CVAR_BOOL, "enables the portal sky" );

	
idCVar ai_debugScript(				"ai_debugScript",			"-1",			CVAR_GAME | CVAR_INTEGER, "displays script calls for the specified monster entity number" );
idCVar ai_debugMove(				"ai_debugMove",				"0",			CVAR_GAME | CVAR_BOOL, "draws movement information for monsters" );
idCVar ai_debugTrajectory(			"ai_debugTrajectory",		"0",			CVAR_GAME | CVAR_BOOL, "draws trajectory tests for monsters" );
idCVar ai_testPredictPath(			"ai_testPredictPath",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar ai_showCombatNodes(			"ai_showCombatNodes",		"0",			CVAR_GAME | CVAR_BOOL, "draws attack cones for monsters" );
idCVar ai_showPaths(				"ai_showPaths",				"0",			CVAR_GAME | CVAR_BOOL, "draws path_* entities" );
idCVar ai_showObstacleAvoidance(	"ai_showObstacleAvoidance",	"0",			CVAR_GAME | CVAR_INTEGER, "draws obstacle avoidance information for monsters.  if 2, draws obstacles for player, as well", 0, 2, idCmdSystem::ArgCompletion_Integer<0,2> );
idCVar ai_blockedFailSafe(			"ai_blockedFailSafe",		"1",			CVAR_GAME | CVAR_BOOL, "enable blocked fail safe handling" );
	
idCVar g_dvTime(					"g_dvTime",					"1",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_dvAmplitude(				"g_dvAmplitude",			"0.001",		CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_dvFrequency(				"g_dvFrequency",			"0.5",			CVAR_GAME | CVAR_FLOAT, "" );

idCVar g_kickTime(					"g_kickTime",				"1",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_kickAmplitude(				"g_kickAmplitude",			"0.0001",		CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_blobTime(					"g_blobTime",				"1",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_blobSize(					"g_blobSize",				"1",			CVAR_GAME | CVAR_FLOAT, "" );

idCVar g_testHealthVision(			"g_testHealthVision",		"0",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_editEntityMode(			"g_editEntityMode",			"0",			CVAR_GAME | CVAR_INTEGER,	"0 = off\n"
																											"1 = lights\n"
																											"2 = sounds\n"
																											"3 = articulated figures\n"
																											"4 = particle systems\n"
																											"5 = monsters\n"
																											"6 = entity names\n"
																											"7 = entity models", 0, 7, idCmdSystem::ArgCompletion_Integer<0,7> );
idCVar g_dragEntity(				"g_dragEntity",				"0",			CVAR_GAME | CVAR_BOOL, "allows dragging physics objects around by placing the crosshair over them and holding the fire button" );
idCVar g_dragDamping(				"g_dragDamping",			"0.5",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_dragShowSelection(			"g_dragShowSelection",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_dropItemRotation(			"g_dropItemRotation",		"",				CVAR_GAME, "" );

idCVar g_vehicleVelocity(			"g_vehicleVelocity",		"1000",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleForce(				"g_vehicleForce",			"50000",		CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleSuspensionUp(		"g_vehicleSuspensionUp",	"32",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleSuspensionDown(		"g_vehicleSuspensionDown",	"20",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleSuspensionKCompress("g_vehicleSuspensionKCompress","200",		CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleSuspensionDamping(	"g_vehicleSuspensionDamping","400",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleTireFriction(		"g_vehicleTireFriction",	"0.8",			CVAR_GAME | CVAR_FLOAT, "" );

idCVar ik_enable(					"ik_enable",				"1",			CVAR_GAME | CVAR_BOOL, "enable IK" );
idCVar ik_debug(					"ik_debug",					"0",			CVAR_GAME | CVAR_BOOL, "show IK debug lines" );

idCVar af_useLinearTime(			"af_useLinearTime",			"1",			CVAR_GAME | CVAR_BOOL, "use linear time algorithm for tree-like structures" );
idCVar af_useImpulseFriction(		"af_useImpulseFriction",	"0",			CVAR_GAME | CVAR_BOOL, "use impulse based contact friction" );
idCVar af_useJointImpulseFriction(	"af_useJointImpulseFriction","0",			CVAR_GAME | CVAR_BOOL, "use impulse based joint friction" );
idCVar af_useSymmetry(				"af_useSymmetry",			"1",			CVAR_GAME | CVAR_BOOL, "use constraint matrix symmetry" );
#ifdef MOD_WATERPHYSICS

idCVar af_useBodyDensityBuoyancy(   "af_useBodyDensityBuoyancy","0",            CVAR_GAME | CVAR_BOOL, "uses density of each body to calculate buoyancy"); // MOD_WATERPHYSICS

idCVar af_useFixedDensityBuoyancy(  "af_useFixedDensityBuoyancy","1",           CVAR_GAME | CVAR_BOOL, "if set, use liquidDensity as a fixed density for each body when calculating buoyancy.  If clear, bodies are floated uniformly by a scalar liquidDensity as defined in the decls." ); // MOD_WATERPHYSICS

#endif  // MOD_WATERPHYSICS

idCVar af_skipSelfCollision(		"af_skipSelfCollision",		"0",			CVAR_GAME | CVAR_BOOL, "skip self collision detection" );
idCVar af_skipLimits(				"af_skipLimits",			"0",			CVAR_GAME | CVAR_BOOL, "skip joint limits" );
idCVar af_skipFriction(				"af_skipFriction",			"0",			CVAR_GAME | CVAR_BOOL, "skip friction" );
idCVar af_forceFriction(			"af_forceFriction",			"-1",			CVAR_GAME | CVAR_FLOAT, "force the given friction value" );
idCVar af_maxLinearVelocity(		"af_maxLinearVelocity",		"128",			CVAR_GAME | CVAR_FLOAT, "maximum linear velocity" );
idCVar af_maxAngularVelocity(		"af_maxAngularVelocity",	"1.57",			CVAR_GAME | CVAR_FLOAT, "maximum angular velocity" );
idCVar af_timeScale(				"af_timeScale",				"1",			CVAR_GAME | CVAR_FLOAT, "scales the time" );
idCVar af_jointFrictionScale(		"af_jointFrictionScale",	"0",			CVAR_GAME | CVAR_FLOAT, "scales the joint friction" );
idCVar af_contactFrictionScale(		"af_contactFrictionScale",	"0",			CVAR_GAME | CVAR_FLOAT, "scales the contact friction" );
idCVar af_highlightBody(			"af_highlightBody",			"",				CVAR_GAME, "name of the body to highlight" );
idCVar af_highlightConstraint(		"af_highlightConstraint",	"",				CVAR_GAME, "name of the constraint to highlight" );
idCVar af_showTimings(				"af_showTimings",			"0",			CVAR_GAME | CVAR_BOOL, "show articulated figure cpu usage" );
idCVar af_showConstraints(			"af_showConstraints",		"0",			CVAR_GAME | CVAR_BOOL, "show constraints" );
idCVar af_showConstraintNames(		"af_showConstraintNames",	"0",			CVAR_GAME | CVAR_BOOL, "show constraint names" );
idCVar af_showConstrainedBodies(	"af_showConstrainedBodies",	"0",			CVAR_GAME | CVAR_BOOL, "show the two bodies contrained by the highlighted constraint" );
idCVar af_showPrimaryOnly(			"af_showPrimaryOnly",		"0",			CVAR_GAME | CVAR_BOOL, "show primary constraints only" );
idCVar af_showTrees(				"af_showTrees",				"0",			CVAR_GAME | CVAR_BOOL, "show tree-like structures" );
idCVar af_showLimits(				"af_showLimits",			"0",			CVAR_GAME | CVAR_BOOL, "show joint limits" );
idCVar af_showBodies(				"af_showBodies",			"0",			CVAR_GAME | CVAR_BOOL, "show bodies" );
idCVar af_showBodyNames(			"af_showBodyNames",			"0",			CVAR_GAME | CVAR_BOOL, "show body names" );
idCVar af_showMass(					"af_showMass",				"0",			CVAR_GAME | CVAR_BOOL, "show the mass of each body" );
idCVar af_showTotalMass(			"af_showTotalMass",			"0",			CVAR_GAME | CVAR_BOOL, "show the total mass of each articulated figure" );
idCVar af_showInertia(				"af_showInertia",			"0",			CVAR_GAME | CVAR_BOOL, "show the inertia tensor of each body" );
idCVar af_showVelocity(				"af_showVelocity",			"0",			CVAR_GAME | CVAR_BOOL, "show the velocity of each body" );
idCVar af_showActive(				"af_showActive",			"0",			CVAR_GAME | CVAR_BOOL, "show tree-like structures of articulated figures not at rest" );
idCVar af_testSolid(				"af_testSolid",				"1",			CVAR_GAME | CVAR_BOOL, "test for bodies initially stuck in solid" );

idCVar rb_showTimings(				"rb_showTimings",			"0",			CVAR_GAME | CVAR_BOOL, "show rigid body cpu usage" );
idCVar rb_showBodies(				"rb_showBodies",			"0",			CVAR_GAME | CVAR_BOOL, "show rigid bodies" );
idCVar rb_showMass(					"rb_showMass",				"0",			CVAR_GAME | CVAR_BOOL, "show the mass of each rigid body" );
idCVar rb_showInertia(				"rb_showInertia",			"0",			CVAR_GAME | CVAR_BOOL, "show the inertia tensor of each rigid body" );
idCVar rb_showVelocity(				"rb_showVelocity",			"0",			CVAR_GAME | CVAR_BOOL, "show the velocity of each rigid body" );
idCVar rb_showActive(				"rb_showActive",			"0",			CVAR_GAME | CVAR_BOOL, "show rigid bodies that are not at rest" );
#ifdef MOD_WATERPHYSICS

idCVar rb_showBuoyancy(             "rb_showBuoyancy",          "0",            CVAR_GAME | CVAR_BOOL, "show rigid body buoyancy information" ); // MOD_WATERPHYSICS

#endif //   MOD_WATERPHYSICS


// The default values for player movement cvars are set in def/player.def
idCVar pm_jumpheight(				"pm_jumpheight",			"48",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "approximate hieght the player can jump" );
idCVar pm_stepsize(					"pm_stepsize",				"16",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "maximum height the player can step up without jumping" );
// replaced by crouch multiplier
//idCVar pm_crouchspeed(				"pm_crouchspeed",			"80",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "speed the player can move while crouched" );
idCVar pm_walkspeed(				"pm_walkspeed",				"70",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "speed the player can move while walking" );
// also replaced by multiplier
//idCVar pm_runspeed(					"pm_runspeed",				"220",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "speed the player can move while running" );
idCVar pm_noclipspeed(				"pm_noclipspeed",			"200",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "speed the player can move while in noclip" );
idCVar pm_spectatespeed(			"pm_spectatespeed",			"450",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "speed the player can move while spectating" );
idCVar pm_spectatebbox(				"pm_spectatebbox",			"32",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "size of the spectator bounding box" );
idCVar pm_usecylinder(				"pm_usecylinder",			"1",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_BOOL, "use a cylinder approximation instead of a bounding box for player collision detection" );
idCVar pm_minviewpitch(				"pm_minviewpitch",			"-89",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "amount player's view can look up (negative values are up)" );
idCVar pm_maxviewpitch(				"pm_maxviewpitch",			"89",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "amount player's view can look down" );
// Commented out by Dram. Not needed as TDM does not use stamina
//idCVar pm_stamina(				"pm_stamina",				"24",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "length of time player can run" );
//idCVar pm_staminathreshold(			"pm_staminathreshold",		"45",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "when stamina drops below this value, player gradually slows to a walk" );
//idCVar pm_staminarate(				"pm_staminarate",			"0.75",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "rate that player regains stamina. divide pm_stamina by this value to determine how long it takes to fully recharge." );
idCVar pm_crouchheight(				"pm_crouchheight",			"38",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "height of player's bounding box while crouched" );
idCVar pm_crouchviewheight(			"pm_crouchviewheight",		"34",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "height of player's view while crouched" );
idCVar pm_normalheight(				"pm_normalheight",			"74",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "height of player's bounding box while standing" );
idCVar pm_normalviewheight(			"pm_normalviewheight",		"68",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "height of player's view while standing" );
idCVar pm_deadheight(				"pm_deadheight",			"20",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "height of player's bounding box while dead" );
idCVar pm_deadviewheight(			"pm_deadviewheight",		"10",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "height of player's view while dead" );
idCVar pm_crouchrate(				"pm_crouchrate",			"0.87",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "time it takes for player's view to change from standing to crouching" );
idCVar pm_bboxwidth(				"pm_bboxwidth",				"32",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "x/y size of player's bounding box" );
idCVar pm_crouchbob(				"pm_crouchbob",				"0.2",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "bob much faster when crouched" );
idCVar pm_walkbob(					"pm_walkbob",				"0.3",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "bob slowly when walking" );
idCVar pm_runbob(					"pm_runbob",				"0.35",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "bob faster when running" );
idCVar pm_runpitch(					"pm_runpitch",				"0.001",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "" );
idCVar pm_runroll(					"pm_runroll",				"0.003",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "" );
idCVar pm_bobup(					"pm_bobup",					"0.035",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "" );
idCVar pm_bobpitch(					"pm_bobpitch",				"0.001",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "" );
idCVar pm_bobroll(					"pm_bobroll",				"0.0015",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "" );
idCVar pm_thirdPersonRange(			"pm_thirdPersonRange",		"80",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "camera distance from player in 3rd person" );
idCVar pm_thirdPersonHeight(		"pm_thirdPersonHeight",		"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "height of camera from normal view height in 3rd person" );
idCVar pm_thirdPersonAngle(			"pm_thirdPersonAngle",		"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "direction of camera from player in 3rd person in degrees (0 = behind player, 180 = in front)" );
idCVar pm_thirdPersonClip(			"pm_thirdPersonClip",		"1",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_BOOL, "clip third person view into world space" );
idCVar pm_thirdPerson(				"pm_thirdPerson",			"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_BOOL, "enables third person view" );
idCVar pm_thirdPersonDeath(			"pm_thirdPersonDeath",		"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_BOOL, "enables third person view when player dies" );
idCVar pm_modelView(				"pm_modelView",				"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_INTEGER, "draws camera from POV of player model (1 = always, 2 = when dead)", 0, 2, idCmdSystem::ArgCompletion_Integer<0,2> );
idCVar pm_airTics(					"pm_air",					"1800",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_INTEGER, "how long in milliseconds the player can go without air before he starts taking damage" );

idCVar g_showPlayerShadow(			"g_showPlayerShadow",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "enables shadow of player model" );
idCVar g_showHud(					"g_showHud",				"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "" );
idCVar g_showProjectilePct(			"g_showProjectilePct",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "enables display of player hit percentage" );
idCVar g_showBrass(					"g_showBrass",				"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "enables ejected shells from weapon" );
idCVar g_gun_x(						"g_gunX",					"0",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_gun_y(						"g_gunY",					"0",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_gun_z(						"g_gunZ",					"0",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_viewNodalX(				"g_viewNodalX",				"0",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_viewNodalZ(				"g_viewNodalZ",				"0",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_fov(						"g_fov",					"90",			CVAR_GAME | CVAR_INTEGER | CVAR_NOCHEAT, "" );
idCVar g_skipViewEffects(			"g_skipViewEffects",		"0",			CVAR_GAME | CVAR_BOOL, "skip damage and other view effects" );
idCVar g_mpWeaponAngleScale(		"g_mpWeaponAngleScale",		"0",			CVAR_GAME | CVAR_FLOAT, "Control the weapon sway in MP" );

idCVar g_testParticle(				"g_testParticle",			"0",			CVAR_GAME | CVAR_INTEGER, "test particle visualation, set by the particle editor" );
idCVar g_testParticleName(			"g_testParticleName",		"",				CVAR_GAME, "name of the particle being tested by the particle editor" );
idCVar g_testModelHead(				"g_testModelHead",			"atdm:ai_head_citywatch",	CVAR_GAME | CVAR_ARCHIVE, "test model head entityDef" );
idCVar g_testModelHeadJoint(				"g_testModelHeadJoint",			"Spine2",	CVAR_GAME | CVAR_ARCHIVE, "test model head joint" );
idCVar g_testModelRotate(			"g_testModelRotate",		"0",			CVAR_GAME, "test model rotation speed" );
idCVar g_testPostProcess(			"g_testPostProcess",		"",				CVAR_GAME, "name of material to draw over screen" );
idCVar g_testModelAnimate(			"g_testModelAnimate",		"0",			CVAR_GAME | CVAR_INTEGER, "test model animation,\n"
																							"0 = cycle anim with origin reset\n"
																							"1 = cycle anim with fixed origin\n"
																							"2 = cycle anim with continuous origin\n"
																							"3 = frame by frame with continuous origin\n"
																							"4 = play anim once", 0, 4, idCmdSystem::ArgCompletion_Integer<0,4> );
idCVar g_testModelBlend(			"g_testModelBlend",			"0",			CVAR_GAME | CVAR_INTEGER, "number of frames to blend" );
idCVar g_testDeath(					"g_testDeath",				"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_exportMask(				"g_exportMask",				"",				CVAR_GAME, "" );
idCVar g_flushSave(					"g_flushSave",				"0",			CVAR_GAME | CVAR_BOOL, "1 = don't buffer file writing for save games." );

idCVar g_rotoscope(					"g_rotoscope",				"0",			CVAR_GAME | CVAR_BOOL, "Noir cartoon-like rendering" );

idCVar aas_test(					"aas_test",					"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_showAreas(				"aas_showAreas",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar aas_showPath(				"aas_showPath",				"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_showFlyPath(				"aas_showFlyPath",			"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_showWallEdges(			"aas_showWallEdges",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar aas_showHideArea(			"aas_showHideArea",			"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_pullPlayer(				"aas_pullPlayer",			"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_randomPullPlayer(		"aas_randomPullPlayer",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar aas_goalArea(				"aas_goalArea",				"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_showPushIntoArea(		"aas_showPushIntoArea",		"0",			CVAR_GAME | CVAR_BOOL, "" );

idCVar g_password(					"g_password",				"",				CVAR_GAME | CVAR_ARCHIVE, "game password" );
idCVar password(					"password",					"",				CVAR_GAME | CVAR_NOCHEAT, "client password used when connecting" );

idCVar g_countDown(					"g_countDown",				"10",			CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "pregame countdown in seconds", 4, 3600 );
idCVar g_gameReviewPause(			"g_gameReviewPause",		"10",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_INTEGER | CVAR_ARCHIVE, "scores review time in seconds (at end game)", 2, 3600 );
idCVar g_TDMArrows(					"g_TDMArrows",				"1",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_BOOL, "draw arrows over teammates in team deathmatch" );
idCVar g_balanceTDM(				"g_balanceTDM",				"1",			CVAR_GAME | CVAR_BOOL, "maintain even teams" );

idCVar net_clientPredictGUI(		"net_clientPredictGUI",		"1",			CVAR_GAME | CVAR_BOOL, "test guis in networking without prediction" );

idCVar g_voteFlags(					"g_voteFlags",				"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_INTEGER | CVAR_ARCHIVE, "vote flags. bit mask of votes not allowed on this server\n"
																					"bit 0 (+1)   restart now\n"
																					"bit 1 (+2)   time limit\n"
																					"bit 2 (+4)   frag limit\n"
																					"bit 3 (+8)   game type\n"
																					"bit 4 (+16)  kick player\n"
																					"bit 5 (+32)  change map\n"
																					"bit 6 (+64)  spectators\n"
																					"bit 7 (+128) next map" );
idCVar g_mapCycle(					"g_mapCycle",				"mapcycle",		CVAR_GAME | CVAR_ARCHIVE, "map cycling script for multiplayer games - see mapcycle.scriptcfg" );

idCVar mod_validSkins(				"mod_validSkins",			"skins/characters/player/marine_mp;skins/characters/player/marine_mp_green;skins/characters/player/marine_mp_blue;skins/characters/player/marine_mp_red;skins/characters/player/marine_mp_yellow",		CVAR_GAME | CVAR_ARCHIVE, "valid skins for the game" );
idCVar net_serverDownload(			"net_serverDownload",		"0",			CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "enable server download redirects. 0: off 1: redirect to si_serverURL 2: use builtin download. see net_serverDl cvars for configuration" );

idCVar net_serverDlBaseURL(			"net_serverDlBaseURL",		"",				CVAR_GAME | CVAR_ARCHIVE, "base URL for the download redirection" );

idCVar net_serverDlTable(			"net_serverDlTable",		"",				CVAR_GAME | CVAR_ARCHIVE, "pak names for which download is provided, seperated by ;" );

// Bloom related - by JC_Denton & Maha_X - added by Dram
idCVar r_bloom( "r_bloom", "0", CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "Enable Bloom Post-Process Effects (requires DX9 Compliant Hardware)" );
idCVar r_bloom_blur_mult( "r_bloom_blur_mult", "0.5", CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "Blurred image multiplier for bloom");
idCVar r_bloom_src_mult( "r_bloom_src_mult", "1.0", CVAR_GAME | CVAR_FLOAT, "Source image multiplier for bloom");
idCVar r_bloom_buffer( "r_bloom_buffer", "4", CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "Bloom buffer image size: \n1 = 64x32\n2 = 128x64\n3 = 256x128\n4 = 512x256\n5 = 1024x512\n6 = 2048x1024"); // New by Clone JCD
idCVar r_bloom_contrast_mult( "r_bloom_contrast_mult", "1.55", CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "Contrast multiplier");
idCVar r_bloom_contrast_min( "r_bloom_contrast_min", "0.1", CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "This is the minimum contrast value when (shift sensitivity based)bloom drops \nWhen shift sensitivty is turned on bloom contrast varies from (constrast_image x r_bloom_contrast_min) to (constrast_image x r_bloom_contrast_mult)");
idCVar r_bloom_shift_delay( "r_bloom_shift_delay", "0", CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "Delay in millisecs for shifting the bloom sensitivity \n0 = No shift sensitivity delay, sensitvity shifts immediately. \n-1 = Disables sensitivity shifting"); // New by Clone JCD
idCVar r_bloom_blurIterations( "r_bloom_blurIterations", "1", CVAR_GAME | CVAR_INTEGER, "Blur iterations for bloom"); // New by Clone JCD
idCVar r_bloom_hud( "r_bloom_hud", "0", CVAR_GAME | CVAR_BOOL | CVAR_ARCHIVE, "Enable blooming of the HUD" );
idCVar r_bloom_lightRayScale( "r_bloom_lightRayScale", "1.3", CVAR_GAME | CVAR_FLOAT | CVAR_ARCHIVE, "Brightness scale of light ray effects when bloom is active");

#ifndef __linux__
idCVar s_driver("s_driver", "0", CVAR_GUI, "Dummy CVAR introduced by TDM to fix a console warning in Windows. Seems to be missing, but D3's mpmain.gui references this.");
#endif

