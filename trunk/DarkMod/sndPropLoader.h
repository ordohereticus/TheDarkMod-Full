/******************************************************************************/
/*                                                                            */
/*         Dark Mod Sound Propagation (C) by Chris Sarantos in USA 2005		  */
/*                          All rights reserved                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************
*
* DESCRIPTION: Sound propagation class for propagating suspicious sounds
* to AI Used on map initialization.
*
* This class will be only for map loading and generation of the Loss Matrix.  
* Actual gameplay functions will be handled by another class, CsndProp, for 
* ease of maintenance.
*
*****************************************************************************/

/******************************************************************************
 *
 * PROJECT: DarkMod
 * $Source$
 * $Revision: 129 $
 * $Date: 2005-07-04 20:46:31 -0400 (Mon, 04 Jul 2005) $
 * $Author: ishtvan $
 * $Name$
 ******************************************************************************/


#ifndef SNDPROPLOADER_H
#define SNDPROPLOADER_H

#include "../idlib/precompiled.h"
#include "DarkModGlobals.h"

template <class type> 
class CMatRUT;

/**
* Global soundprop settings
* Populated from entityDef soundprop_globals
**/
typedef struct SsndPGlobals_s
{
	// names
	const char *AreaPropName; // classname of area properties entity
	const char *doorName; // classname of darkmod door entites
	const char *d3DoorName; // classname of D3 doors ("func_door")
	const char *fileExt; // soundprop file extension
	
	// Map compiling:
	// Maximum number of paths to calculate for a given loss matrix element
	int MaxPaths;

	/**
	* Amount to expand the bounding box of a door model in its thinnest dimension
	* when checking for portals in doors.
	**/
	float DoorExpand;

	/**************
	* The following settings effect both gameplay and compiling:
	* They are not set const so they may be changed dynamically,
	* but keep in mind the compiled soundprop file for the map
	* will NOT change until recompiled.
	***************/

	float Falloff_Outd; // outdoor sound falloff, multiplied by log(dist^2)
	float Falloff_Ind; // indoor sound falloff, multiplied by log(dist^2)
	
	/**
	* Kappa0 is the decay constant [db/M] for a sound wave in air
    * In reality this depends on freqeuncy, humidity, etc.
    * But the effet is small compared to geometric spreading, (~1%)
	* So we assume it's a constant.
	**/
	float kappa0;

	/*************
	* The following settings effect only gameplay, and can
	* safely be linked to cVars if we want.
	**************/

	/**
	* If a func_door is not a func_darkmod_door, this is the default
	* loss that will be applied when sound propagates thru the door.
	**/
	float DefaultDoorLoss;

	// AI settings:
	
	/**
	* Default threshold of hearing for an AI (SPL)
	* This corresponds to the minimum audible loudness before
	* an AI starts getting alerted by a sound.
	* (Think of leaves rustling or a very soft whisper)
	* In reality this is about 20dB SPL for the average human
	**/
	float DefaultThreshold;

	/**
	* AI Volume: All sounds propagated to AI will be scaled
	* by this amount.  This is intended for gameplay balance,
	* and should be linked to a cVar for realtime testing.
	* This is in dB, so the default value is zero.
	**/
	float Vol;

	
	float MaxRange; // Range in meters above which sound propagation is not calculated
	float MaxRangeCalVol; // volume value to normalize by for the max soundrange (in dB)
	// NOT YET IMPLEMENTED:
	float MaxEnvRange; // Range in which environmental sounds are heard by AI

	bool bDebug;

} SsndPGlobals;

//TODO: Move this stuff to documentation

/** 
* NOTE: MaxRange is the maximum distance (in meters) that a sound with
* volume equal to MaxRangeCalVol will be propagated.
* At the default values, these settings are calibrated so that the cutoff
* for a whisper (30dB SWL) is 2 meters (~6 ft)
*
* Formula for cutoff range:
* For a given volume vol0,
* Max Range is calculate with the following "empirical" formula,loosely based on
* the fact that sounds sound twice as loud when they differ by 10 dB
*
* range = pow(2, ((vol0 - MaxRangeCalVol) / 7) ) * MaxRange
*
* The penalty for overestimated the cutoff range is more AI included in the computation
* The penalty for underestimating is AI not hearing a sound that they should have heard.
* Ideally, you want the maxrange to be just a few meters past where the sound falls
* below the AI's threshold volume.
**/

