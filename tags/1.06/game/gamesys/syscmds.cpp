/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4797 $
 * $Date: 2011-04-17 02:34:47 -0400 (Sun, 17 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"

#pragma hdrstop

static bool init_version = FileVersionList("$Id: syscmds.cpp 4797 2011-04-17 06:34:47Z greebo $", init_version);

#include "../game_local.h"
#include "../ai/aas_local.h"
#include "../../DarkMod/sndPropLoader.h"
#include "../../DarkMod/Relations.h"
#include "../../DarkMod/Objectives/MissionData.h"
#include "../../DarkMod/Inventory/Inventory.h"
#include "../../DarkMod/Inventory/Item.h"
#include "../../DarkMod/TimerManager.h"
#include "../../DarkMod/AI/Conversation/ConversationSystem.h"
#include "../../DarkMod/Missions/MissionManager.h"
#include "../../DarkMod/Missions/ModInfo.h"

#include "typeinfo.h"

/*
==================
Cmd_TestSndIO_f
==================
*/
void Cmd_TestSndIO_f( const idCmdArgs &args ) 
{
	idStr inFN;

	if ( args.Argc() < 2 ) {
		gameLocal.Printf( "usage: dm_spr_testIO <file name without extension>\n" );
		goto Quit;
	}
	inFN = args.Args();

	if ( inFN.Length() == 0 ) 
	{
		goto Quit;
	}
	
//	New file IO not yet implemented
//	gameLocal.Printf( "Testing sound prop. IO for file %s\n", inFN.c_str() );
	gameLocal.Printf( "New soundprop file IO not yet implemented\n");
Quit:
	return;
}

/*
==================
Cmd_PrintAIRelations_f
==================
*/
void Cmd_PrintAIRelations_f( const idCmdArgs &args ) 
{
	gameLocal.m_RelationsManager->DebugPrintMat();
	return;
}

/**
 * greebo: This is a helper command, used by the restart.gui
 */
void Cmd_RestartGuiCmd_UpdateObjectives_f(const idCmdArgs &args) 
{
	idUserInterface* gui = uiManager->FindGui("guis/restart.gui", false, true, true);

	if (gui == NULL) 
	{
		gameLocal.Warning("Could not find restart.gui");
		return;
	}
	
	gameLocal.m_MissionData->UpdateGUIState(gui);
}

void Cmd_ListMissions_f(const idCmdArgs& args)
{
	gameLocal.Printf("%d missions registered:\n", gameLocal.m_MissionManager->GetNumMods());

	for (int i = 0; i < gameLocal.m_MissionManager->GetNumMods(); ++i)
	{
		CModInfoPtr missionInfo = gameLocal.m_MissionManager->GetModInfo(i);

		if (missionInfo == NULL) continue;

		gameLocal.Printf("%02d: %s = %s\n", i, missionInfo->modName.c_str(), missionInfo->displayName.c_str());
	}
}

void Cmd_SetMissionCompleted_f(const idCmdArgs& args)
{
	if (args.Argc() < 2)
	{
		gameLocal.Printf("Usage: tdm_set_mission_completed <missionName> [-c]\n");
		gameLocal.Printf("The -c argument is optional, can be used to clear the 'completed' flag, such that the mission isn't listed as completed in the mission menu.\n\n");
		gameLocal.Printf("Example: 'tdm_set_mission_completed heart'\n");
		return;
	}

	bool clearFlag = false;
	idStr missionName;

	for (int i = 1; i < args.Argc(); ++i)
	{
		idStr arg = args.Argv(i);

		if (arg == "-") 
		{
			if (++i < args.Argc() && idStr::Cmp(args.Argv(i), "c") == 0)
			{
				clearFlag = true;
			}
		}
		else
		{
			missionName = arg;
		}
	}

	if (missionName.IsEmpty())
	{
		return;
	}

	CModInfoPtr missionInfo = gameLocal.m_MissionManager->GetModInfo(missionName);

	if (missionInfo == NULL)
	{
		gameLocal.Printf("Mission %s not found.\n", missionName.c_str());
		return;
	}

	if (clearFlag)
	{
		// Remove completed flags
		missionInfo->RemoveKeyValuesMatchingPrefix("mission_completed_");
	}
	else
	{
		// Mark mission as completed on all difficulties
		for (int i = 0; i < DIFFICULTY_COUNT; ++i)
		{
			missionInfo->SetKeyValue(va("mission_completed_%d", i), "1");
		}
	}

	gameLocal.Printf("OK");
}

void Cmd_EndMission_f(const idCmdArgs& args)
{
	if (gameLocal.GameState() != GAMESTATE_ACTIVE)
	{
		gameLocal.Printf("No map running\n");
		return;
	}

	if (gameLocal.GetLocalPlayer() != NULL)
	{
		gameLocal.Printf("=== Triggering mission end ===\n");
		gameLocal.GetLocalPlayer()->PostEventMS(&EV_TriggerMissionEnd, 0);
	}
}

/*
==================
Cmd_AttachmentOffset_f
==================
*/
void Cmd_AttachmentOffset_f( const idCmdArgs &args )
{
	idVec3 offset(vec3_zero);
	
	if( args.Argc() != 6 )
	{
		gameLocal.Printf( "usage: tdm_attach_offset <attachment name> <attachment position> <x> <y> <z>\n" );
		return;
	}

	idEntity* lookedAt = gameLocal.PlayerTraceEntity();
	if (lookedAt == NULL || !(lookedAt->IsType(idActor::Type)) )
	{
		gameLocal.Printf( "tdm_attach_offset must be called when looking at an AI\n" );
		return;
	}

	idActor* actor = static_cast<idActor*>(lookedAt);

	idStr attName = args.Argv(1);
	idStr attPosName = args.Argv(2);

	int attIndex = actor->GetAttachmentIndex(attName);

	SAttachPosition* pos = actor->GetAttachPosition(attPosName);
	if (pos == NULL)
	{
		gameLocal.Printf( "tdm_attach_offset could not find position attPosName %s\n", attPosName.c_str() );
		return;
	}

	idStr joint = actor->GetAnimator()->GetJointName(pos->joint);

	// overwrite the attachment with our new attachment
	offset.x = atof(args.Argv( 3 ));
	offset.y = atof(args.Argv( 4 ));
	offset.z = atof(args.Argv( 5 ));

	actor->ReAttachToCoords( attName, joint, offset, pos->angleOffset );
}

/*
==================
Cmd_AttachmentRot_f
==================
*/
void Cmd_AttachmentRot_f( const idCmdArgs &args )
{
	idVec3 offset(vec3_zero);
	
	if( args.Argc() != 6 )
	{
		gameLocal.Printf( "usage: tdm_attach_rot <attachment name> <attachment position> <pitch> <yaw> <roll>\n" );
		return;
	}

	idEntity* lookedAt = gameLocal.PlayerTraceEntity();
	if (lookedAt == NULL || !(lookedAt->IsType(idActor::Type)) )
	{
		gameLocal.Printf( "tdm_attach_rot must be called when looking at an AI\n" );
		return;
	}

	idActor* actor = static_cast<idActor*>(lookedAt);

	idStr attName = args.Argv(1);
	idStr attPosName = args.Argv(2);

	int attIndex = actor->GetAttachmentIndex(attName);

	SAttachPosition* pos = actor->GetAttachPosition(attPosName);
	if (pos == NULL)
	{
		gameLocal.Printf( "tdm_attach_rot could not find position attPosName %s\n", attPosName.c_str() );
		return;
	}

	idStr joint = actor->GetAnimator()->GetJointName(pos->joint);
	
	// overwrite the attachment rotation with our new one
	idAngles	angles;
	angles.pitch = atof(args.Argv( 3 ));
	angles.yaw = atof(args.Argv( 4 ));
	angles.roll = atof(args.Argv( 5 ));

	actor->ReAttachToCoords( attName, joint, pos->originOffset, angles );
}

/*
==================
Cmd_InventoryHotkey_f
==================
*/
void Cmd_InventoryHotkey_f( const idCmdArgs &args )
{
	if ( 0 > args.Argc() || args.Argc() > 2 ) {
		gameLocal.Printf( "Usage: %s [item]\n", args.Argv(0) );
		return;
	}

	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player == NULL ) {
		gameLocal.Printf( "%s: No player exists.\n", args.Argv(0) );
		return;
	}

	if( player->GetImmobilization() & EIM_ITEM_SELECT )
		return;

	CInventoryCursorPtr cursor = player->InventoryCursor();
	CInventory* inventory = cursor->Inventory();
	
	if (inventory == NULL)
	{
		gameLocal.Printf( "%s: Could not find player inventory.\n", args.Argv(0) );
		return;
	}

	if( args.Argc() == 2)
	{
		idStr itemName = args.Argv(1);

		player->SelectInventoryItem(itemName);
	}
	else if (args.Argc() == 1)
	{
		// greebo: Clear the item if no argument is set
		player->SelectInventoryItem("");
	}
}

/*
==================
Cmd_InventoryUse_f
==================
*/
void Cmd_InventoryUse_f( const idCmdArgs &args )
{
	if ( 0 > args.Argc() || args.Argc() > 2 ) {
		gameLocal.Printf( "Usage: %s [item]\n", args.Argv(0) );
		return;
	}

	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player == NULL ) {
		gameLocal.Printf( "%s: No player exists.\n", args.Argv(0) );
		return;
	}

	if( player->GetImmobilization() & EIM_ITEM_USE )
		return;

	CInventoryCursorPtr cursor = player->InventoryCursor();
	CInventory* inventory = cursor->Inventory();
	
	if (inventory == NULL)
	{
		gameLocal.Printf( "%s: Could not find player inventory.\n", args.Argv(0) );
		return;
	}

	if( args.Argc() == 2)
	{
		idStr itemName = args.Argv(1);

		// Try to lookup the item in the inventory
		CInventoryItemPtr item = inventory->GetItem(itemName);

		if (item != NULL)
		{
			// Item found, set the cursor to it
			player->UseInventoryItem(EPressed, item, 0, false); // false => no frob action
		}
		else
		{
			gameLocal.Printf( "%s: Could not find item in player inventory: %s\n", args.Argv(0), args.Argv(1) );
		}
	}
}

/*
==================
Cmd_InventoryCycleMaps_f
==================
*/
void Cmd_InventoryCycleMaps_f( const idCmdArgs &args )
{
	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player == NULL ) {
		gameLocal.Printf( "%s: No player exists.\n", args.Argv(0) );
		return;
	}

	if( (player->GetImmobilization() & EIM_ITEM_SELECT) || (player->GetImmobilization() & EIM_ITEM_USE) )
		return;

	// Pass the call to the specialised method
	player->NextInventoryMap();
}

/*
==================
Cmd_InventoryCycleGroup_f
==================
*/
void Cmd_InventoryCycleGroup_f( const idCmdArgs &args )
{
	if ( 0 > args.Argc() || args.Argc() > 2 )
	{
		gameLocal.Printf( "Usage: %s [item]\n", args.Argv(0) );
		return;
	}

	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player == NULL )
	{
		gameLocal.Printf( "%s: No player exists.\n", args.Argv(0) );
		return;
	}

	if( (player->GetImmobilization() & EIM_ITEM_SELECT) || (player->GetImmobilization() & EIM_ITEM_USE) )
	{
		return;
	}

	if( args.Argc() == 2)
	{
		idStr categoryName = args.Argv(1);

		// Pass the call to the specialised method
		player->CycleInventoryGroup(categoryName);
	}
}

/*
==================
Cmd_GetFloatArg
==================
*/
float Cmd_GetFloatArg( const idCmdArgs &args, int &argNum ) {
	const char *value;

	value = args.Argv( argNum++ );
	return atof( value );
}

/*
===================
Cmd_EntityList_f
===================
*/
void Cmd_EntityList_f( const idCmdArgs &args ) {
	int			e;
	idEntity	*check;
	int			count;
	size_t		size;
	idStr		match;

	if ( args.Argc() > 1 ) {
		match = args.Args();
		match.Replace( " ", "" );
	} else {
		match = "";
	}

	count = 0;
	size = 0;

	gameLocal.Printf( "%-4s  %-20s %-20s %s\n", " Num", "EntityDef", "Class", "Name" );
	gameLocal.Printf( "--------------------------------------------------------------------\n" );
	for( e = 0; e < MAX_GENTITIES; e++ ) {
		check = gameLocal.entities[ e ];

		if ( !check ) {
			continue;
		}

		if ( !check->name.Filter( match, true ) ) {
			continue;
		}

		gameLocal.Printf( "%4i: %-20s %-20s %s\n", e,
			check->GetEntityDefName(), check->GetClassname(), check->name.c_str() );

		count++;
		size += check->spawnArgs.Allocated();
	}

	gameLocal.Printf( "...%d entities\n...%d bytes of spawnargs\n", count, size );
}

/*
===================
Cmd_ActiveEntityList_f
===================
*/
void Cmd_ActiveEntityList_f( const idCmdArgs &args ) {
	idEntity	*check;
	int			count;

	count = 0;

	gameLocal.Printf( "%-4s  %-20s %-20s %s\n", " Num", "EntityDef", "Class", "Name" );
	gameLocal.Printf( "--------------------------------------------------------------------\n" );
	for( check = gameLocal.activeEntities.Next(); check != NULL; check = check->activeNode.Next() ) {
		char	dormant = check->fl.isDormant ? '-' : ' ';
		gameLocal.Printf( "%4i:%c%-20s %-20s %s\n", check->entityNumber, dormant, check->GetEntityDefName(), check->GetClassname(), check->name.c_str() );
		count++;
	}

	gameLocal.Printf( "...%d active entities\n", count );
}

/*
===================
Cmd_ListSpawnArgs_f
===================
*/
void Cmd_ListSpawnArgs_f( const idCmdArgs &args ) {
	int i;
	idEntity *ent;

	ent = gameLocal.FindEntity( args.Argv( 1 ) );
	if ( !ent ) {
		gameLocal.Printf( "entity not found\n" );
		return;
	}

	for ( i = 0; i < ent->spawnArgs.GetNumKeyVals(); i++ ) {
		const idKeyValue *kv = ent->spawnArgs.GetKeyVal( i );
		gameLocal.Printf( "\"%s\"  "S_COLOR_WHITE"\"%s\"\n", kv->GetKey().c_str(), kv->GetValue().c_str() );
	}
}

/*
===================
Cmd_ReloadScript_f
===================
*/
void Cmd_ReloadScript_f( const idCmdArgs &args ) {
	// shutdown the map because entities may point to script objects
	gameLocal.MapShutdown();

	// recompile the scripts
	gameLocal.program.Startup( SCRIPT_DEFAULT );

	// error out so that the user can rerun the scripts
	gameLocal.Error( "Exiting map to reload scripts" );
}

/*
===================
Cmd_Script_f
===================
*/
void Cmd_Script_f( const idCmdArgs &args ) {
	const char *	script;
	idStr			text;
	idStr			funcname;
	static int		funccount = 0;
	idThread *		thread;
	const function_t *func;
	idEntity		*ent;

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	sprintf( funcname, "ConsoleFunction_%d", funccount++ );

	script = args.Args();
	sprintf( text, "void %s() {%s;}\n", funcname.c_str(), script );
	if ( gameLocal.program.CompileText( "console", text, true ) ) {
		func = gameLocal.program.FindFunction( funcname );
		if ( func ) {
			// set all the entity names in case the user named one in the script that wasn't referenced in the default script
			for( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
				gameLocal.program.SetEntity( ent->name, ent );
			}

			thread = new idThread( func );
			thread->Start();
		}
	}
}

/*
==================
KillEntities

Kills all the entities of the given class in a level.
==================
*/
void KillEntities( const idCmdArgs &args, const idTypeInfo &superClass ) {
	idEntity	*ent;
	idStrList	ignore;
	const char *name;
	int			i;

	if ( !gameLocal.GetLocalPlayer() || !gameLocal.CheatsOk( false ) ) {
		return;
	}

	for( i = 1; i < args.Argc(); i++ ) {
		name = args.Argv( i );
		ignore.Append( name );
	}

	for( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		if ( ent->IsType( superClass ) ) {
			for( i = 0; i < ignore.Num(); i++ ) {
				if ( ignore[ i ] == ent->name ) {
					break;
				}
			}

			if ( i >= ignore.Num() ) {
				ent->PostEventMS( &EV_Remove, 0 );
			}
		}
	}
}

/*
==================
Cmd_KillMonsters_f

Kills all the monsters in a level.
==================
*/
void Cmd_KillMonsters_f( const idCmdArgs &args ) {
	KillEntities( args, idAI::Type );

	// kill any projectiles as well since they have pointers to the monster that created them
	KillEntities( args, idProjectile::Type );
}

