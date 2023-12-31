/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4772 $
 * $Date: 2011-04-11 02:18:42 -0400 (Mon, 11 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/
#ifndef __LIGHTGEM_H__
#define __LIGHTGEM_H__

//----------------------------------
// Preprocessor Constants
//----------------------------------
// Number of passes that we can do at most. This is 6 because it's simply a cube that is rendered 
// from all sides. This is not needed though, because a top and a bottom render with a pyramidic
// shape would be sufficient to cover all lighting situations. For silouhette detection we might
// consider more stages though.
#define DARKMOD_LG_MAX_RENDERPASSES			2
#define DARKMOD_LG_MAX_IMAGESPLIT			4
#define DARKMOD_LG_RENDER_MODEL				"models/darkmod/misc/system/lightgem.lwo"
#define DARKMOD_LG_ENTITY_NAME				"lightgem_surface"
// The lightgem viewid defines the viewid that is to be used for the lightgem surfacetestmodel
#define DARKMOD_LG_VIEWID					-1
#define DARKMOD_LG_RENDER_WIDTH				50

#define DARKMOD_LG_FILENAME					"TDM_lightgem"
// The colour is converted to a grayscale value which determines the state
// of the lightgem.
// LightGem = (0.29900*R+0.58700*G+0.11400*B) * 0.0625

#define DARKMOD_LG_MIN						1
#define DARKMOD_LG_MAX						32
#define DARKMOD_LG_FRACTION					(1.0f/32.0f)
#define DARKMOD_LG_RED						0.29900f
#define DARKMOD_LG_GREEN					0.58700f
#define DARKMOD_LG_BLUE						0.11400f
#define DARKMOD_LG_SCALE					(1.0/255.0)			// scaling factor for grayscale value

//----------------------------------
// Class Declarations.
//----------------------------------

class LightGem
{
private:
	int						m_LightgemShotSpot;
	float					m_LightgemShotValue[DARKMOD_LG_MAX_RENDERPASSES];
	idEntityPtr<idEntity>	m_LightgemSurface;

	// stgatilov: The buffer for captured lightgem image
	idList<char>			m_LightgemRenderBuffer;

public:
	ID_INLINE idList<char> &	GetLightgemRenderBuffer ( void ) { return m_LightgemRenderBuffer; }
	
	//---------------------------------
	// Construction/Destruction
	//---------------------------------
	LightGem	();
	~LightGem	();

	//---------------------------------
	// Initialization
	//---------------------------------
	void Initialize		();
	void Deinitialize	();
	void Clear			();

	//---------------------------------
	// Persistence
	//---------------------------------
	void Save			( idSaveGame &		a_saveGame );
	void Restore		( idRestoreGame &	a_savedGame );
	//---------------------------------
	
	//---------------------------------
	// SpawnlightgemEntity will create exactly one lightgem entity for the map and ensures
	//  that no multiple copies of it will exist.
	//---------------------------------
	void SpawnLightGemEntity( idMapFile *	a_mapFile );
	void InitializeLightGemEntity(); 

	//---------------------------------
	// Calculation
	//---------------------------------
	float	Calculate		( idPlayer *	a_pPlayer );

private:
	void AnalyzeRenderImage	(float fColVal[DARKMOD_LG_MAX_IMAGESPLIT]);
};

#endif // __LIGHTGEM_H__