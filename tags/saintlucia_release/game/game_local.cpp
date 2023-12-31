/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2953 $
 * $Date: 2008-10-16 00:39:17 -0400 (Thu, 16 Oct 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../idlib/precompiled.h"
#pragma hdrstop

#pragma warning(disable : 4127 4996 4805 4800)

static bool init_version = FileVersionList("$Id: game_local.cpp 2953 2008-10-16 04:39:17Z greebo $", init_version);

#include "game_local.h"
#include "../DarkMod/DarkModGlobals.h"
#include "../DarkMod/darkModLAS.h"
#include "../DarkMod/decltdm_matinfo.h"
#include "../DarkMod/declxdata.h"
#include "../DarkMod/Misc.h"
#include "../DarkMod/PlayerData.h"
#include "../DarkMod/Relations.h"
#include "../DarkMod/sndProp.h"
#include "ai/aas_local.h"
#include "../DarkMod/StimResponse/StimResponseCollection.h"
#include "../DarkMod/MissionData.h"
#include "../DarkMod/MultiStateMover.h"
#include "../DarkMod/func_shooter.h"
#include "../DarkMod/shop.h"
#include "../DarkMod/DifficultyMenu.h"
#include "../DarkMod/EscapePointManager.h"
#include "../DarkMod/ModMenu.h"
#include "../DarkMod/renderpipe.h"
#include "../DarkMod/TimerManager.h"
#include "../DarkMod/AI/Conversation/ConversationSystem.h"

#include "IL/config.h"
#include "IL/il.h"
#include "../DarkMod/randomizer/randomc.h"

#include <iostream>

CGlobal g_Global;
TRandomCombined<TRanrotWGenerator,TRandomMersenne> rnd(time(0));

extern CRelations		g_globalRelations;
extern CMissionData		g_MissionData;
extern CsndPropLoader	g_SoundPropLoader;
extern CsndProp			g_SoundProp;
extern CShop			g_Shop;
extern CDifficultyMenu	g_Diff;
extern CModMenu			g_Mods;

#define BUFFER_LEN 4096

#ifdef GAME_DLL

idSys *						sys = NULL;
idCommon *					common = NULL;
idCmdSystem *				cmdSystem = NULL;
idCVarSystem *				cvarSystem = NULL;
idFileSystem *				fileSystem = NULL;
idNetworkSystem *			networkSystem = NULL;
idRenderSystem *			renderSystem = NULL;
idSoundSystem *				soundSystem = NULL;
idRenderModelManager *		renderModelManager = NULL;
idUserInterfaceManager *	uiManager = NULL;
idDeclManager *				declManager = NULL;
idAASFileManager *			AASFileManager = NULL;
idCollisionModelManager *	collisionModelManager = NULL;
idCVar *					idCVar::staticVars = NULL;

idCVar com_forceGenericSIMD( "com_forceGenericSIMD", "0", CVAR_BOOL|CVAR_SYSTEM, "force generic platform independent SIMD" );

#endif

idRenderWorld *				gameRenderWorld = NULL;		// all drawing is done to this world
idSoundWorld *				gameSoundWorld = NULL;		// all audio goes to this world
idSoundWorld *				gameSoundWorldBuf = NULL;

static gameExport_t			gameExport;

// global animation lib
idAnimManager				animationLib;

// the rest of the engine will only reference the "game" variable, while all local aspects stay hidden
idGameLocal					gameLocal;
idGame *					game = &gameLocal;	// statically pointed at an idGameLocal

const char *idGameLocal::sufaceTypeNames[ MAX_SURFACE_TYPES ] = {
	"none",	"metal", "stone", "flesh", "wood", "cardboard", "liquid", "glass", "plastic",
	"ricochet", "surftype10", "surftype11", "surftype12", "surftype13", "surftype14", "surftype15"
};

/* This list isn't actually used by the code, it's here just for reference. The code
   accepts any first word in the description as the surface type name: */
const char *idGameLocal::m_NewSurfaceTypes[ MAX_SURFACE_TYPES * 2 + 1] = {
	"tile", "carpet", "dirt", "gravel", "grass", "rock", "twigs", "foliage", "sand", "mud",
	"brokeglass", "snow", "ice", "squeakboard", "puddle", "moss", "cloth", "ceramic", "slate",
	"straw", "armor_leath", "armor_chain", "armor_plate", "climbable", "paper"
};

fontInfoEx_t font_an;
fontInfoEx_t font_bank;
fontInfoEx_t font_micro;

void PrintMessage( int x, int y, const char *szMessage, idVec4 colour, fontInfoEx_t &font )
{
      renderSystem->SetColor( colour );

      for( const char *p = szMessage; *p; p++ )
      {
            glyphInfo_t &glyph = font.fontInfoSmall.glyphs[int(*p)];

            renderSystem->DrawStretchPic( x, y - glyph.top,
                  glyph.imageWidth, glyph.imageHeight,
                  glyph.s, glyph.t, glyph.s2, glyph.t2,
                  glyph.glyph );

            x += glyph.xSkip;
      }
}

/*
===========
GetGameAPI
============
*/
#if __MWERKS__
#pragma export on
#endif
#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif
extern "C" gameExport_t *GetGameAPI( gameImport_t *import ) {
#if __MWERKS__
#pragma export off
#endif

	if ( import->version == GAME_API_VERSION ) {

		// set interface pointers used by the game
		sys							= import->sys;
		common						= import->common;
		cmdSystem					= import->cmdSystem;
		cvarSystem					= import->cvarSystem;
		fileSystem					= import->fileSystem;
		networkSystem				= import->networkSystem;
		renderSystem				= import->renderSystem;
		soundSystem					= import->soundSystem;
		renderModelManager			= import->renderModelManager;
		uiManager					= import->uiManager;
		declManager					= import->declManager;
		AASFileManager				= import->AASFileManager;
		collisionModelManager		= import->collisionModelManager;
	}
	else {
		// Wrong game version, throw a meaningful error rather than leaving
		// stuff initialised and getting segfaults.
		std::cerr << "FATAL: Incorrect game version: required " 
			<< GAME_API_VERSION << ", got " << import->version << "\n"
			<< "Ensure the correct Doom 3 patches are installed." << std::endl;
		abort();
	}

	// set interface pointers used by idLib
	idLib::sys					= sys;
	idLib::common				= common;
	idLib::cvarSystem			= cvarSystem;
	idLib::fileSystem			= fileSystem;

	// setup export interface
	gameExport.version = GAME_API_VERSION;
	gameExport.game = game;
	gameExport.gameEdit = gameEdit;

	// Initialize logging and all the global stuff for darkmod
	g_Global.Init();

	return &gameExport;
}
#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif

/*
===========
TestGameAPI
============
*/
void TestGameAPI( void ) {
	gameImport_t testImport;
	gameExport_t testExport;

	testImport.sys						= ::sys;
	testImport.common					= ::common;
	testImport.cmdSystem				= ::cmdSystem;
	testImport.cvarSystem				= ::cvarSystem;
	testImport.fileSystem				= ::fileSystem;
	testImport.networkSystem			= ::networkSystem;
	testImport.renderSystem				= ::renderSystem;
	testImport.soundSystem				= ::soundSystem;
	testImport.renderModelManager		= ::renderModelManager;
	testImport.uiManager				= ::uiManager;
	testImport.declManager				= ::declManager;
	testImport.AASFileManager			= ::AASFileManager;
	testImport.collisionModelManager	= ::collisionModelManager;

	testExport = *GetGameAPI( &testImport );
}

/*
===========
idGameLocal::idGameLocal
============
*/

idGameLocal::idGameLocal()
{
	m_HighestSRId = 0;
	m_MissionResult = MISSION_NOTEVENSTARTED;
	successScreenActive = false;
	Clear();
}

/*
===========
idGameLocal::~idGameLocal
============
*/
idGameLocal::~idGameLocal() 
{
}

/*
===========
idGameLocal::Clear
============
*/
void idGameLocal::Clear( void )
{
	int i;

	m_HighestSRId = 0;
	m_StimTimer.Clear();
	m_Timer.Clear();
	m_StimEntity.Clear();
	m_RespEntity.Clear();

	m_sndPropLoader = &g_SoundPropLoader;
	m_sndProp = &g_SoundProp;
	m_RelationsManager = &g_globalRelations;
	m_MissionData = &g_MissionData;
	// greebo: don't clear the Mission Result, Clear() is called during map shutdown
	m_MissionData->ClearGUIState();

	m_Grabber = NULL;
	m_DifficultyManager.Clear();

	m_AreaManager.Clear();
	// Allocate a new ConversationSystem
	m_ConversationSystem = ai::ConversationSystemPtr(new ai::ConversationSystem);

#ifdef TIMING_BUILD
	debugtools::TimerManager::Instance().Clear();
#endif

	m_EscapePointManager = CEscapePointManager::Instance();
	m_EscapePointManager->Clear();

	m_Interleave = 0;
	m_LightgemSurface = NULL;
	m_LightgemShotSpot = 0;
	for(i = 0; i < DARKMOD_LG_MAX_RENDERPASSES; i++)
		m_LightgemShotValue[i] = 0.0;
	m_DoLightgem = true;
	
	serverInfo.Clear();
	numClients = 0;
	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		userInfo[i].Clear();
		persistentPlayerInfo[i].Clear();
	}
	memset( usercmds, 0, sizeof( usercmds ) );
	memset( entities, 0, sizeof( entities ) );
	memset( spawnIds, -1, sizeof( spawnIds ) );
	firstFreeIndex = 0;
	num_entities = 0;
	spawnedEntities.Clear();
	activeEntities.Clear();
	spawnedAI.Clear();
	numEntitiesToDeactivate = 0;
	sortPushers = false;
	sortTeamMasters = false;
	persistentLevelInfo.Clear();
	memset( globalShaderParms, 0, sizeof( globalShaderParms ) );
	random.SetSeed( 0 );
	world = NULL;
	frameCommandThread = NULL;
	testmodel = NULL;
	testFx = NULL;
	clip.Shutdown();
	pvs.Shutdown();
	sessionCommand.Clear();
	locationEntities = NULL;
	smokeParticles = NULL;
	editEntities = NULL;
	entityHash.Clear( 1024, MAX_GENTITIES );
	inCinematic = false;
	cinematicSkipTime = 0;
	cinematicStopTime = 0;
	cinematicMaxSkipTime = 0;
	framenum = 0;
	previousTime = 0;
	time = 0;
	vacuumAreaNum = 0;
	mapFileName.Clear();
	mapFile = NULL;
	spawnCount = INITIAL_SPAWN_COUNT;
	mapSpawnCount = 0;
	camera = NULL;
	aasList.Clear();
	aasNames.Clear();
	lastAIAlertEntity = NULL;
	lastAIAlertTime = 0;
	spawnArgs.Clear();
	gravity.Set( 0, 0, -1 );
	playerPVS.h = (unsigned int)-1;
	playerConnectedAreas.h = (unsigned int)-1;
	gamestate = GAMESTATE_UNINITIALIZED;
	skipCinematic = false;
	influenceActive = false;

	localClientNum = 0;
	isMultiplayer = false;
	isServer = false;
	isClient = false;
	realClientTime = 0;
	isNewFrame = true;
	clientSmoothing = 0.1f;
	entityDefBits = 0;

	nextGibTime = 0;
	globalMaterial = NULL;
	newInfo.Clear();
	lastGUIEnt = NULL;
	lastGUI = 0;

	memset( clientEntityStates, 0, sizeof( clientEntityStates ) );
	memset( clientPVS, 0, sizeof( clientPVS ) );
	memset( clientSnapshots, 0, sizeof( clientSnapshots ) );

	eventQueue.Init();
	savedEventQueue.Init();
	memset( lagometer, 0, sizeof( lagometer ) );

	portalSkyEnt			= NULL;
	portalSkyActive			= false;
	//	ResetSlowTimeVars();

	m_EscapePointManager->Clear();

	m_GamePlayTimer.Clear();
}

/*
===========
idGameLocal::Init

  initialize the game object, only happens once at startup, not each level load
============
*/
void idGameLocal::Init( void ) {
	const idDict *dict;
	idAAS *aas;

#ifndef GAME_DLL

	TestGameAPI();

#else

	// Initialize the image library, so we can use it later on.
	ilInit();

	// initialize idLib
	idLib::Init();

	// register static cvars declared in the game
	idCVar::RegisterStaticVars();

	// initialize processor specific SIMD
	idSIMD::InitProcessor( "game", com_forceGenericSIMD.GetBool() );

#endif

	Printf( "--------- Initializing Game ----------\n" );
	Printf( "gamename: %s\n", GAME_VERSION );
	Printf( "gamedate: %s\n", __DATE__ );

	// register game specific decl types
	declManager->RegisterDeclType( "model",				DECL_MODELDEF,		idDeclAllocator<idDeclModelDef> );
	declManager->RegisterDeclType( "export",			DECL_MODELEXPORT,	idDeclAllocator<idDecl> );
	// TDM specific DECLs
	declManager->RegisterDeclType( "xdata",				DECL_XDATA,			idDeclAllocator<tdmDeclXData> );
	declManager->RegisterDeclType( "tdm_matinfo",		DECL_TDM_MATINFO,	idDeclAllocator<tdmDeclTDM_MatInfo> );

	// register game specific decl folders
	declManager->RegisterDeclFolder( "def",				".def",				DECL_ENTITYDEF );
	declManager->RegisterDeclFolder( "fx",				".fx",				DECL_FX );
	declManager->RegisterDeclFolder( "particles",		".prt",				DECL_PARTICLE );
	declManager->RegisterDeclFolder( "af",				".af",				DECL_AF );
	declManager->RegisterDeclFolder( "newpdas",			".pda",				DECL_PDA );
	// TDM specific DECLs
	declManager->RegisterDeclFolder( "xdata",			".xd",				DECL_XDATA );
	declManager->RegisterDeclFolder( "materials",		".mtr",				DECL_TDM_MATINFO );

	cmdSystem->AddCommand( "listModelDefs", idListDecls_f<DECL_MODELDEF>, CMD_FL_SYSTEM|CMD_FL_GAME, "lists model defs" );
	cmdSystem->AddCommand( "printModelDefs", idPrintDecls_f<DECL_MODELDEF>, CMD_FL_SYSTEM|CMD_FL_GAME, "prints a model def", idCmdSystem::ArgCompletion_Decl<DECL_MODELDEF> );

	Clear();

	idEvent::Init();
	idClass::Init();

	InitConsoleCommands();

	// load default scripts
	program.Startup( SCRIPT_DEFAULT );
	
	smokeParticles = new idSmokeParticles;

	// set up the aas
	dict = FindEntityDefDict( "aas_types" );
	if ( !dict ) {
		Error( "Unable to find entityDef for 'aas_types'" );
	}

	// allocate space for the aas
	const idKeyValue *kv = dict->MatchPrefix( "type" );
	while( kv != NULL ) {
		aas = idAAS::Alloc();
		aasList.Append( aas );
		aasNames.Append( kv->GetValue() );
		kv = dict->MatchPrefix( "type", kv );
	}

	gamestate = GAMESTATE_NOMAP;

	Printf( "...%d aas types\n", aasList.Num() );
	Printf( "game initialized.\n" );
	Printf( "--------------------------------------\n" );
	Printf( "Parsing material files\n" );

	LoadLightMaterial("materials/lights.mtr", &g_Global.m_LightMaterial);

	// load the soundprop globals from the def file
	m_sndPropLoader->GlobalsFromDef();

	//FIX: pm_walkspeed keeps getting reset whenever a map loads.
	// Copy the old value here and set it when the map starts up.
	m_walkSpeed = pm_walkspeed.GetFloat();

	const char *szLang = cvarSystem->GetCVarString( "sys_lang" );
	
	renderSystem->RegisterFont( va( "fonts/%s/%s", szLang, "an" ), font_an );
	renderSystem->RegisterFont( va( "fonts/%s/%s", szLang, "bank" ), font_bank );
	renderSystem->RegisterFont( va( "fonts/%s/%s", szLang, "micro" ), font_micro );

	// Create render pipe
	m_RenderPipe = new CRenderPipe();
	if (m_RenderPipe == NULL) {
		// Out of memory
		DM_LOG(LC_LIGHT, LT_ERROR)LOGSTRING("Out of memory when allocating render pipe\n");
	}
}

/*
===========
idGameLocal::Shutdown

  shut down the entire game
============
*/
void idGameLocal::Shutdown( void ) {
	if ( !common ) {
		return;
	}

	Printf( "------------ Game Shutdown -----------\n" );
	
	delete m_RenderPipe;

	mpGame.Shutdown();

	MapShutdown();
	// greebo: Separately clear the missiondata, which is not 
	// cleared in MapShutdown() (needed for mission statistics)
	m_MissionData->Clear();

	// Destroy the conversation system
	m_ConversationSystem = ai::ConversationSystemPtr();

	aasList.DeleteContents( true );
	aasNames.Clear();

	idAI::FreeObstacleAvoidanceNodes();

	// shutdown the model exporter
	idModelExport::Shutdown();

	idEvent::Shutdown();

	delete[] locationEntities;
	locationEntities = NULL;

	delete smokeParticles;
	smokeParticles = NULL;

	idClass::Shutdown();

	// clear list with forces
	idForce::ClearForceList();

	// free the program data
	program.FreeData();

	// delete the .map file
	delete mapFile;
	mapFile = NULL;

	// free the collision map
	collisionModelManager->FreeMap();

	ShutdownConsoleCommands();

	// free memory allocated by class objects
	Clear();

	// shut down the animation manager
	animationLib.Shutdown();

	Printf( "--------------------------------------\n" );

#ifdef GAME_DLL

	// remove auto-completion function pointers pointing into this DLL
	cvarSystem->RemoveFlaggedAutoCompletion( CVAR_GAME );

	// enable leak test
	Mem_EnableLeakTest( "game" );

	// shutdown idLib
	idLib::ShutDown();

#endif
}

/*
===========
idGameLocal::SaveGame

save the current player state, level name, and level state
the session may have written some data to the file already
============
*/
void idGameLocal::SaveGame( idFile *f ) {
	int i;
	idEntity *ent;
	idEntity *link;

	idSaveGame savegame( f );

	if (g_flushSave.GetBool( ) == true ) { 
		// force flushing with each write... for tracking down
		// save game bugs.
		f->ForceFlush();
	}

	savegame.WriteBuildNumber( BUILD_NUMBER );

	// go through all entities and threads and add them to the object list
	for( i = 0; i < MAX_GENTITIES; i++ ) {
		ent = entities[i];

		if ( ent ) {
			if ( ent->GetTeamMaster() && ent->GetTeamMaster() != ent ) {
				continue;
			}
			for ( link = ent; link != NULL; link = link->GetNextTeamEntity() ) {
				savegame.AddObject( link );
			}
		}
	}

	idList<idThread *> threads;
	threads = idThread::GetThreads();

	for( i = 0; i < threads.Num(); i++ ) {
		savegame.AddObject( threads[i] );
	}

	// DarkMod: Add darkmod specific objects here:

	// Add relationship matrix object
	savegame.AddObject( m_RelationsManager );
	savegame.AddObject( m_Grabber );

	// write out complete object list
	savegame.WriteObjectList();

	program.Save( &savegame );

	// Save the global hiding spot search collection
	CHidingSpotSearchCollection::Instance().Save(&savegame);

	// Save our grabber pointer
	savegame.WriteObject(m_Grabber);

	m_DifficultyManager.Save(&savegame);

	for ( i = 0; i < NumAAS(); i++)
	{
		idAAS* aas = GetAAS(i);
		if (aas != NULL)
		{
			aas->Save(&savegame);
		}
	}

	m_GamePlayTimer.Save(&savegame);
	m_AreaManager.Save(&savegame);
	m_ConversationSystem->Save(&savegame);

#ifdef TIMING_BUILD
	debugtools::TimerManager::Instance().Save(&savegame);
#endif

	savegame.WriteInt( g_skill.GetInteger() );

	savegame.WriteDict( &serverInfo );

	savegame.WriteInt( numClients );
	for( i = 0; i < numClients; i++ ) {
		savegame.WriteDict( &userInfo[ i ] );
		savegame.WriteUsercmd( usercmds[ i ] );
		savegame.WriteDict( &persistentPlayerInfo[ i ] );
	}

	for( i = 0; i < MAX_GENTITIES; i++ ) {
		savegame.WriteObject( entities[ i ] );
		savegame.WriteInt( spawnIds[ i ] );
	}

	savegame.WriteInt( firstFreeIndex );
	savegame.WriteInt( num_entities );

	// enityHash is restored by idEntity::Restore setting the entity name.

	savegame.WriteObject( world );

	savegame.WriteInt( spawnedEntities.Num() );
	for( ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		savegame.WriteObject( ent );
	}

	savegame.WriteInt( activeEntities.Num() );
	for( ent = activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
		savegame.WriteObject( ent );
	}

	savegame.WriteInt(spawnedAI.Num());
	for (idAI* ai = spawnedAI.Next(); ai != NULL; ai = ai->aiNode.Next()) {
		savegame.WriteObject(ai);
	}

	savegame.WriteInt( numEntitiesToDeactivate );
	savegame.WriteBool( sortPushers );
	savegame.WriteBool( sortTeamMasters );
	savegame.WriteDict( &persistentLevelInfo );
	
	for( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ ) {
		savegame.WriteFloat( globalShaderParms[ i ] );
	}

	savegame.WriteBool(successScreenActive);

	savegame.WriteInt( random.GetSeed() );
	savegame.WriteObject( frameCommandThread );

	// clip
	// push
	// pvs

	testmodel = NULL;
	testFx = NULL;

	savegame.WriteString( sessionCommand );

	// FIXME: save smoke particles

	savegame.WriteInt( cinematicSkipTime );
	savegame.WriteInt( cinematicStopTime );
	savegame.WriteInt( cinematicMaxSkipTime );
	savegame.WriteBool( inCinematic );
	savegame.WriteBool( skipCinematic );

	savegame.WriteBool( isMultiplayer );
	savegame.WriteInt( gameType );

	savegame.WriteInt( framenum );
	savegame.WriteInt( previousTime );
	savegame.WriteInt( time );

	savegame.WriteInt( vacuumAreaNum );

	savegame.WriteInt( entityDefBits );
	savegame.WriteBool( isServer );
	savegame.WriteBool( isClient );

	savegame.WriteInt( localClientNum );

	// snapshotEntities is used for multiplayer only

	savegame.WriteInt( realClientTime );
	savegame.WriteBool( isNewFrame );
	savegame.WriteFloat( clientSmoothing );

	portalSkyEnt.Save( &savegame );

	savegame.WriteBool( portalSkyActive );



	savegame.WriteBool( mapCycleLoaded );
	savegame.WriteInt( spawnCount );

	if ( !locationEntities ) {
		savegame.WriteInt( 0 );
	} else {
		savegame.WriteInt( gameRenderWorld->NumAreas() );
		for( i = 0; i < gameRenderWorld->NumAreas(); i++ ) {
			savegame.WriteObject( locationEntities[ i ] );
		}
	}

	savegame.WriteObject( camera );

	savegame.WriteMaterial( globalMaterial );

	lastAIAlertEntity.Save( &savegame );
	savegame.WriteInt( lastAIAlertTime );

	savegame.WriteDict( &spawnArgs );

	savegame.WriteInt( playerPVS.i );
	savegame.WriteInt( playerPVS.h );
	savegame.WriteInt( playerConnectedAreas.i );
	savegame.WriteInt( playerConnectedAreas.h );

	savegame.WriteVec3( gravity );

	// gamestate

	savegame.WriteBool( influenceActive );
	savegame.WriteInt( nextGibTime );

	// Save the lightgem entity spawnId
	m_LightgemSurface.Save(&savegame);
	savegame.WriteBool(m_DoLightgem);
	savegame.WriteInt(m_LightgemShotSpot);
	for (int i = 0; i < DARKMOD_LG_MAX_RENDERPASSES; i++)
	{
		savegame.WriteFloat(m_LightgemShotValue[i]);
	}

	m_sndProp->Save(&savegame);
	m_MissionData->Save(&savegame);
	savegame.WriteInt(static_cast<int>(m_MissionResult));

	savegame.WriteInt(m_HighestSRId);

	savegame.WriteInt(m_Timer.Num());
	for (int i = 0; i < m_Timer.Num(); i++)
	{
		m_Timer[i]->Save(&savegame);
	}

	savegame.WriteInt(m_StimTimer.Num());
	for (int i = 0; i < m_StimTimer.Num(); i++)
	{
		savegame.WriteInt(m_StimTimer[i]->getUniqueId());
	}

	savegame.WriteInt(m_StimEntity.Num());
	for (int i = 0; i < m_StimEntity.Num(); i++)
	{
		m_StimEntity[i].Save(&savegame);
	}

	savegame.WriteInt(m_RespEntity.Num());
	for (int i = 0; i < m_RespEntity.Num(); i++)
	{
		m_RespEntity[i].Save(&savegame);
	}

	m_EscapePointManager->Save(&savegame);

	// greebo: Save the maximum frob distance
	savegame.WriteFloat(g_Global.m_MaxFrobDistance);

	// spawnSpots
	// initialSpots
	// currentInitialSpot
	// newInfo
	// makingBuild
	// shakeSounds

	// write out pending events
	idEvent::Save( &savegame );

	// Save the DarkMod player object, this contains a lot of other TDM-related classes
	g_Global.m_DarkModPlayer->Save(&savegame);

	// greebo: Close the savegame, this will invoke a recursive Save on all registered objects
	savegame.Close();

	// Send a message to the HUD
	GetLocalPlayer()->SendHUDMessage("Game Saved");
}