/*
==================
Cmd_KillMovables_f

Kills all the moveables in a level.
==================
*/
void Cmd_KillMovables_f( const idCmdArgs &args ) {
	if ( !gameLocal.GetLocalPlayer() || !gameLocal.CheatsOk( false ) ) {
		return;
	}
	KillEntities( args, idMoveable::Type );
}

/*
==================
Cmd_KillRagdolls_f

Kills all the ragdolls in a level.
==================
*/
void Cmd_KillRagdolls_f( const idCmdArgs &args ) {
	if ( !gameLocal.GetLocalPlayer() || !gameLocal.CheatsOk( false ) ) {
		return;
	}
	KillEntities( args, idAFEntity_Generic::Type );
	KillEntities( args, idAFEntity_WithAttachedHead::Type );
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f( const idCmdArgs &args ) {
	const char *name;
	int			i;
	bool		give_all;
	idPlayer	*player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	name = args.Argv( 1 );

	if ( idStr::Icmp( name, "all" ) == 0 ) {
		give_all = true;
	} else {
		give_all = false;
	}

	if ( give_all || ( idStr::Cmpn( name, "weapon", 6 ) == 0 ) ) {
		if ( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) {
			gameLocal.world->spawnArgs.SetBool( "no_Weapons", false );
			for( i = 0; i < gameLocal.numClients; i++ ) {
				if ( gameLocal.entities[ i ] ) {
					gameLocal.entities[ i ]->PostEventSec( &EV_Player_SelectWeapon, 0.5f, gameLocal.entities[ i ]->spawnArgs.GetString( "def_weapon1" ) );
				}
			}
		}
	}

	if ( ( idStr::Cmpn( name, "weapon_", 7 ) == 0 ) || ( idStr::Cmpn( name, "item_", 5 ) == 0 ) || ( idStr::Cmpn( name, "ammo_", 5 ) == 0 ) ) {
		player->GiveItem( name );
		return;
	}

	if ( give_all || idStr::Icmp( name, "health" ) == 0 )	{
		player->health = player->maxHealth;
		if ( !give_all ) {
			return;
		}
	}

	if ( idStr::Icmp( name, "berserk" ) == 0 ) {
		player->GivePowerUp( BERSERK, SEC2MS( 30.0f ) );
		return;
	}

	if ( idStr::Icmp( name, "invis" ) == 0 ) {
		player->GivePowerUp( INVISIBILITY, SEC2MS( 30.0f ) );
		return;
	}

	if ( !give_all && !player->Give( args.Argv(1), args.Argv(2) ) ) {
		gameLocal.Printf( "unknown item\n" );
	}
}

/*
==================
Cmd_CenterView_f

Centers the players pitch
==================
*/
void Cmd_CenterView_f( const idCmdArgs &args ) {
	idPlayer	*player;
	idAngles	ang;

	player = gameLocal.GetLocalPlayer();
	if ( !player ) {
		return;
	}

	ang = player->viewAngles;
	ang.pitch = 0.0f;
	player->SetViewAngles( ang );
}

/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f( const idCmdArgs &args ) {
	idPlayer	*player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( player->godmode ) {
		player->godmode = false;
		gameLocal.Printf( "godmode OFF\n" );
	} else {
		player->godmode = true;
		gameLocal.Printf( "godmode ON\n" );
	}

}

/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( const idCmdArgs &args ) {
	idPlayer	*player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( player->fl.notarget ) {
		player->fl.notarget = false;
		gameLocal.Printf( "notarget OFF\n" );
	} else {
		player->fl.notarget = true;
		gameLocal.Printf( "notarget ON\n" );
	}

}

/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( const idCmdArgs &args ) {
	idPlayer	*player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( player->noclip ) {
		player->noclip = false;
		gameLocal.Printf( "noclip OFF\n" );
	} else {
		player->noclip = true;
		gameLocal.Printf( "noclip ON\n" );
	}
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( const idCmdArgs &args ) {
	idPlayer	*player;

	if ( gameLocal.isMultiplayer ) {
		if ( gameLocal.isClient ) {
			idBitMsg	outMsg;
			byte		msgBuf[ MAX_GAME_MESSAGE_SIZE ];
			outMsg.Init( msgBuf, sizeof( msgBuf ) );
			outMsg.WriteByte( GAME_RELIABLE_MESSAGE_KILL );
			networkSystem->ClientSendReliableMessage( outMsg );
		} else {
			player = gameLocal.GetClientByCmdArgs( args );
			if ( !player ) {
				common->Printf( "kill <client nickname> or kill <client index>\n" );
				return;
			}
			player->Kill( false, false );
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "say killed client %d '%s^0'\n", player->entityNumber, gameLocal.userInfo[ player->entityNumber ].GetString( "ui_name" ) ) );
		}
	} else {
		player = gameLocal.GetLocalPlayer();
		if ( !player ) {
			return;
		}
		player->Kill( false, false );
	}
}

/*
=================
Cmd_PlayerModel_f
=================
*/
void Cmd_PlayerModel_f( const idCmdArgs &args ) {
	idPlayer	*player;
	const char *name;
	idVec3		pos;
	idAngles	ang;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc() < 2 ) {
		gameLocal.Printf( "usage: playerModel <modelname>\n" );
		return;
	}

	name = args.Argv( 1 );
	player->spawnArgs.Set( "model", name );

	pos = player->GetPhysics()->GetOrigin();
	ang = player->viewAngles;
	player->SpawnToPoint( pos, ang );
}

/*
==================
Cmd_Say
==================
*/
static void Cmd_Say( bool team, const idCmdArgs &args ) {
	const char *name;
	idStr text;
	const char *cmd = team ? "sayTeam" : "say" ;

	if ( !gameLocal.isMultiplayer ) {
		gameLocal.Printf( "%s can only be used in a multiplayer game\n", cmd );
		return;
	}

	if ( args.Argc() < 2 ) {
		gameLocal.Printf( "usage: %s <text>\n", cmd );
		return;
	}

	text = args.Args();
	if ( text.Length() == 0 ) {
		return;
	}

	if ( text[ text.Length() - 1 ] == '\n' ) {
		text[ text.Length() - 1 ] = '\0';
	}
	name = "player";

	idPlayer *	player;

	// here we need to special case a listen server to use the real client name instead of "server"
	// "server" will only appear on a dedicated server
	if ( gameLocal.isClient || cvarSystem->GetCVarInteger( "net_serverDedicated" ) == 0 ) {
		player = gameLocal.localClientNum >= 0 ? static_cast<idPlayer *>( gameLocal.entities[ gameLocal.localClientNum ] ) : NULL;
		if ( player ) {
			name = player->GetUserInfo()->GetString( "ui_name", "player" );
		}
	} else {
		name = "server";
	}

	if ( gameLocal.isClient ) {
		idBitMsg	outMsg;
		byte		msgBuf[ 256 ];
		outMsg.Init( msgBuf, sizeof( msgBuf ) );
		outMsg.WriteByte( team ? GAME_RELIABLE_MESSAGE_TCHAT : GAME_RELIABLE_MESSAGE_CHAT );
		outMsg.WriteString( name );
		outMsg.WriteString( text, -1, false );
		networkSystem->ClientSendReliableMessage( outMsg );
	} else {
		gameLocal.mpGame.ProcessChatMessage( gameLocal.localClientNum, team, name, text, NULL );
	}
}

/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( const idCmdArgs &args ) {
	Cmd_Say( false, args );
}

/*
==================
Cmd_SayTeam_f
==================
*/
static void Cmd_SayTeam_f( const idCmdArgs &args ) {
	Cmd_Say( true, args );
}

/*
==================
Cmd_AddChatLine_f
==================
*/
static void Cmd_AddChatLine_f( const idCmdArgs &args ) {
	gameLocal.mpGame.AddChatLine( args.Argv( 1 ) );
}

/*
==================
Cmd_Kick_f
==================
*/
static void Cmd_Kick_f( const idCmdArgs &args ) {
	idPlayer *player;

	if ( !gameLocal.isMultiplayer ) {
		gameLocal.Printf( "kick can only be used in a multiplayer game\n" );
		return;
	}

	if ( gameLocal.isClient ) {
		gameLocal.Printf( "You have no such power. This is a server command\n" );
		return;
	}

	player = gameLocal.GetClientByCmdArgs( args );
	if ( !player ) {
		gameLocal.Printf( "usage: kick <client nickname> or kick <client index>\n" );
		return;
	}
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "say kicking out client %d '%s^0'\n", player->entityNumber, gameLocal.userInfo[ player->entityNumber ].GetString( "ui_name" ) ) );
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "kick %d\n", player->entityNumber ) );
}

/*
==================
Cmd_GetViewpos_f
==================
*/
void Cmd_GetViewpos_f( const idCmdArgs &args ) {
	idPlayer	*player;
	idVec3		origin;
	idMat3		axis;

	player = gameLocal.GetLocalPlayer();
	if ( !player ) {
		return;
	}

	const renderView_t *view = player->GetRenderView();
	if ( view ) {
		gameLocal.Printf( "(%s) %.1f\n", view->vieworg.ToString(), view->viewaxis[0].ToYaw() );
	} else {
		player->GetViewPos( origin, axis );
		gameLocal.Printf( "(%s) %.1f\n", origin.ToString(), axis[0].ToYaw() );
	}
}

/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( const idCmdArgs &args ) {
	idVec3		origin;
	idAngles	angles;
	int			i;
	idPlayer	*player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( ( args.Argc() != 4 ) && ( args.Argc() != 5 ) ) {
		gameLocal.Printf( "usage: setviewpos <x> <y> <z> <yaw>\n" );
		return;
	}

	angles.Zero();
	if ( args.Argc() == 5 ) {
		angles.yaw = atof( args.Argv( 4 ) );
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		origin[i] = atof( args.Argv( i + 1 ) );
	}
	origin.z -= pm_normalviewheight.GetFloat() - 0.25f;

	player->Teleport( origin, angles, NULL );
}

/*
=================
Cmd_Teleport_f
=================
*/
void Cmd_Teleport_f( const idCmdArgs &args ) {
	idVec3		origin;
	idAngles	angles;
	idPlayer	*player;
	idEntity	*ent;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc() != 2 ) {
		gameLocal.Printf( "usage: teleport <name of entity to teleport to>\n" );
		return;
	}

	ent = gameLocal.FindEntity( args.Argv( 1 ) );
	if ( !ent ) {
		gameLocal.Printf( "entity not found\n" );
		return;
	}

	angles.Zero();
	angles.yaw = ent->GetPhysics()->GetAxis()[ 0 ].ToYaw();
	origin = ent->GetPhysics()->GetOrigin();

	player->Teleport( origin, angles, ent );
}

/*
=================
Cmd_Trigger_f
=================
*/
void Cmd_Trigger_f( const idCmdArgs &args ) {
	idVec3		origin;
	idAngles	angles;
	idPlayer	*player;
	idEntity	*ent;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc() != 2 ) {
		gameLocal.Printf( "usage: trigger <name of entity to trigger>\n" );
		return;
	}

	ent = gameLocal.FindEntity( args.Argv( 1 ) );
	if ( !ent ) {
		gameLocal.Printf( "entity not found\n" );
		return;
	}

	ent->Signal( SIG_TRIGGER );
	ent->ProcessEvent( &EV_Activate, player );
	ent->TriggerGuis();
}

/*
===================
Cmd_Spawn_f
===================
*/
void Cmd_Spawn_f( const idCmdArgs &args ) {
	const char *key, *value;
	int			i;
	float		yaw;
	idVec3		org;
	idPlayer	*player;
	idDict		dict;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk( false ) ) {
		return;
	}

	if ( args.Argc() & 1 ) {	// must always have an even number of arguments
		gameLocal.Printf( "usage: spawn classname [key/value pairs]\n" );
		return;
	}

	yaw = player->viewAngles.yaw;

	value = args.Argv( 1 );
	dict.Set( "classname", value );
	dict.Set( "angle", va( "%f", yaw + 180 ) );

	org = player->GetPhysics()->GetOrigin() + idAngles( 0, yaw, 0 ).ToForward() * 80 + idVec3( 0, 0, 1 );
	dict.Set( "origin", org.ToString() );

	for( i = 2; i < args.Argc() - 1; i += 2 ) {

		key = args.Argv( i );
		value = args.Argv( i + 1 );

		dict.Set( key, value );
	}

	gameLocal.SpawnEntityDef( dict );
}

/*
==================
Cmd_Damage_f

Damages the specified entity
==================
*/
void Cmd_Damage_f( const idCmdArgs &args ) {
	if ( !gameLocal.GetLocalPlayer() || !gameLocal.CheatsOk( false ) ) {
		return;
	}
	if ( args.Argc() != 3 ) {
		gameLocal.Printf( "usage: damage <name of entity to damage> <damage>\n" );
		return;
	}

	idEntity *ent = gameLocal.FindEntity( args.Argv( 1 ) );
	if ( !ent ) {
		gameLocal.Printf( "entity not found\n" );
		return;
	}

	ent->Damage( gameLocal.world, gameLocal.world, idVec3( 0, 0, 1 ), "damage_moverCrush", atoi( args.Argv( 2 ) ), INVALID_JOINT );
}


/*
==================
Cmd_Remove_f

Removes the specified entity
==================
*/
void Cmd_Remove_f( const idCmdArgs &args ) {
	if ( !gameLocal.GetLocalPlayer() || !gameLocal.CheatsOk( false ) ) {
		return;
	}
	if ( args.Argc() != 2 ) {
		gameLocal.Printf( "usage: remove <name of entity to remove>\n" );
		return;
	}

	idEntity *ent = gameLocal.FindEntity( args.Argv( 1 ) );
	if ( !ent ) {
		gameLocal.Printf( "entity not found\n" );
		return;
	}

	delete ent;
}

/*
===================
Cmd_TestLight_f
===================
*/
void Cmd_TestLight_f( const idCmdArgs &args ) {
	int			i;
	idStr		filename;
	const char *key, *value, *name(NULL);
	idPlayer *	player;
	idDict		dict;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk( false ) ) {
		return;
	}

	renderView_t	*rv = player->GetRenderView();

	float fov = tan( idMath::M_DEG2RAD * rv->fov_x / 2 );


	dict.SetMatrix( "rotation", mat3_default );
	dict.SetVector( "origin", rv->vieworg );
	dict.SetVector( "light_target", rv->viewaxis[0] );
	dict.SetVector( "light_right", rv->viewaxis[1] * -fov );
	dict.SetVector( "light_up", rv->viewaxis[2] * fov );
	dict.SetVector( "light_start", rv->viewaxis[0] * 16 );
	dict.SetVector( "light_end", rv->viewaxis[0] * 1000 );

	if ( args.Argc() >= 2 ) {
		value = args.Argv( 1 );
		filename = args.Argv(1);
		filename.DefaultFileExtension( ".tga" );
		dict.Set( "texture", filename );
	}

	dict.Set( "classname", "light" );
	for( i = 2; i < args.Argc() - 1; i += 2 ) {

		key = args.Argv( i );
		value = args.Argv( i + 1 );

		dict.Set( key, value );
	}

	for ( i = 0; i < MAX_GENTITIES; i++ ) {
		name = va( "spawned_light_%d", i );		// not just light_, or it might pick up a prelight shadow
		if ( !gameLocal.FindEntity( name ) ) {
			break;
		}
	}
	dict.Set( "name", name );

	gameLocal.SpawnEntityDef( dict );

	gameLocal.Printf( "Created new light\n");
}

/*
===================
Cmd_TestPointLight_f
===================
*/
void Cmd_TestPointLight_f( const idCmdArgs &args ) {
	const char *key, *value, *name(NULL);
	int			i;
	idPlayer	*player;
	idDict		dict;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk( false ) ) {
		return;
	}

	dict.SetVector("origin", player->GetRenderView()->vieworg);

	if ( args.Argc() >= 2 ) {
		value = args.Argv( 1 );
		dict.Set("light", value);
	} else {
		dict.Set("light", "300");
	}

	dict.Set( "classname", "light" );
	for( i = 2; i < args.Argc() - 1; i += 2 ) {

		key = args.Argv( i );
		value = args.Argv( i + 1 );

		dict.Set( key, value );
	}

	for ( i = 0; i < MAX_GENTITIES; i++ ) {
		name = va( "light_%d", i );
		if ( !gameLocal.FindEntity( name ) ) {
			break;
		}
	}
	dict.Set( "name", name );

	gameLocal.SpawnEntityDef( dict );

	gameLocal.Printf( "Created new point light\n");
}