/**
* ADDITIONAL NOTES:
* The formula for calculating how much an AI is alerted from a sound is:
*
* psychLoud [in "alert" units] = 1 + (propParms->loudness - threshold) 
*
* An alert unit of 1 is a barely heard sound (whisper) heard for ~100mS
* An alert unit of 10 corresponds to hearing twice as loud a sound.
**/


/**
* Structure for containing special area properties
* lossMultiplier defaults to 1.0 if not set.
**/
typedef struct SAreaProp_s
{
	int area; // number of the area, for list lookup
	float LossMult; // loss multiplier
	bool SpherSpread; // if TRUE, sound spreads spherically in this area.  Otherwise cyllindrical spreading is assumed
} SAreaProp;

/**
* SsndPortal and SSndArea are structures for intermediate data
* that is stored when compiling sound propagation info from a mapfile
**/

typedef struct SsndPortal_s
{
	qhandle_t handle; // portal handle
	int portalNum; // integer ID of the portal in the area
	int from; // area the portal is in
	int to; // area the portal goes to
	idVec3 center; // coordinates of the center of the portal
	char *doorName; // name of door entity associated with portal, if any
} SsndPortal;

typedef SsndPortal* sndPortalPtr;

typedef struct SsndArea_s 
{
	float				LossMult; // loss multiplier (in dB/meter, less than 1 => less loss)
	bool				SpherSpread; // if TRUE, sound spreads spherically in this area.  Otherwise cyllindrical spreading is assumed
	int					numPortals;	// number of portals in this area
	idVec3				center; // approximate center of the area
	SsndPortal_s		*portals;	// array containing the portals of this area
} SsndArea;

typedef SsndArea* sndAreaPtr; 

/**
* Structure for linking doors to portals
**/
typedef struct SDoorRef_s
{
	int				area; // area the door is in
	int				portalNum; // portal ID of the portal in the area
	const char *	doorName; //string name of the door
	qhandle_t		portalH; //handle to the portal that corresponds to the door name
}SDoorRef;

/**
* Structure for referencing door names to door IDs
**/

typedef struct SDNEntry_s
{
	idStr		name;
	int			doorID;
}SDNEntry;

/**
* Structure for an entry in the Loss Matrix that will be used during gameplay
**/

typedef struct SPropPath_s
{
	float			loss; // total path loss in dB
	idVec3			start; // coordinates of the center of first portal in path
	idVec3			end; // same for last portal in path
	int				numDoors; // size of door array
	int *			doors; // array of doorIDs (doorID is the door's index in m_doorRef list)
}SPropPath;

typedef struct SLMEntry_s
{
	int				from, to; // the 2 areas we are propagating to/from
	int				numPaths; // size of path array
	SPropPath *		paths; // sound paths.  Dynamic array for now. Consider idList
}SLMEntry;

/**
* CLASS DESCRIPTION: CsndPropBase has functions and members
* inherited by both CsndPropLoader and CsndProp (the gameplay class)
**/

class CsndPropBase {

public:
	CsndPropBase( void ) {}
	virtual ~CsndPropBase( void ) {}

	/**
	* Structure containing global sound properties.
	**/
	SsndPGlobals		m_SndGlobals;

	/**
	* Fill the global properties structure from the
	* soundpropglobals entityDef (defined in soundprop.def.
	* This could probably be put on CsndPropLoader instead of 
	* the base class, but just in case CsndProp needs it, it's here.
	*
	* Logs a warning and sets defaults if it cannot find the def
	* for soundprop_globals.
	**/
	void GlobalsFromDef( void );


protected:

	/**
	* Return a pointer to the loss matrix entry for given areas
	* if row > col, they will be reversed to get the entry, and the 
	* start/finish coords of the SLMEntry will also be reversed.
	**/
	SLMEntry *GetLM(int row, int col, bool *reversed=NULL);

	/**
	* Updates any global sound properties that are linked 
	* to console vars or other realtime vars
	**/
	void UpdateGlobals( void );