/*
===========
idGameLocal::GetPersistentPlayerInfo
============
*/
const idDict &idGameLocal::GetPersistentPlayerInfo( int clientNum ) {
	idEntity	*ent;

	persistentPlayerInfo[ clientNum ].Clear();
	ent = entities[ clientNum ];
	if ( ent && ent->IsType( idPlayer::Type ) ) {
		static_cast<idPlayer *>(ent)->SavePersistantInfo();
	}

	return persistentPlayerInfo[ clientNum ];
}

/*
===========
idGameLocal::SetPersistentPlayerInfo
============
*/
void idGameLocal::SetPersistentPlayerInfo( int clientNum, const idDict &playerInfo ) {
	persistentPlayerInfo[ clientNum ] = playerInfo;
}

/*
============
idGameLocal::Printf
============
*/
void idGameLocal::Printf( const char *fmt, ... ) const {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	common->Printf( "%s", text );
}

/*
============
idGameLocal::DPrintf
============
*/
void idGameLocal::DPrintf( const char *fmt, ... ) const {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	if ( !developer.GetBool() ) {
		return;
	}

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	common->Printf( "%s", text );
}

/*
============
idGameLocal::Warning
============
*/
void idGameLocal::Warning( const char *fmt, ... ) const {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];
	idThread *	thread;

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	thread = idThread::CurrentThread();
	if ( thread ) {
		thread->Warning( "%s", text );
	} else {
		common->Warning( "%s", text );
	}
}

/*
============
idGameLocal::DWarning
============
*/
void idGameLocal::DWarning( const char *fmt, ... ) const {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];
	idThread *	thread;

	if ( !developer.GetBool() ) {
		return;
	}

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	thread = idThread::CurrentThread();
	if ( thread ) {
		thread->Warning( "%s", text );
	} else {
		common->DWarning( "%s", text );
	}
}

/*
============
idGameLocal::Error
============
*/
void idGameLocal::Error( const char *fmt, ... ) const {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];
	idThread *	thread;

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	thread = idThread::CurrentThread();
	if ( thread ) {
		thread->Error( "%s", text );
	} else {
		common->Error( "%s", text );
	}
}

/*
===============
gameError
===============
*/
void gameError( const char *fmt, ... ) {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	gameLocal.Error( "%s", text );
}

/*
===========
idGameLocal::SetLocalClient
============
*/
void idGameLocal::SetLocalClient( int clientNum ) {
	localClientNum = clientNum;
}

/*
===========
idGameLocal::SetUserInfo
============
*/
const idDict* idGameLocal::SetUserInfo( int clientNum, const idDict &userInfo, bool isClient, bool canModify ) {
	int i;
	bool modifiedInfo = false;

	this->isClient = isClient;

	if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
		idGameLocal::userInfo[ clientNum ] = userInfo;

		// server sanity
		if ( canModify  ) {

			// don't let numeric nicknames, it can be exploited to go around kick and ban commands from the server
			if ( idStr::IsNumeric( this->userInfo[ clientNum ].GetString( "ui_name" ) ) ) {
				idGameLocal::userInfo[ clientNum ].Set( "ui_name", va( "%s_", idGameLocal::userInfo[ clientNum ].GetString( "ui_name" ) ) );
				modifiedInfo = true;
			}
		
			// don't allow dupe nicknames
			for ( i = 0; i < numClients; i++ ) {
				if ( i == clientNum ) {
					continue;
				}
				if ( entities[ i ] && entities[ i ]->IsType( idPlayer::Type ) ) {
					if ( !idStr::Icmp( idGameLocal::userInfo[ clientNum ].GetString( "ui_name" ), idGameLocal::userInfo[ i ].GetString( "ui_name" ) ) ) {
						idGameLocal::userInfo[ clientNum ].Set( "ui_name", va( "%s_", idGameLocal::userInfo[ clientNum ].GetString( "ui_name" ) ) );
						modifiedInfo = true;
						i = -1;	// rescan
						continue;
					}
				}
			}
		}

		if ( entities[ clientNum ] && entities[ clientNum ]->IsType( idPlayer::Type ) ) {
			modifiedInfo |= static_cast<idPlayer *>( entities[ clientNum ] )->UserInfoChanged(canModify);
		}

		if ( !isClient ) {
			// now mark this client in game
			mpGame.EnterGame( clientNum );
		}
	}

	if ( modifiedInfo ) {
		assert( canModify );

		newInfo = idGameLocal::userInfo[ clientNum ];
		return &newInfo;
	}
	return NULL;
}

/*
===========
idGameLocal::GetUserInfo
============
*/
const idDict* idGameLocal::GetUserInfo( int clientNum ) {
	if ( entities[ clientNum ] && entities[ clientNum ]->IsType( idPlayer::Type ) ) {
		return &userInfo[ clientNum ];
	}
	return NULL;
}

/*
===========
idGameLocal::SetServerInfo
============
*/
void idGameLocal::SetServerInfo( const idDict &_serverInfo ) {
	idBitMsg	outMsg;
	byte		msgBuf[MAX_GAME_MESSAGE_SIZE];

	serverInfo = _serverInfo;
	UpdateServerInfoFlags();

	if ( !isClient ) {
		// Let our clients know the server info changed
		outMsg.Init( msgBuf, sizeof( msgBuf ) );
		outMsg.WriteByte( GAME_RELIABLE_MESSAGE_SERVERINFO );
		outMsg.WriteDeltaDict( gameLocal.serverInfo, NULL );
		networkSystem->ServerSendReliableMessage( -1, outMsg );
	}
}


/*
===================
idGameLocal::LoadMap

Initializes all map variables common to both save games and spawned games.
===================
*/
void idGameLocal::LoadMap( const char *mapName, int randseed ) {
	int i;
	bool sameMap = (mapFile && idStr::Icmp(mapFileName, mapName) == 0);

	// clear the sound system
	gameSoundWorld->ClearAllSoundEmitters();

	InitAsyncNetwork();

	if ( !sameMap || ( mapFile && mapFile->NeedsReload() ) ) {
		// load the .map file
		if ( mapFile ) {
			delete mapFile;
		}
		mapFile = new idMapFile;
		if ( !mapFile->Parse( idStr( mapName ) + ".map" ) ) {
			delete mapFile;
			mapFile = NULL;
			Error( "Couldn't load %s", mapName );
		}
		tdmDeclTDM_MatInfo::precacheMap( mapFile );
	}
	mapFileName = mapFile->GetName();

	// load the collision map
	collisionModelManager->LoadMap( mapFile );

	numClients = 0;

	// initialize all entities for this game
	memset( entities, 0, sizeof( entities ) );
	memset( usercmds, 0, sizeof( usercmds ) );
	memset( spawnIds, -1, sizeof( spawnIds ) );
	spawnCount = INITIAL_SPAWN_COUNT;
	
	spawnedEntities.Clear();
	activeEntities.Clear();
	spawnedAI.Clear();
	numEntitiesToDeactivate = 0;
	sortTeamMasters = false;
	sortPushers = false;
	lastGUIEnt = NULL;
	lastGUI = 0;

	globalMaterial = NULL;

	memset( globalShaderParms, 0, sizeof( globalShaderParms ) );

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	num_entities	= MAX_CLIENTS;
	firstFreeIndex	= MAX_CLIENTS;

	// reset the random number generator.
	random.SetSeed( isMultiplayer ? randseed : 0 );

	camera			= NULL;
	world			= NULL;
	testmodel		= NULL;
	testFx			= NULL;

	lastAIAlertEntity = NULL;
	lastAIAlertTime = 0;
	
	previousTime	= 0;
	time			= 0;
	framenum		= 0;
	sessionCommand = "";
	nextGibTime		= 0;

	portalSkyEnt			= NULL;

	portalSkyActive			= false;



	vacuumAreaNum = -1;		// if an info_vacuum is spawned, it will set this

	if ( !editEntities ) {
		editEntities = new idEditEntities;
	}

	gravity.Set( 0, 0, -g_gravity.GetFloat() );

	spawnArgs.Clear();

	skipCinematic = false;
	inCinematic = false;
	cinematicSkipTime = 0;
	cinematicStopTime = 0;
	cinematicMaxSkipTime = 0;

	clip.Init();
	pvs.Init();


	// this will always fail for now, have not yet written the map compile
	m_sndPropLoader->CompileMap( mapFile );

	playerPVS.i = -1;
	playerConnectedAreas.i = -1;

	// load navigation system for all the different monster sizes
	for( i = 0; i < aasNames.Num(); i++ ) {
		aasList[ i ]->Init( idStr( mapFileName ).SetFileExtension( aasNames[ i ] ).c_str(), mapFile->GetGeometryCRC() );
	}

	/*!
	* The Dark Mod LAS: Init the Light Awareness System
	* This must occur AFTER the AAS list is loaded
	*/
	LAS.initialize();

	// clear the smoke particle free list
	smokeParticles->Init();

	// cache miscellanious media references
	FindEntityDef( "preCacheExtras", false );

	if ( !sameMap ) {
		mapFile->RemovePrimitiveData();
	}

	// greebo: Reset the flag. When a map is loaded, the success screen is definitely not shown anymore.
	// This is meant to catch cases where the player is reloading a map from the console without clicking
	// the "Continue" button on the success GUI. I can't stop him, so I need to track this here.
	successScreenActive = false;
}

/*
===================
idGameLocal::LocalMapRestart
===================
*/
void idGameLocal::LocalMapRestart( ) {
	int i, latchSpawnCount;

	Printf( "----------- Game Map Restart ------------\n" );

	gamestate = GAMESTATE_SHUTDOWN;

	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		if ( entities[ i ] && entities[ i ]->IsType( idPlayer::Type ) ) {
			static_cast< idPlayer * >( entities[ i ] )->PrepareForRestart();
		}
	}

	eventQueue.Shutdown();
	savedEventQueue.Shutdown();

	MapClear( false );

	// clear the smoke particle free list
	smokeParticles->Init();

	// clear the sound system
	if ( gameSoundWorld ) {
		gameSoundWorld->ClearAllSoundEmitters();
	}

	/*!
	* The Dark Mod LAS: Init the LAS
	*/
	LAS.initialize();

	// the spawnCount is reset to zero temporarily to spawn the map entities with the same spawnId
	// if we don't do that, network clients are confused and don't show any map entities
	latchSpawnCount = spawnCount;
	spawnCount = INITIAL_SPAWN_COUNT;

	gamestate = GAMESTATE_STARTUP;

	program.Restart();

	InitScriptForMap();

	MapPopulate();

	// once the map is populated, set the spawnCount back to where it was so we don't risk any collision
	// (note that if there are no players in the game, we could just leave it at it's current value)
	spawnCount = latchSpawnCount;

	// setup the client entities again
	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		if ( entities[ i ] && entities[ i ]->IsType( idPlayer::Type ) ) {
			static_cast< idPlayer * >( entities[ i ] )->Restart();
		}
	}

	gamestate = GAMESTATE_ACTIVE;
	m_MissionResult = MISSION_INPROGRESS;

	Printf( "--------------------------------------\n" );
}

/*
===================
idGameLocal::MapRestart
===================
*/
void idGameLocal::MapRestart( ) {
	idBitMsg	outMsg;
	byte		msgBuf[MAX_GAME_MESSAGE_SIZE];
	idDict		newInfo;
	int			i;
	const idKeyValue *keyval, *keyval2;

	if ( isClient ) {
		LocalMapRestart();
	} else {
		newInfo = *cvarSystem->MoveCVarsToDict( CVAR_SERVERINFO );
		for ( i = 0; i < newInfo.GetNumKeyVals(); i++ ) {
			keyval = newInfo.GetKeyVal( i );
			keyval2 = serverInfo.FindKey( keyval->GetKey() );
			if ( !keyval2 ) {
				break;
			}
			// a select set of si_ changes will cause a full restart of the server
			if ( keyval->GetValue().Cmp( keyval2->GetValue() ) &&
				( !keyval->GetKey().Cmp( "si_pure" ) || !keyval->GetKey().Cmp( "si_map" ) ) ) {
				break;
			}
		}
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "rescanSI" );
		if ( i != newInfo.GetNumKeyVals() ) {
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "nextMap" );
		} else {
			outMsg.Init( msgBuf, sizeof( msgBuf ) );
			outMsg.WriteByte( GAME_RELIABLE_MESSAGE_RESTART );
			outMsg.WriteBits( 1, 1 );
			outMsg.WriteDeltaDict( serverInfo, NULL );
			networkSystem->ServerSendReliableMessage( -1, outMsg );

			LocalMapRestart();
			mpGame.MapRestart();
		}
	}
}

/*
===================
idGameLocal::MapRestart_f
===================
*/
void idGameLocal::MapRestart_f( const idCmdArgs &args ) {
	if ( !gameLocal.isMultiplayer || gameLocal.isClient ) {
		common->Printf( "server is not running - use spawnServer\n" );
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "spawnServer\n" );
		return;
	}

	gameLocal.MapRestart( );
}

/*
===================
idGameLocal::NextMap
===================
*/
bool idGameLocal::NextMap( void ) {
	const function_t	*func;
	idThread			*thread;
	idDict				newInfo;
	const idKeyValue	*keyval, *keyval2;
	int					i;

	if ( !g_mapCycle.GetString()[0] ) {
		Printf( common->GetLanguageDict()->GetString( "#str_04294" ) );
		return false;
	}
	if ( fileSystem->ReadFile( g_mapCycle.GetString(), NULL, NULL ) < 0 ) {
		if ( fileSystem->ReadFile( va( "%s.scriptcfg", g_mapCycle.GetString() ), NULL, NULL ) < 0 ) {
			Printf( "map cycle script '%s': not found\n", g_mapCycle.GetString() );
			return false;
		} else {
			g_mapCycle.SetString( va( "%s.scriptcfg", g_mapCycle.GetString() ) );
		}
	}

	Printf( "map cycle script: '%s'\n", g_mapCycle.GetString() );
	func = program.FindFunction( "mapcycle::cycle" );
	if ( !func ) {
		program.CompileFile( g_mapCycle.GetString() );
		func = program.FindFunction( "mapcycle::cycle" );
	}
	if ( !func ) {
		Printf( "Couldn't find mapcycle::cycle\n" );
		return false;
	}
	thread = new idThread( func );
	thread->Start();
	delete thread;

	newInfo = *cvarSystem->MoveCVarsToDict( CVAR_SERVERINFO );
	for ( i = 0; i < newInfo.GetNumKeyVals(); i++ ) {
		keyval = newInfo.GetKeyVal( i );
		keyval2 = serverInfo.FindKey( keyval->GetKey() );
		if ( !keyval2 || keyval->GetValue().Cmp( keyval2->GetValue() ) ) {
			break;
		}
	}
	return ( i != newInfo.GetNumKeyVals() );
}

/*
===================
idGameLocal::NextMap_f
===================
*/
void idGameLocal::NextMap_f( const idCmdArgs &args ) {
	if ( !gameLocal.isMultiplayer || gameLocal.isClient ) {
		common->Printf( "server is not running\n" );
		return;
	}

	gameLocal.NextMap( );
	// next map was either voted for or triggered by a server command - always restart
	gameLocal.MapRestart( );
}

/*
===================
idGameLocal::MapPopulate
Dark Mod: Sound prop initialization added
===================
*/
void idGameLocal::MapPopulate( void ) {

	if ( isMultiplayer ) {
		cvarSystem->SetCVarBool( "r_skipSpecular", false );
	}

	// parse the key/value pairs and spawn entities
	SpawnMapEntities();

	// mark location entities in all connected areas
	SpreadLocations();

	// prepare the list of randomized initial spawn spots
	RandomizeInitialSpawns( );

	// spawnCount - 1 is the number of entities spawned into the map, their indexes started at MAX_CLIENTS (included)
	// mapSpawnCount is used as the max index of map entities, it's the first index of non-map entities
	mapSpawnCount = MAX_CLIENTS + spawnCount - 1;

	// read in the soundprop data for various locations
	m_sndPropLoader->FillLocationData();

	// Transfer sound prop data from loader to gameplay object
	m_sndProp->SetupFromLoader( m_sndPropLoader );

	m_sndPropLoader->Shutdown();
	
	// Initialise the escape point manager after all the entities have been spawned.
	m_EscapePointManager->InitAAS();

	// execute pending events before the very first game frame
	// this makes sure the map script main() function is called
	// before the physics are run so entities can bind correctly
	Printf( "==== Processing events ====\n" );
	idEvent::ServiceEvents();
}

/*
===================
idGameLocal::InitFromNewMap
===================
*/
void idGameLocal::InitFromNewMap( const char *mapName, idRenderWorld *renderWorld, idSoundWorld *soundWorld, bool isServer, bool isClient, int randseed ) {

	this->isServer = isServer;
	this->isClient = isClient;
	this->isMultiplayer = isServer || isClient;

	if ( mapFileName.Length() ) {
		MapShutdown();
	}

	// greebo: Clear the mission data, it might have been filled during the objectives screen display
	m_MissionData->Clear();

	Printf( "----------- Game Map Init ------------\n" );

	gamestate = GAMESTATE_STARTUP;

	gameRenderWorld = renderWorld;
	gameSoundWorld = soundWorld;

	LoadMap( mapName, randseed );

	// Instantiate our grabber entity
	m_Grabber = static_cast<CGrabber*>(CGrabber::Type.CreateInstance());

	// greebo: Initialize the Difficulty Manager, before any entities are spawned
	m_DifficultyManager.Init(mapFile);
	m_ConversationSystem->Init(mapFile);
	
	InitScriptForMap();

	MapPopulate();

	// ishtvan: Set the player variable on the grabber
	//m_Grabber->SetPlayer( GetLocalPlayer() ); // greebo: GetLocalPlayer() returns NULL, moved to idPlayer::Spawn()

	// greebo: Add the elevator reachabilities to the AAS
	SetupEAS();

	// initialize the AI relationships based on worldspawn
	m_RelationsManager->SetFromArgs( &world->spawnArgs );
	mpGame.Reset();

	mpGame.Precache();

	// free up any unused animations
	animationLib.FlushUnusedAnims();

	gamestate = GAMESTATE_ACTIVE;
	m_MissionResult = MISSION_INPROGRESS;

	// We need an objectives update now that we've loaded the map
	m_MissionData->ClearGUIState();

	Printf( "--------------------------------------\n" );
}