/*
==================
Cmd_PopLight_f
==================
*/
void Cmd_PopLight_f( const idCmdArgs &args ) {
	idEntity	*ent;
	idMapEntity *mapEnt;
	idMapFile	*mapFile = gameLocal.GetLevelMap();
	idLight		*lastLight;
	int			last;

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	bool removeFromMap = ( args.Argc() > 1 );

	lastLight = NULL;
	last = -1;
	for( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		if ( !ent->IsType( idLight::Type ) ) {
			continue;
		}

		if ( gameLocal.spawnIds[ ent->entityNumber ] > last ) {
			last = gameLocal.spawnIds[ ent->entityNumber ];
			lastLight = static_cast<idLight*>( ent );
		}
	}

	if ( lastLight ) {
		// find map file entity
		mapEnt = mapFile->FindEntity( lastLight->name );

		if ( removeFromMap && mapEnt ) {
			mapFile->RemoveEntity( mapEnt );
		}
		gameLocal.Printf( "Removing light %i\n", lastLight->GetLightDefHandle() );
		delete lastLight;
	} else {
		gameLocal.Printf( "No lights to clear.\n" );
	}

}

/*
====================
Cmd_ClearLights_f
====================
*/
void Cmd_ClearLights_f( const idCmdArgs &args ) {
	idEntity *ent;
	idEntity *next;
	idLight *light;
	idMapEntity *mapEnt;
	idMapFile *mapFile = gameLocal.GetLevelMap();

	bool removeFromMap = ( args.Argc() > 1 );

	gameLocal.Printf( "Clearing all lights.\n" );
	for( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = next ) {
		next = ent->spawnNode.Next();
		if ( !ent->IsType( idLight::Type ) ) {
			continue;
		}

		light = static_cast<idLight*>( ent );
		mapEnt = mapFile->FindEntity( light->name );

		if ( removeFromMap && mapEnt ) {
			mapFile->RemoveEntity( mapEnt );
		}

		delete light;
	}
}

/*
==================
Cmd_TestFx_f
==================
*/
void Cmd_TestFx_f( const idCmdArgs &args ) {
	idVec3		offset;
	const char *name;
	idPlayer *	player;
	idDict		dict;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	// delete the testModel if active
	if ( gameLocal.testFx ) {
		delete gameLocal.testFx;
		gameLocal.testFx = NULL;
	}

	if ( args.Argc() < 2 ) {
		return;
	}

	name = args.Argv( 1 );

	offset = player->GetPhysics()->GetOrigin() + player->viewAngles.ToForward() * 100.0f;

	dict.Set( "origin", offset.ToString() );
	dict.Set( "test", "1");
	dict.Set( "fx", name );
	gameLocal.testFx = ( idEntityFx * )gameLocal.SpawnEntityType( idEntityFx::Type, &dict );
}

#define MAX_DEBUGLINES	128

typedef struct {
	bool used;
	idVec3 start, end;
	int color;
	bool blink;
	bool arrow;
} gameDebugLine_t;

gameDebugLine_t debugLines[MAX_DEBUGLINES];

/*
==================
Cmd_AddDebugLine_f
==================
*/
static void Cmd_AddDebugLine_f( const idCmdArgs &args ) {
	int i, argNum;
	const char *value;

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc () < 7 ) {
		gameLocal.Printf( "usage: addline <x y z> <x y z> <color>\n" );
		return;
	}
	for ( i = 0; i < MAX_DEBUGLINES; i++ ) {
		if ( !debugLines[i].used ) {
			break;
		}
	}
	if ( i >= MAX_DEBUGLINES ) {
		gameLocal.Printf( "no free debug lines\n" );
		return;
	}
	value = args.Argv( 0 );
	if ( !idStr::Icmp( value, "addarrow" ) ) {
		debugLines[i].arrow = true;
	} else {
		debugLines[i].arrow = false;
	}
	debugLines[i].used = true;
	debugLines[i].blink = false;
	argNum = 1;
	debugLines[i].start.x = Cmd_GetFloatArg( args, argNum );
	debugLines[i].start.y = Cmd_GetFloatArg( args, argNum );
	debugLines[i].start.z = Cmd_GetFloatArg( args, argNum );
	debugLines[i].end.x = Cmd_GetFloatArg( args, argNum );
	debugLines[i].end.y = Cmd_GetFloatArg( args, argNum );
	debugLines[i].end.z = Cmd_GetFloatArg( args, argNum );
	debugLines[i].color = static_cast<int>(Cmd_GetFloatArg( args, argNum ));
}

/*
==================
Cmd_RemoveDebugLine_f
==================
*/
static void Cmd_RemoveDebugLine_f( const idCmdArgs &args ) {
	int i, num;
	const char *value;

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc () < 2 ) {
		gameLocal.Printf( "usage: removeline <num>\n" );
		return;
	}
	value = args.Argv( 1 );
	num = atoi(value);
	for ( i = 0; i < MAX_DEBUGLINES; i++ ) {
		if ( debugLines[i].used ) {
			if ( --num < 0 ) {
				break;
			}
		}
	}
	if ( i >= MAX_DEBUGLINES ) {
		gameLocal.Printf( "line not found\n" );
		return;
	}
	debugLines[i].used = false;
}

/*
==================
Cmd_BlinkDebugLine_f
==================
*/
static void Cmd_BlinkDebugLine_f( const idCmdArgs &args ) {
	int i, num;
	const char *value;

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc () < 2 ) {
		gameLocal.Printf( "usage: blinkline <num>\n" );
		return;
	}
	value = args.Argv( 1 );
	num = atoi( value );
	for ( i = 0; i < MAX_DEBUGLINES; i++ ) {
		if ( debugLines[i].used ) {
			if ( --num < 0 ) {
				break;
			}
		}
	}
	if ( i >= MAX_DEBUGLINES ) {
		gameLocal.Printf( "line not found\n" );
		return;
	}
	debugLines[i].blink = !debugLines[i].blink;
}

/*
==================
PrintFloat
==================
*/
static void PrintFloat( float f ) {
	char buf[128];
	int i;

	for (i = sprintf( buf, "%3.2f", f ); i < 7; i++) {
		buf[i] = ' ';
	}
	buf[i] = '\0';
	gameLocal.Printf( buf );
}

/*
==================
Cmd_ListDebugLines_f
==================
*/
static void Cmd_ListDebugLines_f( const idCmdArgs &args ) {
	int i, num;

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	num = 0;
	gameLocal.Printf( "line num: x1     y1     z1     x2     y2     z2     c  b  a\n" );
	for ( i = 0; i < MAX_DEBUGLINES; i++ ) {
		if ( debugLines[i].used ) {
			gameLocal.Printf( "line %3d: ", num );
			PrintFloat( debugLines[i].start.x );
			PrintFloat( debugLines[i].start.y );
			PrintFloat( debugLines[i].start.z );
			PrintFloat( debugLines[i].end.x );
			PrintFloat( debugLines[i].end.y );
			PrintFloat( debugLines[i].end.z );
			gameLocal.Printf( "%d  %d  %d\n", debugLines[i].color, debugLines[i].blink, debugLines[i].arrow );
			num++;
		}
	}
	if ( !num ) {
		gameLocal.Printf( "no debug lines\n" );
	}
}

/*
==================
D_DrawDebugLines
==================
*/
void D_DrawDebugLines( void ) {
	int i;
	idVec3 forward, right, up, p1, p2;
	idVec4 color;
	float l;

	for ( i = 0; i < MAX_DEBUGLINES; i++ ) {
		if ( debugLines[i].used ) {
			if ( !debugLines[i].blink || (gameLocal.time & (1<<9)) ) {
				color = idVec4( debugLines[i].color&1, (debugLines[i].color>>1)&1, (debugLines[i].color>>2)&1, 1 );
				gameRenderWorld->DebugLine( color, debugLines[i].start, debugLines[i].end );
				//
				if ( debugLines[i].arrow ) {
					// draw a nice arrow
					forward = debugLines[i].end - debugLines[i].start;
					l = forward.Normalize() * 0.2f;
					forward.NormalVectors( right, up);

					if ( l > 3.0f ) {
						l = 3.0f;
					}
					p1 = debugLines[i].end - l * forward + (l * 0.4f) * right;
					p2 = debugLines[i].end - l * forward - (l * 0.4f) * right;
					gameRenderWorld->DebugLine( color, debugLines[i].end, p1 );
					gameRenderWorld->DebugLine( color, debugLines[i].end, p2 );
					gameRenderWorld->DebugLine( color, p1, p2 );
				}
			}
		}
	}
}

/*
==================
Cmd_ListCollisionModels_f
==================
*/
static void Cmd_ListCollisionModels_f( const idCmdArgs &args ) {
	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	collisionModelManager->ListModels();
}

/*
==================
Cmd_CollisionModelInfo_f
==================
*/
static void Cmd_CollisionModelInfo_f( const idCmdArgs &args ) {
	const char *value;

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc () < 2 ) {
		gameLocal.Printf( "usage: collisionModelInfo <modelNum>\n"
					"use 'all' instead of the model number for accumulated info\n" );
		return;
	}

	value = args.Argv( 1 );
	if ( !idStr::Icmp( value, "all" ) ) {
		collisionModelManager->ModelInfo( -1 );
	} else {
		collisionModelManager->ModelInfo( atoi(value) );
	}
}

/*
==================
Cmd_ExportModels_f
==================
*/
static void Cmd_ExportModels_f( const idCmdArgs &args ) {
	idModelExport	exporter;
	idStr			name;

	// don't allow exporting models when cheats are disabled,
	// but if we're not in the game, it's ok
	if ( gameLocal.GetLocalPlayer() && !gameLocal.CheatsOk( false ) ) {
		return;
	}

	if ( args.Argc() < 2 ) {
		exporter.ExportModels( "def", ".def" );
	} else {
		name = args.Argv( 1 );
		name = "def/" + name;
		name.DefaultFileExtension( ".def" );
		exporter.ExportDefFile( name );
	}
}

/*
==================
Cmd_ReexportModels_f
==================
*/
static void Cmd_ReexportModels_f( const idCmdArgs &args ) {
	idModelExport	exporter;
	idStr			name;

	// don't allow exporting models when cheats are disabled,
	// but if we're not in the game, it's ok
	if ( gameLocal.GetLocalPlayer() && !gameLocal.CheatsOk( false ) ) {
		return;
	}

	idAnimManager::forceExport = true;
	if ( args.Argc() < 2 ) {
		exporter.ExportModels( "def", ".def" );
	} else {
		name = args.Argv( 1 );
		name = "def/" + name;
		name.DefaultFileExtension( ".def" );
		exporter.ExportDefFile( name );
	}
	idAnimManager::forceExport = false;
}

/*
==================
Cmd_ReloadAnims_f
==================
*/
static void Cmd_ReloadAnims_f( const idCmdArgs &args ) {
	// don't allow reloading anims when cheats are disabled,
	// but if we're not in the game, it's ok
	if ( gameLocal.GetLocalPlayer() && !gameLocal.CheatsOk( false ) ) {
		return;
	}

	animationLib.ReloadAnims();
}

/*
==================
Cmd_ListAnims_f
==================
*/
static void Cmd_ListAnims_f( const idCmdArgs &args ) {
	idEntity *		ent;
	int				num;
	size_t			size;
	size_t			alloced;
	idAnimator *	animator;
	const char *	classname;
	const idDict *	dict;
	int				i;

	if ( args.Argc() > 1 ) {
		idAnimator animator;

		classname = args.Argv( 1 );

		dict = gameLocal.FindEntityDefDict( classname, false );
		if ( !dict ) {
			gameLocal.Printf( "Entitydef '%s' not found\n", classname );
			return;
		}
		animator.SetModel( dict->GetString( "model" ) );

		gameLocal.Printf( "----------------\n" );
		num = animator.NumAnims();
		for( i = 0; i < num; i++ ) {
			gameLocal.Printf( "%s\n", animator.AnimFullName( i ) );
		}
		gameLocal.Printf( "%d anims\n", num );
	} else {
		animationLib.ListAnims();

		size = 0;
		num = 0;
		for( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
			animator = ent->GetAnimator();
			if ( animator ) {
				alloced = animator->Allocated();
				size += alloced;
				num++;
			}
		}

		gameLocal.Printf( "%d memory used in %d entity animators\n", size, num );
	}
}

// greebo: Reload the xdata declarations by forcing the declaration manager to perform a reload
static void Cmd_ReloadXData_f( const idCmdArgs &args )
{
	// Reload the declarations, this triggers a re-parse of all xdata files (and all other files on that matter)
	declManager->Reload(true);
	// greebo: This is called twice on purpose. For some reason, the first reload doesn't reload the xdata files.
	declManager->Reload(true);

	// Now cycle through all active entities and call "refreshReadables" on them
	for (idEntity* ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next())
	{
		idThread* thread = ent->CallScriptFunctionArgs("reloadXData", true, 0, "e", ent);

		if (thread != NULL)
		{
			thread->Execute();
		}
	}
}

/*
==================
Cmd_AASStats_f
==================
*/
static void Cmd_AASStats_f( const idCmdArgs &args ) {
	int aasNum;

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	aasNum = aas_test.GetInteger();
	idAAS *aas = gameLocal.GetAAS( aasNum );
	if ( !aas ) {
		gameLocal.Printf( "No aas #%d loaded\n", aasNum );
	} else {
		aas->Stats();
	}
}

/*
==================
Cmd_TestDamage_f
==================
*/
static void Cmd_TestDamage_f( const idCmdArgs &args ) {
	idPlayer *player;
	const char *damageDefName;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc() < 2 || args.Argc() > 3 ) {
		gameLocal.Printf( "usage: testDamage <damageDefName> [angle]\n" );
		return;
	}

	damageDefName = args.Argv( 1 );

	idVec3	dir;
	if ( args.Argc() == 3 ) {
		float angle = atof( args.Argv( 2 ) );

		idMath::SinCos( DEG2RAD( angle ), dir[1], dir[0] );
		dir[2] = 0;
	} else {
		dir.Zero();
	}

	// give the player full health before and after
	// running the damage
	player->health = player->maxHealth;
	player->Damage( NULL, NULL, dir, damageDefName, 1.0f, INVALID_JOINT );
	player->health = player->maxHealth;
}

/*
==================
Cmd_TestBoneFx_f
==================
*/
static void Cmd_TestBoneFx_f( const idCmdArgs &args ) {
	idPlayer *player;
	const char *bone, *fx;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc() < 3 || args.Argc() > 4 ) {
		gameLocal.Printf( "usage: testBoneFx <fxName> <boneName>\n" );
		return;
	}

	fx = args.Argv( 1 );
	bone = args.Argv( 2 );

	player->StartFxOnBone( fx, bone );
}

/*
==================
Cmd_TestDamage_f
==================
*/
static void Cmd_TestDeath_f( const idCmdArgs &args ) {
	idPlayer *player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	idVec3 dir;
	idMath::SinCos( DEG2RAD( 45.0f ), dir[1], dir[0] );
	dir[2] = 0;

	g_testDeath.SetBool( 1 );
	player->Damage( NULL, NULL, dir, "damage_triggerhurt_1000", 1.0f, INVALID_JOINT );
	if ( args.Argc() >= 2) {
		player->SpawnGibs( dir, "damage_triggerhurt_1000" );
	}

}

/*
==================
Cmd_WeaponSplat_f
==================
*/
static void Cmd_WeaponSplat_f( const idCmdArgs &args ) {
	idPlayer *player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	player->weapon.GetEntity()->BloodSplat( 2.0f );
}

/*
==================
Cmd_SaveSelected_f
==================
*/
static void Cmd_SaveSelected_f( const idCmdArgs &args ) {
	int i;
	idPlayer *player;
	idEntity *s;
	idMapEntity *mapEnt;
	idMapFile *mapFile = gameLocal.GetLevelMap();
	idDict dict;
	idStr mapName;
	const char *name(NULL);

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	s = player->dragEntity.GetSelected();
	if ( !s ) {
		gameLocal.Printf( "no entity selected, set g_dragShowSelection 1 to show the current selection\n" );
		return;
	}

	if ( args.Argc() > 1 ) {
		mapName = args.Argv( 1 );
		mapName = "maps/" + mapName;
	}
	else {
		mapName = mapFile->GetName();
	}

	// find map file entity
	mapEnt = mapFile->FindEntity( s->name );
	// create new map file entity if there isn't one for this articulated figure
	if ( !mapEnt ) {
		mapEnt = new idMapEntity();
		mapFile->AddEntity( mapEnt );
		for ( i = 0; i < 9999; i++ ) {
			name = va( "%s_%d", s->GetEntityDefName(), i );
			if ( !mapFile->FindEntity( name ) )
				break;
		}
		s->name = name;
		mapEnt->epairs.Set( "classname", s->GetEntityDefName() );
		mapEnt->epairs.Set( "name", s->name );
	}

	if ( s->IsType( idMoveable::Type ) ) {
		// save the moveable state
		mapEnt->epairs.Set( "origin", s->GetPhysics()->GetOrigin().ToString( 8 ) );
		mapEnt->epairs.Set( "rotation", s->GetPhysics()->GetAxis().ToString( 8 ) );
	}
	else if ( s->IsType( idAFEntity_Generic::Type ) || s->IsType( idAFEntity_WithAttachedHead::Type ) ) {
		// save the articulated figure state
		dict.Clear();
		static_cast<idAFEntity_Base *>(s)->SaveState( dict );
		mapEnt->epairs.Copy( dict );
	}

	// write out the map file
	mapFile->Write( mapName, ".map" );
}