	/**
	* Set the globals to some default values if the soundprop_globals
	* def is missing.
	**/
	void DefaultGlobals( void );

protected:

	/**
	* This gets set to TRUE when a .spr file is successfully loaded
	* for the current map.
	**/
	bool				m_bLoadSuccess;

	/**
	* If set to true, the default sound model will be indoor propagation
	* This bool is read from the worldspawn entity, and defaults to false
	**/
	bool				m_bDefaultSpherical;


	/********************************************************************
	* GAMEPLAY MEMBERS
	* Members after this point must be passed along to be used in gameplay!
	*********************************************************************/
	
	/**
	* m_LossMatrix is the Area to Area Loss Matrix for the map
	* This is the matrix used in gameplay to actually propagate sounds
	* It is a right upper triangular (RUT) matrix with zero diagonals
	**/
	CMatRUT<SLMEntry>	*m_LossMatrix;

	/**
	* m_AreaPropsG contains the area properties of ALL areas for use
	* during propagation.
	* defaults are loss multiplier = 1.0, and spherical spreading = false.
	**/
	idList<SAreaProp>	 m_AreaPropsG;

};


/**
* CLASS DESCRIPTION: CsndPropLoader class.  Handles parsing of mapfile
* into sound prop data, pathfinding to create loss matrix.  Also handles
* loading of existing sound propagation files (.spr) for a given map,
* and writing of these files after the mapfile compilation is done.
* 
**/

class CsndPropLoader : public CsndPropBase {
public:

	friend class CsndProp;

	CsndPropLoader ( void );
	~CsndPropLoader ( void );

	/**
	* Load sound prop system from a loaded mapfile.
	* This does the actual parsing of the map file and will do
	* the generating of the Loss Matrix, as opposed to LoadSprFile
	* which just loads the file for the map.
	* For now, this is run in game_local every map load as a test
	* When the actual system is in place, we must check to see if the map
	* has an existing .spr file, and load that instead of running this.
	* 
	* This should probably be renamed to CompileMap. This is what we
	* call from the editor console to compile the .spr AFTER compiling
	* the map with dmap.  
	*
	* Also, this function will have to load the mapfile
	* idMapFile pointer for the appropriate map itself instead of relying on
	* something else to pass it.
	**/
	void CompileMap( idMapFile *MapFile );

	/**
	* Read the loss matrix, door hash and area losses from
	* a sound propagation file associated with a map name (.spr?)
	* calls common->Error if the sound file is not found.
	**/
	bool LoadSprFile (const char *pFN);

	/**
	* Write the loss matrix and door hash to a sound prop file.
	* MapFN is the filename of the associated map.  MapTS is the
	* timestamp of the associated map file for verification purposes.
	* if fromBasePath is true, file will be written to Doom3 directory
	* TODO: Write the file to maps directory by default
	**/
	bool WriteSprFile (const char *MapFN, bool fromBasePath, unsigned mapTS = 0 );
	
	/**
	* Destroy sound prop data when switching to a new map, game ends, etc
	**/
	void Shutdown( void );

	/**
	* Test the file IO system by reading in a mapfile and outputting a different name
	* Right now the output name is defined in the function.
	* TODO: pass the output name as a 2nd argument instead of defining it in the function.
	**/
	void testReadWrite( const char *MapFN );

private:
	
	/**
	* The following functions go through the mapfile and identify map entities
	* that are associated with sound prop. ParseMapEntities calls ParseWorldSpawn,
	* ParseAreapropEnt and ParseDoor.
	**/

	void ParseMapEntities ( idMapFile *MapFile );

	/**
	* ParseWorldSpawn gets map-wide sound information from the worldspawn entity
	* This information includes whether the default sound prop model should be
	* indoor or outdoor (whether the map is predominantly indoor or outdoor)
	**/
	void ParseWorldSpawn ( idDict args );

	/**
	* Area property entities are parsed to add their properties to 
	* the area properties array.
	**/
	void ParseAreaPropEnt ( idDict args );