/*
=================
idGameLocal::InitFromSaveGame
=================
*/
bool idGameLocal::InitFromSaveGame( const char *mapName, idRenderWorld *renderWorld, idSoundWorld *soundWorld, idFile *saveGameFile ) {
	int i;
	int num;
	idEntity *ent;
	idDict si;

	if ( mapFileName.Length() ) {
		MapShutdown();
	}

	Printf( "------- Game Map Init SaveGame -------\n" );

	gamestate = GAMESTATE_STARTUP;

	gameRenderWorld = renderWorld;
	gameSoundWorld = soundWorld;

	idRestoreGame savegame( saveGameFile );

	savegame.ReadBuildNumber();

	// Create the list of all objects in the game
	savegame.CreateObjects();

	// Load the idProgram, also checking to make sure scripting hasn't changed since the savegame
	if ( program.Restore( &savegame ) == false ) {

		// Abort the load process, and let the session know so that it can restart the level
		// with the player persistent data.
		savegame.DeleteObjects();
		program.Restart();

		return false;
	}

	// load the map needed for this savegame
	LoadMap( mapName, 0 );

	// Restore the global hiding spot search collection
	CHidingSpotSearchCollection::Instance().Restore(&savegame);

	// Restore our grabber pointer
	savegame.ReadObject( reinterpret_cast<idClass*&>(m_Grabber) );

	m_DifficultyManager.Restore(&savegame);

	for ( i = 0; i < NumAAS(); i++)
	{
		idAAS* aas = GetAAS(i);
		if (aas != NULL)
		{
			aas->Restore(&savegame);
		}
	}

	m_GamePlayTimer.Restore(&savegame);
	m_GamePlayTimer.SetEnabled(false);
	m_AreaManager.Restore(&savegame);
	m_ConversationSystem->Restore(&savegame);

#ifdef TIMING_BUILD
	debugtools::TimerManager::Instance().Restore(&savegame);
#endif

	savegame.ReadInt( i );
	g_skill.SetInteger( i );

	// precache the player
	FindEntityDef( "player_tdm_thief", false );

	// precache the empty model (used by idEntity::m_renderTrigger)
	renderModelManager->FindModel( cv_empty_model.GetString() );

	SpawnLightgemEntity();

	// precache any media specified in the map
	for ( i = 0; i < mapFile->GetNumEntities(); i++ ) {
		idMapEntity *mapEnt = mapFile->GetEntity( i );

		if ( !InhibitEntitySpawn( mapEnt->epairs ) ) {
			CacheDictionaryMedia( &mapEnt->epairs );
			const char *classname = mapEnt->epairs.GetString( "classname" );
			if ( classname != '\0' ) {
				FindEntityDef( classname, false );
			}
		}
	}

	savegame.ReadDict( &si );
	SetServerInfo( si );

	savegame.ReadInt( numClients );
	for( i = 0; i < numClients; i++ ) {
		savegame.ReadDict( &userInfo[ i ] );
		savegame.ReadUsercmd( usercmds[ i ] );
		savegame.ReadDict( &persistentPlayerInfo[ i ] );
	}

	for( i = 0; i < MAX_GENTITIES; i++ ) {
		savegame.ReadObject( reinterpret_cast<idClass *&>( entities[ i ] ) );
		savegame.ReadInt( spawnIds[ i ] );

		// restore the entityNumber
		if ( entities[ i ] != NULL ) {
			entities[ i ]->entityNumber = i;
		}
	}

	savegame.ReadInt( firstFreeIndex );
	savegame.ReadInt( num_entities );

	// enityHash is restored by idEntity::Restore setting the entity name.

	savegame.ReadObject( reinterpret_cast<idClass *&>( world ) );

	savegame.ReadInt( num );
	for( i = 0; i < num; i++ ) {
		savegame.ReadObject( reinterpret_cast<idClass *&>( ent ) );
		assert( ent );
		if ( ent ) {
			ent->spawnNode.AddToEnd( spawnedEntities );
		}
	}

	savegame.ReadInt( num );
	for( i = 0; i < num; i++ ) {
		savegame.ReadObject( reinterpret_cast<idClass *&>( ent ) );
		assert( ent );
		if ( ent ) {
			ent->activeNode.AddToEnd( activeEntities );
		}
	}

	savegame.ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		idAI* ai(NULL);
		savegame.ReadObject(reinterpret_cast<idClass *&>(ai));
		assert(ai != NULL);

		if (ai != NULL)
		{
			ai->aiNode.AddToEnd(spawnedAI);
		}
	}

	savegame.ReadInt( numEntitiesToDeactivate );
	savegame.ReadBool( sortPushers );
	savegame.ReadBool( sortTeamMasters );
	savegame.ReadDict( &persistentLevelInfo );

	for( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ ) {
		savegame.ReadFloat( globalShaderParms[ i ] );
	}

	savegame.ReadBool(successScreenActive);

	savegame.ReadInt( i );
	random.SetSeed( i );

	savegame.ReadObject( reinterpret_cast<idClass *&>( frameCommandThread ) );

	// clip
	// push
	// pvs

	// testmodel = "<NULL>"
	// testFx = "<NULL>"

	savegame.ReadString( sessionCommand );

	// FIXME: save smoke particles

	savegame.ReadInt( cinematicSkipTime );
	savegame.ReadInt( cinematicStopTime );
	savegame.ReadInt( cinematicMaxSkipTime );
	savegame.ReadBool( inCinematic );
	savegame.ReadBool( skipCinematic );

	savegame.ReadBool( isMultiplayer );
	savegame.ReadInt( (int &)gameType );

	savegame.ReadInt( framenum );
	savegame.ReadInt( previousTime );
	savegame.ReadInt( time );

	savegame.ReadInt( vacuumAreaNum );

	savegame.ReadInt( entityDefBits );
	savegame.ReadBool( isServer );
	savegame.ReadBool( isClient );

	savegame.ReadInt( localClientNum );

	// snapshotEntities is used for multiplayer only

	savegame.ReadInt( realClientTime );
	savegame.ReadBool( isNewFrame );
	savegame.ReadFloat( clientSmoothing );

	portalSkyEnt.Restore( &savegame );

	savegame.ReadBool( portalSkyActive );



	savegame.ReadBool( mapCycleLoaded );
	savegame.ReadInt( spawnCount );

	savegame.ReadInt( num );
	if ( num ) {
		if ( num != gameRenderWorld->NumAreas() ) {
			savegame.Error( "idGameLocal::InitFromSaveGame: number of areas in map differs from save game." );
		}

		locationEntities = new idLocationEntity *[ num ];
		for( i = 0; i < num; i++ ) {
			savegame.ReadObject( reinterpret_cast<idClass *&>( locationEntities[ i ] ) );
		}
	}

	savegame.ReadObject( reinterpret_cast<idClass *&>( camera ) );

	savegame.ReadMaterial( globalMaterial );

	lastAIAlertEntity.Restore( &savegame );
	savegame.ReadInt( lastAIAlertTime );

	savegame.ReadDict( &spawnArgs );

	savegame.ReadInt( playerPVS.i );
	savegame.ReadInt( (int &)playerPVS.h );
	savegame.ReadInt( playerConnectedAreas.i );
	savegame.ReadInt( (int &)playerConnectedAreas.h );

	savegame.ReadVec3( gravity );

	// gamestate is restored after restoring everything else

	savegame.ReadBool( influenceActive );
	savegame.ReadInt( nextGibTime );

	// Restore the lightgem entity pointer
	m_LightgemSurface.Restore(&savegame);
	savegame.ReadBool(m_DoLightgem);
	savegame.ReadInt(m_LightgemShotSpot);
	for (int i = 0; i < DARKMOD_LG_MAX_RENDERPASSES; i++)
	{
		savegame.ReadFloat(m_LightgemShotValue[i]);
	}

	m_LightgemSurface.GetEntity()->GetRenderEntity()->allowSurfaceInViewID = DARKMOD_LG_VIEWID;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->suppressShadowInViewID = 0;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noDynamicInteractions = false;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noShadow = true;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noSelfShadow = true;
	DM_LOG(LC_LIGHT, LT_INFO)LOGSTRING("LightgemSurface: [%08lX]\r", m_LightgemSurface.GetEntity());

	m_sndProp->Restore(&savegame);
	m_MissionData->Restore(&savegame);

	int missResult;
	savegame.ReadInt(missResult);
	m_MissionResult = static_cast<EMissionResult>(missResult);

	savegame.ReadInt(m_HighestSRId);

	savegame.ReadInt(num);
	m_Timer.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		m_Timer[i] = new CStimResponseTimer;
		m_Timer[i]->Restore(&savegame);
	}

	// The list to take all the values, they will be restored later on
	idList<int> tempStimTimerIdList;
	savegame.ReadInt(num);
	tempStimTimerIdList.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		savegame.ReadInt(tempStimTimerIdList[i]);
	}

	savegame.ReadInt(num);
	m_StimEntity.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		m_StimEntity[i].Restore(&savegame);
	}

	savegame.ReadInt(num);
	m_RespEntity.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		m_RespEntity[i].Restore(&savegame);
	}

	m_EscapePointManager->Restore(&savegame);

	// greebo: Restore the maximum frob distance
	savegame.ReadFloat(g_Global.m_MaxFrobDistance);

	// spawnSpots
	// initialSpots
	// currentInitialSpot
	// newInfo
	// makingBuild
	// shakeSounds

	// Read out pending events
	idEvent::Restore( &savegame );

	// Restore the DarkMod player object, this contains a lot of other TDM-related classes
	g_Global.m_DarkModPlayer->Restore(&savegame);

	savegame.RestoreObjects();

	mpGame.Reset();

	mpGame.Precache();

	// free up any unused animations
	animationLib.FlushUnusedAnims();

	gamestate = GAMESTATE_ACTIVE;

	//FIX: Set the walkspeed back to the stored value.
	pm_walkspeed.SetFloat( m_walkSpeed );

	// Restore the physics pointer in the grabber.
	gameLocal.m_Grabber->SetPhysicsFromDragEntity();

	// Restore the CStim* pointers in the m_StimTimer list
	m_StimTimer.SetNum(tempStimTimerIdList.Num());
	for (int i = 0; i < tempStimTimerIdList.Num(); i++)
	{
		m_StimTimer[i] = static_cast<CStim*>(FindStimResponse(tempStimTimerIdList[i]));
	}

	Printf( "--------------------------------------\n" );

	// Restart the timer
	m_GamePlayTimer.Start();

	return true;
}

/*
===========
idGameLocal::MapClear
===========
*/
void idGameLocal::MapClear( bool clearClients ) {
	int i;

	for( i = ( clearClients ? 0 : MAX_CLIENTS ); i < MAX_GENTITIES; i++ ) {
		delete entities[ i ];
		// ~idEntity is in charge of setting the pointer to NULL
		// it will also clear pending events for this entity
		assert( !entities[ i ] );
		spawnIds[ i ] = -1;
	}

	entityHash.Clear( 1024, MAX_GENTITIES );

	if ( !clearClients ) {
		// add back the hashes of the clients
		for ( i = 0; i < MAX_CLIENTS; i++ ) {
			if ( !entities[ i ] ) {
				continue;
			}
			entityHash.Add( entityHash.GenerateKey( entities[ i ]->name.c_str(), true ), i );
		}
	}

	delete frameCommandThread;
	frameCommandThread = NULL;

	if ( editEntities ) {
		delete editEntities;
		editEntities = NULL;
	}

	delete[] locationEntities;
	locationEntities = NULL;
}

/*
===========
idGameLocal::MapShutdown
============
*/
void idGameLocal::MapShutdown( void ) {
	Printf( "--------- Game Map Shutdown ----------\n" );
	
	gamestate = GAMESTATE_SHUTDOWN;

	if ( gameRenderWorld ) {
		// clear any debug lines, text, and polygons
		gameRenderWorld->DebugClearLines( 0 );
		gameRenderWorld->DebugClearPolygons( 0 );
	}

	// clear out camera if we're in a cinematic
	if ( inCinematic ) {
		camera = NULL;
		inCinematic = false;
	}

	// Run the grabber->clear() method before the entities get deleted from the map
	if (m_Grabber != NULL)
	{
		m_Grabber->Clear();
	}
	
	MapClear( true );

	// reset the script to the state it was before the map was started
	program.Restart();

	if ( smokeParticles ) {
		smokeParticles->Shutdown();
	}

	/*!
	* The Dark Mod LAS: shut down the LAS
	*/
	LAS.shutDown();

	pvs.Shutdown();

	// Remove the grabber entity itself (note that it's safe to pass NULL pointers to delete)
	delete m_Grabber;
	m_Grabber = NULL;

	m_sndProp->Clear();
	m_RelationsManager->Clear();
	m_ConversationSystem->Clear();

	clip.Shutdown();
	idClipModel::ClearTraceModelCache();

	ShutdownAsyncNetwork();

	mapFileName.Clear();

	gameRenderWorld = NULL;
	gameSoundWorld = NULL;

	gamestate = GAMESTATE_NOMAP;

	// Save the current walkspeed
	m_walkSpeed = pm_walkspeed.GetFloat();

	Printf( "--------------------------------------\n" );
}

/*
===================
idGameLocal::DumpOggSounds
===================
*/
void idGameLocal::DumpOggSounds( void ) {
	int i, j, k, size, totalSize;
	idFile *file;
	idStrList oggSounds, weaponSounds;
	const idSoundShader *soundShader;
	const soundShaderParms_t *parms;
	idStr soundName;

	for ( i = 0; i < declManager->GetNumDecls( DECL_SOUND ); i++ ) {
		soundShader = static_cast<const idSoundShader *>(declManager->DeclByIndex( DECL_SOUND, i, false ));
		parms = soundShader->GetParms();

		if ( soundShader->EverReferenced() && soundShader->GetState() != DS_DEFAULTED ) {

			const_cast<idSoundShader *>(soundShader)->EnsureNotPurged();

			for ( j = 0; j < soundShader->GetNumSounds(); j++ ) {
				soundName = soundShader->GetSound( j );
				soundName.BackSlashesToSlashes();

				// don't OGG sounds that cause a shake because that would
				// cause continuous seeking on the OGG file which is expensive
				if ( parms->shakes != 0.0f ) {
					shakeSounds.AddUnique( soundName );
					continue;
				}

				// if not voice over or combat chatter
				if (	soundName.Find( "/vo/", false ) == -1 &&
						soundName.Find( "/combat_chatter/", false ) == -1 &&
						soundName.Find( "/bfgcarnage/", false ) == -1 &&
						soundName.Find( "/enpro/", false ) == - 1 &&
						soundName.Find( "/soulcube/energize_01.wav", false ) == -1 ) {
					// don't OGG weapon sounds
					if (	soundName.Find( "weapon", false ) != -1 ||
							soundName.Find( "gun", false ) != -1 ||
							soundName.Find( "bullet", false ) != -1 ||
							soundName.Find( "bfg", false ) != -1 ||
							soundName.Find( "plasma", false ) != -1 ) {
						weaponSounds.AddUnique( soundName );
						continue;
					}
				}

				for ( k = 0; k < shakeSounds.Num(); k++ ) {
					if ( shakeSounds[k].IcmpPath( soundName ) == 0 ) {
						break;
					}
				}
				if ( k < shakeSounds.Num() ) {
					continue;
				}

				oggSounds.AddUnique( soundName );
			}
		}
	}

	file = fileSystem->OpenFileWrite( "makeogg.bat", "fs_savepath" );
	if ( file == NULL ) {
		common->Warning( "Couldn't open makeogg.bat" );
		return;
	}

	// list all the shake sounds
	totalSize = 0;
	for ( i = 0; i < shakeSounds.Num(); i++ ) {
		size = fileSystem->ReadFile( shakeSounds[i], NULL, NULL );
		totalSize += size;
		shakeSounds[i].Replace( "/", "\\" );
		file->Printf( "echo \"%s\" (%d kB)\n", shakeSounds[i].c_str(), size >> 10 );
	}
	file->Printf( "echo %d kB in shake sounds\n\n\n", totalSize >> 10 );

	// list all the weapon sounds
	totalSize = 0;
	for ( i = 0; i < weaponSounds.Num(); i++ ) {
		size = fileSystem->ReadFile( weaponSounds[i], NULL, NULL );
		totalSize += size;
		weaponSounds[i].Replace( "/", "\\" );
		file->Printf( "echo \"%s\" (%d kB)\n", weaponSounds[i].c_str(), size >> 10 );
	}
	file->Printf( "echo %d kB in weapon sounds\n\n\n", totalSize >> 10 );

	// list commands to convert all other sounds to ogg
	totalSize = 0;
	for ( i = 0; i < oggSounds.Num(); i++ ) {
		size = fileSystem->ReadFile( oggSounds[i], NULL, NULL );
		totalSize += size;
		oggSounds[i].Replace( "/", "\\" );
		file->Printf( "w:\\doom\\ogg\\oggenc -q 0 \"c:\\doom\\base\\%s\"\n", oggSounds[i].c_str() );
		file->Printf( "del \"c:\\doom\\base\\%s\"\n", oggSounds[i].c_str() );
	}
	file->Printf( "\n\necho %d kB in OGG sounds\n\n\n", totalSize >> 10 );

	fileSystem->CloseFile( file );

	shakeSounds.Clear();
}

/*
===================
idGameLocal::GetShakeSounds
===================
*/
void idGameLocal::GetShakeSounds( const idDict *dict ) {
	const idSoundShader *soundShader;
	const char *soundShaderName;
	idStr soundName;

	soundShaderName = dict->GetString( "s_shader" );
	if ( soundShaderName != '\0' && dict->GetFloat( "s_shakes" ) != 0.0f ) {
		soundShader = declManager->FindSound( soundShaderName );

		for ( int i = 0; i < soundShader->GetNumSounds(); i++ ) {
			soundName = soundShader->GetSound( i );
			soundName.BackSlashesToSlashes();

			shakeSounds.AddUnique( soundName );
		}
	}
}

/*
===================
idGameLocal::CacheDictionaryMedia

This is called after parsing an EntityDef and for each entity spawnArgs before
merging the entitydef.  It could be done post-merge, but that would
avoid the fast pre-cache check associated with each entityDef
===================
*/
void idGameLocal::CacheDictionaryMedia( const idDict *dict ) {
	const idKeyValue *kv;

	if ( dict == NULL ) {
		if ( cvarSystem->GetCVarBool( "com_makingBuild") ) {
			DumpOggSounds();
		}
		return;
	}

	if ( cvarSystem->GetCVarBool( "com_makingBuild" ) ) {
		GetShakeSounds( dict );
	}

	kv = dict->MatchPrefix( "model" );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->MediaPrint( "Precaching model %s\n", kv->GetValue().c_str() );
			// precache model/animations
			if ( declManager->FindType( DECL_MODELDEF, kv->GetValue(), false ) == NULL ) {
				// precache the render model
				renderModelManager->FindModel( kv->GetValue() );
				// precache .cm files only
				collisionModelManager->LoadModel( kv->GetValue(), true );
				// load any tdm_matinfo decls for materials referenced by the model
				tdmDeclTDM_MatInfo::precacheModel( renderModelManager->FindModel( kv->GetValue() ) );
			}
		}
		kv = dict->MatchPrefix( "model", kv );
	}

	kv = dict->FindKey( "s_shader" );
	if ( kv && kv->GetValue().Length() ) {
		declManager->FindType( DECL_SOUND, kv->GetValue() );
	}

	kv = dict->MatchPrefix( "snd", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->FindType( DECL_SOUND, kv->GetValue() );
		}
		kv = dict->MatchPrefix( "snd", kv );
	}


	kv = dict->MatchPrefix( "gui", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			if ( !idStr::Icmp( kv->GetKey(), "gui_noninteractive" )
				|| !idStr::Icmpn( kv->GetKey(), "gui_parm", 8 )	
				|| !idStr::Icmp( kv->GetKey(), "gui_inventory" ) ) {
				// unfortunate flag names, they aren't actually a gui
			} else {
				declManager->MediaPrint( "Precaching gui %s\n", kv->GetValue().c_str() );
				idUserInterface *gui = uiManager->Alloc();
				if ( gui ) {
					gui->InitFromFile( kv->GetValue() );
					uiManager->DeAlloc( gui );
				}
			}
		}
		kv = dict->MatchPrefix( "gui", kv );
	}

	kv = dict->FindKey( "texture" );
	if ( kv && kv->GetValue().Length() ) {
		declManager->FindType( DECL_MATERIAL, kv->GetValue() );
		declManager->FindType( DECL_TDM_MATINFO, kv->GetValue() );
	}

	kv = dict->MatchPrefix( "mtr", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->FindType( DECL_MATERIAL, kv->GetValue() );
			declManager->FindType( DECL_TDM_MATINFO, kv->GetValue() );
		}
		kv = dict->MatchPrefix( "mtr", kv );
	}

	// handles hud icons
	kv = dict->MatchPrefix( "inv_icon", NULL );
	while ( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->FindType( DECL_MATERIAL, kv->GetValue() );
		}
		kv = dict->MatchPrefix( "inv_icon", kv );
	}

	// handles teleport fx.. this is not ideal but the actual decision on which fx to use
	// is handled by script code based on the teleport number
	kv = dict->MatchPrefix( "teleport", NULL );
	if ( kv && kv->GetValue().Length() ) {
		int teleportType = atoi( kv->GetValue() );
		const char *p = ( teleportType ) ? va( "fx/teleporter%i.fx", teleportType ) : "fx/teleporter.fx";
		declManager->FindType( DECL_FX, p );
	}

	kv = dict->MatchPrefix( "fx", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->MediaPrint( "Precaching fx %s\n", kv->GetValue().c_str() );
			declManager->FindType( DECL_FX, kv->GetValue() );
		}
		kv = dict->MatchPrefix( "fx", kv );
	}

	kv = dict->MatchPrefix( "smoke", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			idStr prtName = kv->GetValue();
			int dash = prtName.Find('-');
			if ( dash > 0 ) {
				prtName = prtName.Left( dash );
			}
			declManager->FindType( DECL_PARTICLE, prtName );
		}
		kv = dict->MatchPrefix( "smoke", kv );
	}

	kv = dict->MatchPrefix( "skin", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->MediaPrint( "Precaching skin %s\n", kv->GetValue().c_str() );
			declManager->FindType( DECL_SKIN, kv->GetValue() );
		}
		kv = dict->MatchPrefix( "skin", kv );
	}

	kv = dict->MatchPrefix( "def", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			FindEntityDef( kv->GetValue().c_str(), false );
		}
		kv = dict->MatchPrefix( "def", kv );
	}

	kv = dict->MatchPrefix( "pda_name", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->FindType( DECL_PDA, kv->GetValue().c_str(), false );
		}
		kv = dict->MatchPrefix( "pda_name", kv );
	}

	kv = dict->MatchPrefix( "video", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->FindType( DECL_VIDEO, kv->GetValue().c_str(), false );
		}
		kv = dict->MatchPrefix( "video", kv );
	}

	kv = dict->MatchPrefix( "audio", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->FindType( DECL_AUDIO, kv->GetValue().c_str(), false );
		}
		kv = dict->MatchPrefix( "audio", kv );
	}

	kv = dict->MatchPrefix( "xdata", NULL );
	while( kv ) {
		if ( kv->GetValue().Length() ) {
			declManager->FindType( DECL_XDATA, kv->GetValue().c_str(), false );
		}
		kv = dict->MatchPrefix( "xdata", kv );
	}
}

/*
===========
idGameLocal::InitScriptForMap
============
*/
void idGameLocal::InitScriptForMap( void ) {
	// create a thread to run frame commands on
	frameCommandThread = new idThread();
	frameCommandThread->ManualDelete();
	frameCommandThread->SetThreadName( "frameCommands" );

	// run the main game script function (not the level specific main)
	const function_t *func = program.FindFunction( SCRIPT_DEFAULTFUNC );
	if ( func != NULL ) {
		idThread *thread = new idThread( func );
		if ( thread->Start() ) {
			// thread has finished executing, so delete it
			delete thread;
		}
	}
}

/*
===========
idGameLocal::SpawnPlayer
============
*/
void idGameLocal::SpawnPlayer( int clientNum )
{
	idEntity	*ent;
	idDict		args;

	// they can connect
	Printf( "SpawnPlayer: %i\n", clientNum );

	args.SetInt( "spawn_entnum", clientNum );
	args.Set( "name", va( "player%d", clientNum + 1 ) );
	args.Set( "classname", isMultiplayer ? "player_tdm_thief_mp" : "player_tdm_thief" );
	if ( !SpawnEntityDef( args, &ent ) || !entities[ clientNum ] ) {
		Error( "Failed to spawn player as '%s'", args.GetString( "classname" ) );
	}

	// make sure it's a compatible class
	if ( !ent->IsType( idPlayer::Type ) ) {
		Error( "'%s' spawned the player as a '%s'.  Player spawnclass must be a subclass of idPlayer.", args.GetString( "classname" ), ent->GetClassname() );
	}

	if ( clientNum >= numClients ) {
		numClients = clientNum + 1;
	}

	idPlayer *player;	
	player = GetLocalPlayer();

	mpGame.SpawnPlayer( clientNum );
}