/*
==================
Cmd_DeleteSelected_f
==================
*/
static void Cmd_DeleteSelected_f( const idCmdArgs &args ) {
	idPlayer *player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( player ) {
		player->dragEntity.DeleteSelected();
	}
}

/*
==================
Cmd_SaveMoveables_f
==================
*/
static void Cmd_SaveMoveables_f( const idCmdArgs &args ) {
	int e, i;
	idMoveable *m;
	idMapEntity *mapEnt;
	idMapFile *mapFile = gameLocal.GetLevelMap();
	idStr mapName;
	const char *name(NULL);

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	for( e = 0; e < MAX_GENTITIES; e++ ) {
		m = static_cast<idMoveable *>(gameLocal.entities[ e ]);

		if ( !m || !m->IsType( idMoveable::Type ) ) {
			continue;
		}

		if ( m->IsBound() ) {
			continue;
		}

		if ( !m->IsAtRest() ) {
			break;
		}
	}

	if ( e < MAX_GENTITIES ) {
		gameLocal.Warning( "map not saved because the moveable entity %s is not at rest", gameLocal.entities[ e ]->name.c_str() );
		return;
	}

	if ( args.Argc() > 1 ) {
		mapName = args.Argv( 1 );
		mapName = "maps/" + mapName;
	}
	else {
		mapName = mapFile->GetName();
	}

	for( e = 0; e < MAX_GENTITIES; e++ ) {
		m = static_cast<idMoveable *>(gameLocal.entities[ e ]);

		if ( !m || !m->IsType( idMoveable::Type ) ) {
			continue;
		}

		if ( m->IsBound() ) {
			continue;
		}

		// find map file entity
		mapEnt = mapFile->FindEntity( m->name );
		// create new map file entity if there isn't one for this articulated figure
		if ( !mapEnt ) {
			mapEnt = new idMapEntity();
			mapFile->AddEntity( mapEnt );
			for ( i = 0; i < 9999; i++ ) {
				name = va( "%s_%d", m->GetEntityDefName(), i );
				if ( !mapFile->FindEntity( name ) )
					break;
			}
			m->name = name;
			mapEnt->epairs.Set( "classname", m->GetEntityDefName() );
			mapEnt->epairs.Set( "name", m->name );
		}
		// save the moveable state
		mapEnt->epairs.Set( "origin", m->GetPhysics()->GetOrigin().ToString( 8 ) );
		mapEnt->epairs.Set( "rotation", m->GetPhysics()->GetAxis().ToString( 8 ) );
	}

	// write out the map file
	mapFile->Write( mapName, ".map" );
}

/*
==================
Cmd_SaveRagdolls_f
==================
*/
static void Cmd_SaveRagdolls_f( const idCmdArgs &args ) {
	int e, i;
	idAFEntity_Base *af;
	idMapEntity *mapEnt;
	idMapFile *mapFile = gameLocal.GetLevelMap();
	idDict dict;
	idStr mapName;
	const char *name(NULL);

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc() > 1 ) {
		mapName = args.Argv( 1 );
		mapName = "maps/" + mapName;
	}
	else {
		mapName = mapFile->GetName();
	}

	for( e = 0; e < MAX_GENTITIES; e++ ) {
		af = static_cast<idAFEntity_Base *>(gameLocal.entities[ e ]);

		if ( !af ) {
			continue;
		}

		if ( !af->IsType( idAFEntity_WithAttachedHead::Type ) && !af->IsType( idAFEntity_Generic::Type ) ) {
			continue;
		}

		// Ish: Not sure why they did this
		/*
		if ( af->IsBound() ) {
			continue;
		}
		*/

		if ( !af->IsAtRest() ) {
			gameLocal.Warning( "the articulated figure for entity %s is not at rest", gameLocal.entities[ e ]->name.c_str() );
		}

		dict.Clear();
		af->SaveState( dict );

		// find map file entity
		mapEnt = mapFile->FindEntity( af->name );
		// create new map file entity if there isn't one for this articulated figure
		if ( !mapEnt ) {
			mapEnt = new idMapEntity();
			mapFile->AddEntity( mapEnt );
			for ( i = 0; i < 9999; i++ ) {
				name = va( "%s_%d", af->GetEntityDefName(), i );
				if ( !mapFile->FindEntity( name ) )
					break;
			}
			af->name = name;
			mapEnt->epairs.Set( "classname", af->GetEntityDefName() );
			mapEnt->epairs.Set( "name", af->name );
		}
		// save the articulated figure state
		mapEnt->epairs.Copy( dict );
	}

	// write out the map file
	mapFile->Write( mapName, ".map" );
}

/*
==================
Cmd_BindRagdoll_f
==================
*/
static void Cmd_BindRagdoll_f( const idCmdArgs &args ) {
	idPlayer *player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( player ) {
		player->dragEntity.BindSelected();
	}
}

/*
==================
Cmd_UnbindRagdoll_f
==================
*/
static void Cmd_UnbindRagdoll_f( const idCmdArgs &args ) {
	idPlayer *player;

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( player ) {
		player->dragEntity.UnbindSelected();
	}
}

/*
==================
Cmd_GameError_f
==================
*/
static void Cmd_GameError_f( const idCmdArgs &args ) {
	gameLocal.Error( "game error" );
}

/*
==================
Cmd_SaveLights_f
==================
*/
static void Cmd_SaveLights_f( const idCmdArgs &args ) {
	int e, i;
	idLight *light;
	idMapEntity *mapEnt;
	idMapFile *mapFile = gameLocal.GetLevelMap();
	idDict dict;
	idStr mapName;
	const char *name(NULL);

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc() > 1 ) {
		mapName = args.Argv( 1 );
		mapName = "maps/" + mapName;
	}
	else {
		mapName = mapFile->GetName();
	}

	for( e = 0; e < MAX_GENTITIES; e++ ) {
		light = static_cast<idLight*>(gameLocal.entities[ e ]);

		if ( !light || !light->IsType( idLight::Type ) ) {
			continue;
		}

		dict.Clear();
		light->SaveState( &dict );

		// find map file entity
		mapEnt = mapFile->FindEntity( light->name );
		// create new map file entity if there isn't one for this light
		if ( !mapEnt ) {
			mapEnt = new idMapEntity();
			mapFile->AddEntity( mapEnt );
			for ( i = 0; i < 9999; i++ ) {
				name = va( "%s_%d", light->GetEntityDefName(), i );
				if ( !mapFile->FindEntity( name ) )
					break;
			}
			light->name = name;
			mapEnt->epairs.Set( "classname", light->GetEntityDefName() );
			mapEnt->epairs.Set( "name", light->name );
		}
		// save the light state
		mapEnt->epairs.Copy( dict );
	}

	// write out the map file
	mapFile->Write( mapName, ".map" );
}


/*
==================
Cmd_SaveParticles_f
==================
*/
static void Cmd_SaveParticles_f( const idCmdArgs &args ) {
	int e;
	idEntity *ent;
	idMapEntity *mapEnt;
	idMapFile *mapFile = gameLocal.GetLevelMap();
	idDict dict;
	idStr mapName, strModel;

	if ( !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc() > 1 ) {
		mapName = args.Argv( 1 );
		mapName = "maps/" + mapName;
	}
	else {
		mapName = mapFile->GetName();
	}

	for( e = 0; e < MAX_GENTITIES; e++ ) {

		ent = static_cast<idStaticEntity*> ( gameLocal.entities[ e ] );

		if ( !ent ) {
			continue;
		}

		strModel = ent->spawnArgs.GetString( "model" );
		if ( strModel.Length() && strModel.Find( ".prt") > 0 ) {
			dict.Clear();
			dict.Set( "model", ent->spawnArgs.GetString( "model" ) );
			dict.SetVector( "origin", ent->GetPhysics()->GetOrigin() );

			// find map file entity
			mapEnt = mapFile->FindEntity( ent->name );
			// create new map file entity if there isn't one for this entity
			if ( !mapEnt ) {
				continue;
			}
			// save the particle state
			mapEnt->epairs.Copy( dict );
		}
	}

	// write out the map file
	mapFile->Write( mapName, ".map" );
}


/*
==================
Cmd_DisasmScript_f
==================
*/
static void Cmd_DisasmScript_f( const idCmdArgs &args ) {
	gameLocal.program.Disassemble();
}

/*
==================
Cmd_TestSave_f
==================
*/
static void Cmd_TestSave_f( const idCmdArgs &args ) {
	idFile *f;

	f = fileSystem->OpenFileWrite( "test.sav" );
	gameLocal.SaveGame( f );
	fileSystem->CloseFile( f );
}

/*
==================
Cmd_RecordViewNotes_f
==================
*/
static void Cmd_RecordViewNotes_f( const idCmdArgs &args ) {
	idPlayer *player;
	idVec3 origin;
	idMat3 axis;

	if ( args.Argc() <= 3 ) {
		return;
	}

	player = gameLocal.GetLocalPlayer();
	if ( !player ) {
		return;
	}

	player->GetViewPos( origin, axis );

	// Argv(1) = filename for map (viewnotes/mapname/person)
	// Argv(2) = note number (person0001)
	// Argv(3) = comments

	idStr str = args.Argv(1);
	str.SetFileExtension( ".txt" );
	idFile *file = fileSystem->OpenFileAppend( str );
	if ( file ) {
		file->WriteFloatString( "\"view\"\t( %s )\t( %s )\r\n", origin.ToString(), axis.ToString() );
		file->WriteFloatString( "\"comments\"\t\"%s: %s\"\r\n\r\n", args.Argv(2), args.Argv(3) );
		fileSystem->CloseFile( file );
	}

	idStr viewComments = args.Argv(1);
	viewComments.StripLeading("viewnotes/");
	viewComments += " -- Loc: ";
	viewComments += origin.ToString();
	viewComments += "\n";
	viewComments += args.Argv(3);
	player->hud->SetStateString( "viewcomments", viewComments );
	player->hud->HandleNamedEvent( "showViewComments" );
}

/*
==================
Cmd_CloseViewNotes_f
==================
*/
static void Cmd_CloseViewNotes_f( const idCmdArgs &args ) {
	idPlayer *player = gameLocal.GetLocalPlayer();

	if ( !player ) {
		return;
	}
	
	player->hud->SetStateString( "viewcomments", "" );
	player->hud->HandleNamedEvent( "hideViewComments" );
}

/*
==================
Cmd_ShowViewNotes_f
==================
*/
static void Cmd_ShowViewNotes_f( const idCmdArgs &args ) {
	static idLexer parser( LEXFL_ALLOWPATHNAMES | LEXFL_NOSTRINGESCAPECHARS | LEXFL_NOSTRINGCONCAT | LEXFL_NOFATALERRORS );
	idToken	token;
	idPlayer *player;
	idVec3 origin;
	idMat3 axis;

	player = gameLocal.GetLocalPlayer();

	if ( !player ) {
		return;
	}

	if ( !parser.IsLoaded() ) {
		idStr str = "viewnotes/";
		str += gameLocal.GetMapName();
		str.StripFileExtension();
		str += "/";
		if ( args.Argc() > 1 ) {
			str += args.Argv( 1 );
		} else {
			str += "comments";
		}
		str.SetFileExtension( ".txt" );
		if ( !parser.LoadFile( str ) ) {
			gameLocal.Printf( "No view notes for %s\n", gameLocal.GetMapName() );
			return;
		}
	}

	if ( parser.ExpectTokenString( "view" ) && parser.Parse1DMatrix( 3, origin.ToFloatPtr() ) && 
		parser.Parse1DMatrix( 9, axis.ToFloatPtr() ) && parser.ExpectTokenString( "comments" ) && parser.ReadToken( &token ) ) {
		player->hud->SetStateString( "viewcomments", token );
		player->hud->HandleNamedEvent( "showViewComments" );
		player->Teleport( origin, axis.ToAngles(), NULL );
	} else {
		parser.FreeSource();
		player->hud->HandleNamedEvent( "hideViewComments" );
		return;
	}
}

/*
=================
FindEntityGUIs

helper function for Cmd_NextGUI_f.  Checks the passed entity to determine if it
has any valid gui surfaces.
=================
*/
bool FindEntityGUIs( idEntity *ent, const modelSurface_t ** surfaces,  int maxSurfs, int &guiSurfaces ) {
	renderEntity_t			*renderEnt;
	idRenderModel			*renderModel;
	const modelSurface_t	*surf;
	const idMaterial		*shader;
	int						i;

	assert( surfaces != NULL );
	assert( ent != NULL );

	memset( surfaces, 0x00, sizeof( modelSurface_t *) * maxSurfs );
	guiSurfaces = 0;

	renderEnt  = ent->GetRenderEntity();
	renderModel = renderEnt->hModel;
	if ( renderModel == NULL ) {
		return false;
	}

	for( i = 0; i < renderModel->NumSurfaces(); i++ ) {
		surf = renderModel->Surface( i );
		if ( surf == NULL ) {
			continue;
		}
		shader = surf->shader;
		if ( shader == NULL ) {
			continue;
		}
		if ( shader->GetEntityGui() > 0 ) {
			surfaces[ guiSurfaces++ ] = surf;
		}
	}

	return ( guiSurfaces != 0 );
}

/*
=================
Cmd_NextGUI_f
=================
*/
void Cmd_NextGUI_f( const idCmdArgs &args ) {
	idVec3					origin;
	idAngles				angles;
	idPlayer				*player;
	idEntity				*ent;
	int						guiSurfaces;
	bool					newEnt;
	renderEntity_t			*renderEnt;
	int						surfIndex;
	srfTriangles_t			*geom;
	idMat4					modelMatrix;
	idVec3					normal;
	idVec3					center;
	const modelSurface_t	*surfaces[ MAX_RENDERENTITY_GUI ];

	player = gameLocal.GetLocalPlayer();
	if ( !player || !gameLocal.CheatsOk() ) {
		return;
	}

	if ( args.Argc() != 1 ) {
		gameLocal.Printf( "usage: nextgui\n" );
		return;
	}

	// start at the last entity
	ent = gameLocal.lastGUIEnt.GetEntity();

	// see if we have any gui surfaces left to go to on the current entity.
	guiSurfaces = 0;
	newEnt = false;
	if ( ent == NULL ) {
		newEnt = true;
	} else if ( FindEntityGUIs( ent, surfaces, MAX_RENDERENTITY_GUI, guiSurfaces ) == true ) {
		if ( gameLocal.lastGUI >= guiSurfaces ) {
			newEnt = true;
		}
	} else {
		// no actual gui surfaces on this ent, so skip it
		newEnt = true;
	}

	if ( newEnt == true ) {
		// go ahead and skip to the next entity with a gui...
		if ( ent == NULL ) {
			ent = gameLocal.spawnedEntities.Next();
		} else {
			ent = ent->spawnNode.Next();
		}

		for ( ; ent != NULL; ent = ent->spawnNode.Next() ) {
			if ( ent->spawnArgs.GetString( "gui", NULL ) != NULL ) {
				break;
			}
			
			if ( ent->spawnArgs.GetString( "gui2", NULL ) != NULL ) {
				break;
			}

			if ( ent->spawnArgs.GetString( "gui3", NULL ) != NULL ) {
				break;
			}
			
			// try the next entity
			gameLocal.lastGUIEnt = ent;
		}

		gameLocal.lastGUIEnt = ent;
		gameLocal.lastGUI = 0;

		if ( !ent ) {
			gameLocal.Printf( "No more gui entities. Starting over...\n" );
			return;
		}
	}

	if ( FindEntityGUIs( ent, surfaces, MAX_RENDERENTITY_GUI, guiSurfaces ) == false ) {
		gameLocal.Printf( "Entity \"%s\" has gui properties but no gui surfaces.\n", ent->name.c_str() );
	}

	if ( guiSurfaces == 0 ) {
		gameLocal.Printf( "Entity \"%s\" has gui properties but no gui surfaces!\n", ent->name.c_str() );
		return;
	}

	gameLocal.Printf( "Teleporting to gui entity \"%s\", gui #%d.\n" , ent->name.c_str (), gameLocal.lastGUI );

	renderEnt = ent->GetRenderEntity();
	surfIndex = gameLocal.lastGUI++;
	geom = surfaces[ surfIndex ]->geometry;
	if ( geom == NULL ) {
		gameLocal.Printf( "Entity \"%s\" has gui surface %d without geometry!\n", ent->name.c_str(), surfIndex );
		return;
	}

	assert( geom->facePlanes != NULL );

	modelMatrix = idMat4( renderEnt->axis, renderEnt->origin );	
	normal = geom->facePlanes[ 0 ].Normal() * renderEnt->axis;
	center = geom->bounds.GetCenter() * modelMatrix;

	origin = center + (normal * 32.0f);
	origin.z -= player->EyeHeight();
	normal *= -1.0f;
	angles = normal.ToAngles ();

	//	make sure the player is in noclip
	player->noclip = true;
	player->Teleport( origin, angles, NULL );
}