	/**
	* ParseMapDoor 
	* Takes spawn args of the door and the bounds of the door,
	* adds it to sound propagation data if the door contains a portal
	* and if the bounds of the door are touching at least two areas.
	*
	* Doors are ignored if the door bounds do not contain a visportal,
	* or if the door bounds contact less than or greater than two areas.
	*
	* Due to the varying placement of "origins" within doors with models,
	* some doors with models that should be parsed are skipped.
	* 
	* Check the sound prop map compile output to see which doors were ignored.
	*
	* To counteract this error, one can tweak the sound prop variable
	* s_DoorExpand.  This expands the bounds of the door in their thinnest
	* direction.  Default is 100% expansion (corresponding to a value of 1.0)
	* 
	* Keep in mind if door bounds expand TOO much, they can touch more than
	* 1 portal, and soundprop will ignore them for that reason.
	**/

	bool ParseMapDoor ( idDict args, idBounds *b );
	
	/**
	* Searches the provided area number for the portal handle pHandle.
	* Then returns the integer index of the portal within the portal list 
	* of that area. (As defined in gameRenderWorld->GetPortal(area num,portal num))
	* Returns -1 if the portal is not found.
	**/

	int  FindSndPortal( int area, qhandle_t pHandle );

	/**
	* Finds the bounds of a map entity and writes it to bounds argument.
	* 
	* If the map entity has a model, the bounds of the collision model are returned.
	* If no collision model is available for the given model, the rendermodel
	* is used as the collision model.  This is usually not good since rendermodels can
	* be high poly.
	* 
	* If the map entity has no model, the first primitive brush found is used to
	* generate a bounding box.  Primitive patches are ignored.
	*
	* This function is currently only used for finding the bounds of doors
	* to check if a given door contains a portal.
	**/

	bool MapEntBounds( idBounds &bounds, idMapEntity *mapEnt );
	
	/**
	* Helper function : TODO : move to a more general library, perhaps idBounds
	* Expand the bounds only in the direction that they are the thinnest.
	* Expands by percent * the thinnest width of the box (note 1.0 = 100%)
	**/
	void ExpandBoundsMinAxis( idBounds *bounds, float percent );
	
	/**
	* Create and destroy the Areas array that stores which portals connect which areas
	* as well as area sound loss multipliers.
	* CreateAreasData must be run AFTER ParseLossEntities.
	**/
	void CreateAreasData ( void );
	void DestroyAreasData( void );

	/**
	* Fill the m_AreaPropsG array from the m_AreaProps array.
	* Default loss multiplier = 1.0, default sound model = indoor
	**/
	void FillAPGfromAP ( int numAreas );

	/**
	* Write the loss matrix entry i,j
	* Returns false if it encounters a bad matrix element
	**/
	bool WriteLMEntry(idFile *fp, int i, int j);
	
	/**
	* Write loss matrix path entry
	**/
	bool WriteLMPath( idFile *fp, SLMEntry *Entry, int pathNum );

	/**
	* Helper functions used by LoadSprFile
	**/

	bool ParsePath( idLexer *src, int pathNum, int *indices, SLMEntry *pLMEntry );
	bool ParseDoorTable( idLexer *src );

protected:

	/************************************************************
	* PRE-GAMEPLAY MEMBERS
	* These members are only used by the pre-gameplay object and don't need
	* to be copied to the gameplay object.
	***********************************************************/
	
	/**
	* Area and portal connectivity database
	* Intermediate data that gets destroyed when Loss Matrix is created.
	**/
	SsndArea			*m_sndAreas;

	/**
	* DoorRefs References door name strings to portals
	* Intermediate data that gets destroyed
	**/
	idList <SDoorRef>	 m_DoorRefs;

	/**
	* DoorName hash References door name strings to doorIDs
	* This data must be kept around in case the map is restarted
	* at a different difficulty level, or entity spawns otherwise change
	* it is stored in the file .spr file and may be deleted during a single
	* map run (after it is used to create m_DoorIDHash), then reloaded on
	* map restart to save RAM.
	**/
	idList<SDNEntry>	m_DoorNameTable;

	/**
	* List of area properties.
	* Only contains areas with non-default properties.
	**/
	idList<SAreaProp>	m_AreaProps;

	/**
	* Count of the number of areas in a map
	**/
	int					m_numAreas;

};

#endif