/*
================
idGameLocal::GetClientByNum
================
*/
idPlayer *idGameLocal::GetClientByNum( int current ) const {
	if ( current < 0 || current >= numClients ) {
		current = 0;
	}
	if ( entities[current] ) {
		return static_cast<idPlayer *>( entities[ current ] );
	}
	return NULL;
}

/*
================
idGameLocal::GetClientByName
================
*/
idPlayer *idGameLocal::GetClientByName( const char *name ) const {
	int i;
	idEntity *ent;
	for ( i = 0 ; i < numClients ; i++ ) {
		ent = entities[ i ];
		if ( ent && ent->IsType( idPlayer::Type ) ) {
			if ( idStr::IcmpNoColor( name, userInfo[ i ].GetString( "ui_name" ) ) == 0 ) {
				return static_cast<idPlayer *>( ent );
			}
		}
	}
	return NULL;
}

/*
================
idGameLocal::GetClientByCmdArgs
================
*/
idPlayer *idGameLocal::GetClientByCmdArgs( const idCmdArgs &args ) const {
	idPlayer *player;
	idStr client = args.Argv( 1 );
	if ( !client.Length() ) {
		return NULL;
	}
	// we don't allow numeric ui_name so this can't go wrong
	if ( client.IsNumeric() ) {
		player = GetClientByNum( atoi( client.c_str() ) );
	} else {
		player = GetClientByName( client.c_str() );
	}
	if ( !player ) {
		common->Printf( "Player '%s' not found\n", client.c_str() );
	}
	return player;
}

/*
================
idGameLocal::GetNextClientNum
================
*/
int idGameLocal::GetNextClientNum( int _current ) const {
	int i, current;

	current = 0;
	for ( i = 0; i < numClients; i++) {
		current = ( _current + i + 1 ) % numClients;
		if ( entities[ current ] && entities[ current ]->IsType( idPlayer::Type ) ) {
			return current;
		}
	}

	return current;
}

/*
================
idGameLocal::GetLocalPlayer

Nothing in the game tic should EVER make a decision based on what the
local client number is, it shouldn't even be aware that there is a
draw phase even happening.  This just returns client 0, which will
be correct for single player.
================
*/
idPlayer *idGameLocal::GetLocalPlayer() const {
	if ( localClientNum < 0 ) {
		return NULL;
	}

	if ( !entities[ localClientNum ] || !entities[ localClientNum ]->IsType( idPlayer::Type ) ) {
		// not fully in game yet
		return NULL;
	}
	return static_cast<idPlayer *>( entities[ localClientNum ] );
}

/*
================
idGameLocal::SetupClientPVS
================
*/
pvsHandle_t idGameLocal::GetClientPVS( idPlayer *player, pvsType_t type ) {
	if ( player->GetPrivateCameraView() ) {
		return pvs.SetupCurrentPVS( player->GetPrivateCameraView()->GetPVSAreas(), player->GetPrivateCameraView()->GetNumPVSAreas() );
	} else if ( camera ) {
		return pvs.SetupCurrentPVS( camera->GetPVSAreas(), camera->GetNumPVSAreas() );
	} else {
		return pvs.SetupCurrentPVS( player->GetPVSAreas(), player->GetNumPVSAreas() );
	}
}

/*
================
idGameLocal::SetupPlayerPVS
================
*/
void idGameLocal::SetupPlayerPVS( void ) {
	int			i;
	idEntity *	ent;
	idPlayer *	player;
	pvsHandle_t	otherPVS, newPVS;

	playerPVS.i = -1;
	for ( i = 0; i < numClients; i++ ) {
		ent = entities[i];
		if ( !ent || !ent->IsType( idPlayer::Type ) ) {
			continue;
		}

		player = static_cast<idPlayer *>(ent);

		if ( playerPVS.i == -1 ) {
			playerPVS = GetClientPVS( player, PVS_NORMAL );
		} else {
			otherPVS = GetClientPVS( player, PVS_NORMAL );
			newPVS = pvs.MergeCurrentPVS( playerPVS, otherPVS );
			pvs.FreeCurrentPVS( playerPVS );
			pvs.FreeCurrentPVS( otherPVS );
			playerPVS = newPVS;
		}

		if ( playerConnectedAreas.i == -1 ) {
			playerConnectedAreas = GetClientPVS( player, PVS_CONNECTED_AREAS );
		} else {
			otherPVS = GetClientPVS( player, PVS_CONNECTED_AREAS );
			newPVS = pvs.MergeCurrentPVS( playerConnectedAreas, otherPVS );
			pvs.FreeCurrentPVS( playerConnectedAreas );
			pvs.FreeCurrentPVS( otherPVS );
			playerConnectedAreas = newPVS;
		}


		// if portalSky is preset, then merge into pvs so we get rotating brushes, etc

		if ( portalSkyEnt.GetEntity() ) {
			idEntity *skyEnt = portalSkyEnt.GetEntity();

			otherPVS = pvs.SetupCurrentPVS( skyEnt->GetPVSAreas(), skyEnt->GetNumPVSAreas() );
			newPVS = pvs.MergeCurrentPVS( playerPVS, otherPVS );
			pvs.FreeCurrentPVS( playerPVS );
			pvs.FreeCurrentPVS( otherPVS );
			playerPVS = newPVS;

			otherPVS = pvs.SetupCurrentPVS( skyEnt->GetPVSAreas(), skyEnt->GetNumPVSAreas() );
			newPVS = pvs.MergeCurrentPVS( playerConnectedAreas, otherPVS );
			pvs.FreeCurrentPVS( playerConnectedAreas );
			pvs.FreeCurrentPVS( otherPVS );
			playerConnectedAreas = newPVS;
		}
	}
}

/*
================
idGameLocal::FreePlayerPVS
================
*/
void idGameLocal::FreePlayerPVS( void ) {
	if ( playerPVS.i != -1 ) {
		pvs.FreeCurrentPVS( playerPVS );
		playerPVS.i = -1;
	}
	if ( playerConnectedAreas.i != -1 ) {
		pvs.FreeCurrentPVS( playerConnectedAreas );
		playerConnectedAreas.i = -1;
	}
}

/*
================
idGameLocal::InPlayerPVS

  should only be called during entity thinking and event handling
================
*/
bool idGameLocal::InPlayerPVS( idEntity *ent ) const {
	if ( playerPVS.i == -1 ) {
		return false;
	}
    return pvs.InCurrentPVS( playerPVS, ent->GetPVSAreas(), ent->GetNumPVSAreas() );
}

/*
================
idGameLocal::InPlayerConnectedArea

  should only be called during entity thinking and event handling
================
*/
bool idGameLocal::InPlayerConnectedArea( idEntity *ent ) const {
	if ( playerConnectedAreas.i == -1 ) {
		return false;
	}
    return pvs.InCurrentPVS( playerConnectedAreas, ent->GetPVSAreas(), ent->GetNumPVSAreas() );
}

/*
================
idGameLocal::UpdateGravity
================
*/
void idGameLocal::UpdateGravity( void ) {
	idEntity *ent;

	if ( g_gravity.IsModified() ) {
		if ( g_gravity.GetFloat() == 0.0f ) {
			g_gravity.SetFloat( 1.0f );
		}
        gravity.Set( 0, 0, -g_gravity.GetFloat() );

		// update all physics objects
		for( ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
			if ( ent->IsType( idAFEntity_Generic::Type ) ) {
				idPhysics *phys = ent->GetPhysics();
				if ( phys ) {
					phys->SetGravity( gravity );
				}
			}
		}
		g_gravity.ClearModified();
	}
}

/*
================
idGameLocal::GetGravity
================
*/
const idVec3 &idGameLocal::GetGravity( void ) const {
	return gravity;
}

/*
================
idGameLocal::SortActiveEntityList

  Sorts the active entity list such that pushing entities come first,
  actors come next and physics team slaves appear after their master.
================
*/
void idGameLocal::SortActiveEntityList( void ) {
	idEntity *ent, *next_ent, *master, *part;

	// if the active entity list needs to be reordered to place physics team masters at the front
	if ( sortTeamMasters ) {
		for ( ent = activeEntities.Next(); ent != NULL; ent = next_ent ) {
			next_ent = ent->activeNode.Next();
			master = ent->GetTeamMaster();
			if ( master && master == ent ) {
				ent->activeNode.Remove();
				ent->activeNode.AddToFront( activeEntities );
			}
		}
	}

	// if the active entity list needs to be reordered to place pushers at the front
	if ( sortPushers ) {

		for ( ent = activeEntities.Next(); ent != NULL; ent = next_ent ) {
			next_ent = ent->activeNode.Next();
			master = ent->GetTeamMaster();
			if ( !master || master == ent ) {
				// check if there is an actor on the team
				for ( part = ent; part != NULL; part = part->GetNextTeamEntity() ) {
					if ( part->GetPhysics()->IsType( idPhysics_Actor::Type ) ) {
						break;
					}
				}
				// if there is an actor on the team
				if ( part ) {
					ent->activeNode.Remove();
					ent->activeNode.AddToFront( activeEntities );
				}
			}
		}

		for ( ent = activeEntities.Next(); ent != NULL; ent = next_ent ) {
			next_ent = ent->activeNode.Next();
			master = ent->GetTeamMaster();
			if ( !master || master == ent ) {
				// check if there is an entity on the team using parametric physics
				for ( part = ent; part != NULL; part = part->GetNextTeamEntity() ) {
					if ( part->GetPhysics()->IsType( idPhysics_Parametric::Type ) ) {
						break;
					}
				}
				// if there is an entity on the team using parametric physics
				if ( part ) {
					ent->activeNode.Remove();
					ent->activeNode.AddToFront( activeEntities );
				}
			}
		}
	}

	sortTeamMasters = false;
	sortPushers = false;
}

/*
================
idGameLocal::RunFrame
================
*/
gameReturn_t idGameLocal::RunFrame( const usercmd_t *clientCmds ) {
	idEntity *	ent;
	int			num(-1);
	float		ms;
	idTimer		timer_think, timer_events, timer_singlethink;
	gameReturn_t ret;
	idPlayer	*player;
	const renderView_t *view;
	int curframe = framenum;

	g_Global.m_Frame = curframe;
	DM_LOG(LC_FRAME, LT_INFO)LOGSTRING("Frame start\r");

#ifdef _DEBUG
	if ( isMultiplayer ) {
		assert( !isClient );
	}
#endif

	player = GetLocalPlayer();

/*
* Ishtvan: I don't think we need this anymore.
* Will delete it soon if there are no objections.

	// Before we do the actual impulse, we check if there are some impulses pending for processing.
	// FIXME: It MIGHT be that this can cause problems in case if usercmd is evaluated while the
	// impulse is processed, because it would use the same usercmd states as the actual impulse
	// key that is triggered, which could cause inconsistencies.
	for(int i = 0; i < IR_COUNT; i++)
	{
		k = &m_KeyData[i];
		if(k->KeyState == KS_UPDATED)
		{
			ucmd = player->usercmd;
			player->usercmd.impulse = k->Impulse;
			player->PerformImpulse(k->Impulse);
			player->usercmd = ucmd;
		}
	}
*/

	// Check for any activated signals, and trigger them.
	CheckSDKSignals();

	// Update the gameplay timer
	m_GamePlayTimer.Update();

	if ( !isMultiplayer && g_stopTime.GetBool() ) {
		// clear any debug lines from a previous frame
		gameRenderWorld->DebugClearLines( time + 1 );

		// set the user commands for this frame
		memcpy( usercmds, clientCmds, numClients * sizeof( usercmds[ 0 ] ) );

		if ( player ) {
			player->Think();
		}
	}
	else do
	{
		// update the game time
		framenum++;
		previousTime = time;
		time += (int)(msec * g_timeModifier.GetFloat());
		realClientTime = time;

#ifdef GAME_DLL
		// allow changing SIMD usage on the fly
		if ( com_forceGenericSIMD.IsModified() ) {
			idSIMD::InitProcessor( "game", com_forceGenericSIMD.GetBool() );
		}
#endif

		// make sure the random number counter is used each frame so random events
		// are influenced by the player's actions
		random.RandomInt();

		if ( player ) {
			// update the renderview so that any gui videos play from the right frame
			view = player->GetRenderView();
			if ( view ) {
				gameRenderWorld->SetRenderView( view );
			}
		}

		// clear any debug lines from a previous frame
		gameRenderWorld->DebugClearLines( time );

		// clear any debug polygons from a previous frame
		gameRenderWorld->DebugClearPolygons( time );

		// set the user commands for this frame
		memcpy( usercmds, clientCmds, numClients * sizeof( usercmds[ 0 ] ) );

		// free old smoke particles
		smokeParticles->FreeSmokes();

		// process events on the server
		ServerProcessEntityNetworkEventQueue();

		// update our gravity vector if needed.
		UpdateGravity();

		// create a merged pvs for all players
		SetupPlayerPVS();

		idTimer lasTimer;
		lasTimer.Clear();
		lasTimer.Start();
		// The Dark Mod
		// 10/9/2005: SophisticatedZombie
		// Update the Light Awareness System
		LAS.updateLASState();
		lasTimer.Stop();
		DM_LOG(LC_LIGHT, LT_INFO)LOGSTRING("Time to update LAS: %lf\r", lasTimer.Milliseconds());

		unsigned long ticks = static_cast<unsigned long>(sys->GetClockTicks());

		// Tick the timers. Should be done before stim/response, just to be safe. :)
		ProcessTimer(ticks);

		// TDM: Work through the active stims/responses
		ProcessStimResponse(ticks);

		// TDM: Update objective system
		m_MissionData->UpdateObjectives();

		// sort the active entity list
		SortActiveEntityList();

		timer_think.Clear();
		timer_think.Start();

		// let entities think
		if ( g_timeentities.GetFloat() ) {
			num = 0;
			for( ent = activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
				if ( g_cinematic.GetBool() && inCinematic && !ent->cinematic ) {
					ent->GetPhysics()->UpdateTime( time );
					continue;
				}
				timer_singlethink.Clear();
				timer_singlethink.Start();
				ent->Think();
				timer_singlethink.Stop();
				ms = timer_singlethink.Milliseconds();
				if ( ms >= g_timeentities.GetFloat() ) {
					//Printf( "%d: entity '%s': %.1f ms\n", time, ent->name.c_str(), ms );
					DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("%d: entity '%s': %.3f ms\r", time, ent->name.c_str(), ms );
				}
				num++;
			}
		} else {
			if ( inCinematic ) {
				num = 0;
				for( ent = activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
					if ( g_cinematic.GetBool() && !ent->cinematic ) {
						ent->GetPhysics()->UpdateTime( time );
						continue;
					}
					ent->Think();
					num++;
				}
			} else {
				num = 0;
				for( ent = activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
					ent->Think();
					num++;
				}
			}
		}

		// remove any entities that have stopped thinking
		if ( numEntitiesToDeactivate ) {
			idEntity *next_ent;
			int c = 0;
			for( ent = activeEntities.Next(); ent != NULL; ent = next_ent ) {
				next_ent = ent->activeNode.Next();
				if ( !ent->thinkFlags ) {
					ent->activeNode.Remove();
					c++;
				}
			}
			//assert( numEntitiesToDeactivate == c );
			numEntitiesToDeactivate = 0;
		}

		timer_think.Stop();
	
		//DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("Thinking timer: %lfms\r", timer_think.Milliseconds());

		timer_events.Clear();
		timer_events.Start();

		// service any pending events
		idEvent::ServiceEvents();

		timer_events.Stop();

		// Process the active AI conversations
		m_ConversationSystem->ProcessConversations();

		// free the player pvs
		FreePlayerPVS();

		// do multiplayer related stuff
		if ( isMultiplayer ) {
			mpGame.Run();
		}

		// display how long it took to calculate the current game frame
		if ( g_frametime.GetBool() ) {
			Printf( "game %d: all:%.1f th:%.1f ev:%.1f %d ents \n",
				time, timer_think.Milliseconds() + timer_events.Milliseconds(),
				timer_think.Milliseconds(), timer_events.Milliseconds(), num );
		}

		// build the return value
		ret.consistencyHash = 0;
		ret.sessionCommand[0] = 0;

		if ( !isMultiplayer && player ) {
			ret.health = player->health;
			ret.heartRate = player->heartRate;
			ret.stamina = idMath::FtoiFast( player->stamina );
			// combat is a 0-100 value based on lastHitTime and lastDmgTime
			// each make up 50% of the time spread over 10 seconds
			ret.combat = 0;
			if ( player->lastDmgTime > 0 && time < player->lastDmgTime + 10000 ) {
				ret.combat += int(50.0f * (float) ( time - player->lastDmgTime ) / 10000);
			}
			if ( player->lastHitTime > 0 && time < player->lastHitTime + 10000 ) {
				ret.combat += int(50.0f * (float) ( time - player->lastHitTime ) / 10000);
			}
		}

		// see if a target_sessionCommand has forced a changelevel
		if ( sessionCommand.Length() ) {
			strncpy( ret.sessionCommand, sessionCommand, sizeof( ret.sessionCommand ) );
			break;
		}

		// make sure we don't loop forever when skipping a cinematic
		if ( skipCinematic && ( time > cinematicMaxSkipTime ) ) {
			Warning( "Exceeded maximum cinematic skip length.  Cinematic may be looping infinitely." );
			skipCinematic = false;
			break;
		}
		if(m_DoLightgem == true)
		{
			ProcessLightgem(player, (cv_lg_hud.GetInteger() == 0));
			m_DoLightgem = false;
		}
	} while( ( inCinematic || ( time < cinematicStopTime ) ) && skipCinematic );

	ret.syncNextGameFrame = skipCinematic;
	if ( skipCinematic ) {
		soundSystem->SetMute( false );
		skipCinematic = false;		
	}

	// show any debug info for this frame
	RunDebugInfo();
	D_DrawDebugLines();

	DM_LOG(LC_FRAME, LT_INFO)LOGSTRING("Frame end %d - %d: all:%.1f th:%.1f ev:%.1f %d ents \r", 
		time, timer_think.Milliseconds() + timer_events.Milliseconds(),
		timer_think.Milliseconds(), timer_events.Milliseconds(), num );

	g_Global.m_Frame = 0;

	return ret;
}


/*
======================================================================

  Game view drawing

======================================================================
*/

/*
====================
idGameLocal::CalcFov

Calculates the horizontal and vertical field of view based on a horizontal field of view and custom aspect ratio
====================
*/
void idGameLocal::CalcFov( float base_fov, float &fov_x, float &fov_y ) const {
	float	x;
	float	y;
	float	ratio_x;
	float	ratio_y;
	
	if ( !sys->FPU_StackIsEmpty() ) {
		Printf( sys->FPU_GetState() );
		Error( "idGameLocal::CalcFov: FPU stack not empty" );
	}

	// first, calculate the vertical fov based on a 640x480 view
	x = 640.0f / tan( base_fov / 360.0f * idMath::PI );
	y = atan2( 480.0f, x );
	fov_y = y * 360.0f / idMath::PI;

	// FIXME: somehow, this is happening occasionally
	assert( fov_y > 0 );
	if ( fov_y <= 0 ) {
		Printf( sys->FPU_GetState() );
		Error( "idGameLocal::CalcFov: bad result" );
	}

	switch( r_aspectRatio.GetInteger() ) {
	default :
	case 0 :
		// 4:3
		fov_x = base_fov;
		return;
		break;

	case 1 :
		// 16:9
		ratio_x = 16.0f;
		ratio_y = 9.0f;
		break;

	case 2 :
		// 16:10
		ratio_x = 16.0f;
		ratio_y = 10.0f;
		break;
	}

	y = ratio_y / tan( fov_y / 360.0f * idMath::PI );
	fov_x = atan2( ratio_x, y ) * 360.0f / idMath::PI;

	if ( fov_x < base_fov ) {
		fov_x = base_fov;
		x = ratio_x / tan( fov_x / 360.0f * idMath::PI );
		fov_y = atan2( ratio_y, x ) * 360.0f / idMath::PI;
	}

	// FIXME: somehow, this is happening occasionally
	assert( ( fov_x > 0 ) && ( fov_y > 0 ) );
	if ( ( fov_y <= 0 ) || ( fov_x <= 0 ) ) {
		Printf( sys->FPU_GetState() );
		Error( "idGameLocal::CalcFov: bad result" );
	}
}

/*
================
idGameLocal::Draw

makes rendering and sound system calls
================
*/
bool idGameLocal::Draw( int clientNum )
{
	if ( isMultiplayer ) {
		return mpGame.Draw( clientNum );
	}

	idPlayer *player = static_cast<idPlayer *>(entities[ clientNum ]);

	if ( !player ) {
		return false;
	}

	// render the scene
	player->playerView.RenderPlayerView(player->hud);

	// Make the rendershot appear on the hud
	if(cv_lg_hud.GetInteger() != 0)
		ProcessLightgem(player, true);

	m_DoLightgem = true;
	return true;
}

void idGameLocal::ProcessLightgem(idPlayer *player, bool bProcessing)
{
	int n;
	CDarkModPlayer *pDM = g_Global.m_DarkModPlayer;
	float fColVal = pDM->m_fColVal;

	n = cv_lg_interleave.GetInteger();
	if(cv_lg_weak.GetBool() == false)
	{
		if(bProcessing == true)
		{
			if(n > 0)
			{
				// Skip every nth frame according to the value set in 
				m_Interleave++;
				if(m_Interleave >= n)
				{
					m_Interleave = 0;
					fColVal = CalcLightgem(player);
				}
			}
		}
	}

	DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("Averaged colorvalue total: %f\r", fColVal);

	fColVal += cv_lg_adjust.GetFloat();
	DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("Adjustment %f\r", cv_lg_adjust.GetFloat());

	pDM->m_fColVal = fColVal;
	pDM->m_LightgemValue = int(DARKMOD_LG_MAX * fColVal);

	// Give the player and inventory items a chance to adjust the lightgem (fire arrow, crouching)
	pDM->m_LightgemValue = player->GetLightgemModifier(pDM->m_LightgemValue);

	DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("After player adjustment %d\r", pDM->m_LightgemValue);

	if(pDM->m_LightgemValue < DARKMOD_LG_MIN)
		pDM->m_LightgemValue = DARKMOD_LG_MIN;
	else
	if(pDM->m_LightgemValue > DARKMOD_LG_MAX)
		pDM->m_LightgemValue = DARKMOD_LG_MAX;
}

/*
================
idGameLocal::HandleESC
================
*/
escReply_t idGameLocal::HandleESC( idUserInterface **gui ) {

	if ( isMultiplayer ) {
		*gui = StartMenu();
		// we may set the gui back to NULL to hide it
		return ESC_GUI;
	}

	// If we're in the process of ending the mission, ignore all ESC keys.
	if (GameState() == GAMESTATE_COMPLETED) {
		return ESC_IGNORE;
	}

	// greebo: Hitting the ESC key means that the main menu is about to be entered => stop the timer
	m_GamePlayTimer.Stop();

	idPlayer *player = GetLocalPlayer();
	if ( player ) {
		if ( player->HandleESC() ) {
			m_GamePlayTimer.SetEnabled(true);
			return ESC_IGNORE;
		} else {
			return ESC_MAIN;
		}
	}
	return ESC_MAIN;
}