static void ArgCompletion_DefFile( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "def/", true, ".def", NULL );
}

/*
===============
Cmd_TestId_f
outputs a string from the string table for the specified id
===============
*/
void Cmd_TestId_f( const idCmdArgs &args ) {
	idStr	id;
	int		i;
	if ( args.Argc() == 1 ) {
		common->Printf( "usage: testid <string id>\n" );
		return;
	}

	for ( i = 1; i < args.Argc(); i++ ) {
		id += args.Argv( i );
	}
	if ( idStr::Cmpn( id, STRTABLE_ID, STRTABLE_ID_LENGTH ) != 0 ) {
		id = STRTABLE_ID + id;
	}
	gameLocal.mpGame.AddChatLine( common->GetLanguageDict()->GetString( id ), "<nothing>", "<nothing>", "<nothing>" );	
}

void Cmd_SetClipMask(const idCmdArgs& args)
{
	if (args.Argc() != 3)
	{
		common->Printf( "usage: setClipMask <entity> <clipMask>\n" );
		return;
	}

	idStr targetEntity = args.Argv(1);
	int clipMask = atoi(args.Argv(2));

	idEntity* ent = gameLocal.FindEntity(targetEntity.c_str());

	if (ent != NULL && ent->GetPhysics() != NULL)
	{
		ent->GetPhysics()->SetClipMask(clipMask);
		common->Printf("Clipmask of entity %s set to %d\n", ent->name.c_str(), clipMask);
	}
}

void Cmd_SetClipContents(const idCmdArgs& args)
{
	if (args.Argc() != 3)
	{
		common->Printf( "usage: setClipContents <entity> <contents>\n" );
		return;
	}

	idStr targetEntity = args.Argv(1);
	int contents = atoi(args.Argv(2));

	idEntity* ent = gameLocal.FindEntity(targetEntity.c_str());

	if (ent != NULL && ent->GetPhysics() != NULL)
	{
		ent->GetPhysics()->SetContents(contents);
		common->Printf("Contents of entity %s set to %d\n", ent->name.c_str(), contents);
	}
}

void Cmd_ShowWalkPath_f(const idCmdArgs& args)
{
	if (args.Argc() != 2)
	{
		common->Printf( "usage: aas_showWalkPath <areaNum>\n" );
		return;
	}

	idPlayer* player = gameLocal.GetLocalPlayer();
	if ( !player ) {
		return;
	}

	idAASLocal* aas = dynamic_cast<idAASLocal*>(gameLocal.GetAAS("aas32"));
	if (aas != NULL)
	{
		aas->ShowWalkPath(player->GetPhysics()->GetOrigin(), atoi(args.Argv(1)), aas->AreaCenter(atoi(args.Argv(1))));
	}
}

void Cmd_ShowReachabilities_f(const idCmdArgs& args)
{
	if (args.Argc() != 2)
	{
		common->Printf( "usage: aas_showReachabilities <areaNum>\n" );
		return;
	}

	idAASLocal* aas = dynamic_cast<idAASLocal*>(gameLocal.GetAAS("aas32"));
	if (aas != NULL)
	{
		idReachability* reach = aas->GetAreaFirstReachability(atoi(args.Argv(1)));

		while (reach != NULL)
		{
			aas->DrawReachability(reach);		
			reach = reach->next;
		}
	}
}

void Cmd_ShowAASStats_f(const idCmdArgs& args)
{
	for (int i = 0; i < gameLocal.NumAAS(); i++)
	{
		idAAS* aas = dynamic_cast<idAASLocal*>(gameLocal.GetAAS(i));
		if (aas != NULL)
		{
			aas->Stats();
		}
	}
}

void Cmd_ShowEASRoute_f(const idCmdArgs& args)
{
	if (args.Argc() != 2)
	{
		common->Printf( "usage: eas_showRoute <targetAreaNum>\n" );
		return;
	}

	idPlayer* player = gameLocal.GetLocalPlayer();
	if (player == NULL) 
	{
		common->Printf( "no player found\n" );
		return;
	}

	idAASLocal* aas = dynamic_cast<idAASLocal*>(gameLocal.GetAAS("aas32"));
	if (aas != NULL)
	{
		int areaNum = atoi(args.Argv(1));

		aas->DrawEASRoute(player->GetPhysics()->GetOrigin(), areaNum);
	}
}

void Cmd_StartConversation_f(const idCmdArgs& args)
{
	if (args.Argc() != 2)
	{
		gameLocal.Printf("Usage: tdm_start_conversation <conversationName>. Use tdm_list_conversations to get a list of names.\n" );
		return;
	}

	if (gameLocal.GameState() != GAMESTATE_ACTIVE)
	{
		gameLocal.Printf("No map running\n");
		return;
	}

	int idx = gameLocal.m_ConversationSystem->GetConversationIndex(args.Argv(1));
	if (idx != -1)
	{
		gameLocal.m_ConversationSystem->StartConversation(idx);
	}
	else
	{
		gameLocal.Printf("No conversation with name: %s\n", args.Argv(1));
	}
}

void Cmd_ListConversations_f(const idCmdArgs& args)
{
	if (gameLocal.GameState() != GAMESTATE_ACTIVE)
	{
		gameLocal.Printf("No map running\n");
		return;
	}

	for (int i = 0; i < gameLocal.m_ConversationSystem->GetNumConversations(); i++)
	{
		ai::ConversationPtr conversation = gameLocal.m_ConversationSystem->GetConversation(i);
		
		if (conversation == NULL) continue;

		gameLocal.Printf("%d: %s (%d commands)\n", i, conversation->GetName().c_str(), conversation->GetNumCommands());
	}
}

void Cmd_ShowLoot_f(const idCmdArgs& args)
{
	if (gameLocal.GameState() != GAMESTATE_ACTIVE)
	{
		gameLocal.Printf("No map running\n");
		return;
	}

	int items = 0;
	int gold = 0;
	int jewels = 0;
	int goods = 0;

	for (idEntity* ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next())
	{
		if (ent == NULL) continue;

		int value = ent->spawnArgs.GetInt("inv_loot_value", "-1");

		if (value <= 0) continue; // no loot item

		items++;

		LootType lootType = CInventoryItem::GetLootTypeFromSpawnargs(ent->spawnArgs);
		
		idVec4 colour(colorWhite);

		switch(lootType)
		{
			case LOOT_GOLD:
				gold += value;
				colour = idVec4(0.97f, 0.93f, 0.58f, 1);
			break;

			case LOOT_GOODS:
				goods += value;
				colour = idVec4(0.3f, 0.91f, 0.3f, 1);
			break;

			case LOOT_JEWELS:
				jewels += value;
				colour = idVec4(0.96f, 0.2f, 0.2f, 1);
			break;
			
			default: break;
		} 
		
		gameRenderWorld->DebugBox(colour, idBox(ent->GetPhysics()->GetAbsBounds()), 5000);
	}

	gameLocal.Printf("Highlighing loot items for 5 seconds...\n");
	gameLocal.Printf("Loot items remaining: %d\n", items);
	gameLocal.Printf("Gold: %d, Jewels: %d, Goods: %d\n", gold, jewels, goods);
}

void Cmd_ActivateLog_f(const idCmdArgs& args)
{
	if (args.Argc() != 2)
	{
		gameLocal.Printf("Usage: tdm_activatelogclass <logclass>. Use the TAB to get auto-complete logclasses.\n" );
		return;
	}

	LC_LogClass logclassIndex = CGlobal::GetLogClassForString(args.Argv(1));
	
	if (logclassIndex != LC_COUNT)
	{
		// Log class found
		g_Global.m_ClassArray[logclassIndex] = true;

		// activate all types too
		g_Global.m_LogArray[LT_WARNING] = true;
		g_Global.m_LogArray[LT_ERROR] = true;
		g_Global.m_LogArray[LT_INFO] = true;
		g_Global.m_LogArray[LT_DEBUG] = true;

		gameLocal.Printf("Logclass %d activated.", static_cast<int>(logclassIndex));
	}
}

void Cmd_DeactivateLog_f(const idCmdArgs& args)
{
	if (args.Argc() != 2)
	{
		gameLocal.Printf("Usage: tdm_deactivatelogclass <logclass>. Use the TAB to get auto-complete logclasses.\n" );
		return;
	}

	LC_LogClass logclassIndex = CGlobal::GetLogClassForString(args.Argv(1));
	
	if (logclassIndex != LC_COUNT)
	{
		// Log class found
		g_Global.m_ClassArray[logclassIndex] = false;

		// activate all types too
		g_Global.m_LogArray[LT_WARNING] = true;
		g_Global.m_LogArray[LT_ERROR] = true;
		g_Global.m_LogArray[LT_INFO] = true;
		g_Global.m_LogArray[LT_DEBUG] = true;

		gameLocal.Printf("Logclass %d deactivated.", static_cast<int>(logclassIndex));
	}
}

//-------------------------------------------------------------
// Do not account for centerScale or Scroll for now.
typedef struct _ImageInfo{
	idStr m_strImageName;
	idStr m_strUVScale;

	_ImageInfo() :
		m_strImageName(),
		m_strUVScale("1, 1")
	{
	}
}ImageInfo_s;

enum eVertexBlendType {
	eVertexBlendType_None,
	eVertexBlendType_VertexColored,
	eVertexBlendType_InvVertexColored
};
//------------------------------------------------------------------
typedef std::multimap< eVertexBlendType, ImageInfo_s > ImageInfoMap;
//------------------------------------------------------------------

//-------------------------------------------------------------
// GetValidStageTextureName
//
// Description: Obtains a valid texture name with trailing white spaces and curly brackets (if found any) removed.
//				A check for valid number of opening and closing brackets performed.
//-------------------------------------------------------------
bool GetValidStageExpression( idLexer &a_lexSource, idStr & a_strStageTextureName )
{
	int iOffset, nBrackets;
	a_strStageTextureName.Empty();

	std::vector< idStr > arrStrInvalidTokens;
	
	arrStrInvalidTokens.push_back( "bumpmap" );
	arrStrInvalidTokens.push_back( "diffusemap" );
	arrStrInvalidTokens.push_back( "specularmap" );
	arrStrInvalidTokens.push_back( "map" );

// 	gameLocal.Printf("Entering loop. \n" );

	idToken tknParsedLine;
	int i=0;
	for( nBrackets = 0 ; !a_lexSource.EndOfFile() ; )
	{
		while(a_lexSource.ReadToken( &tknParsedLine )) 
		{
			if ( tknParsedLine.linesCrossed ) {
// 				gameLocal.Printf("End of the line reached. \n" );
				break;
			}
			if ( a_strStageTextureName.Length() ) {
				a_strStageTextureName += " ";
			}
			bool bIsValidToken = true;
			for( std::vector<idStr>::iterator iter = arrStrInvalidTokens.begin() ; arrStrInvalidTokens.end() != iter; iter ++ )
			{
				if( 0 != tknParsedLine.Icmp( *iter ) )
					continue;

				bIsValidToken = false;
// 				gameLocal.Printf("Invalid token found. \n" );
				break;
			}
			if( bIsValidToken )
			{
				a_strStageTextureName += tknParsedLine;
// 				gameLocal.Printf("constructing \"%s\" with a valid token. \n", a_strStageTextureName.c_str() );
			}
			else
			{
// 				gameLocal.Printf("%s is an invalid token. \n", tknParsedLine.c_str() );
				break;
			}
		}

		a_strStageTextureName.Strip('\n');
		a_strStageTextureName.Strip('\t');
		a_strStageTextureName.Strip('\r');
		a_strStageTextureName.Strip(' ');

		// Make sure that we have equal number of opening bracket and closing brackets.
		for(iOffset = 0, nBrackets = 0; 0 <= (iOffset = a_strStageTextureName.Find( '(', iOffset )); nBrackets ++, iOffset ++ );
		for(iOffset = 0; 0 <= (iOffset = a_strStageTextureName.Find( ')', iOffset )); nBrackets --, iOffset ++ );

		if ( 0 == nBrackets )
		{
			// We have gone one token ahead than needed in the lexer so offset it back again.
// 			gameLocal.Printf("Unreading token %s. \n", tknParsedLine.c_str() );
			a_lexSource.UnreadToken( &tknParsedLine );

			a_strStageTextureName.Strip('}');
			a_strStageTextureName.Strip('{');
			if( a_strStageTextureName.Length() <= 0 )
				return false;

			gameLocal.Printf(" Found valid expression: %s \n ", a_strStageTextureName.c_str() );
			return true;
		}

		// Append the first token of the newly read line.
		a_strStageTextureName += tknParsedLine;
	}

	return false;
}

//-------------------------------------------------------------
// GetMaterialStageInfo
//
// Description: For a given material stage (any one of "diffusemap", "bumpmap" & specular map ) finds out number of 
//				textures (pathnames) being used along with their UV scales. 
//-------------------------------------------------------------
void GetMaterialStageInfo ( const char* a_strMatStageName, idLexer &a_lexSource, ImageInfoMap & a_arrMatStageInfo )
{
	a_lexSource.Reset();
// 	gameLocal.Printf( "Looking for valid %s stage information (w/o blend). \n", a_strMatStageName );
	while ( 1 == a_lexSource.SkipUntilString( a_strMatStageName ) )
	{
		ImageInfo_s currentImageInfo ;

		if( true == GetValidStageExpression( a_lexSource, currentImageInfo.m_strImageName ) )
		{
// 			gameLocal.Printf( "Found valid %s stage information (w/o blend). \n", a_strMatStageName );
			a_arrMatStageInfo.insert( ImageInfoMap::value_type( eVertexBlendType_None, currentImageInfo ) );
		}
	}
// 	if( a_arrMatStageInfo.size() == 0 )
// 		gameLocal.Printf( "Could not find valid %s stage information w/o blend. \n", a_strMatStageName);

	a_lexSource.Reset();

// 	gameLocal.Printf( "Looking for %s stage information with blend. \n", a_strMatStageName );
	while ( 1 == a_lexSource.SkipUntilString( "blend" ) )
	{
		idToken tknMatStage;
		if( 1 == a_lexSource.ReadToken( &tknMatStage ) && 0 == tknMatStage.Icmp(a_strMatStageName) )
		{
			idToken tknMap;
			if( 0 != a_lexSource.ReadToken( &tknMap ) && 0 == tknMap.Icmp("map") )
			{
				ImageInfo_s currentImageInfo;
				eVertexBlendType vertBlendType = eVertexBlendType_None;

				if( true == GetValidStageExpression( a_lexSource, currentImageInfo.m_strImageName ) )
				{
					bool bIsScaleFound = false;
					while( "}" != tknMatStage )
					{
						if( !a_lexSource.ReadToken( &tknMatStage ) )
						{
							gameLocal.Warning( "Unexpected end of material when trying to obtain scale. \n");
							break;
						}
						// Not using expectTokenString anymore since "Map" is treated as different token than "map". 
						// So doing a manual non-case sensitive checks instead.
						if( !bIsScaleFound && 0 == tknMatStage.Icmp("scale") )
						{
							idStr strScale;
							if( true == GetValidStageExpression( a_lexSource, strScale ) )
								currentImageInfo.m_strUVScale = strScale;

							gameLocal.Printf(" Scale: %s \n ", strScale.c_str() );
							bIsScaleFound = true;
						}

						if( eVertexBlendType_None == vertBlendType )  
						{
							if( 0 == tknMatStage.Icmp("vertexColor") )
							{
								gameLocal.Printf(" The stage is vertex-colored \n " );
								vertBlendType = eVertexBlendType_VertexColored;
							}
							else if( 0 == tknMatStage.Icmp("inverseVertexColor") )
							{
								gameLocal.Printf(" The stage is inverse vertex-colored \n " );
								vertBlendType = eVertexBlendType_InvVertexColored;
							}
						}
						else if( bIsScaleFound )
							break;

					}
					a_arrMatStageInfo.insert( ImageInfoMap::value_type( vertBlendType, currentImageInfo ) );
	
				}
			}
		}

	}
}


//-------------------------------------------------------------
// FindBlockContainingWords
//
// Description: Inside a specified material shader block, finds the character offsets for start & end of the block 
//				that contains the words (specified by vector of strings) in their exact order.
//	Return value: True if the block is found.
//-------------------------------------------------------------