/*
================
idGameLocal::StartMenu
================
*/
idUserInterface* idGameLocal::StartMenu( void ) {
	if ( !isMultiplayer ) {
		return NULL;
	}
	return mpGame.StartMenu();
}

/*
================
idGameLocal::HandleGuiCommands
================
*/
const char* idGameLocal::HandleGuiCommands( const char *menuCommand ) {
	if ( !isMultiplayer ) {
		return NULL;
	}
	return mpGame.HandleGuiCommands( menuCommand );
}

/*
================
idGameLocal::HandleMainMenuCommands
================
*/
void idGameLocal::HandleMainMenuCommands( const char *menuCommand, idUserInterface *gui )
{
	idStr cmd(menuCommand);

	// Watch out for objectives GUI-related commands
	m_MissionData->HandleMainMenuCommands(cmd, gui);

	if (cmd == "mainmenu_heartbeat")
	{
		// greebo: Stop the timer, this is already done in HandleESC, but just to make sure...
		m_GamePlayTimer.Stop();

		if (GetMissionResult() == MISSION_COMPLETE)
		{
			// Check if we should show the success screen (also check the member variable
			// to catch cases where the player reloaded the map via the console)
			if (!gui->GetStateBool("SuccessScreenActive") || !successScreenActive)
			{
				// Load the statistics into the GUI
				m_MissionData->UpdateStatisticsGUI(gui, "listStatistics");

				// Show the success GUI
				gui->HandleNamedEvent("ShowSuccessScreen");

				// Avoid duplicate triggering
				gui->SetStateBool("SuccessScreenActive", true);
				successScreenActive = true;
			}
			return;
		}

		// greebo: Check for the right ambient music
		if (GameState() == GAMESTATE_ACTIVE)
		{
			if (!gui->GetStateBool("ObjectivesMusicPlaying")) 
			{
				gui->HandleNamedEvent("StartObjectivesMusic");
				gui->SetStateBool("ObjectivesMusicPlaying", true);
			}
		}
		else // GameState != ACTIVE
		{
			if (gui->GetStateBool("ObjectivesMusicPlaying"))
			{
				gui->HandleNamedEvent("StopObjectivesMusic");
				gui->SetStateBool("ObjectivesMusicPlaying", false);
			}
		}
	}
	// greebo: This is used for Saint Lucia only (comment this out after release)
	else if (cmd == "showMods" || cmd == "briefing_start_request") // Called by "Start Mission"
	{
		// User requested a map start
		gui->HandleNamedEvent("ShowBriefingScreen");
		gui->SetStateInt("BriefingIsVisible", 1);

		// greebo: During briefing, we play the in-game background music
		gui->HandleNamedEvent("StartObjectivesMusic");
	}
	else if (cmd == "close") 
	{
		gui->HandleNamedEvent("HideBriefingScreen");
		gui->SetStateInt("BriefingIsVisible", 0);
		gui->HandleNamedEvent("HideSuccessScreen");

		// Start the timer again, we're closing the menu
		m_GamePlayTimer.Start();
	}
	else if (cmd == "close_success_screen")
	{
		// Clear the mission result flag
		SetMissionResult(MISSION_NOTEVENSTARTED);

		// Set the boolean back to false for the next map start
		gui->SetStateBool("SuccessScreenActive", false);
		successScreenActive = false;
	}

	g_Shop.HandleCommands(menuCommand, gui, GetLocalPlayer());
	g_Mods.HandleCommands(menuCommand, gui);

	if (cv_debug_mainmenu.GetBool())
	{
		const idDict& state = gui->State();
		
		for (int i = 0; i < state.GetNumKeyVals(); ++i)
		{
			const idKeyValue* kv = state.GetKeyVal(i);

			const idStr key = kv->GetKey();
			const idStr value = kv->GetValue();

			// Force the log file to write its stuff
			g_Global.m_ClassArray[LC_MISC] = true;
			g_Global.m_LogArray[LT_INFO] = true;

			DM_LOG(LC_MISC, LT_INFO)LOGSTRING("Mainmenu GUI State %s = %s\r", key.c_str(), value.c_str());
		}

		// Clear the cvar again
		cv_debug_mainmenu.SetBool(false);
	}
}

/*
================
idGameLocal::GetLevelMap

  should only be used for in-game level editing
================
*/
idMapFile *idGameLocal::GetLevelMap( void ) {
	if ( mapFile && mapFile->HasPrimitiveData()) {
		return mapFile;
	}
	if ( !mapFileName.Length() ) {
		return NULL;
	}

	if ( mapFile ) {
		delete mapFile;
	}

	mapFile = new idMapFile;
	if ( !mapFile->Parse( mapFileName ) ) {
		delete mapFile;
		mapFile = NULL;
	}

	return mapFile;
}

/*
================
idGameLocal::GetMapName
================
*/
const char *idGameLocal::GetMapName( void ) const {
	return mapFileName.c_str();
}

/*
================
idGameLocal::CallFrameCommand
================
*/
void idGameLocal::CallFrameCommand( idEntity *ent, const function_t *frameCommand ) {
	frameCommandThread->CallFunction( ent, frameCommand, true );
	frameCommandThread->Execute();
}

/*
================
idGameLocal::CallObjectFrameCommand
================
*/
void idGameLocal::CallObjectFrameCommand( idEntity *ent, const char *frameCommand ) {
	const function_t *func;

	func = ent->scriptObject.GetFunction( frameCommand );
	if ( !func ) {
		if ( !ent->IsType( idTestModel::Type ) ) {
			Error( "Unknown function '%s' called for frame command on entity '%s'", frameCommand, ent->name.c_str() );
		}
	} else {
		frameCommandThread->CallFunction( ent, func, true );
		frameCommandThread->Execute();
	}
}

/*
================
idGameLocal::ShowTargets
================
*/
void idGameLocal::ShowTargets( void ) {
	idMat3		axis = GetLocalPlayer()->viewAngles.ToMat3();
	idVec3		up = axis[ 2 ] * 5.0f;
	const idVec3 &viewPos = GetLocalPlayer()->GetPhysics()->GetOrigin();
	idBounds	viewTextBounds( viewPos );
	idBounds	viewBounds( viewPos );
	idBounds	box( idVec3( -4.0f, -4.0f, -4.0f ), idVec3( 4.0f, 4.0f, 4.0f ) );
	idEntity	*ent;
	idEntity	*target;
	int			i;
	idBounds	totalBounds;

	viewTextBounds.ExpandSelf( 128.0f );
	viewBounds.ExpandSelf( 512.0f );
	for( ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		totalBounds = ent->GetPhysics()->GetAbsBounds();
		for( i = 0; i < ent->targets.Num(); i++ ) {
			target = ent->targets[ i ].GetEntity();
			if ( target ) {
				totalBounds.AddBounds( target->GetPhysics()->GetAbsBounds() );
			}
		}

		if ( !viewBounds.IntersectsBounds( totalBounds ) ) {
			continue;
		}

		float dist;
		idVec3 dir = totalBounds.GetCenter() - viewPos;
		dir.NormalizeFast();
		totalBounds.RayIntersection( viewPos, dir, dist );
		float frac = ( 512.0f - dist ) / 512.0f;
		if ( frac < 0.0f ) {
			continue;
		}

		gameRenderWorld->DebugBounds( ( ent->IsHidden() ? colorLtGrey : colorOrange ) * frac, ent->GetPhysics()->GetAbsBounds() );
		if ( viewTextBounds.IntersectsBounds( ent->GetPhysics()->GetAbsBounds() ) ) {
			idVec3 center = ent->GetPhysics()->GetAbsBounds().GetCenter();
			gameRenderWorld->DrawText( ent->name.c_str(), center - up, 0.1f, colorWhite * frac, axis, 1 );
			gameRenderWorld->DrawText( ent->GetEntityDefName(), center, 0.1f, colorWhite * frac, axis, 1 );
			gameRenderWorld->DrawText( va( "#%d", ent->entityNumber ), center + up, 0.1f, colorWhite * frac, axis, 1 );
		}

		for( i = 0; i < ent->targets.Num(); i++ ) {
			target = ent->targets[ i ].GetEntity();
			if ( target ) {
				gameRenderWorld->DebugArrow( colorYellow * frac, ent->GetPhysics()->GetAbsBounds().GetCenter(), target->GetPhysics()->GetOrigin(), 10, 0 );
				gameRenderWorld->DebugBounds( colorGreen * frac, box, target->GetPhysics()->GetOrigin() );
			}
		}
	}
}

/*
================
idGameLocal::RunDebugInfo
================
*/
void idGameLocal::RunDebugInfo( void ) {
	idEntity *ent;
	idPlayer *player;

	player = GetLocalPlayer();
	if ( !player ) {
		return;
	}

	const idVec3 &origin = player->GetPhysics()->GetOrigin();

	if ( g_showEntityInfo.GetBool() ) {
		idMat3		axis = player->viewAngles.ToMat3();
		idVec3		up = axis[ 2 ] * 5.0f;
		idBounds	viewTextBounds( origin );
		idBounds	viewBounds( origin );

		viewTextBounds.ExpandSelf( 128.0f );
		viewBounds.ExpandSelf( 512.0f );
		for( ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
			// don't draw the worldspawn
			if ( ent == world ) {
				continue;
			}

			// skip if the entity is very far away
			if ( !viewBounds.IntersectsBounds( ent->GetPhysics()->GetAbsBounds() ) ) {
				continue;
			}

			const idBounds &entBounds = ent->GetPhysics()->GetAbsBounds();
			int contents = ent->GetPhysics()->GetContents();
			if ( contents & CONTENTS_BODY ) {
				gameRenderWorld->DebugBounds( colorCyan, entBounds );
			} else if ( contents & CONTENTS_TRIGGER ) {
				gameRenderWorld->DebugBounds( colorOrange, entBounds );
			} else if ( contents & CONTENTS_SOLID ) {
				gameRenderWorld->DebugBounds( colorGreen, entBounds );
			} else {
				if ( !entBounds.GetVolume() ) {
					gameRenderWorld->DebugBounds( colorMdGrey, entBounds.Expand( 8.0f ) );
				} else {
					gameRenderWorld->DebugBounds( colorMdGrey, entBounds );
				}
			}
			if ( viewTextBounds.IntersectsBounds( entBounds ) ) {
				gameRenderWorld->DrawText( ent->name.c_str(), entBounds.GetCenter(), 0.1f, colorWhite, axis, 1 );
				gameRenderWorld->DrawText( va( "#%d", ent->entityNumber ), entBounds.GetCenter() + up, 0.1f, colorWhite, axis, 1 );
				gameRenderWorld->DrawText( 
					va( "%d / %d", ent->GetPhysics()->GetContents(), ent->GetPhysics()->GetClipMask() ), 
					entBounds.GetCenter() - up, 0.1f, 
					colorWhite, 
					axis, 1 
				);
			}
		}
	}

	// debug tool to draw bounding boxes around active entities
	if ( g_showActiveEntities.GetBool() ) {
		for( ent = activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
			idBounds	b = ent->GetPhysics()->GetBounds();
			if ( b.GetVolume() <= 0 ) {
				b[0][0] = b[0][1] = b[0][2] = -8;
				b[1][0] = b[1][1] = b[1][2] = 8;
			}
			if ( ent->fl.isDormant ) {
				gameRenderWorld->DebugBounds( colorYellow, b, ent->GetPhysics()->GetOrigin() );
			} else {
				gameRenderWorld->DebugBounds( colorGreen, b, ent->GetPhysics()->GetOrigin() );
			}
		}
	}

	if ( g_showTargets.GetBool() ) {
		ShowTargets();
	}

	if ( g_showTriggers.GetBool() ) {
		idTrigger::DrawDebugInfo();
	}

	if ( ai_showCombatNodes.GetBool() ) {
		idCombatNode::DrawDebugInfo();
	}

	if ( ai_showPaths.GetBool() ) {
		idPathCorner::DrawDebugInfo();
	}

	if ( g_editEntityMode.GetBool() ) {
		editEntities->DisplayEntities();
	}

	if ( g_showCollisionWorld.GetBool() ) {
		collisionModelManager->DrawModel( 0, vec3_origin, mat3_identity, origin, 128.0f );
	}

	if ( g_showCollisionModels.GetBool() ) {
		clip.DrawClipModels( player->GetEyePosition(), g_maxShowDistance.GetFloat(), pm_thirdPerson.GetBool() ? NULL : player );
	}

	if ( g_showCollisionTraces.GetBool() ) {
		clip.PrintStatistics();
	}

	if ( g_showPVS.GetInteger() ) {
		pvs.DrawPVS( origin, ( g_showPVS.GetInteger() == 2 ) ? PVS_ALL_PORTALS_OPEN : PVS_NORMAL );
	}

	if ( aas_test.GetInteger() >= 0 ) {
		idAAS *aas = GetAAS( aas_test.GetInteger() );
		if ( aas ) {
			aas->Test( origin );
			if ( ai_testPredictPath.GetBool() ) {
				idVec3 velocity;
				predictedPath_t path;

				velocity.x = cos( DEG2RAD( player->viewAngles.yaw ) ) * 100.0f;
				velocity.y = sin( DEG2RAD( player->viewAngles.yaw ) ) * 100.0f;
				velocity.z = 0.0f;
				idAI::PredictPath( player, aas, origin, velocity, 1000, 100, SE_ENTER_OBSTACLE | SE_BLOCKED | SE_ENTER_LEDGE_AREA, path );
			}
		}
	}

	if ( ai_showObstacleAvoidance.GetInteger() == 2 ) {
		idAAS *aas = GetAAS("aas32");
		if ( aas ) {
			idVec3 seekPos;
			obstaclePath_t path;

			seekPos = player->GetPhysics()->GetOrigin() + player->viewAxis[0] * 200.0f;
			idAI::FindPathAroundObstacles( player->GetPhysics(), aas, NULL, player->GetPhysics()->GetOrigin(), seekPos, path, player );
		}
	}

	// collision map debug output
	collisionModelManager->DebugOutput( player->GetEyePosition() );
}

/*
==================
idGameLocal::NumAAS
==================
*/
int	idGameLocal::NumAAS( void ) const {
	return aasList.Num();
}

/*
==================
idGameLocal::GetAASId (TDM)
==================
*/
int	idGameLocal::GetAASId( idAAS* aas ) const
{
	// Do a reverse lookup in the aasList array
	for (int i = 0; i < aasList.Num(); i++)
	{
		if (aasList[i] == aas)
		{
			// Found the pointer, return it
			return i;
		}
	}
	// Not found
	return -1;
}

/*
==================
idGameLocal::GetAAS
==================
*/
idAAS *idGameLocal::GetAAS( int num ) const {
	if ( ( num >= 0 ) && ( num < aasList.Num() ) ) {
		if ( aasList[ num ] && aasList[ num ]->GetSettings() ) {
			return aasList[ num ];
		}
	}
	return NULL;
}

/*
==================
idGameLocal::GetAAS
==================
*/
idAAS *idGameLocal::GetAAS( const char *name ) const {
	int i;

	for ( i = 0; i < aasNames.Num(); i++ ) {
		if ( aasNames[ i ] == name ) {
			if ( !aasList[ i ]->GetSettings() ) {
				return NULL;
			} else {
				return aasList[ i ];
			}
		}
	}
	return NULL;
}

/*
==================
idGameLocal::SetAASAreaState
==================
*/
void idGameLocal::SetAASAreaState( const idBounds &bounds, const int areaContents, bool closed ) {
	int i;

	for( i = 0; i < aasList.Num(); i++ ) {
		aasList[ i ]->SetAreaState( bounds, areaContents, closed );
	}
}

/*
==================
idGameLocal::AddAASObstacle
==================
*/
aasHandle_t idGameLocal::AddAASObstacle( const idBounds &bounds ) {
	int i;
	aasHandle_t obstacle;
	aasHandle_t check;

	if ( !aasList.Num() ) {
		return -1;
	}

	obstacle = aasList[ 0 ]->AddObstacle( bounds );
	for( i = 1; i < aasList.Num(); i++ ) {
		check = aasList[ i ]->AddObstacle( bounds );
		assert( check == obstacle );
	}

	return obstacle;
}

/*
==================
idGameLocal::RemoveAASObstacle
==================
*/
void idGameLocal::RemoveAASObstacle( const aasHandle_t handle ) {
	int i;

	for( i = 0; i < aasList.Num(); i++ ) {
		aasList[ i ]->RemoveObstacle( handle );
	}
}

/*
==================
idGameLocal::RemoveAllAASObstacles
==================
*/
void idGameLocal::RemoveAllAASObstacles( void ) {
	int i;

	for( i = 0; i < aasList.Num(); i++ ) {
		aasList[ i ]->RemoveAllObstacles();
	}
}

void idGameLocal::SetupEAS()
{
	// Cycle through the entities and find all elevators
	for (idEntity* ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next())
	{
		if (!ent->IsType(CMultiStateMover::Type))
		{
			continue;
		}

		// Get all position entities of that mover
		CMultiStateMover* mover = static_cast<CMultiStateMover*>(ent);

		for (int aasNum = 0; aasNum < NumAAS(); aasNum++)
		{
			idAASLocal* aas = dynamic_cast<idAASLocal*>(GetAAS(aasNum));
			if (aas == NULL) continue;

			aas->AddElevator(mover);
		}
	}

	// All elevators registered, compile the routing information
	for (int aasNum = 0; aasNum < NumAAS(); aasNum++)
	{
		idAASLocal* aas = dynamic_cast<idAASLocal*>(GetAAS(aasNum));
		if (aas == NULL) continue;

		aas->CompileEAS();
	}
}

/*
==================
idGameLocal::CheatsOk
==================
*/
bool idGameLocal::CheatsOk( bool requirePlayer ) {
	idPlayer *player;

	if ( isMultiplayer && !cvarSystem->GetCVarBool( "net_allowCheats" ) ) {
		Printf( "Not allowed in multiplayer.\n" );
		return false;
	}

	if ( developer.GetBool() ) {
		return true;
	}

	player = GetLocalPlayer();
	if ( !requirePlayer || ( player && ( player->health > 0 ) ) ) {
		return true;
	}

	Printf( "You must be alive to use this command.\n" );

	return false;
}

/*
===================
idGameLocal::RegisterEntity
===================
*/
void idGameLocal::RegisterEntity( idEntity *ent ) {
	int spawn_entnum;

	if ( spawnCount >= ( 1 << ( 32 - GENTITYNUM_BITS ) ) ) {
		Error( "idGameLocal::RegisterEntity: spawn count overflow" );
	}

	if ( !spawnArgs.GetInt( "spawn_entnum", "0", spawn_entnum ) ) {
		while( entities[firstFreeIndex] && firstFreeIndex < ENTITYNUM_MAX_NORMAL ) {
			firstFreeIndex++;
		}
		if ( firstFreeIndex >= ENTITYNUM_MAX_NORMAL ) {
			Error( "no free entities" );
		}
		spawn_entnum = firstFreeIndex++;
	}

	entities[ spawn_entnum ] = ent;
	spawnIds[ spawn_entnum ] = spawnCount++;
	ent->entityNumber = spawn_entnum;
	ent->spawnNode.AddToEnd( spawnedEntities );
	ent->spawnArgs.TransferKeyValues( spawnArgs );

	if ( spawn_entnum >= num_entities ) {
		num_entities++;
	}
}

/*
===================
idGameLocal::UnregisterEntity
===================
*/
void idGameLocal::UnregisterEntity( idEntity *ent ) {
	assert( ent );

	if ( editEntities ) {
		editEntities->RemoveSelectedEntity( ent );
	}

	if ( ( ent->entityNumber != ENTITYNUM_NONE ) && ( entities[ ent->entityNumber ] == ent ) ) {
		ent->spawnNode.Remove();
		entities[ ent->entityNumber ] = NULL;
		spawnIds[ ent->entityNumber ] = -1;
		if ( ent->entityNumber >= MAX_CLIENTS && ent->entityNumber < firstFreeIndex ) {
			firstFreeIndex = ent->entityNumber;
		}
		ent->entityNumber = ENTITYNUM_NONE;
	}
}

/*
================
idGameLocal::SpawnEntityType
================
*/
idEntity *idGameLocal::SpawnEntityType( const idTypeInfo &classdef, const idDict *args, bool bIsClientReadSnapshot ) {
	idClass *obj;

#if _DEBUG
	if ( isClient ) {
		assert( bIsClientReadSnapshot );
	}
#endif

	if ( !classdef.IsType( idEntity::Type ) ) {
		Error( "Attempted to spawn non-entity class '%s'", classdef.classname );
	}

	try {
		if ( args ) {
			spawnArgs = *args;
		} else {
			spawnArgs.Clear();
		}
		obj = classdef.CreateInstance();
		obj->CallSpawn();
	}
	
	catch( idAllocError & ) {
		obj = NULL;
	}
	spawnArgs.Clear();

	return static_cast<idEntity *>(obj);
}

/*
===================
idGameLocal::SpawnEntityDef

Finds the spawn function for the entity and calls it,
returning false if not found
===================
*/
bool idGameLocal::SpawnEntityDef( const idDict &args, idEntity **ent, bool setDefaults ) {
	const char	*classname;
	const char	*spawn;
	idTypeInfo	*cls;
	idClass		*obj;
	idStr		error;
	const char  *name;

	if ( ent ) {
		*ent = NULL;
	}

	spawnArgs = args;

	if ( spawnArgs.GetString( "name", "", &name ) ) {
		sprintf( error, " on '%s'", name);
	}

	spawnArgs.GetString( "classname", NULL, &classname );

	const idDeclEntityDef *def = FindEntityDef( classname, false );

	if ( !def ) {
		Warning( "Unknown classname '%s'%s.", classname, error.c_str() );
		return false;
	}

	spawnArgs.SetDefaults( &def->dict );

	// greebo: Apply the difficulty settings before any values get filled from the spawnarg data
	m_DifficultyManager.ApplyDifficultySettings(spawnArgs);

	// check if we should spawn a class object
	spawnArgs.GetString( "spawnclass", NULL, &spawn );
	if ( spawn ) {

		cls = idClass::GetClass( spawn );
		if ( !cls ) {
			Warning( "Could not spawn '%s'.  Class '%s' not found%s.", classname, spawn, error.c_str() );
			return false;
		}

		obj = cls->CreateInstance();
		if ( !obj ) {
			Warning( "Could not spawn '%s'. Instance could not be created%s.", classname, error.c_str() );
			return false;
		}

		obj->CallSpawn();

		if ( ent && obj->IsType( idEntity::Type ) ) {
			*ent = static_cast<idEntity *>(obj);
		}

		return true;
	}

	// check if we should call a script function to spawn
	spawnArgs.GetString( "spawnfunc", NULL, &spawn );
	if ( spawn ) {
		const function_t *func = program.FindFunction( spawn );
		if ( !func ) {
			Warning( "Could not spawn '%s'.  Script function '%s' not found%s.", classname, spawn, error.c_str() );
			return false;
		}
		idThread *thread = new idThread( func );
		thread->DelayedStart( 0 );
		return true;
	}

	Warning( "%s doesn't include a spawnfunc or spawnclass%s.", classname, error.c_str() );
	return false;
}

/*
================
idGameLocal::FindEntityDef
================
*/
const idDeclEntityDef *idGameLocal::FindEntityDef( const char *name, bool makeDefault ) const {
	const idDecl *decl = NULL;
	if ( isMultiplayer ) {
		decl = declManager->FindType( DECL_ENTITYDEF, va( "%s_mp", name ), false );
	}
	if ( !decl ) {
		decl = declManager->FindType( DECL_ENTITYDEF, name, makeDefault );
	}
	return static_cast<const idDeclEntityDef *>( decl );
}

/*
================
idGameLocal::FindEntityDefDict
================
*/
const idDict *idGameLocal::FindEntityDefDict( const char *name, bool makeDefault ) const {
	const idDeclEntityDef *decl = FindEntityDef( name, makeDefault );
	return decl ? &decl->dict : NULL;
}

/*
================
idGameLocal::InhibitEntitySpawn
================
*/
bool idGameLocal::InhibitEntitySpawn( idDict &spawnArgs ) {
	
	bool result = false;

	/* greebo: Disabled vanilla D3 stuff, will be handled by our DifficultyManager 
	if ( isMultiplayer ) {
		spawnArgs.GetBool( "not_multiplayer", "0", result );
	} else if ( g_skill.GetInteger() == 0 ) {
		spawnArgs.GetBool( "not_easy", "0", result );
	} else if ( g_skill.GetInteger() == 1 ) {
		spawnArgs.GetBool( "not_medium", "0", result );
	} else {
		spawnArgs.GetBool( "not_hard", "0", result );
	}

	const char *name;
#ifndef ID_DEMO_BUILD
	if ( g_skill.GetInteger() == 3 ) { 
		name = spawnArgs.GetString( "classname" );
		if ( idStr::Icmp( name, "item_medkit" ) == 0 || idStr::Icmp( name, "item_medkit_small" ) == 0 ) {
			result = true;
		}
	}
#endif

	if ( gameLocal.isMultiplayer ) {
		name = spawnArgs.GetString( "classname" );
		if ( idStr::Icmp( name, "weapon_bfg" ) == 0 || idStr::Icmp( name, "weapon_soulcube" ) == 0 ) {
			result = true;
		}
	}*/

	// Consult the difficulty manager, whether this entity should be prevented from being spawned.
	result = m_DifficultyManager.InhibitEntitySpawn(spawnArgs);

	return result;
}

/*
================
idGameLocal::SetSkill
================
*/
void idGameLocal::SetSkill( int value ) {
	int skill_level;

	if ( value < 0 ) {
		skill_level = 0;
	} else if ( value > 3 ) {
		skill_level = 3;
	} else {
		skill_level = value;
	}

	g_skill.SetInteger( skill_level );
}

/*
==============
idGameLocal::GameState

Used to allow entities to know if they're being spawned during the initial spawn.
==============
*/
gameState_t	idGameLocal::GameState( void ) const {
	return gamestate;
}

void idGameLocal::PrepareForMissionEnd() {
	// Raise the gamestate to "Completed"
	gamestate = GAMESTATE_COMPLETED;
}

/*
==============
idGameLocal::SpawnMapEntities

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void idGameLocal::SpawnMapEntities( void ) {
	int			i;
	int			num;
	int			inhibit;
	idMapEntity	*mapEnt;
	int			numEntities;
	idDict		args;

	Printf( "Spawning entities\n" );

	if ( mapFile == NULL ) {
		Printf("No mapfile present\n");
		return;
	}

	SetSkill( g_skill.GetInteger() );

	// Add the lightgem to the map before anything else happened
	// so it will be included as if it were a regular map entity.
	SpawnLightgemEntity();

	numEntities = mapFile->GetNumEntities();
	if ( numEntities == 0 ) {
		Error( "...no entities" );
	}

	// the worldspawn is a special that performs any global setup
	// needed by a level
	mapEnt = mapFile->GetEntity( 0 );
	args = mapEnt->epairs;
	args.SetInt( "spawn_entnum", ENTITYNUM_WORLD );
	if ( !SpawnEntityDef( args ) || !entities[ ENTITYNUM_WORLD ] || !entities[ ENTITYNUM_WORLD ]->IsType( idWorldspawn::Type ) ) {
		Error( "Problem spawning world entity" );
	}

	num = 1;
	inhibit = 0;

	for ( i = 1 ; i < numEntities ; i++ ) {
		mapEnt = mapFile->GetEntity( i );
		args = mapEnt->epairs;

		for(int x = 0; x < args.GetNumKeyVals(); x++)
		{
			const idKeyValue *p = args.GetKeyVal(x);
			const idStr k = p->GetKey();
			const idStr v = p->GetValue();
			DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("Entity[%u] Key:[%s] = [%s]\r", i, k.c_str(), v.c_str());
		}

		if ( !InhibitEntitySpawn( args ) ) {
			// precache any media specified in the map entity
			CacheDictionaryMedia( &args );

			SpawnEntityDef( args );
			num++;
		} else {
			inhibit++;
		}
	}

	m_LightgemSurface = gameLocal.FindEntity(DARKMOD_LG_ENTITY_NAME);
	m_LightgemSurface.GetEntity()->GetRenderEntity()->allowSurfaceInViewID = DARKMOD_LG_VIEWID;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->suppressShadowInViewID = 0;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noDynamicInteractions = false;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noShadow = true;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noSelfShadow = true;
	DM_LOG(LC_LIGHT, LT_INFO)LOGSTRING("LightgemSurface: [%08lX]\r", m_LightgemSurface.GetEntity());

	Printf( "...%i entities spawned, %i inhibited\n\n", num, inhibit );
	DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("... %i entities spawned, %i inhibited\r", num, inhibit);
}

/*
================
idGameLocal::AddEntityToHash
================
*/
void idGameLocal::AddEntityToHash( const char *name, idEntity *ent ) {
	if ( FindEntity( name ) ) {
		Error( "Multiple entities named '%s'", name );
	}
	entityHash.Add( entityHash.GenerateKey( name, true ), ent->entityNumber );
}

/*
================
idGameLocal::RemoveEntityFromHash
================
*/
bool idGameLocal::RemoveEntityFromHash( const char *name, idEntity *ent ) {
	int hash, i;

	hash = entityHash.GenerateKey( name, true );
	for ( i = entityHash.First( hash ); i != -1; i = entityHash.Next( i ) ) {
		if ( entities[i] && entities[i] == ent && entities[i]->name.Icmp( name ) == 0 ) {
			entityHash.Remove( hash, i );
			return true;
		}
	}
	return false;
}

/*
================
idGameLocal::GetTargets
================
*/
int idGameLocal::GetTargets( const idDict &args, idList< idEntityPtr<idEntity> > &list, const char *ref ) const {
	int i, num, refLength;
	const idKeyValue *arg;
	idEntity *ent;

	list.Clear();

	refLength = strlen( ref );
	num = args.GetNumKeyVals();
	for( i = 0; i < num; i++ ) {

		arg = args.GetKeyVal( i );
		if ( arg->GetKey().Icmpn( ref, refLength ) == 0 ) {

			ent = FindEntity( arg->GetValue() );
			if ( ent ) {
				idEntityPtr<idEntity> &entityPtr = list.Alloc();
                entityPtr = ent;
			}
		}
	}

	return list.Num();
}

/*
=============
idGameLocal::GetTraceEntity

returns the master entity of a trace.  for example, if the trace entity is the player's head, it will return the player.
=============
*/
idEntity *idGameLocal::GetTraceEntity( const trace_t &trace ) const {
	idEntity *master;

	if ( !entities[ trace.c.entityNum ] ) {
		return NULL;
	}
	master = entities[ trace.c.entityNum ]->GetBindMaster();
	if ( master ) {
		return master;
	}
	return entities[ trace.c.entityNum ];
}

/*
=============
idGameLocal::ArgCompletion_EntityName

Argument completion for entity names
=============
*/
void idGameLocal::ArgCompletion_EntityName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	int i;

	for( i = 0; i < gameLocal.num_entities; i++ ) {
		if ( gameLocal.entities[ i ] ) {
			callback( va( "%s %s", args.Argv( 0 ), gameLocal.entities[ i ]->name.c_str() ) );
		}
	}
}

/*
=============
idGameLocal::FindEntity

Returns the entity whose name matches the specified string.
=============
*/
idEntity *idGameLocal::FindEntity( const char *name ) const {
	int hash, i;

	hash = entityHash.GenerateKey( name, true );
	for ( i = entityHash.First( hash ); i != -1; i = entityHash.Next( i ) ) {
		if ( entities[i] && entities[i]->name.Icmp( name ) == 0 ) {
			return entities[i];
		}
	}

	return NULL;
}

/*
=============
idGameLocal::FindEntityUsingDef

Searches all active entities for the next one using the specified entityDef.

Searches beginning at the entity after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.
=============
*/
idEntity *idGameLocal::FindEntityUsingDef( idEntity *from, const char *match ) const {
	idEntity	*ent;

	if ( !from ) {
		ent = spawnedEntities.Next();
	} else {
		ent = from->spawnNode.Next();
	}

	for ( ; ent != NULL; ent = ent->spawnNode.Next() ) {
		assert( ent );
		if ( idStr::Icmp( ent->GetEntityDefName(), match ) == 0 ) {
			return ent;
		}
	}

	return NULL;
}

/*
=============
idGameLocal::FindTraceEntity

Searches all active entities for the closest ( to start ) match that intersects
the line start,end
=============
*/
idEntity *idGameLocal::FindTraceEntity( idVec3 start, idVec3 end, const idTypeInfo &c, const idEntity *skip ) const {
	idEntity *ent;
	idEntity *bestEnt;
	float scale;
	float bestScale;
	idBounds b;

	bestEnt = NULL;
	bestScale = 1.0f;
	for( ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		if ( ent->IsType( c ) && ent != skip ) {
			b = ent->GetPhysics()->GetAbsBounds().Expand( 16 );
			if ( b.RayIntersection( start, end-start, scale ) ) {
				if ( scale >= 0.0f && scale < bestScale ) {
					bestEnt = ent;
					bestScale = scale;
				}
			}
		}
	}

	return bestEnt;
}

/*
================
idGameLocal::EntitiesWithinRadius
================
*/
int idGameLocal::EntitiesWithinRadius( const idVec3 org, float radius, idEntity **entityList, int maxCount ) const {
	idEntity *ent;
	idBounds bo( org );
	int entCount = 0;

	bo.ExpandSelf( radius );
	for( ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		if ( ent->GetPhysics()->GetAbsBounds().IntersectsBounds( bo ) ) {
			entityList[entCount++] = ent;
		}
	}

	return entCount;
}

/*
=================
idGameLocal::KillBox

Kills all entities that would touch the proposed new positioning of ent. The ent itself will not being killed.
Checks if player entities are in the teleporter, and marks them to die at teleport exit instead of immediately.
If catch_teleport, this only marks teleport players for death on exit
=================
*/
void idGameLocal::KillBox( idEntity *ent, bool catch_teleport ) {
	int			i;
	int			num;
	idEntity *	hit;
	idClipModel *cm;
	idClipModel *clipModels[ MAX_GENTITIES ];
	idPhysics	*phys;

	phys = ent->GetPhysics();
	if ( !phys->GetNumClipModels() ) {
		return;
	}

	num = clip.ClipModelsTouchingBounds( phys->GetAbsBounds(), phys->GetClipMask(), clipModels, MAX_GENTITIES );
	for ( i = 0; i < num; i++ ) {
		cm = clipModels[ i ];

		// don't check render entities
		if ( cm->IsRenderModel() ) {
			continue;
		}

		hit = cm->GetEntity();
		if ( ( hit == ent ) || !hit->fl.takedamage ) {
			continue;
		}

		if ( !phys->ClipContents( cm ) ) {
			continue;
		}

		// nail it
		if ( hit->IsType( idPlayer::Type ) && static_cast< idPlayer * >( hit )->IsInTeleport() ) {
			static_cast< idPlayer * >( hit )->TeleportDeath( ent->entityNumber );
		} else if ( !catch_teleport ) {
			hit->Damage( ent, ent, vec3_origin, "damage_telefrag", 1.0f, INVALID_JOINT );
		}

		if ( !gameLocal.isMultiplayer ) {
			// let the mapper know about it
			Warning( "'%s' telefragged '%s'", ent->name.c_str(), hit->name.c_str() );
		}
	}
}

/*
================
idGameLocal::RequirementMet
================
*/
bool idGameLocal::RequirementMet( idEntity *activator, const idStr &requires, int removeItem ) {
	if ( requires.Length() ) {
		if ( activator->IsType( idPlayer::Type ) ) {
			idPlayer *player = static_cast<idPlayer *>(activator);
			idDict *item = NULL;//player->FindInventoryItem( requires );
			if ( item ) {
				if ( removeItem ) {
					//player->RemoveInventoryItem( item );
				}
				return true;
			} else {
				return false;
			}
		}
	}

	return true;
}

/*
============
idGameLocal::AlertAI
============
*/
void idGameLocal::AlertAI( idEntity *ent ) {
	if ( ent && ent->IsType( idActor::Type ) ) {
		// alert them for the next frame
		lastAIAlertTime = time + msec;
		lastAIAlertEntity = static_cast<idActor *>( ent );
	}
}

/*
============
idGameLocal::GetAlertEntity
============
*/
idActor *idGameLocal::GetAlertEntity( void ) {
	if ( lastAIAlertTime >= time ) {
		return lastAIAlertEntity.GetEntity();
	}

	return NULL;
}

/*
============
idGameLocal::RadiusDamage
============
*/
void idGameLocal::RadiusDamage( const idVec3 &origin, idEntity *inflictor, idEntity *attacker, idEntity *ignoreDamage, idEntity *ignorePush, const char *damageDefName, float dmgPower ) {
	float		dist, damageScale, attackerDamageScale, attackerPushScale;
	idEntity *	ent;
	idEntity *	entityList[ MAX_GENTITIES ];
	int			numListedEntities;
	idBounds	bounds;
	idVec3 		v, damagePoint, dir;
	int			i, e, damage, radius, push;

	const idDict *damageDef = FindEntityDefDict( damageDefName, false );
	if ( !damageDef ) {
		Warning( "Unknown damageDef '%s'", damageDefName );
		return;
	}

	damageDef->GetInt( "damage", "20", damage );
	damageDef->GetInt( "radius", "50", radius );
	damageDef->GetInt( "push", va( "%d", damage * 100 ), push );
	damageDef->GetFloat( "attackerDamageScale", "0.5", attackerDamageScale );
	damageDef->GetFloat( "attackerPushScale", "0", attackerPushScale );

	if ( radius < 1 ) {
		radius = 1;
	}

	bounds = idBounds( origin ).Expand( radius );
	// get all entities touching the bounds
	numListedEntities = clip.EntitiesTouchingBounds( bounds, -1, entityList, MAX_GENTITIES );

	if ( inflictor && inflictor->IsType( idAFAttachment::Type ) ) {
		inflictor = static_cast<idAFAttachment*>(inflictor)->GetBody();
	}
	if ( attacker && attacker->IsType( idAFAttachment::Type ) ) {
		attacker = static_cast<idAFAttachment*>(attacker)->GetBody();
	}
	if ( ignoreDamage && ignoreDamage->IsType( idAFAttachment::Type ) ) {
		ignoreDamage = static_cast<idAFAttachment*>(ignoreDamage)->GetBody();
	}

	// apply damage to the entities
	for ( e = 0; e < numListedEntities; e++ ) {
		ent = entityList[ e ];
		assert( ent );

		if ( !ent->fl.takedamage ) {
			continue;
		}

		if ( ent == inflictor || ( ent->IsType( idAFAttachment::Type ) && static_cast<idAFAttachment*>(ent)->GetBody() == inflictor ) ) {
			continue;
		}

		if ( ent == ignoreDamage || ( ent->IsType( idAFAttachment::Type ) && static_cast<idAFAttachment*>(ent)->GetBody() == ignoreDamage ) ) {
			continue;
		}

		// don't damage a dead player
		if ( isMultiplayer && ent->entityNumber < MAX_CLIENTS && ent->IsType( idPlayer::Type ) && static_cast< idPlayer * >( ent )->health < 0 ) {
			continue;
		}

		// find the distance from the edge of the bounding box
		for ( i = 0; i < 3; i++ ) {
			if ( origin[ i ] < ent->GetPhysics()->GetAbsBounds()[0][ i ] ) {
				v[ i ] = ent->GetPhysics()->GetAbsBounds()[0][ i ] - origin[ i ];
			} else if ( origin[ i ] > ent->GetPhysics()->GetAbsBounds()[1][ i ] ) {
				v[ i ] = origin[ i ] - ent->GetPhysics()->GetAbsBounds()[1][ i ];
			} else {
				v[ i ] = 0;
			}
		}

		dist = v.Length();
		if ( dist >= radius ) {
			continue;
		}

		if ( ent->CanDamage( origin, damagePoint ) ) {
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir = ent->GetPhysics()->GetOrigin() - origin;
			dir[ 2 ] += 24;

			// get the damage scale
			damageScale = dmgPower * ( 1.0f - dist / radius );
			if ( ent == attacker || ( ent->IsType( idAFAttachment::Type ) && static_cast<idAFAttachment*>(ent)->GetBody() == attacker ) ) {
				damageScale *= attackerDamageScale;
			}

			ent->Damage( inflictor, attacker, dir, damageDefName, damageScale, INVALID_JOINT );
		} 
	}

	// push physics objects
	if ( push ) {
		RadiusPush( origin, radius, push * dmgPower, attacker, ignorePush, attackerPushScale, false );
	}
}

/*
==============
idGameLocal::RadiusPush
==============
*/
void idGameLocal::RadiusPush( const idVec3 &origin, const float radius, const float push, const idEntity *inflictor, const idEntity *ignore, float inflictorScale, const bool quake ) {
	int i, numListedClipModels;
	idClipModel *clipModel;
	idClipModel *clipModelList[ MAX_GENTITIES ];
	idVec3 dir;
	idBounds bounds;
	modelTrace_t result;
	idEntity *ent;
	float scale;

	dir.Set( 0.0f, 0.0f, 1.0f );

	bounds = idBounds( origin ).Expand( radius );

	// get all clip models touching the bounds
	numListedClipModels = clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );

	if ( inflictor && inflictor->IsType( idAFAttachment::Type ) ) {
		inflictor = static_cast<const idAFAttachment*>(inflictor)->GetBody();
	}
	if ( ignore && ignore->IsType( idAFAttachment::Type ) ) {
		ignore = static_cast<const idAFAttachment*>(ignore)->GetBody();
	}

	// apply impact to all the clip models through their associated physics objects
	for ( i = 0; i < numListedClipModels; i++ ) {

		clipModel = clipModelList[i];

		// never push render models
		if ( clipModel->IsRenderModel() ) {
			continue;
		}

		ent = clipModel->GetEntity();

		// never push projectiles
		if ( ent->IsType( idProjectile::Type ) ) {
			continue;
		}

		// players use "knockback" in idPlayer::Damage
		if ( ent->IsType( idPlayer::Type ) && !quake ) {
			continue;
		}

		// don't push the ignore entity
		if ( ent == ignore || ( ent->IsType( idAFAttachment::Type ) && static_cast<idAFAttachment*>(ent)->GetBody() == ignore ) ) {
			continue;
		}

		if ( gameRenderWorld->FastWorldTrace( result, origin, clipModel->GetOrigin() ) ) {
			continue;
		}

		// scale the push for the inflictor
		if ( ent == inflictor || ( ent->IsType( idAFAttachment::Type ) && static_cast<idAFAttachment*>(ent)->GetBody() == inflictor ) ) {
			scale = inflictorScale;
		} else {
			scale = 1.0f;
		}

		if ( quake ) {
			clipModel->GetEntity()->ApplyImpulse( world, clipModel->GetId(), clipModel->GetOrigin(), scale * push * dir );
		} else {
			RadiusPushClipModel( origin, scale * push, clipModel );
		}
	}
}

/*
==============
idGameLocal::RadiusPushClipModel
==============
*/
void idGameLocal::RadiusPushClipModel( const idVec3 &origin, const float push, const idClipModel *clipModel ) {
	int i, j;
	float dot, dist, area;
	const idTraceModel *trm;
	const traceModelPoly_t *poly;
	idFixedWinding w;
	idVec3 v, localOrigin, center, impulse;

	trm = clipModel->GetTraceModel();
	if ( !trm || 1 ) {
		impulse = clipModel->GetAbsBounds().GetCenter() - origin;
		impulse.Normalize();
		impulse.z += 1.0f;
		clipModel->GetEntity()->ApplyImpulse( world, clipModel->GetId(), clipModel->GetOrigin(), push * impulse );
		return;
	}

	localOrigin = ( origin - clipModel->GetOrigin() ) * clipModel->GetAxis().Transpose();
	for ( i = 0; i < trm->numPolys; i++ ) {
		poly = &trm->polys[i];

		center.Zero();
		for ( j = 0; j < poly->numEdges; j++ ) {
			v = trm->verts[ trm->edges[ abs(poly->edges[j]) ].v[ INTSIGNBITSET( poly->edges[j] ) ] ];
			center += v;
			v -= localOrigin;
			v.NormalizeFast();	// project point on a unit sphere
			w.AddPoint( v );
		}
		center /= poly->numEdges;
		v = center - localOrigin;
		dist = v.NormalizeFast();
		dot = v * poly->normal;
		if ( dot > 0.0f ) {
			continue;
		}
		area = w.GetArea();
		// impulse in polygon normal direction
		impulse = poly->normal * clipModel->GetAxis();
		// always push up for nicer effect
		impulse.z -= 1.0f;
		// scale impulse based on visible surface area and polygon angle
		impulse *= push * ( dot * area * ( 1.0f / ( 4.0f * idMath::PI ) ) );
		// scale away distance for nicer effect
		impulse *= ( dist * 2.0f );
		// impulse is applied to the center of the polygon
		center = clipModel->GetOrigin() + center * clipModel->GetAxis();

		clipModel->GetEntity()->ApplyImpulse( world, clipModel->GetId(), center, impulse );
	}
}