bool FindBlockContainingWords(  const char *a_text, std::vector<idStr>& a_arrSearchWords, unsigned int & a_uiStartOffset, unsigned int & a_uiEndOffset,
								const char a_cBlockStart = '{', const char a_cBlockEnd = '}' )
{
	int	uiSearchIndex;
	unsigned int uiSearchOffset = 0;
	unsigned int iTextLength = idStr::Length(a_text);
	bool bAreAllWordsFound = false;

	for(std::vector<idStr>::iterator iter = a_arrSearchWords.begin(); ; )
	{

		uiSearchIndex = idStr::FindText( a_text, (*iter).c_str(), false, uiSearchOffset );
		//  		gameLocal.Printf( " Searched %s from offset %d and found index %d \n", (*iter).c_str(),uiSearchOffset, uiSearchIndex );

		if( uiSearchIndex < 0 )
		{
			//  			gameLocal.Warning( " Could not find search word %s\n", (*iter).c_str() );
			return false;
		}

		bAreAllWordsFound = true;

		// Make sure that, this is not the first word we have found.
		if( a_arrSearchWords.begin() != iter )
		{
			if( uiSearchIndex != uiSearchOffset )
			{
				//  				gameLocal.Warning( " Could not find search word %s in the expected order\n", (*iter).c_str() );

				//Start the search from the first word again, since all of the search words are important.
				bAreAllWordsFound = false;
				iter = a_arrSearchWords.begin();
				continue;
			}
		}

		// Increment the iterator.
		iter ++;

		if( a_arrSearchWords.end() == iter )
			break;

		//Read white spaces and adjust the search offset accordingly for the next search.
		for( uiSearchOffset = uiSearchIndex + (*(iter-1)).Length(); uiSearchOffset < iTextLength; uiSearchOffset++ )
		{
			if( ' ' == a_text[ uiSearchOffset ] || '\t' == a_text[ uiSearchOffset ] || '\r' == a_text[ uiSearchOffset ] )
				continue;

			break;
		}
	}

	if( bAreAllWordsFound )
	{
		// 		gameLocal.Printf( " Total %d word(s) found \n", a_arrSearchWords.size() );

		if( a_arrSearchWords.size() == 1 )
		{
			uiSearchOffset = uiSearchIndex;
		}

		// Start tracking offsets to the opening and closing of the block from the last search-Index.
		bool bIsStartOffsetFound	= false;
		bool bIsEndOffsetFound		= false;
		for( a_uiStartOffset = a_uiEndOffset = uiSearchOffset ; a_uiStartOffset > 0 &&  a_uiEndOffset < iTextLength ; )
		{
			if( a_cBlockStart == a_text[ a_uiStartOffset ] )
				bIsStartOffsetFound = true;
			else
				a_uiStartOffset --;

			if( a_cBlockEnd == a_text[ a_uiEndOffset ] )
				bIsEndOffsetFound = true;
			else
				a_uiEndOffset ++;

			if( bIsStartOffsetFound && bIsEndOffsetFound )
			{
				// Adjust end offset by one extra character to make sure that we account the closing block.
				a_uiEndOffset ++;
				//  				idStr myBlock( a_text, a_uiStartOffset, a_uiEndOffset );
				//  				gameLocal.Printf( "Found block from %d to %d, search offset is %d\n", a_uiStartOffset, a_uiEndOffset, uiSearchOffset );
				//  				gameLocal.Printf( "%s \n", myBlock.c_str() );
				return true;
			}
		}
		// 		gameLocal.Warning( " Block start found:%d Block End Found:%d, Returning false.\n", (int)bIsStartOffsetFound, (int)bIsEndOffsetFound );
	}

	// 	if( !bAreAllWordsFound )
	//  		gameLocal.Warning( " Returning false since given words can't be found in the exact given order.\n" );
	return false;
}

void CreateNewAmbientBlock( const ImageInfoMap& a_arrDiffusemapInfo, const ImageInfoMap& a_arrBumpmapInfo, const ImageInfoMap& a_arrSpecularmapInfo, std::vector<char>& a_arrCharNewAmbientBlock )
{
	static const char newAmbientBlock[] = {	
		"\n	{							\n"
		"		if (global5 == 1)		\n"
		"		blend add				\n"
		"		map				%s		\n"
		"		scale			%s		\n"
		"		red				global2	\n"
		"		green			global3	\n"
		"		blue			global4	\n"
		"	}							\n"
		"	{							\n"
		"		if (global5 == 2)		\n"
		"		blend add				\n"
		"		program	ambientEnvironment.vfp	\n"
		"		vertexParm		0		%s, %s		// UV Scales for Diffuse and Bump	\n"
		"		vertexParm		1		%s, 1, 1	// (X,Y) UV Scale for specular		\n"
		"		vertexParm		2		global2, global3, global4, 1	\n"
		"																\n"
		"		fragmentMap		0		cubeMap env/gen1				\n"
		"		fragmentMap		1		%s			// Bump				\n"
		"		fragmentMap		2		%s			// Diffuse			\n"
		"		fragmentMap		3		%s			// Specular			\n"
		"	}"
	};

	static const char newAmbientBlockVertColorBlended[] = {	
		"\n	{							\n"
		"		if (global5 == 1)		\n"
		"		blend add				\n"
		"		map				%s		\n"
		"		scale			%s		\n"
		"		red				global2	\n"
		"		green			global3	\n"
		"		blue			global4	\n"
		"		vertexColor				\n"
		"	}							\n"
		"	{							\n"
		"		if (global5 == 1)		\n"
		"		blend add				\n"
		"		map				%s		\n"
		"		scale			%s		\n"
		"		red				global2	\n"
		"		green			global3	\n"
		"		blue			global4	\n"
		"		inverseVertexColor		\n"
		"	}							\n"
		"	{							\n"
		"		if (global5 == 2)		\n"
		"		blend add				\n"
		"		program	ambientEnvVertexBlend.vfp	\n"
		"		vertexParm		0		%s, %s		// UV Scales for Diffuse1 and Bump1	resp.	\n"
		"		vertexParm		1		%s, %s		// UV Scale for specular1 and Diffuse2 resp.\n"
		"		vertexParm		2		%s, %s		// UV Scale for Bump2 and specular2 resp.	\n"
		"		vertexParm		3		global2, global3, global4, 1	\n"
		"		//----------- VertexColored -------------------			\n"
		"		fragmentMap		0		cubeMap env/gen1				\n"
		"		fragmentMap		1		%s			// Bump1			\n"
		"		fragmentMap		2		%s			// Diffuse1			\n"
		"		fragmentMap		3		%s			// Specular1		\n"
		"		//----------- InverseVertexColored ------------			\n"
		"		fragmentMap		4		%s			// Bump2			\n"
		"		fragmentMap		5		%s			// Diffuse2			\n"
		"		fragmentMap		6		%s			// Specular2		\n"
		"	}"
	};



	ImageInfoMap::const_iterator itrDiffusemapInfoVertexColored		= a_arrDiffusemapInfo.find( eVertexBlendType_VertexColored );
	ImageInfoMap::const_iterator itrBumpmapInfoVertexColored		= a_arrBumpmapInfo.find( eVertexBlendType_VertexColored );
	ImageInfoMap::const_iterator itrSpecularmapInfoVertexColored	= a_arrSpecularmapInfo.find( eVertexBlendType_VertexColored );

	ImageInfoMap::const_iterator itrDiffusemapInfoInvVertexColored	= a_arrDiffusemapInfo.find( eVertexBlendType_InvVertexColored );
	ImageInfoMap::const_iterator itrBumpmapInfoInvVertexColored		= a_arrBumpmapInfo.find( eVertexBlendType_InvVertexColored );
	ImageInfoMap::const_iterator itrSpecularmapInfoInvVertexColored	= a_arrSpecularmapInfo.find( eVertexBlendType_InvVertexColored );

	bool bIsVertexColorBlended = ( ( a_arrDiffusemapInfo.end() != itrDiffusemapInfoVertexColored || a_arrBumpmapInfo.end() != itrBumpmapInfoVertexColored || 
		a_arrSpecularmapInfo.end() != itrSpecularmapInfoVertexColored )	&&
		(a_arrDiffusemapInfo.end() != itrDiffusemapInfoInvVertexColored || a_arrBumpmapInfo.end() != itrBumpmapInfoInvVertexColored || 
		a_arrSpecularmapInfo.end() != itrSpecularmapInfoInvVertexColored ) );

	ImageInfoMap::const_iterator itrDiffusemapInfo	= a_arrDiffusemapInfo.find( eVertexBlendType_None );
	ImageInfoMap::const_iterator itrBumpmapInfo		= a_arrBumpmapInfo.find( eVertexBlendType_None );
	ImageInfoMap::const_iterator itrSpecularmapInfo	= a_arrSpecularmapInfo.find( eVertexBlendType_None );


	if ( bIsVertexColorBlended )
	{
		gameLocal.Printf("This material is vertex-color blended. \n");
	
		// Find out normal maps for vertex blending.

		// Handle cases where vertexColor is not used for bumpmaps
		if ( a_arrBumpmapInfo.end() == itrBumpmapInfoVertexColored )
			itrBumpmapInfoVertexColored = itrBumpmapInfo;

		if ( a_arrBumpmapInfo.end() == itrBumpmapInfoInvVertexColored )
		{
			ImageInfoMap::const_iterator itrBumpmapInfo2 = itrBumpmapInfo;
			if( a_arrBumpmapInfo.end() != itrBumpmapInfo2 )
			{
				// Try and see if there's a second bumpmap with no vertex-color blend.
				++itrBumpmapInfo2;
				itrBumpmapInfoInvVertexColored =  a_arrBumpmapInfo.end() != itrBumpmapInfo2 ? ( eVertexBlendType_None == (*itrBumpmapInfo2).first ? itrBumpmapInfo2 : itrBumpmapInfo ) : itrBumpmapInfo;  
			}
		}
		unsigned int uiBlockSize =	idStr::Length( newAmbientBlockVertColorBlended ) + 1 + 
			//------------------ For vertexColor ----------------
			(a_arrDiffusemapInfo.end() != itrDiffusemapInfoVertexColored	? (*itrDiffusemapInfoVertexColored).second.m_strImageName.Length()	: idStr::Length("_black")	) * 2	+
			(a_arrDiffusemapInfo.end() != itrDiffusemapInfoVertexColored	? (*itrDiffusemapInfoVertexColored).second.m_strUVScale.Length()	: idStr::Length("1, 1")		) * 2	+ 
			(	a_arrBumpmapInfo.end() != itrBumpmapInfoVertexColored		? (*itrBumpmapInfoVertexColored).second.m_strImageName.Length()		: idStr::Length("_flat")	) 		+ 
			(	a_arrBumpmapInfo.end() != itrBumpmapInfoVertexColored		? (*itrBumpmapInfoVertexColored).second.m_strUVScale.Length()		: idStr::Length("1, 1")		)		+ 
			(a_arrSpecularmapInfo.end() != itrSpecularmapInfoVertexColored	? (*itrSpecularmapInfoVertexColored).second.m_strImageName.Length()	: idStr::Length("_black")	)		+ 
			(a_arrSpecularmapInfo.end() != itrSpecularmapInfoVertexColored	? (*itrSpecularmapInfoVertexColored).second.m_strUVScale.Length()	: idStr::Length("1, 1")		)		+ 
			//------------------ For inverseVertexColor ---------
			(a_arrDiffusemapInfo.end() != itrDiffusemapInfoInvVertexColored		? (*itrDiffusemapInfoInvVertexColored).second.m_strImageName.Length()	: idStr::Length("_black")	) * 2	+
			(a_arrDiffusemapInfo.end() != itrDiffusemapInfoInvVertexColored		? (*itrDiffusemapInfoInvVertexColored).second.m_strUVScale.Length()		: idStr::Length("1, 1")		) * 2	+ 
			(	a_arrBumpmapInfo.end() != itrBumpmapInfoInvVertexColored		? (*itrBumpmapInfoInvVertexColored).second.m_strImageName.Length()		: idStr::Length("_flat")	) 		+ 
			(	a_arrBumpmapInfo.end() != itrBumpmapInfoInvVertexColored		? (*itrBumpmapInfoInvVertexColored).second.m_strUVScale.Length()		: idStr::Length("1, 1")		)		+ 
			(a_arrSpecularmapInfo.end() != itrSpecularmapInfoInvVertexColored	? (*itrSpecularmapInfoInvVertexColored).second.m_strImageName.Length()	: idStr::Length("_black")	)		+ 
			(a_arrSpecularmapInfo.end() != itrSpecularmapInfoInvVertexColored	? (*itrSpecularmapInfoInvVertexColored).second.m_strUVScale.Length()	: idStr::Length("1, 1")		);


		a_arrCharNewAmbientBlock.resize( uiBlockSize, 0 );

		idStr::snPrintf( &a_arrCharNewAmbientBlock[0], uiBlockSize, newAmbientBlockVertColorBlended, 
			//------------------ For vertexColor ----------------
			a_arrDiffusemapInfo.end() != itrDiffusemapInfoVertexColored ?	(*itrDiffusemapInfoVertexColored).second.m_strImageName.c_str()				: "_black", 
			a_arrDiffusemapInfo.end() != itrDiffusemapInfoVertexColored ?	(*itrDiffusemapInfoVertexColored).second.m_strUVScale.c_str()				: "1, 1", 
			//------------------ For inverseVertexColor ---------
			a_arrDiffusemapInfo.end() != itrDiffusemapInfoInvVertexColored ?	(*itrDiffusemapInfoInvVertexColored).second.m_strImageName.c_str()		: "_black", 
			a_arrDiffusemapInfo.end() != itrDiffusemapInfoInvVertexColored ?	(*itrDiffusemapInfoInvVertexColored).second.m_strUVScale.c_str()		: "1, 1", 
			//---------------------------------------------------

			//------------------ For vertexColor ----------------
			a_arrDiffusemapInfo.end() != itrDiffusemapInfoVertexColored ?	(*itrDiffusemapInfoVertexColored).second.m_strUVScale.c_str()			: "1, 1", 
			a_arrBumpmapInfo.end() != itrBumpmapInfoVertexColored ?			(*itrBumpmapInfoVertexColored).second.m_strUVScale.c_str()				: "1, 1", 
			a_arrSpecularmapInfo.end() != itrSpecularmapInfoVertexColored ?	(*itrSpecularmapInfoVertexColored).second.m_strUVScale.c_str()			: "1, 1", 
			//------------------ For inverseVertexColor ---------
			a_arrDiffusemapInfo.end() != itrDiffusemapInfoInvVertexColored ?	(*itrDiffusemapInfoInvVertexColored).second.m_strUVScale.c_str()	: "1, 1", 
			a_arrBumpmapInfo.end() != itrBumpmapInfoInvVertexColored ?			(*itrBumpmapInfoInvVertexColored).second.m_strUVScale.c_str()		: "1, 1", 
			a_arrSpecularmapInfo.end() != itrSpecularmapInfoInvVertexColored ?	(*itrSpecularmapInfoInvVertexColored).second.m_strUVScale.c_str()	: "1, 1", 
			//---------------------------------------------------

			//------------------ For vertexColor ----------------
			a_arrBumpmapInfo.end() != itrBumpmapInfoVertexColored ?			(*itrBumpmapInfoVertexColored).second.m_strImageName.c_str()			: "_flat", 
			a_arrDiffusemapInfo.end() != itrDiffusemapInfoVertexColored ?	(*itrDiffusemapInfoVertexColored).second.m_strImageName.c_str()			: "_black", 
			a_arrSpecularmapInfo.end() != itrSpecularmapInfoVertexColored ?	(*itrSpecularmapInfoVertexColored).second.m_strImageName.c_str()		: "_black",
			//------------------ For inverseVertexColor ---------
			a_arrBumpmapInfo.end() != itrBumpmapInfoInvVertexColored ?			(*itrBumpmapInfoInvVertexColored).second.m_strImageName.c_str()			: "_flat", 
			a_arrDiffusemapInfo.end() != itrDiffusemapInfoInvVertexColored ?	(*itrDiffusemapInfoInvVertexColored).second.m_strImageName.c_str()		: "_black", 
			a_arrSpecularmapInfo.end() != itrSpecularmapInfoInvVertexColored ?	(*itrSpecularmapInfoInvVertexColored).second.m_strImageName.c_str()		: "_black"
			//---------------------------------------------------
			);
	}
	else
	{
		gameLocal.Printf("This material is vertex-color blended. \n");
		unsigned int uiBlockSize =	idStr::Length( newAmbientBlock ) + 1 + 
			(a_arrDiffusemapInfo.end() != itrDiffusemapInfo		? (*itrDiffusemapInfo).second.m_strImageName.Length()	: idStr::Length("_black")	) * 2	+
			(a_arrDiffusemapInfo.end() != itrDiffusemapInfo		? (*itrDiffusemapInfo).second.m_strUVScale.Length()		: idStr::Length("1, 1")		) * 2	+ 
			(	a_arrBumpmapInfo.end() != itrBumpmapInfo		? (*itrBumpmapInfo).second.m_strImageName.Length()		: idStr::Length("_flat")	)		+ 
			(	a_arrBumpmapInfo.end() != itrBumpmapInfo		? (*itrBumpmapInfo).second.m_strUVScale.Length()		: idStr::Length("1, 1")		)		+ 
			(a_arrSpecularmapInfo.end() != itrSpecularmapInfo	? (*itrSpecularmapInfo).second.m_strImageName.Length()	: idStr::Length("_black")	)		+ 
			(a_arrSpecularmapInfo.end() != itrSpecularmapInfo	? (*itrSpecularmapInfo).second.m_strUVScale.Length()	: idStr::Length("1, 1")		); 

		a_arrCharNewAmbientBlock.resize( uiBlockSize, 0 );

		idStr::snPrintf( &a_arrCharNewAmbientBlock[0], uiBlockSize, newAmbientBlock, 
			a_arrDiffusemapInfo.end() != itrDiffusemapInfo ?	(*itrDiffusemapInfo).second.m_strImageName.c_str()		: "_black", 
			a_arrDiffusemapInfo.end() != itrDiffusemapInfo ?	(*itrDiffusemapInfo).second.m_strUVScale.c_str()		: "1, 1", 
			a_arrDiffusemapInfo.end() != itrDiffusemapInfo ?	(*itrDiffusemapInfo).second.m_strUVScale.c_str()		: "1, 1", 
			a_arrBumpmapInfo.end() != itrBumpmapInfo ?			(*itrBumpmapInfo).second.m_strUVScale.c_str()			: "1, 1", 
			a_arrSpecularmapInfo.end() != itrSpecularmapInfo ?	(*itrSpecularmapInfo).second.m_strUVScale.c_str()		: "1, 1", 
			a_arrBumpmapInfo.end() != itrBumpmapInfo ?			(*itrBumpmapInfo).second.m_strImageName.c_str()			: "_flat", 
			a_arrDiffusemapInfo.end() != itrDiffusemapInfo ?	(*itrDiffusemapInfo).second.m_strImageName.c_str()		: "_black", 
			a_arrSpecularmapInfo.end() != itrSpecularmapInfo ?	(*itrSpecularmapInfo).second.m_strImageName.c_str()		: "_black"
			);
	}

}

void Cmd_BatchConvertMaterials_f( const idCmdArgs& args )
{

	if( args.Argc() < 3 )
	{
		gameLocal.Printf( "Usage: tdm_batchConvertMaterials <StartIndex> <nMaterials> [forceUpdateAll] \n" );
		return;
	}

	bool bForceUpdateAllMaterials = false;
	if( args.Argc() > 3 )
	{
		 bForceUpdateAllMaterials = (0 == idStr::Icmp( args.Argv(3), "forceupdateall" ));
	}
		
	const unsigned int uiStartIndex			= atoi(args.Argv(1));
	const unsigned int uiMaterialsToProcess	= atoi(args.Argv(2));

	const unsigned long uiTotalMats = declManager->GetNumDecls( DECL_MATERIAL );

	gameLocal.Printf("Parsing %d materials, this may take few minutes...\n", uiTotalMats );

	unsigned long ulMaterialsProcessed = 0;
	unsigned long i = uiStartIndex > (uiTotalMats - 1) ? uiTotalMats : uiStartIndex;
	const unsigned uiMaxMaterialsToProcess = i + uiMaterialsToProcess;
	for ( ; i < uiTotalMats && i < uiMaxMaterialsToProcess; i++ )
	{

		idMaterial *mat = const_cast<idMaterial *>(declManager->MaterialByIndex( i ));

		// for testing
		//idMaterial *mat = const_cast<idMaterial *>(declManager->FindMaterial( "textures/base_wall/xiantex3_dark_burn" ));
	
		if( NULL == mat )
			continue;

		gameLocal.Printf("Material %s loaded. \n", mat->GetName() );

		std::vector< char > charBuffer; 

		ImageInfoMap arrDiffusemapInfo;	
		ImageInfoMap arrBumpMapInfo;	
		ImageInfoMap arrSpecularmapInfo;	

		charBuffer.resize(  mat->GetTextLength() + 1, 0 );
		mat->GetText( &charBuffer[0] );

		idLexer lexSource( &charBuffer[0], charBuffer.size(), mat->GetName(), LEXFL_NOFATALERRORS | LEXFL_ALLOWPATHNAMES );

		gameLocal.Printf("Finding out shader stages... \n" );

		bool bAreBumpmapsExtracted		=false;
		bool bAreDiffusemapsExtracted	=false;
		bool bAreSpecularmapsExtracted	=false;
		for ( int j=0; j < mat->GetNumStages(); j++ )
		{
			const shaderStage_t *pShaderStage = mat->GetStage(j);

			if( NULL == pShaderStage )
			{
// 				mat->Invalidate();
// 				mat->FreeData();			
				continue;
			}

			//GetMaterialStageInfo extracts all the stages in material of given type so don't loop again for multiple similar stages. 
			if ( bAreBumpmapsExtracted && bAreDiffusemapsExtracted && bAreSpecularmapsExtracted	)
				break;


			switch( pShaderStage->lighting )
			{
			case SL_BUMP:
				if( bAreBumpmapsExtracted )
					continue;

				gameLocal.Printf("Bumpmap stage found, extracting bumpmap information... \n" );
				GetMaterialStageInfo( "bumpmap", lexSource, arrBumpMapInfo );
				bAreBumpmapsExtracted = true;
				break;
			case SL_DIFFUSE:
				if( bAreDiffusemapsExtracted )
					continue;

				gameLocal.Printf("Diffusemap stage found, extracting diffusemap information... \n" );
				GetMaterialStageInfo( "diffusemap", lexSource, arrDiffusemapInfo );
				bAreDiffusemapsExtracted = true;
				break;
			case SL_SPECULAR:
				if( bAreSpecularmapsExtracted )
					continue;

				gameLocal.Printf("Specularmap stage found, extracting specularmap information... \n" );
				GetMaterialStageInfo( "specularmap", lexSource, arrSpecularmapInfo );
				bAreSpecularmapsExtracted = true;
				break;
			default:
				continue;
			}
		}
		gameLocal.Printf("Done. \n" );
// 		break; // remove me!!!

		if( arrBumpMapInfo.size() == 0 && arrDiffusemapInfo.size() == 0 && arrSpecularmapInfo.size() == 0 )
		{
// 			mat->Invalidate();
// 			mat->FreeData();		
			continue;
		}
		unsigned int uiBlockStartOffset, uiBlockEndOffset;
		std::vector< idStr >arrSearchWords;
		bool bIsAmbientBlockFound = false;

		// Note that, the spaces in the string:"if (global5 == 1)" and an externally modified material may not match. 
		// So avoid changing new ambient lighting blocks by hand, at least "if (global5 == 1)" part.
		arrSearchWords.clear();
		arrSearchWords.push_back("if (global5 == 1)");
		bIsAmbientBlockFound = FindBlockContainingWords( &charBuffer[0], arrSearchWords, uiBlockStartOffset, uiBlockEndOffset );
		
		gameLocal.Printf( "ForceUpdate is: %s\n", bForceUpdateAllMaterials? "true": "false" );
		if( bIsAmbientBlockFound )
		{
			gameLocal.Printf( "Found new ambient block\n" );

			gameLocal.Printf( "Removing new ambient block\n" );
			charBuffer.erase( charBuffer.begin() + uiBlockStartOffset, charBuffer.begin() + uiBlockEndOffset );

			// Try the search again with global5 == 1 in case the material is vertex color blended.
			bool bIsSecondAmbientBlockFound = FindBlockContainingWords( &charBuffer[0], arrSearchWords, uiBlockStartOffset, uiBlockEndOffset );

			if( bIsSecondAmbientBlockFound )
				charBuffer.erase( charBuffer.begin() + uiBlockStartOffset, charBuffer.begin() + uiBlockEndOffset );

			arrSearchWords.clear();
			arrSearchWords.push_back("if (global5 == 2)");
			bIsAmbientBlockFound = FindBlockContainingWords( &charBuffer[0], arrSearchWords, uiBlockStartOffset, uiBlockEndOffset );
			if( bIsAmbientBlockFound  )
				charBuffer.erase( charBuffer.begin() + uiBlockStartOffset, charBuffer.begin() + uiBlockEndOffset );
		}

		//Some materials may have old ambient block along with the new one. So remove it if found.
		{
			bool bIsOldAmbientBlockFound;
			arrSearchWords.clear();
			arrSearchWords.push_back("red");
			arrSearchWords.push_back("global2");
			bIsOldAmbientBlockFound = FindBlockContainingWords( &charBuffer[0], arrSearchWords, uiBlockStartOffset, uiBlockEndOffset );
			if( bIsOldAmbientBlockFound  )
			{
				gameLocal.Printf( "Found old ambient block\n" );
				gameLocal.Printf( "Removing old ambient block\n" );
				charBuffer.erase( charBuffer.begin() + uiBlockStartOffset, charBuffer.begin() + uiBlockEndOffset );
				bIsAmbientBlockFound = true;
			}

			// Try the search again, in case the material is vertex color blended and there is second inverse-vertex-colored ambient block.
			bIsOldAmbientBlockFound = FindBlockContainingWords( &charBuffer[0], arrSearchWords, uiBlockStartOffset, uiBlockEndOffset );

			if( bIsOldAmbientBlockFound  )
				charBuffer.erase( charBuffer.begin() + uiBlockStartOffset, charBuffer.begin() + uiBlockEndOffset );

			// If we couldn't find old ambient block and we have new ambient block in place, 
			// then we can safely skip this material.
			else if( !bForceUpdateAllMaterials && bIsAmbientBlockFound )
				continue;
		}

		
		unsigned int uiOffset = 0;

		if( bIsAmbientBlockFound )
		{
			//Remove the old comment.
			unsigned int uiCommentStart, uiCommentEnd;
			arrSearchWords.clear();
			
			arrSearchWords.push_back("TDM");
			arrSearchWords.push_back("Ambient");
			arrSearchWords.push_back("Method");
			arrSearchWords.push_back("Related");
			if ( FindBlockContainingWords( &charBuffer[0], arrSearchWords, uiCommentStart, uiCommentEnd, '\n', '\n' ) )
			{
				charBuffer.erase( charBuffer.begin() + uiCommentStart, charBuffer.begin() + uiCommentEnd );
				gameLocal.Printf( " Ambient method related comment found and removed. \n" );
			}
		}
		//------------------------------------
		int i;
		unsigned int uiEndoftheBlock = 0;
		for( i= charBuffer.size() - 1; i > 0; i-- )
		{
			if( '}' == charBuffer[i] )
			{
				uiEndoftheBlock = i--;
				// Find additional white spaces and new line characters before end of the block.
				while( '\n' == charBuffer[i] || ' ' == charBuffer[i] || '\t' == charBuffer[i] || '\r' == charBuffer[i] ) 
				{
					if(0 >= i)
						break;

					i--;
				}
				// Remove white spaces and new line characters that are found before end of the block.
				if( unsigned(i + 1) <= uiEndoftheBlock - 1 )
				{
					charBuffer.erase( charBuffer.begin() + i + 1 , charBuffer.begin() + uiEndoftheBlock - 1 );
					gameLocal.Printf( "%d trailing white spaces found at end of the block and are removed. %d, %d, %c \n", uiEndoftheBlock - i - 1, uiEndoftheBlock, i + 1, charBuffer[i+2] );
					// Update end of the block's position.
					uiEndoftheBlock = i + 2;
				}
				else
				{
					gameLocal.Printf( "No trailing white spaces found at end of the block. %c\n", charBuffer[i] );
				}
				break;
			}
		}

		idStr strMatTextWithNewBlock( &charBuffer[0] );

		if( i > 1 )
		{
			strMatTextWithNewBlock.Insert( "\n\n	// TDM Ambient Method Related ", uiEndoftheBlock - 1 );
			uiOffset = uiEndoftheBlock - 1 + idStr::Length(  "\n\n	// TDM Ambient Method Related " );
			//strMatTextWithNewBlock.Insert( '\n', uiOffset );
		}
		else
		{
			gameLocal.Warning( "Could not determine end of the material block. Skipping this material.\n" );
			// 				mat->Invalidate();
			// 				mat->FreeData();
			continue;
		}


		gameLocal.Printf( "Processing Material %s \n", mat->GetName() );

		std::vector<char> arrCharNewAmbientBlock;

		CreateNewAmbientBlock( arrDiffusemapInfo, arrBumpMapInfo, arrSpecularmapInfo, arrCharNewAmbientBlock );

		strMatTextWithNewBlock.Insert( &arrCharNewAmbientBlock[0], uiOffset );

		ulMaterialsProcessed ++;
	
		// Update the material text and save to the file.
		mat->SetText( strMatTextWithNewBlock.c_str() );

		if( !mat->Parse( strMatTextWithNewBlock.c_str(), strMatTextWithNewBlock.Length() ) )
		{
		  gameLocal.Warning( "Material %s had error in the newly inserted text %s. \n Aborting.\n", mat->GetName(), &arrCharNewAmbientBlock[0] );
		  break;
		}
		mat->ReplaceSourceFileText();
		mat->Invalidate();
		mat->FreeData();
	}
	gameLocal.Printf(" %d Materials processed and changed in total.\n", ulMaterialsProcessed );
}


void Cmd_updateCookedMathData_f( const idCmdArgs& args )
{
	r_postprocess_colorCurveBias.SetModified();
}

#ifdef TIMING_BUILD
void Cmd_ListTimers_f(const idCmdArgs& args) 
{
	PRINT_TIMERS;
}

void Cmd_WriteTimerCSV_f(const idCmdArgs& args) 
{
	if (args.Argc() == 2) // Only separator
	{
		debugtools::TimerManager::Instance().DumpTimerResults(args.Argv(1));
		return;
	}

	if (args.Argc() == 3) // separator + comma
	{
		debugtools::TimerManager::Instance().DumpTimerResults(args.Argv(1), args.Argv(2));
		return;
	}

	// No arguments, call default
	debugtools::TimerManager::Instance().DumpTimerResults();
}

void Cmd_ResetTimers_f(const idCmdArgs& args)
{
	debugtools::TimerManager::Instance().ResetTimers();
}
#endif // TIMING_BUILD 