/*
===============
idGameLocal::ProjectDecal
===============
*/
void idGameLocal::ProjectDecal( const idVec3 &origin, const idVec3 &dir, float depth, bool parallel, float size, const char *material, float angle ) {
	float s, c;
	idMat3 axis, axistemp;
	idFixedWinding winding;
	idVec3 windingOrigin, projectionOrigin;

	static idVec3 decalWinding[4] = {
		idVec3(  1.0f,  1.0f, 0.0f ),
		idVec3( -1.0f,  1.0f, 0.0f ),
		idVec3( -1.0f, -1.0f, 0.0f ),
		idVec3(  1.0f, -1.0f, 0.0f )
	};

	if ( !g_decals.GetBool() ) {
		return;
	}

	// randomly rotate the decal winding
	idMath::SinCos16( ( angle ) ? angle : random.RandomFloat() * idMath::TWO_PI, s, c );

	// winding orientation
	axis[2] = dir;
	axis[2].Normalize();
	axis[2].NormalVectors( axistemp[0], axistemp[1] );
	axis[0] = axistemp[ 0 ] * c + axistemp[ 1 ] * -s;
	axis[1] = axistemp[ 0 ] * -s + axistemp[ 1 ] * -c;

	windingOrigin = origin + depth * axis[2];
	if ( parallel ) {
		projectionOrigin = origin - depth * axis[2];
	} else {
		projectionOrigin = origin;
	}

	size *= 0.5f;

	winding.Clear();
	winding += idVec5( windingOrigin + ( axis * decalWinding[0] ) * size, idVec2( 1, 1 ) );
	winding += idVec5( windingOrigin + ( axis * decalWinding[1] ) * size, idVec2( 0, 1 ) );
	winding += idVec5( windingOrigin + ( axis * decalWinding[2] ) * size, idVec2( 0, 0 ) );
	winding += idVec5( windingOrigin + ( axis * decalWinding[3] ) * size, idVec2( 1, 0 ) );
	gameRenderWorld->ProjectDecalOntoWorld( winding, projectionOrigin, parallel, depth * 0.5f, declManager->FindMaterial( material ), time );
}

/*
==============
idGameLocal::BloodSplat
==============
*/
void idGameLocal::BloodSplat( const idVec3 &origin, const idVec3 &dir, float size, const char *material ) {
	float halfSize = size * 0.5f;
	idVec3 verts[] = {	idVec3( 0.0f, +halfSize, +halfSize ),
						idVec3( 0.0f, +halfSize, -halfSize ),
						idVec3( 0.0f, -halfSize, -halfSize ),
						idVec3( 0.0f, -halfSize, +halfSize ) };
	idTraceModel trm;
	idClipModel mdl;
	trace_t results;

	// FIXME: get from damage def
	if ( !g_bloodEffects.GetBool() ) {
		return;
	}

	size = halfSize + random.RandomFloat() * halfSize;
	trm.SetupPolygon( verts, 4 );
	mdl.LoadModel( trm );
	clip.Translation( results, origin, origin + dir * 64.0f, &mdl, mat3_identity, CONTENTS_SOLID, NULL );
	ProjectDecal( results.endpos, dir, 2.0f * size, true, size, material );
}

/*
=============
idGameLocal::SetCamera
=============
*/
void idGameLocal::SetCamera( idCamera *cam ) {
	int i;
	idEntity *ent;
	idAI *ai;

	// this should fix going into a cinematic when dead.. rare but happens
	idPlayer *client = GetLocalPlayer();
	if ( client->health <= 0 || client->AI_DEAD ) {
		return;
	}

	camera = cam;
	if ( camera ) {
		inCinematic = true;

		if ( skipCinematic && camera->spawnArgs.GetBool( "disconnect" ) ) {
			camera->spawnArgs.SetBool( "disconnect", false );
			cvarSystem->SetCVarFloat( "r_znear", 3.0f );
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "disconnect\n" );
			skipCinematic = false;
			return;
		}

		if ( time > cinematicStopTime ) {
			cinematicSkipTime = time + CINEMATIC_SKIP_DELAY;
		}

		// set r_znear so that transitioning into/out of the player's head doesn't clip through the view
		cvarSystem->SetCVarFloat( "r_znear", 1.0f );
		
		// hide all the player models
		for( i = 0; i < numClients; i++ ) {
			if ( entities[ i ] ) {
				client = static_cast< idPlayer* >( entities[ i ] );
				client->EnterCinematic();
			}
		}

		if ( !cam->spawnArgs.GetBool( "ignore_enemies" ) ) {
			// kill any active monsters that are enemies of the player
			for ( ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
				if ( ent->cinematic || ent->fl.isDormant ) {
					// only kill entities that aren't needed for cinematics and aren't dormant
					continue;
				}
				
				if ( ent->IsType( idAI::Type ) ) {
					ai = static_cast<idAI *>( ent );
					if ( !ai->GetEnemy() || !ai->IsActive() ) {
						// no enemy, or inactive, so probably safe to ignore
						continue;
					}
				} else if ( ent->IsType( idProjectile::Type ) ) {
					// remove all projectiles
				} else if ( ent->spawnArgs.GetBool( "cinematic_remove" ) ) {
					// remove anything marked to be removed during cinematics
				} else {
					// ignore everything else
					continue;
				}

				// remove it
				DPrintf( "removing '%s' for cinematic\n", ent->GetName() );
				ent->PostEventMS( &EV_Remove, 0 );
			}
		}

	} else {
		inCinematic = false;
		cinematicStopTime = time + msec;

		// restore r_znear
		cvarSystem->SetCVarFloat( "r_znear", 3.0f );

		// show all the player models
		for( i = 0; i < numClients; i++ ) {
			if ( entities[ i ] ) {
				idPlayer *client = static_cast< idPlayer* >( entities[ i ] );
				client->ExitCinematic();
			}
		}
	}
}

/*
=============
idGameLocal::GetCamera
=============
*/
idCamera *idGameLocal::GetCamera( void ) const {
	return camera;
}

/*
=============
idGameLocal::SkipCinematic
=============
*/
bool idGameLocal::SkipCinematic( void ) {
	if ( camera ) {
		if ( camera->spawnArgs.GetBool( "disconnect" ) ) {
			camera->spawnArgs.SetBool( "disconnect", false );
			cvarSystem->SetCVarFloat( "r_znear", 3.0f );
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "disconnect\n" );
			skipCinematic = false;
			return false;
		}

		if ( camera->spawnArgs.GetBool( "instantSkip" ) ) {
			camera->Stop();
			return false;
		}
	}

	soundSystem->SetMute( true );
	if ( !skipCinematic ) {
		skipCinematic = true;
		cinematicMaxSkipTime = gameLocal.time + SEC2MS( g_cinematicMaxSkipTime.GetFloat() );
	}

	return true;
}


/*
======================
idGameLocal::SpreadLocations

Now that everything has been spawned, associate areas with location entities
======================
*/
void idGameLocal::SpreadLocations() {
	idEntity *ent;

	// allocate the area table
	int	numAreas = gameRenderWorld->NumAreas();
	locationEntities = new idLocationEntity *[ numAreas ];
	memset( locationEntities, 0, numAreas * sizeof( *locationEntities ) );

	// for each location entity, make pointers from every area it touches
	for( ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		if ( !ent->IsType( idLocationEntity::Type ) ) {
			continue;
		}
		idVec3	point = ent->spawnArgs.GetVector( "origin" );
		int areaNum = gameRenderWorld->PointInArea( point );
		if ( areaNum < 0 ) {
			Printf( "SpreadLocations: location '%s' is not in a valid area\n", ent->spawnArgs.GetString( "name" ) );
			continue;
		}
		if ( areaNum >= numAreas ) {
			Error( "idGameLocal::SpreadLocations: areaNum >= gameRenderWorld->NumAreas()" );
		}
		if ( locationEntities[areaNum] ) {
			Warning( "location entity '%s' overlaps '%s'", ent->spawnArgs.GetString( "name" ),
				locationEntities[areaNum]->spawnArgs.GetString( "name" ) );
			continue;
		}
		locationEntities[areaNum] = static_cast<idLocationEntity *>(ent);

		// spread to all other connected areas
		for ( int i = 0 ; i < numAreas ; i++ ) {
			if ( i == areaNum ) {
				continue;
			}
			if ( gameRenderWorld->AreasAreConnected( areaNum, i, PS_BLOCK_LOCATION ) ) {
				locationEntities[i] = static_cast<idLocationEntity *>(ent);
			}
		}
	}
}

/*
===================
idGameLocal::LocationForPoint

The player checks the location each frame to update the HUD text display
May return NULL
===================
*/
idLocationEntity *idGameLocal::LocationForPoint( const idVec3 &point ) {
	if ( !locationEntities ) {
		// before SpreadLocations() has been called
		return NULL;
	}

	int areaNum = gameRenderWorld->PointInArea( point );
	if ( areaNum < 0 ) {
		return NULL;
	}
	if ( areaNum >= gameRenderWorld->NumAreas() ) {
		Error( "idGameLocal::LocationForPoint: areaNum >= gameRenderWorld->NumAreas()" );
	}

	return locationEntities[ areaNum ];
}

/*
============
idGameLocal::SetPortalState
============
*/
void idGameLocal::SetPortalState( qhandle_t portal, int blockingBits ) {
	idBitMsg outMsg;
	byte msgBuf[ MAX_GAME_MESSAGE_SIZE ];

	if ( !gameLocal.isClient ) {
		outMsg.Init( msgBuf, sizeof( msgBuf ) );
		outMsg.WriteByte( GAME_RELIABLE_MESSAGE_PORTAL );
		outMsg.WriteLong( portal );
		outMsg.WriteBits( blockingBits, NUM_RENDER_PORTAL_BITS );
		networkSystem->ServerSendReliableMessage( -1, outMsg );
	}
	gameRenderWorld->SetPortalState( portal, blockingBits );
}

/*
============
idGameLocal::sortSpawnPoints
============
*/
int idGameLocal::sortSpawnPoints( const void *ptr1, const void *ptr2 ) {
	const spawnSpot_t *spot1 = static_cast<const spawnSpot_t *>( ptr1 );
	const spawnSpot_t *spot2 = static_cast<const spawnSpot_t *>( ptr2 );
	float diff;

	diff = spot1->dist - spot2->dist;
	if ( diff < 0.0f ) {
		return 1;
	} else if ( diff > 0.0f ) {
		return -1;
	} else {
		return 0;
	}
}

/*
===========
idGameLocal::RandomizeInitialSpawns
randomize the order of the initial spawns
prepare for a sequence of initial player spawns
============
*/
void idGameLocal::RandomizeInitialSpawns( void ) {
	spawnSpot_t	spot;
	int i, j;
	idEntity *ent;

	if ( !isMultiplayer || isClient ) {
		return;
	}
	spawnSpots.Clear();
	initialSpots.Clear();
	spot.dist = 0;
	spot.ent = FindEntityUsingDef( NULL, "info_player_deathmatch" );
	while( spot.ent ) {
		spawnSpots.Append( spot );
		if ( spot.ent->spawnArgs.GetBool( "initial" ) ) {
			initialSpots.Append( spot.ent );
		}
		spot.ent = FindEntityUsingDef( spot.ent, "info_player_deathmatch" );
	}
	if ( !spawnSpots.Num() ) {
		common->Warning( "no info_player_deathmatch in map" );
		return;
	}
	common->Printf( "%d spawns (%d initials)\n", spawnSpots.Num(), initialSpots.Num() );
	// if there are no initial spots in the map, consider they can all be used as initial
	if ( !initialSpots.Num() ) {
		common->Warning( "no info_player_deathmatch entities marked initial in map" );
		for ( i = 0; i < spawnSpots.Num(); i++ ) {
			initialSpots.Append( spawnSpots[ i ].ent );
		}
	}
	for ( i = 0; i < initialSpots.Num(); i++ ) {
		j = random.RandomInt( initialSpots.Num() );
		ent = initialSpots[ i ];
		initialSpots[ i ] = initialSpots[ j ];
		initialSpots[ j ] = ent;
	}
	// reset the counter
	currentInitialSpot = 0;
}

/*
===========
idGameLocal::SelectInitialSpawnPoint
spectators are spawned randomly anywhere
in-game clients are spawned based on distance to active players (randomized on the first half)
upon map restart, initial spawns are used (randomized ordered list of spawns flagged "initial")
  if there are more players than initial spots, overflow to regular spawning
============
*/
idEntity *idGameLocal::SelectInitialSpawnPoint( idPlayer *player ) {
	int				i, j, which;
	spawnSpot_t		spot;
	idVec3			pos;
	float			dist;
	bool			alone;

	if ( !isMultiplayer || !spawnSpots.Num() ) {
		spot.ent = FindEntityUsingDef( NULL, "info_player_start" );
		if ( !spot.ent ) {
			Error( "No info_player_start on map.\n" );
		}
		return spot.ent;
	}
	if ( player->spectating ) {
		// plain random spot, don't bother
		return spawnSpots[ random.RandomInt( spawnSpots.Num() ) ].ent;
	} else if ( player->useInitialSpawns && currentInitialSpot < initialSpots.Num() ) {
		return initialSpots[ currentInitialSpot++ ];
	} else {
		// check if we are alone in map
		alone = true;
		for ( j = 0; j < MAX_CLIENTS; j++ ) {
			if ( entities[ j ] && entities[ j ] != player ) {
				alone = false;
				break;
			}
		}
		if ( alone ) {
			// don't do distance-based
			return spawnSpots[ random.RandomInt( spawnSpots.Num() ) ].ent;
		}

		// find the distance to the closest active player for each spawn spot
		for( i = 0; i < spawnSpots.Num(); i++ ) {
			pos = spawnSpots[ i ].ent->GetPhysics()->GetOrigin();
			spawnSpots[ i ].dist = 0x7fffffff;
			for( j = 0; j < MAX_CLIENTS; j++ ) {
				if ( !entities[ j ] || !entities[ j ]->IsType( idPlayer::Type )
					|| entities[ j ] == player
					|| static_cast< idPlayer * >( entities[ j ] )->spectating ) {
					continue;
				}
				
				dist = ( pos - entities[ j ]->GetPhysics()->GetOrigin() ).LengthSqr();
				if ( dist < spawnSpots[ i ].dist ) {
					spawnSpots[ i ].dist = int(dist);
				}
			}
		}

		// sort the list
		qsort( ( void * )spawnSpots.Ptr(), spawnSpots.Num(), sizeof( spawnSpot_t ), ( int (*)(const void *, const void *) )sortSpawnPoints );

		// choose a random one in the top half
		which = random.RandomInt( spawnSpots.Num() / 2 );
		spot = spawnSpots[ which ];
	}
	return spot.ent;
}

/*
================
idGameLocal::UpdateServerInfoFlags
================
*/
void idGameLocal::UpdateServerInfoFlags() {
	gameType = GAME_SP;
	if ( ( idStr::Icmp( serverInfo.GetString( "si_gameType" ), "deathmatch" ) == 0 ) ) {
		gameType = GAME_DM;
	} else if ( ( idStr::Icmp( serverInfo.GetString( "si_gameType" ), "Tourney" ) == 0 ) ) {
		gameType = GAME_TOURNEY;
	} else if ( ( idStr::Icmp( serverInfo.GetString( "si_gameType" ), "Team DM" ) == 0 ) ) {
		gameType = GAME_TDM;
	} else if ( ( idStr::Icmp( serverInfo.GetString( "si_gameType" ), "Last Man" ) == 0 ) ) {
		gameType = GAME_LASTMAN;
	}
	if ( gameType == GAME_LASTMAN ) {
		if ( !serverInfo.GetInt( "si_warmup" ) ) {
			common->Warning( "Last Man Standing - forcing warmup on" );
			serverInfo.SetInt( "si_warmup", 1 );
		}
		if ( serverInfo.GetInt( "si_fraglimit" ) <= 0 ) {
			common->Warning( "Last Man Standing - setting fraglimit 1" );
			serverInfo.SetInt( "si_fraglimit", 1 );
		}
	}
}


/*
================
idGameLocal::SetGlobalMaterial
================
*/
void idGameLocal::SetGlobalMaterial( const idMaterial *mat ) {
	globalMaterial = mat;
}

/*
================
idGameLocal::GetGlobalMaterial
================
*/
const idMaterial *idGameLocal::GetGlobalMaterial() {
	return globalMaterial;
}

/*
================
idGameLocal::GetSpawnId
================
*/
int idGameLocal::GetSpawnId( const idEntity* ent ) const {
	return ( gameLocal.spawnIds[ ent->entityNumber ] << GENTITYNUM_BITS ) | ent->entityNumber;
}

/*
================
idGameLocal::ThrottleUserInfo
================
*/
void idGameLocal::ThrottleUserInfo( void ) {
	mpGame.ThrottleUserInfo();
}

/*

=================

idPlayer::SetPortalSkyEnt

=================

*/

void idGameLocal::SetPortalSkyEnt( idEntity *ent ) {
	portalSkyEnt = ent;
}



/*
=================
idPlayer::IsPortalSkyAcive
=================
*/
bool idGameLocal::IsPortalSkyAcive() {
	return portalSkyActive;
}



/*
===========
idGameLocal::SelectTimeGroup
============
*/

void idGameLocal::SelectTimeGroup( int timeGroup ) { }



/*
===========
idGameLocal::GetTimeGroupTime
============
*/
int idGameLocal::GetTimeGroupTime( int timeGroup ) {
	return gameLocal.time;
}



/*
===========
idGameLocal::GetBestGameType
============
*/
void idGameLocal::GetBestGameType( const char* map, const char* gametype, char buf[ MAX_STRING_CHARS ] ) {
	strncpy( buf, gametype, MAX_STRING_CHARS );
	buf[ MAX_STRING_CHARS - 1 ] = '\0';
}

/*
===========
idGameLocal::NeedRestart
============
*/
bool idGameLocal::NeedRestart() {
	idDict		newInfo;
	const idKeyValue *keyval, *keyval2;

	newInfo = *cvarSystem->MoveCVarsToDict( CVAR_SERVERINFO );

	for ( int i = 0; i < newInfo.GetNumKeyVals(); i++ ) {
		keyval = newInfo.GetKeyVal( i );
		keyval2 = serverInfo.FindKey( keyval->GetKey() );
		if ( !keyval2 ) {
			return true;
		}

		// a select set of si_ changes will cause a full restart of the server
		if ( keyval->GetValue().Cmp( keyval2->GetValue() ) && ( !keyval->GetKey().Cmp( "si_pure" ) || !keyval->GetKey().Cmp( "si_map" ) ) ) {
			return true;
		}
	}
	return false;
}



/*
================
idGameLocal::GetClientStats
================
*/

void idGameLocal::GetClientStats( int clientNum, char *data, const int len ) {

	mpGame.PlayerStats( clientNum, data, len );
}

void idGameLocal::SwitchTeam( int clientNum, int team )
{
	idPlayer *   player;

	player = clientNum >= 0 ? static_cast<idPlayer *>( gameLocal.entities[ clientNum ] ) : NULL;
	if ( !player )
		return;
	int oldTeam = player->team;

	// Put in spectator mode
	if ( team == -1 ) {
		static_cast< idPlayer * >( entities[ clientNum ] )->Spectate( true );
	}

	// Switch to a team
	else {
		mpGame.SwitchToTeam ( clientNum, oldTeam, team );
	}
}

void idGameLocal::GetMapLoadingGUI( char gui[ MAX_STRING_CHARS ] )
{
}

void idGameLocal::LoadLightMaterial(const char *pFN, idList<CLightMaterial *> *ml)
{
	idToken token;
	idLexer src;
	idStr Material, FallOff, Map, *add;
	int level;		// Nestinglevel for brackets
	bool bAmbient;
	CLightMaterial *mat;

	if(pFN == NULL || ml == NULL)
		goto Quit;

	src.LoadFile(pFN);

	level = 0;
	add = NULL;
	bAmbient = false;

	while(1)
	{
		if(!src.ReadToken(&token))
			goto Quit;

//		DM_LOG(LC_SYSTEM, LT_DEBUG)LOGSTRING("Token: [%s]\r", token.c_str());

		if(token == "table")
		{
			src.SkipBracedSection(true);
			continue;
		}

		if(token == "lights")
		{
			Material = token;
			while(src.ReadTokenOnLine(&token) == true)
			{
				Material += token;
//				DM_LOG(LC_SYSTEM, LT_DEBUG)LOGSTRING("Material: [%s]\r", token.c_str());
			}

			continue;
		}
		else if(level == 1 && token == "ambientLight")
		{
			bAmbient = true;
			continue;
		}
		else if(token == "{")
		{
			level++;
			if(level == 1)
				bAmbient = false;

			continue;
		}
		else if(token == "}")
		{
			level--;
			if(level == 0)
			{
				if(FallOff.Length()  == 0 && Map.Length() == 0)
					continue;

				mat = new CLightMaterial(Material, FallOff, Map);
				mat->m_AmbientLight = bAmbient;
				ml->Append(mat);
				DM_LOG(LC_SYSTEM, LT_INFO)LOGSTRING("Texture: [%s] - [%s]/[%s] - Ambient: %u\r", Material.c_str(), FallOff.c_str(), Map.c_str(), bAmbient);
			}
			continue;
		}
		else if(token == "map")
		{
			Map = "";
			while(src.ReadTokenOnLine(&token) == true)
			{
				if(token == "makeintensity")
					continue;
				else if(token == "(")
					continue;
				else if(token == ")")
					break;
				else
					Map += token;
//				DM_LOG(LC_SYSTEM, LT_DEBUG)LOGSTRING("Map: [%s]\r", token.c_str());
			}
			continue;
		}
		else if(token == "lightFalloffImage")
		{
			FallOff = "";

			while(1)
			{
				if(!src.ReadToken(&token))
				{
					DM_LOG(LC_SYSTEM, LT_ERROR)LOGSTRING("Invalid material file structure on line %u\r", src.GetLineNum());
					goto Quit;
				}

				// Ignore makeintensity tag
				if(token == "makeintensity")
					continue;
				else if(token == "(")
					continue;
				else if(token == ")")
					break;
				else
				{
					do
					{
						if(token == ")")
							break;

						FallOff += token;
//						DM_LOG(LC_SYSTEM, LT_DEBUG)LOGSTRING("FallOff: [%s]\r", token.c_str());
					}
					while(src.ReadTokenOnLine(&token) == true);
					break;
				}
			}
			continue;
		}
	}

Quit:
	return;
}