/*
=================
idGameLocal::InitConsoleCommands

Let the system know about all of our commands
so it can perform tab completion
=================
*/
void idGameLocal::InitConsoleCommands( void ) {
	cmdSystem->AddCommand( "listTypeInfo",			ListTypeInfo_f,				CMD_FL_GAME,				"list type info" );
	cmdSystem->AddCommand( "writeGameState",		WriteGameState_f,			CMD_FL_GAME,				"write game state" );
	cmdSystem->AddCommand( "testSaveGame",			TestSaveGame_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"test a save game for a level" );
	cmdSystem->AddCommand( "game_memory",			idClass::DisplayInfo_f,		CMD_FL_GAME,				"displays game class info" );
	cmdSystem->AddCommand( "listClasses",			idClass::ListClasses_f,		CMD_FL_GAME,				"lists game classes" );
	cmdSystem->AddCommand( "listThreads",			idThread::ListThreads_f,	CMD_FL_GAME|CMD_FL_CHEAT,	"lists script threads" );
	cmdSystem->AddCommand( "listEntities",			Cmd_EntityList_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"lists game entities" );
	cmdSystem->AddCommand( "listActiveEntities",	Cmd_ActiveEntityList_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"lists active game entities" );
	cmdSystem->AddCommand( "listMonsters",			idAI::List_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"lists monsters" );
	cmdSystem->AddCommand( "listSpawnArgs",			Cmd_ListSpawnArgs_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"list the spawn args of an entity", idGameLocal::ArgCompletion_EntityName );
	cmdSystem->AddCommand( "say",					Cmd_Say_f,					CMD_FL_GAME,				"text chat" );
	cmdSystem->AddCommand( "sayTeam",				Cmd_SayTeam_f,				CMD_FL_GAME,				"team text chat" );
	cmdSystem->AddCommand( "addChatLine",			Cmd_AddChatLine_f,			CMD_FL_GAME,				"internal use - core to game chat lines" );
	cmdSystem->AddCommand( "gameKick",				Cmd_Kick_f,					CMD_FL_GAME,				"same as kick, but recognizes player names" );
	cmdSystem->AddCommand( "give",					Cmd_Give_f,					CMD_FL_GAME|CMD_FL_CHEAT,	"gives one or more items" );
	cmdSystem->AddCommand( "centerview",			Cmd_CenterView_f,			CMD_FL_GAME,				"centers the view" );
	cmdSystem->AddCommand( "god",					Cmd_God_f,					CMD_FL_GAME|CMD_FL_CHEAT,	"enables god mode" );
	cmdSystem->AddCommand( "notarget",				Cmd_Notarget_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"disables the player as a target" );
	cmdSystem->AddCommand( "noclip",				Cmd_Noclip_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"disables collision detection for the player" );
	cmdSystem->AddCommand( "kill",					Cmd_Kill_f,					CMD_FL_GAME,				"kills the player" );
	cmdSystem->AddCommand( "where",					Cmd_GetViewpos_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"prints the current view position" );
	cmdSystem->AddCommand( "getviewpos",			Cmd_GetViewpos_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"prints the current view position" );
	cmdSystem->AddCommand( "setviewpos",			Cmd_SetViewpos_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"sets the current view position" );
	cmdSystem->AddCommand( "teleport",				Cmd_Teleport_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"teleports the player to an entity location", idGameLocal::ArgCompletion_EntityName );
	cmdSystem->AddCommand( "trigger",				Cmd_Trigger_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"triggers an entity", idGameLocal::ArgCompletion_EntityName );
	cmdSystem->AddCommand( "spawn",					Cmd_Spawn_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"spawns a game entity", idCmdSystem::ArgCompletion_Decl<DECL_ENTITYDEF> );
	cmdSystem->AddCommand( "damage",				Cmd_Damage_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"apply damage to an entity", idGameLocal::ArgCompletion_EntityName );
	cmdSystem->AddCommand( "remove",				Cmd_Remove_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"removes an entity", idGameLocal::ArgCompletion_EntityName );
	cmdSystem->AddCommand( "killMonsters",			Cmd_KillMonsters_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"removes all monsters" );
	cmdSystem->AddCommand( "killMoveables",			Cmd_KillMovables_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"removes all moveables" );
	cmdSystem->AddCommand( "killRagdolls",			Cmd_KillRagdolls_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"removes all ragdolls" );
	cmdSystem->AddCommand( "addline",				Cmd_AddDebugLine_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"adds a debug line" );
	cmdSystem->AddCommand( "addarrow",				Cmd_AddDebugLine_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"adds a debug arrow" );
	cmdSystem->AddCommand( "removeline",			Cmd_RemoveDebugLine_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"removes a debug line" );
	cmdSystem->AddCommand( "blinkline",				Cmd_BlinkDebugLine_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"blinks a debug line" );
	cmdSystem->AddCommand( "listLines",				Cmd_ListDebugLines_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"lists all debug lines" );
	cmdSystem->AddCommand( "playerModel",			Cmd_PlayerModel_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"sets the given model on the player", idCmdSystem::ArgCompletion_Decl<DECL_MODELDEF> );
	cmdSystem->AddCommand( "testFx",				Cmd_TestFx_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"tests an FX system", idCmdSystem::ArgCompletion_Decl<DECL_FX> );
	cmdSystem->AddCommand( "testBoneFx",			Cmd_TestBoneFx_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"tests an FX system bound to a joint", idCmdSystem::ArgCompletion_Decl<DECL_FX> );
	cmdSystem->AddCommand( "testLight",				Cmd_TestLight_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"tests a light" );
	cmdSystem->AddCommand( "testPointLight",		Cmd_TestPointLight_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"tests a point light" );
	cmdSystem->AddCommand( "popLight",				Cmd_PopLight_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"removes the last created light" );
	cmdSystem->AddCommand( "testDeath",				Cmd_TestDeath_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"tests death" );
	cmdSystem->AddCommand( "testSave",				Cmd_TestSave_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"writes out a test savegame" );
	cmdSystem->AddCommand( "testModel",				idTestModel::TestModel_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"tests a model", idTestModel::ArgCompletion_TestModel );
	cmdSystem->AddCommand( "testSkin",				idTestModel::TestSkin_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"tests a skin on an existing testModel", idCmdSystem::ArgCompletion_Decl<DECL_SKIN> );
	cmdSystem->AddCommand( "testShaderParm",		idTestModel::TestShaderParm_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"sets a shaderParm on an existing testModel" );
	cmdSystem->AddCommand( "keepTestModel",			idTestModel::KeepTestModel_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"keeps the last test model in the game" );
	cmdSystem->AddCommand( "testAnim",				idTestModel::TestAnim_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"tests an animation", idTestModel::ArgCompletion_TestAnim );
	cmdSystem->AddCommand( "testParticleStopTime",	idTestModel::TestParticleStopTime_f,CMD_FL_GAME|CMD_FL_CHEAT,	"tests particle stop time on a test model" );
	cmdSystem->AddCommand( "nextAnim",				idTestModel::TestModelNextAnim_f,	CMD_FL_GAME|CMD_FL_CHEAT,	"shows next animation on test model" );
	cmdSystem->AddCommand( "prevAnim",				idTestModel::TestModelPrevAnim_f,	CMD_FL_GAME|CMD_FL_CHEAT,	"shows previous animation on test model" );
	cmdSystem->AddCommand( "nextFrame",				idTestModel::TestModelNextFrame_f,	CMD_FL_GAME|CMD_FL_CHEAT,	"shows next animation frame on test model" );
	cmdSystem->AddCommand( "prevFrame",				idTestModel::TestModelPrevFrame_f,	CMD_FL_GAME|CMD_FL_CHEAT,	"shows previous animation frame on test model" );
	cmdSystem->AddCommand( "testBlend",				idTestModel::TestBlend_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"tests animation blending" );
	cmdSystem->AddCommand( "reloadScript",			Cmd_ReloadScript_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"reloads scripts" );

	cmdSystem->AddCommand( "tdm_updateCookedMathData",	Cmd_updateCookedMathData_f,		CMD_FL_RENDERER,	"Updates lookup textures" );

	cmdSystem->AddCommand( "script",				Cmd_Script_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"executes a line of script" );
	cmdSystem->AddCommand( "listCollisionModels",	Cmd_ListCollisionModels_f,	CMD_FL_GAME,				"lists collision models" );
	cmdSystem->AddCommand( "collisionModelInfo",	Cmd_CollisionModelInfo_f,	CMD_FL_GAME,				"shows collision model info" );
	cmdSystem->AddCommand( "reexportmodels",		Cmd_ReexportModels_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"reexports models", ArgCompletion_DefFile );
	cmdSystem->AddCommand( "reloadanims",			Cmd_ReloadAnims_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"reloads animations" );
	cmdSystem->AddCommand( "listAnims",				Cmd_ListAnims_f,			CMD_FL_GAME,				"lists all animations" );
	cmdSystem->AddCommand( "aasStats",				Cmd_AASStats_f,				CMD_FL_GAME,				"shows AAS stats" );
	cmdSystem->AddCommand( "testDamage",			Cmd_TestDamage_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"tests a damage def", idCmdSystem::ArgCompletion_Decl<DECL_ENTITYDEF> );
	cmdSystem->AddCommand( "weaponSplat",			Cmd_WeaponSplat_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"projects a blood splat on the player weapon" );
	cmdSystem->AddCommand( "saveSelected",			Cmd_SaveSelected_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"saves the selected entity to the .map file" );
	cmdSystem->AddCommand( "deleteSelected",		Cmd_DeleteSelected_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"deletes selected entity" );
	cmdSystem->AddCommand( "saveMoveables",			Cmd_SaveMoveables_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"save all moveables to the .map file" );
	cmdSystem->AddCommand( "saveRagdolls",			Cmd_SaveRagdolls_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"save all ragdoll poses to the .map file" );
	cmdSystem->AddCommand( "bindRagdoll",			Cmd_BindRagdoll_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"binds ragdoll at the current drag position" );
	cmdSystem->AddCommand( "unbindRagdoll",			Cmd_UnbindRagdoll_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"unbinds the selected ragdoll" );
	cmdSystem->AddCommand( "saveLights",			Cmd_SaveLights_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"saves all lights to the .map file" );
	cmdSystem->AddCommand( "saveParticles",			Cmd_SaveParticles_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"saves all lights to the .map file" );
	cmdSystem->AddCommand( "clearLights",			Cmd_ClearLights_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"clears all lights" );
	cmdSystem->AddCommand( "gameError",				Cmd_GameError_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"causes a game error" );

	cmdSystem->AddCommand( "tdm_spr_testIO",		Cmd_TestSndIO_f,			CMD_FL_GAME,				"test soundprop file IO (needs a .spr file)" );
	cmdSystem->AddCommand( "tdm_ai_rel_print",		Cmd_PrintAIRelations_f,		CMD_FL_GAME,				"print the relationship matrix determining relations between AI teams." );

	cmdSystem->AddCommand( "tdm_attach_offset",		Cmd_AttachmentOffset_f,		CMD_FL_GAME,				"Set the vector offset (x y z) for an attachment on an AI you are looking at.  Usage: tdm_attach_offset <attachment index> <x> <y> <z>" );
	cmdSystem->AddCommand( "tdm_attach_rot",		Cmd_AttachmentRot_f,		CMD_FL_GAME,				"Set the rotation (pitch yaw roll) for an attachment on an AI you are looking at.  Usage: tdm_attach_rot <atachment index> <pitch> <yaw> <roll>  (NOTE: Rotation is applied before translation, angles are relative to the joint orientation)" );

	cmdSystem->AddCommand( "inventory_hotkey",		Cmd_InventoryHotkey_f,		CMD_FL_GAME,				"Usage: inventory_hotkey [item]\nSelects an item from the currently available inventory. If 'item' is omitted, it will return the current item's hotkey name, if any." );
	cmdSystem->AddCommand( "inventory_use",			Cmd_InventoryUse_f,			CMD_FL_GAME,				"Usage: inventory_use [item]\nUses an item in the currently available inventory without selectign it. If 'item' is omitted, it will use the currently selected item." );
	cmdSystem->AddCommand( "inventory_cycle_maps",	Cmd_InventoryCycleMaps_f,	CMD_FL_GAME,				"Usage: Bind a key to this command to cycle through the inventory maps." );
	cmdSystem->AddCommand( "inventory_cycle_group",	Cmd_InventoryCycleGroup_f,	CMD_FL_GAME,				"Usage: Bind a key to this command to cycle through the specified inventory group." );

	cmdSystem->AddCommand( "reloadXData",			Cmd_ReloadXData_f,			CMD_FL_GAME,				"Reloads the xdata declarations and refreshes all readables." );

	cmdSystem->AddCommand( "aas_showWalkPath",		Cmd_ShowWalkPath_f,			CMD_FL_GAME,				"Shows the walk path from the player to the given area number (AAS32)." );
	cmdSystem->AddCommand( "aas_showReachabilities",Cmd_ShowReachabilities_f,			CMD_FL_GAME,				"Shows the reachabilities for the given area number (AAS32)." );
	cmdSystem->AddCommand( "aas_showStats",			Cmd_ShowAASStats_f,			CMD_FL_GAME,				"Shows the AAS statistics." );
	cmdSystem->AddCommand( "eas_showRoute",			Cmd_ShowEASRoute_f,			CMD_FL_GAME,				"Shows the EAS route to the goal area." );

	cmdSystem->AddCommand( "tdm_start_conversation",	Cmd_StartConversation_f,	CMD_FL_GAME,			"Starts the conversation with the given name." );
	cmdSystem->AddCommand( "tdm_list_conversations",	Cmd_ListConversations_f,	CMD_FL_GAME,			"List all available conversations by name." );

	cmdSystem->AddCommand( "tdm_show_loot",			Cmd_ShowLoot_f,	CMD_FL_GAME|CMD_FL_CHEAT,			"Highlight all loot items in the map." );

	cmdSystem->AddCommand( "tdm_activatelogclass",		Cmd_ActivateLog_f,			CMD_FL_GAME,	"Activates a specific log class during run-time (as defined in darkmod.ini)", CGlobal::ArgCompletion_LogClasses );
	cmdSystem->AddCommand( "tdm_deactivatelogclass",	Cmd_DeactivateLog_f,		CMD_FL_GAME,	"De-activates a specific log class during run-time (as defined in darkmod.ini)", CGlobal::ArgCompletion_LogClasses );
	cmdSystem->AddCommand( "tdm_batchConvertMaterials",	Cmd_BatchConvertMaterials_f,	CMD_FL_GAME,	"Converts specified number of materials to support new ambient lighting" );

	cmdSystem->AddCommand( "tdm_restart_gui_update_objectives", Cmd_RestartGuiCmd_UpdateObjectives_f, CMD_FL_GAME, "Don't use. Reserved for internal use to dispatch restart GUI commands to the local game instance.");

	cmdSystem->AddCommand( "tdm_list_missions", Cmd_ListMissions_f, CMD_FL_GAME, "Lists all available missions.");
	cmdSystem->AddCommand( "tdm_set_mission_completed", Cmd_SetMissionCompleted_f, CMD_FL_GAME, "Sets or clears the 'completed' flag of a named mission.");

	cmdSystem->AddCommand( "tdm_end_mission", Cmd_EndMission_f, CMD_FL_GAME, "Ends this mission and proceeds to the next.");

#ifndef	ID_DEMO_BUILD
	cmdSystem->AddCommand( "disasmScript",			Cmd_DisasmScript_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"disassembles script" );
	cmdSystem->AddCommand( "recordViewNotes",		Cmd_RecordViewNotes_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"record the current view position with notes" );
	cmdSystem->AddCommand( "showViewNotes",			Cmd_ShowViewNotes_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"show any view notes for the current map, successive calls will cycle to the next note" );
	cmdSystem->AddCommand( "closeViewNotes",		Cmd_CloseViewNotes_f,		CMD_FL_GAME|CMD_FL_CHEAT,	"close the view showing any notes for this map" );
	cmdSystem->AddCommand( "exportmodels",			Cmd_ExportModels_f,			CMD_FL_GAME|CMD_FL_CHEAT,	"exports models", ArgCompletion_DefFile );

	// multiplayer client commands ( replaces old impulses stuff )
	cmdSystem->AddCommand( "clientDropWeapon",		idMultiplayerGame::DropWeapon_f, CMD_FL_GAME,			"drop current weapon" );
	cmdSystem->AddCommand( "clientMessageMode",		idMultiplayerGame::MessageMode_f, CMD_FL_GAME,			"ingame gui message mode" );
	// FIXME: implement
//	cmdSystem->AddCommand( "clientVote",			idMultiplayerGame::Vote_f,	CMD_FL_GAME,				"cast your vote: clientVote yes | no" );
//	cmdSystem->AddCommand( "clientCallVote",		idMultiplayerGame::CallVote_f,	CMD_FL_GAME,			"call a vote: clientCallVote si_.. proposed_value" );
	cmdSystem->AddCommand( "clientVoiceChat",		idMultiplayerGame::VoiceChat_f,	CMD_FL_GAME,			"voice chats: clientVoiceChat <sound shader>" );
	cmdSystem->AddCommand( "clientVoiceChatTeam",	idMultiplayerGame::VoiceChatTeam_f,	CMD_FL_GAME,		"team voice chats: clientVoiceChat <sound shader>" );

	// multiplayer server commands
	cmdSystem->AddCommand( "serverMapRestart",		idGameLocal::MapRestart_f,	CMD_FL_GAME,				"restart the current game" );
	cmdSystem->AddCommand( "serverForceReady",	idMultiplayerGame::ForceReady_f,CMD_FL_GAME,				"force all players ready" );
	cmdSystem->AddCommand( "serverNextMap",			idGameLocal::NextMap_f,		CMD_FL_GAME,				"change to the next map" );
#endif

	// greebo: Added commands to alter the clipmask/contents of entities.
	cmdSystem->AddCommand( "setClipMask",			Cmd_SetClipMask,			CMD_FL_GAME,				"Set the clipmask of the target entity, usage: 'setClipMask crate01 1313'", idGameLocal::ArgCompletion_EntityName);
	cmdSystem->AddCommand( "setClipContents",		Cmd_SetClipContents,		CMD_FL_GAME,				"Set the contents flags of the target entity, usage: 'setClipContents crate01 1313'", idGameLocal::ArgCompletion_EntityName);

	// localization help commands
	cmdSystem->AddCommand( "nextGUI",				Cmd_NextGUI_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"teleport the player to the next func_static with a gui" );
	cmdSystem->AddCommand( "testid",				Cmd_TestId_f,				CMD_FL_GAME|CMD_FL_CHEAT,	"output the string for the specified id." );
#ifdef TIMING_BUILD
	cmdSystem->AddCommand( "listTimers",			Cmd_ListTimers_f,			CMD_FL_GAME,				"Shows total run time and max time of timers (TIMING_BUILD only)." );
	cmdSystem->AddCommand( "writeTimerCSV",			Cmd_WriteTimerCSV_f,		CMD_FL_GAME,				"Writes the timer data to a csv file (usage: writeTimerCSV <separator> <commaChar>). The default separator is ';', the default comma is '.'");
	cmdSystem->AddCommand( "resetTimers",			Cmd_ResetTimers_f,			CMD_FL_GAME,				"Resets the timer data so far.");
#endif
}

/*
=================
idGameLocal::ShutdownConsoleCommands
=================
*/
void idGameLocal::ShutdownConsoleCommands( void ) {
	cmdSystem->RemoveFlaggedCommands( CMD_FL_GAME );
}