float idGameLocal::CalcLightgem(idPlayer *player)
{
	float dist = cv_lg_distance.GetFloat();			// reasonable distance to get a good look at the player/test model
	float fColVal[DARKMOD_LG_MAX_IMAGESPLIT];
	float fRetVal = 0.0;
	int playerid;			// player viewid
	int headid;				// head viewid
	int pdef;				// player modeldef
	int hdef;				// head modeldef
	int psid;				// player shadow viewid
	int hsid;				// head shadow viewid
	int i, n, k, dim, l;
	idStr dps;
	renderView_t rv, rv1;
	idEntity *lg;
	renderEntity_t *prent;			// Player renderentity
	renderEntity_t *hrent;			// Head renderentity
	renderEntity_t *lgrend;
	const char *dp = NULL;

	lg = m_LightgemSurface.GetEntity();
	idVec3 Cam = player->GetEyePosition();
	idVec3 Pos = player->GetPhysics()->GetOrigin();
	idVec3 LGPos = Pos; // Set the lightgem position to that of the player
	LGPos.x += ( Cam.x - Pos.x ) * 0.3; // Move the lightgem out a fraction along the leaning x vector
	LGPos.y += ( Cam.y - Pos.y ) * 0.3; // Move the lightgem out a fraction along the leaning y vector
	LGPos.z = Cam.z; // Set the lightgem's Z-axis position to that of the player's eyes

	if (LGPos.z < Pos.z + 50 && static_cast<idPhysics_Player*>(player->GetPlayerPhysics())->IsCrouching())
	{
		// Prevent lightgem from clipping into the floor while crouching
		LGPos.z = Pos.z + 50;
	}

	// Adjust the modelposition with userdefined offsets.
	// Move the lightgem testmodel to the players feet based on the eye position
	LGPos.x += cv_lg_oxoffs.GetInteger();
	LGPos.y += cv_lg_oyoffs.GetInteger();
	LGPos.z += cv_lg_ozoffs.GetInteger();
	lg->SetOrigin(LGPos);

	memset(&rv, 0, sizeof(rv));

	for(i = 0; i < DARKMOD_LG_MAX_IMAGESPLIT; i++)
		fColVal[i] = 0.0;

	for(i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ )
		rv.shaderParms[i] = gameLocal.globalShaderParms[i];

	rv.globalMaterial = gameLocal.GetGlobalMaterial();
//	rv.width = SCREEN_WIDTH;
//	rv.height = SCREEN_HEIGHT;
	rv.width = cv_lg_screen_width.GetInteger();
	rv.height = cv_lg_screen_height.GetInteger();
	rv.fov_x = cv_lg_fov.GetInteger();
	rv.fov_y = cv_lg_fov.GetInteger();		// Bigger values means more compressed view
	rv.forceUpdate = false;
	rv.x = 0;
	rv.y = 0;
	rv.time = time;

	n = cv_lg_renderpasses.GetInteger();
	// limit the renderpasses between 1 and 4
	if(n < 1) n = 1;
	if(n > DARKMOD_LG_MAX_RENDERPASSES) n = DARKMOD_LG_MAX_RENDERPASSES;

	k = cv_lg_hud.GetInteger()-1;
	lgrend = lg->GetRenderEntity();

	// Set the viewid to our private screenshot snapshot. If this number is changed 
	// for some reason, it has to be changed in player.cpp as well.
	rv.viewID = DARKMOD_LG_VIEWID;
	lgrend->suppressShadowInViewID = 0;

	if(cv_lg_player.GetBool() == false)
		lgrend->allowSurfaceInViewID = rv.viewID;
	else
		lgrend->allowSurfaceInViewID = 0;

	// Tell the renderengine about the change for this entity.
	prent = lg->GetRenderEntity();
	if((pdef = lg->GetModelDefHandle()) != -1)
		gameRenderWorld->UpdateEntityDef(pdef, prent);

	prent = player->GetRenderEntity();
	hrent = player->GetHeadEntity()->GetRenderEntity();

	playerid = prent->suppressSurfaceInViewID;
	psid = prent->suppressShadowInViewID;
	prent->suppressShadowInViewID = rv.viewID;
	prent->suppressSurfaceInViewID = rv.viewID;

	headid = hrent->suppressSurfaceInViewID;
	hsid = hrent->suppressShadowInViewID;
	hrent->suppressShadowInViewID = rv.viewID;
	hrent->suppressSurfaceInViewID = rv.viewID;

	if((pdef = player->GetModelDefHandle()) != -1)
		gameRenderWorld->UpdateEntityDef(pdef, prent);

	if((hdef = player->GetHeadEntity()->GetModelDefHandle()) != -1)
		gameRenderWorld->UpdateEntityDef(hdef, hrent);

	dim = cv_lg_image_width.GetInteger();
	if(dim <= 0 || dim > 1024)
		dim = DARKMOD_LG_RENDER_WIDTH;

	fRetVal = 0.0;

	rv1 = rv;
	DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("RenderTurn %u", m_LightgemShotSpot);
	for(i = 0; i < n; i++)
	{
		rv = rv1;
		rv.vieworg = LGPos;

		// If it's not the turn for this lightgem shot, then
		// we skip over it. We also skip it if the splitting is disabled.
		if(cv_lg_split.GetBool() == true)
		{
			if(m_LightgemShotSpot != i)
				continue;
		}

		m_LightgemShotValue[i] = 0.0;

		switch(i)
		{
			case 0:	// From the top to bottom
			{
				rv.vieworg.z += cv_lg_zoffs.GetInteger();
				rv.vieworg.z += dist;
				rv.viewaxis = idMat3(	
					0.0, 0.0, -1.0,
					0.0, 1.0, 0.0,
					1.0, 0.0, 0.0
				);
			}
			break;

			case 1:
			{
				// From bottom to top
				rv.vieworg.z -= cv_lg_zoffs.GetInteger();
				rv.vieworg.z -= dist;
				rv.viewaxis = idMat3(	
					0.0, 0.0, 1.0,
					0.0, 1.0, 0.0,
					-1.0, 0.0, 0.0
				);
			}
			break;
		}

		// If the hud is enabled we either process all of them in case it is set to 0,
		// then we don't care which one is actually displayed (most likely the last or
		// the first one), or we only show the one that should be shown.
		if(k == -1 || k == i)
		{
			m_RenderPipe->Prepare();

			// We always use a square image, because we render now an overhead shot which
			// covers all four side of the player at once, using a diamond or pyramid shape.
			// The result is an image that is split in four triangles with an angle of 
			// 45 degree, thus the square shape.
			renderSystem->CropRenderSize(dim, dim, true);
			gameRenderWorld->SetRenderView(&rv);
			gameRenderWorld->RenderScene(&rv);
			renderSystem->CaptureRenderToFile(m_RenderPipe->FileName());
			dp = cv_lg_path.GetString();
			if(dp != NULL && strlen(dp) != 0)
			{
				sprintf(dps, "%s_%u.tga", dp, i);
				dp = dps.c_str();
				renderSystem->CaptureRenderToFile(dp);
			}
			else
				dp = NULL;

			DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("Rendering to file [%s]\n", m_RenderPipe->FileName());
			renderSystem->UnCrop();

			AnalyzeRenderImage(m_RenderPipe, fColVal);
			m_RenderPipe->CleanUp();

			// Check which of the images has the brightest value, and this is what we will use.
			for(l = 0; l < DARKMOD_LG_MAX_IMAGESPLIT; l++)
			{
				DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("fColVal[%u] = %f/%f\n", l, fColVal[l], m_LightgemShotValue[i]);
				if(fColVal[l] > m_LightgemShotValue[i])
					m_LightgemShotValue[i] = fColVal[l];
			}
		}
	}

	m_LightgemShotSpot++;
	if(m_LightgemShotSpot >= n)
		m_LightgemShotSpot = 0;

	for(i = 0; i < n; i++)
	{
		if(m_LightgemShotValue[i] > fRetVal)
			fRetVal = m_LightgemShotValue[i];
	}

	prent->suppressSurfaceInViewID = playerid;
	prent->suppressShadowInViewID = psid;
	hrent->suppressSurfaceInViewID = headid;
	hrent->suppressShadowInViewID = hsid;

	// and switch back our renderdefinition.
	if(pdef != -1)
		gameRenderWorld->UpdateEntityDef(pdef, prent);

	if(hdef != -1)
		gameRenderWorld->UpdateEntityDef(hdef, hrent);

	return(fRetVal);
}

void idGameLocal::AnalyzeRenderImage(CRenderPipe* pipe, float fColVal[DARKMOD_LG_MAX_IMAGESPLIT])
{
	CImage *im = &g_Global.m_RenderImage ;
	unsigned long counter[DARKMOD_LG_MAX_IMAGESPLIT];
	int i, in, k, kn, h, x;
	
	im->LoadImage(pipe);
	unsigned char *buffer = im->GetImage();

	// This is just an errorhandling to inform the player that something is wrong.
	// The lightgem will simply blink if the renderpipe doesn't work.
	if(buffer == NULL)
	{
		static int indicator = 0;
		static int lasttime;
		DM_LOG(LC_SYSTEM, LT_ERROR)LOGSTRING("Unable to read image from renderpipe\r");
		for(i = 0; i < DARKMOD_LG_MAX_IMAGESPLIT; i++)
			fColVal[i] = indicator;

		if(gameLocal.time/1000 != lasttime)
		{
			lasttime = gameLocal.time/1000;
			indicator = !indicator;
		}

		goto Quit;
	}

	for(i = 0; i < DARKMOD_LG_MAX_IMAGESPLIT; i++)
		counter[i] = 0;

	// We always assume a BPP 4 here. We also always assume a square image with an even 
	// number of lines. An odd number might have only a very small influence though and
	// most likely get canceled out if a bigger image is used.
	kn = im->m_Height;
	h = kn/2;
	in = im->m_Width;

	// First we do the top half
	for(k = 0; k < h; k++)
	{
		for(i = 0; i < in; i++)
		{
			if(i < k)
				x = 0;
			else if(i > kn-k-1)
				x = 2;
			else
				x = 1;

			// The order is RGBA.
			fColVal[x] += ((buffer[0] * DARKMOD_LG_RED + buffer[1] * DARKMOD_LG_GREEN + buffer[2] * DARKMOD_LG_BLUE) * DARKMOD_LG_SCALE);
			counter[x]++;
			buffer += im->m_Bpp;
		}
	}

	// Then we do the bottom half where the triangles are inverted.
	for(k = (h-1); k >= 0; k--)
	{
		for(i = 0; i < in; i++)
		{
			if(i < k)
				x = 0;
			else if(i > kn-k-1)
				x = 2;
			else
				x = 3;

			// The order is RGBA.
			fColVal[x] += ((buffer[0] * DARKMOD_LG_RED + buffer[1] * DARKMOD_LG_GREEN + buffer[2] * DARKMOD_LG_BLUE) * DARKMOD_LG_SCALE);
			counter[x]++;
			buffer += im->m_Bpp;
		}
	}

	// Calculate the average for each value
	for(i = 0; i < DARKMOD_LG_MAX_IMAGESPLIT; i++)
		fColVal[i] = fColVal[i]/counter[i];

Quit:
	return;
}



void idGameLocal::SpawnLightgemEntity(void)
{
	static const char *LightgemName = DARKMOD_LG_ENTITY_NAME;
	idMapEntity *mapEnt = NULL;

	mapEnt = mapFile->FindEntity(LightgemName);
	if(mapEnt == NULL)
	{
		mapEnt = new idMapEntity();
		mapFile->AddEntity(mapEnt);
		mapEnt->epairs.Set("classname", "func_static");
		mapEnt->epairs.Set("name", LightgemName);
		if(strlen(cv_lg_model.GetString()) == 0)
			mapEnt->epairs.Set("model", DARKMOD_LG_RENDER_MODEL);
		else
			mapEnt->epairs.Set("model", cv_lg_model.GetString());
		mapEnt->epairs.Set("origin", "0 0 0");
		mapEnt->epairs.Set("noclipmodel", "1");
	}
}

int idGameLocal::CheckStimResponse(idList< idEntityPtr<idEntity> > &list, idEntity *e)
{

	int rc = -1;
	int i, n;

	n = list.Num();
	for(i = 0; i < n; i++)
	{
		if(list[i].GetEntity() == e)
		{
			rc = i;
			break;
		}
	}

	return(rc);
}


bool idGameLocal::AddStim(idEntity *e)
{
	bool rc = true;

	if(CheckStimResponse(m_StimEntity, e) == -1)
	{
		idEntityPtr<idEntity> entPtr;
		entPtr = e;
		m_StimEntity.Append(entPtr);
	}

	return rc;
}

void idGameLocal::RemoveStim(idEntity *e)
{
	int i;

	if((i = CheckStimResponse(m_StimEntity, e)) != -1)
	{
		m_StimEntity.RemoveIndex(i);
	}
}

bool idGameLocal::AddResponse(idEntity *e)
{
	bool rc = true;

	if(CheckStimResponse(m_RespEntity, e) == -1)
	{
		idEntityPtr<idEntity> entPtr;
		entPtr = e;
		m_RespEntity.Append(entPtr);
	}

	return rc;
}


void idGameLocal::RemoveResponse(idEntity *e)
{
	int i;

	if((i = CheckStimResponse(m_RespEntity, e)) != -1)
	{
		m_RespEntity.RemoveIndex(i);
	}
}

int idGameLocal::DoResponseAction(CStim* stim, int numEntities, idEntity* originator, const idVec3& stimOrigin)
{
	int numResponses = 0;

	for (int i = 0; i < numEntities; i++)
	{
		// ignore the original entity because an entity shouldn't respond 
		// to it's own stims.
		if (srEntities[i] == originator || srEntities[i]->GetResponseEntity() == originator)
			continue;

		// Check if the radius is really fitting. EntitiesTouchingBounds is using a rectangular volume
		// greebo: Be sure to use this check only if "use bounds" is set to false
		if (!stim->m_bCollisionBased && !stim->m_bUseEntBounds)
		{
			// take the square radius, is faster
			float radiusSqr = stim->GetRadius();
			radiusSqr *= radiusSqr; 

			if ((srEntities[i]->GetPhysics()->GetOrigin() - stimOrigin).LengthSqr() > radiusSqr)
			{
				// Too far away
				continue;
			}
		}

		// Check for a shooter entity, these don't need to have a response
		if (srEntities[i]->IsType(tdmFuncShooter::Type))
		{
			static_cast<tdmFuncShooter*>(srEntities[i])->stimulate(static_cast<StimType>(stim->m_StimTypeId));
		}

		CResponse* response = srEntities[i]->GetStimResponseCollection()->GetResponse(stim->m_StimTypeId);

		if (response != NULL)
		{
			if (response->m_State == SS_ENABLED && stim->CheckResponseIgnore(srEntities[i]) == false)
			{
				// Check duration, disable if past duration
				if (response->m_Duration && (gameLocal.time - response->m_EnabledTimeStamp) > response->m_Duration )
				{
					response->EnableSR(false);
					continue;
				}

				// Fire the response and pass the originating entity plus the stim object itself
				// The stim object can be queried for values like magnitude, falloff and such.
				response->TriggerResponse(originator, stim);
				numResponses++;
			}
		}
	}

	// Return number of responses triggered
	return numResponses;
}

void idGameLocal::ProcessTimer(unsigned long ticks)
{
	int i, n;
	CStimResponseTimer *t;

	n = m_Timer.Num();
	for(i = 0; i < n; i++)
	{
		t = m_Timer[i];

		// We just let the timers tick. Querying them must be done by the respective owner,
		// because we can not know from where it was called, and it doesn't make sense
		// to use a callback, because the timer user still has to wait until it is expired.
		// So the owner has to check it from time to time anyway.
		if(t->GetState() == CStimResponseTimer::SRTS_RUNNING)
		{
			t = m_Timer[i];
			t->Tick(ticks);
		}
	}
}

CStimResponse* idGameLocal::FindStimResponse(int uniqueId)
{
	for (int i = 0; i < MAX_GENTITIES; i++)
	{
		if (entities[i] != NULL && entities[i]->GetStimResponseCollection() != NULL)
		{
			CStimResponse* candidate = 
				entities[i]->GetStimResponseCollection()->FindStimResponse(uniqueId);

			if (candidate != NULL)
			{
				// We found the stim/response, return it
				DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("StimResponse with Id %d found.\r", uniqueId);
				return candidate;
			}
		}
	}

	DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Warning: StimResponse with Id %d NOT found.\r", uniqueId);
	// Search did not produce any results
	return NULL;
}

void idGameLocal::ProcessStimResponse(unsigned long ticks)
{
	if (cv_sr_disable.GetBool())
	{
		return; // S/R disabled, skip this
	}

	idTimer srTimer;
	srTimer.Clear();
	srTimer.Start();

	// Check the timed stims first.
	for (int i = 0; i < m_StimTimer.Num(); i++)
	{
		CStim* stim = m_StimTimer[i];

		// Only advance the timer if the stim can be fired in the first place
		if (stim->m_MaxFireCount > 0 || stim->m_MaxFireCount == -1)
		{
			// Advance the timer
			CStimResponseTimer* timer = stim->GetTimer();

			if (timer->GetState() == CStimResponseTimer::SRTS_RUNNING)
			{
				if (timer->Tick(ticks))
				{
					//gameLocal.Printf("Timer elapsed!\n");
					// Enable the stim when the timer has expired
					stim->EnableSR(true);
				}
			}
		}
	}

	int n;
	idBounds bounds;

	// Now check the rest of the stims.
	for (int i = 0; i < m_StimEntity.Num(); i++)
	{
		idEntity* entity = m_StimEntity[i].GetEntity();

		// greebo: Get the S/R collection, this is always non-NULL
		CStimResponseCollection* srColl = entity->GetStimResponseCollection();
		assert(srColl != NULL);

		idVec3 origin(entity->GetPhysics()->GetOrigin());

		idList<CStim*>& stimList = srColl->GetStimList();

		// Traverse through all the stims on this entity
		for (int stimIdx = 0; stimIdx < stimList.Num(); stimIdx++)
		{
			CStim* stim = stimList[stimIdx];

			if (stim->m_MaxFireCount == 0)
				// Maximum number of firings reached, do not process this stim
				continue;

			if (stim->m_State != SS_ENABLED)
				continue; // Stim is not enabled

			if (stim->m_bCollisionBased && !stim->m_bCollisionFired)
				continue; // Collision-based stim that did not fire with ::Collide this frame

			// Check the interleaving timer and don't eval stim if it's not up yet
			if (gameLocal.time - stim->m_TimeInterleaveStamp < stim->m_TimeInterleave)
				continue;

			// If stim has a finite duration, check if it expired. If so, disable the stim.
			if (stim->m_Duration != 0 && (gameLocal.time - stim->m_EnabledTimeStamp) > stim->m_Duration )
			{
				stim->EnableSR(false);
				continue;
			}

			// Initial checks passed, decrease the fire count
			if (stim->m_MaxFireCount > 0)
			{
				stim->m_MaxFireCount--;
			}

			// Check if a stim velocity has been specified
			if (stim->m_Velocity != idVec3(0,0,0)) {
				// The velocity mutliplied by the time gives the translation
				// Updates the location of this stim relative to the time it last fired.
				origin += stim->m_Velocity * (gameLocal.time - stim->m_EnabledTimeStamp)/1000;
			}

			// Save the current timestamp into the stim, so that we know when it was last fired
			stim->m_TimeInterleaveStamp = gameLocal.time;

			// greebo: Check if the stim passes the "chance" test
			// Do this AFTER the m_TimeInterleaveStamp has been set to avoid the stim
			// from being re-evaluated in the very next frame in case it failed the test.
			if (!stim->checkChance()) {
				continue;
			}

			// If stim is not disabled
			if (stim->m_State == SS_DISABLED)
				continue;

			float radius = stim->GetRadius();

			if (radius != 0.0 || stim->m_bCollisionBased ||
				stim->m_bUseEntBounds || stim->m_Bounds.GetVolume() > 0)
			{
				int numResponses = 0;

				// Check if we have fixed bounds to work with (sr_bounds_mins & maxs set)
				if (stim->m_Bounds.GetVolume() > 0) {
					bounds = idBounds(stim->m_Bounds[0] + origin, stim->m_Bounds[1] + origin);
				}
				else {
					// No bounds set, check for radius and useBounds

					// Find entities in the radius of the stim
					if (stim->m_bUseEntBounds )
					{
						bounds = entity->GetPhysics()->GetAbsBounds();
					}
					else
					{
						bounds = idBounds(origin);
					}

					bounds.ExpandSelf(radius);
				}

				// Collision-based stims
				if (stim->m_bCollisionBased)
				{
					n = stim->m_CollisionEnts.Num();

					for (int n2 = 0; n2 < n; n2++)
					{
						srEntities[n2] = stim->m_CollisionEnts[n2];
					}

					// clear the collision vars for the next frame
					stim->m_bCollisionFired = false;
					stim->m_CollisionEnts.Clear();
				}
				else 
				{
					// Radius based stims
					n = clip.EntitiesTouchingBounds(bounds, CONTENTS_RESPONSE, srEntities, MAX_GENTITIES);
					//DM_LOG(LC_STIM_RESPONSE, LT_INFO)LOGSTRING("Entities touching bounds: %d\r", n);
				}
				
				if (n > 0)
				{
					// Do responses for entities within the radius of the stim
					numResponses = DoResponseAction(stim, n, entity, origin);
				}

				// The stim has fired, let it do any post-firing activity it may have
				stim->PostFired(numResponses);
			}
		}
	}

	srTimer.Stop();
	DM_LOG(LC_STIM_RESPONSE, LT_INFO)LOGSTRING("Processing S/R took %lf\r", srTimer.Milliseconds());
}

/*
===================
Dark Mod:
idGameLocal::LocationForArea
===================
*/
idLocationEntity *idGameLocal::LocationForArea( const int areaNum ) 
{
	idLocationEntity *pReturnval( NULL );
	
	if ( !locationEntities || areaNum < 0 || areaNum >= gameRenderWorld->NumAreas() ) 
	{
		pReturnval = NULL;
		goto Quit;
	}

	pReturnval = locationEntities[ areaNum ];

Quit:
	return pReturnval;
}

/*
===================
Dark Mod:
idGameLocal::PlayerTraceEntity
===================
*/
idEntity *idGameLocal::PlayerTraceEntity( void )
{
	idVec3		start, end;
	idEntity	*returnEnt = NULL;
	idPlayer	*player = NULL;
	trace_t		trace;
	int			cm = 0;

	player = gameLocal.GetLocalPlayer();
	if( ! player )
		goto Quit;

	start = player->GetEyePosition();
	// do a trace 512 doom units ahead
	end = start + 512.0f * (player->viewAngles.ToForward());

	cm = CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_TRIGGER;
	gameLocal.clip.TracePoint( trace, start, end, cm, player );
	if( trace.fraction < 1.0f )
	{
		returnEnt = gameLocal.entities[ trace.c.entityNum ];
		// don't return the world
		if( returnEnt == gameLocal.world )
			returnEnt = NULL;
	}
	
Quit:
	return returnEnt;
}

void idGameLocal::AddSDKSignal(idEntity *oObject)
{
	if(oObject != NULL)
		m_SignalList.Append(oObject);
}

void idGameLocal::CheckSDKSignals()
{
	int n = m_SignalList.Num();
	for(int i = 0; i < n; i++)
	{
		m_SignalList[i]->CheckSDKSignal();
	}
}

void idGameLocal::PauseGame( bool bPauseState )
{
	if( bPauseState )
	{
		gameSoundWorldBuf = soundSystem->GetPlayingSoundWorld();
		gameSoundWorldBuf->Pause();
		soundSystem->SetPlayingSoundWorld( NULL );
	
		g_stopTime.SetBool( true );
	}
	else
	{
		soundSystem->SetPlayingSoundWorld( gameSoundWorldBuf );
		soundSystem->GetPlayingSoundWorld()->UnPause();
		
		g_stopTime.SetBool( false );
	}
}

void idGameLocal::SetMissionResult(EMissionResult result)
{
	m_MissionResult = result;
}

EMissionResult idGameLocal::GetMissionResult() const 
{
	return m_MissionResult;
}
