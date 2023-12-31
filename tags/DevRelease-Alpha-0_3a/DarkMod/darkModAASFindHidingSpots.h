/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 462 $
 * $Date: 2006-06-16 16:26:15 -0400 (Fri, 16 Jun 2006) $
 * $Author: sophisticatedzombie $
 *
 * $Log$
 * Revision 1.8  2006/06/16 20:26:15  sophisticatedzombie
 * no message
 *
 * Revision 1.7  2006/05/26 04:43:17  sophisticatedzombie
 * The search for hiding spots has been broken into two functions:
 * startSearchForHidingSpots and continueSearchForHidingSpots.  The hiding spot search object remembers its state so it can pick up where it left off on the next continueSearchForHidingSpots call.
 *
 * Revision 1.6  2006/05/25 02:30:12  sophisticatedzombie
 * Hiding spot search is setup to be accessible from a static member function for thread function compatibility. But, we don't have an os independent threading library so we won't be doing that for now.
 *
 * Revision 1.5  2006/05/19 19:56:50  sparhawk
 * CVSHeader added
 *
 *
 ***************************************************************************/

#pragma once

// Required includes
#include "..\game\ai\aas.h"
#include "..\game\pvs.h"
#include "..\game\Game_local.h"
#include "..\game\Entity.h"
#include "..\darkMod\PVSToAASMapping.h"

/*!
* This defines hiding spot characteristics as bit flags
*/
typedef enum darkModHidingSpotType
{
	NONE_HIDING_SPOT_TYPE				= 0x00,				
	PVS_AREA_HIDING_SPOT_TYPE			= 0x01,				// Hidden by the world geometry (ie, around a corner, through a closed portal)
	DARKNESS_HIDING_SPOT_TYPE			= 0x02,				// Hidden by darkness
	VISUAL_OCCLUSION_HIDING_SPOT_TYPE	= 0x04,				// Hidden by visual occlusions (objects, entities, water, etc..)
	ANY_HIDING_SPOT_TYPE				= 0xFFFFFFFF		// Utility combination value

};


/*!
// @structure darkModHidingSpot_t
// @author SophisticatedAZombie (DMH)
// This structure holds information about a hiding spot.
*/
typedef struct darkModHidingSpot
{
	aasGoal_t goal;

	// The hiding spot characteristic bit flags
	// as defined by the darkModHidingSpotType enumeration
	int hidingSpotTypes;

	// The hiding spot "hidingness" quality, from 0 to 1.0
	float quality;

} darkModHidingSpot_t;

/*!
* This enumeration is used to track the hiding spot search
* state so that the search can be broken up among a sequence
* of function calls on different AI frames.
*
*/
typedef enum
{
	buildingPVSList_searchState,
	newPVSArea_searchState,
	iteratingVisibleAASAreas_searchState,
	iteratingNonVisibleAASAreas_searchState,
	testingInsideVisibleAASArea_searchState,
	done_searchState
} TDarkModHidingSpotSearchState;

/*!
// @class darkmodAASFindHidingSpots
// @author SophisticatedZombie (DMH)
// 
// This class acts similarly to an AAS goal locator, but does not use the
// AAS callback scheme due to its single goal response system.
// Rathewr, this creates a list of goals.  A goal is simply a place within 
// the map. In this case, the goals found are potential hiding spots.
//
// This can be used both for hiding and searching behavior.
// - An actor wishing to hide can evaluate hiding spots and choose one.
// - An actor looking for something hiding can evaluate hiding
//  spots in determining how to proceed.
// 
//
*/
class darkModAASFindHidingSpots
{
protected:


	// The search state
	TDarkModHidingSpotSearchState searchState;

	// The position from which the entity may be hiding
	idVec3 hideFromPosition;

	// Local PVS area list used to hold the areas being hidden FROM
	int	hideFromPVSAreas[ idEntity::MAX_PVS_AREAS ];
	int numHideFromPVSAreas;

	// The handle to the PVS system describing the PVS areas we are hiding from
	pvsHandle_t h_hideFromPVS;

	// PVS areas we need to test as good hiding spots
	int numPVSAreas;
	int	PVSAreas[ idEntity::MAX_PVS_AREAS ];

	// The number of PVS areas iterated so far
	int numPVSAreasIterated;

	// The list of aas areas in the current pvs area
	idList<int> aasAreaIndices;

	// The number of aas areas in the current pvs area already searched
	int numAASAreaIndicesSearched;


	/* These are set when the background thread is created */
	idAAS *p_aas;
	float hidingHeight;
	idBounds searchLimits;
	idVec3 searchCenter;
	float searchRadius;
	int hidingSpotTypesAllowed;
	idEntity* p_ignoreEntity;

	// These variables are for doing a gridded sweep of a visible AAS area
	// for lighting and occlusion tests
	int currentGridSearchAASAreaNum;
	idBounds currentGridSearchBounds;
	idVec3 currentGridSearchBoundMins;
	idVec3 currentGridSearchBoundMaxes;
	idVec3 currentGridSearchPoint;
	//idVec3 currentareaCenter = aas->AreaCenter (AASAreaNum);
	



	/*
	* This internal method is used for finding hiding spots within an area that
	* is visible from the hideFromPosition.
	*
	* @param inout_hidingSpots The list of hiding spots to which found spots will be added
	* @param hidingHeight Height of the hiding object in world units
	* @param aas Pointer to the Area Awareness System in use
	* @param AASAreaNum The index of the AAS area being tested for hiding spots
	* @param searchLimits The limiting bounds which may be smaller or greater than the area being 
			searched.  The searched region is the intersection of the two.
	* @param hidingSpotTypesAllowed The types of hiding spot characteristics for which we should test
	* @param p_ignoreEntity An entity that should be ignored for testing visual occlusions (usually the self)
	*/
	void FindHidingSpotsInVisibleAASArea
	(
		idList<darkModHidingSpot_t>& inout_hidingSpots, 
		float hidingHeight,
		const idAAS* aas, 
		int AASAreaNum, 
		idBounds searchLimits, 
		int hidingSpotTypesAllowed, 
		idEntity* p_ignoreEntity
	);

	/*!
	* This internal method is used to test if a visible point would make a good hiding
	* spot due to visibility occlusion or lighting.  Hiding spot quality is reduced by being
	* further from the center of the search.
	*
	* If it is a good hiding point, it is added to out_areaHidingSpots with some
	* flags set for why it is a good hiding spot.
	*
	* @param testPoint The point to be tested for hiding spot characteristics
	* @param searchCenter The center of the search area.
	* @param searchRadius The radius of the search area
	* @param hidingHeight The height in the z plane above the test point taken up by the hider's height
	* @param hidingSpotTypesAllowed The types of hiding spot characteristics for which we should test
	* @param p_ignoreEntity An entity that should be ignored for testing visual occlusions (usually the self)
	* @param out_quality Returns the quality of any hiding spot found as a ratio from 0.0 to 1.0 where 1.0 is perfect.
	*
	* @return An integer with the bit flags for the allowed hiding spot characteristics
	*   that were found to be true
	*/
	int TestHidingPoint 
	(
		idVec3 testPoint,
		idVec3 searchCenter,
		float searchRadius,
		float hidingHeight,
		int hidingSpotTypesAllowed, 
		idEntity* p_ignoreEntity,
		float& out_quality
	);

	/*!
	* The following static variables are used for rendering a debug display of
	* hiding spot find results
	*/
	static idList<darkModHidingSpot_t> DebugDrawList;


	/*!
	* This method combines hiding spots which are in the same area
	* and have the same qualities 
	*/
	static void CombineRedundantHidingSpots
	(
		idList<darkModHidingSpot_t>& inout_hidingSpots,
		float distanceAtWhichToCombine
	);

	/*!
	* This method sorts the hiding spots by quality.
	*/
	static void sortByQuality
	(
		idList<darkModHidingSpot_t>& inout_hidingSpots
	);


	/*!
	* This method inserts a hiding spot into the given list
	* and keeps the list sorted from highest to lowest
	* quality, assuming that it was already sorted or empty.
	*/
	static void insertHidingSpotWithQualitySorting
	(
		darkModHidingSpot_t& hidingSpot,
		idList<darkModHidingSpot_t>& inout_hidingSpots
	);

	/*!
	* These methods handle the search when it is being picked back
	* up in a particular state
	*
	* @return true if there is mor search to go
	* @return false if end of search was reached
	*
	*/
	bool testNewPVSArea 
	(
		idList<darkModHidingSpot_t>& inout_hidingSpots,
		int numPointsToTestThisPass,
		int& inout_numPointsTestedThisPass
	);
	bool testingAASAreas_InNonVisiblePVSArea
	(
		idList<darkModHidingSpot_t>& inout_hidingSpots,
		int numPointsToTestThisPass,
		int& inout_numPointsTestedThisPass
	);
	bool testingAASAreas_InVisiblePVSArea
	(
		idList<darkModHidingSpot_t>& inout_hidingSpots,
		int numPointsToTestThisPass,
		int& inout_numPointsTestedThisPass
	);
	bool testingInsideVisibleAASArea
	(
		idList<darkModHidingSpot_t>& inout_hidingSpots,
		int numPointsToTestThisPass,
		int& inout_numPointsTestedThisPass
	);

	/*!
	* This method resumes the hiding spot test where it
	* left off and tests up to numPointsToTestThisPass
	* points before returning.
	*
	* @param inout_hidingSpotList The list to which
	*	hiding spots found will be inserted. The list
	*	is kept in quality order from highest to lowest.
	*
	* @param numPointsToTestThisPass The number of points
	*	to test this pass before returning
	*
	* @param inout_numPointsTestedThisPass The number of points
	*	that were tested in this pass. This should be set
	*	to 0 if this is the first call in the pass. It is
	*	updated during this call.
	*
	* @return true if their are more points to search
	*
	* @return false if ther are no more points to search,
	*	so no need to call this again the search is done
	*
	*/
	bool darkModAASFindHidingSpots::findMoreHidingSpots
	(
		idList<darkModHidingSpot_t>& inout_hidingSpots,
		int numPointsToTestThisPass,
		int& inout_numPointsTestedThisPass
	);


public:

	/*!
	* Constructor
	* We use this to set up the aspects of the PVS which are used
	* to determine visibility of adjacent areas for the actor.
	*
	* @param hideFromPos[in] This defines the position from which
	*  the actor is seeking to find hiding spots.  It can be their
	*  own positoin, when considering the general case, or the posiiton
	*  of an entity to be avoided, in the specific case.
	*
	* @param in_p_aas[in] The Area Awareness System to use
	*
	* @param in_p_aas Pointer to the Area Awareness System in use
	* @param hidingHeight The height of the object that would be hiding
	* @param searchLimits The limiting bounds which may be smaller or greater than the area being 
			searched.  The searched region is the intersection of the two.
	* @param hidingSpotTypesAllowed The types of hiding spot characteristics for which we should test
	* @param p_ignoreEntity An entity that should be ignored for testing visual occlusions (usually the self)
	*/
	darkModAASFindHidingSpots
	(
		const idVec3 &hideFromPos , 
		idAAS* in_p_aas, 
		float in_hidingHeight,
		idBounds in_searchLimits, 
		int in_hidingSpotTypesAllowed, 
		idEntity* in_p_ignoreEntity
	);

	/*!
	* This constuctor is NOT suitable for starting a search
	*/
	darkModAASFindHidingSpots();

	/*!
	* Destructor
	*/
	virtual ~darkModAASFindHidingSpots(void);

	/*!
	* This hiding spot list which is a member of the object can be used for the first
	* parameter of findHidingSpots if so desired.
	*/
	idList<darkModHidingSpot_t> hidingSpotList;


	/*!
	* This method starts a search within the given search bounds for hiding spots and returns
	* a list of them as darkModHidingSpot structures.
	*
	* @param out_hidingSpots The list of hiding spots being generated by this call. Will
	*	be cleared before we start testing points
	*
	* @param numPointsToTestThisPass The maximum number of hiding spots to test during this
	* call before remembering the search state and returning
	*
	* @return true if there are more spots to search
	*
	* @return false if there are no more spots to search (end of the search)
	*/
	bool startHidingSpotSearch
	(
		idList<darkModHidingSpot_t>& out_hidingSpots,
		int maxSpotsPerRound
	);

	/*
	* This method is used to continue a hiding spot search that was started with startHidingSpotSearch.
	*
	* @param inout_hidingSPots The list of hiding spots to which new ones found will be added
	*	by this call
	*
	* @param numPointsToTestThisPass The maximum number of hiding spots to test during this
	* call before remembering the search state and returning
	*
	* @return true if there are more spots to search
	*
	* @return false if there are no more spots to search (end of the search)
	*/
	bool continueSearchForHidingSpots
	(
		idList<darkModHidingSpot_t>& inout_hidingSpots,
		int numPointsToTestThisPass
	);


	/*!
	* This method clears the debug rendering hiding spot list. After this call,
	* if debug hiding spot rendering is on, no hiding spots will be drawn until
	* hiding spots are again added to the debug draw list.
	*/
	static void debugClearHidingSpotDrawList();

	/*!
	* This method appends a copy of the hiding spot list into the set of hiding
	* spots drawn during a debug draw of the hiding spots.
	*
	* @param hidingSpotsToAppend The list of hiding spots to be appended to the hiding spot debug list
	*/
	static void debugAppendHidingSpotsToDraw (const idList<darkModHidingSpot_t>& hidingSpotsToAppend);

	/*!
	* This method renders a display of all the hiding spots in the debug draw hiding
	* spot list. It is intended for testing of the results of the hiding spot identification algorithm
	*
	* @param viewLifetime: The lifetime of the debugging visualization, passed to the render world
	*/
	static void debugDrawHidingSpots(int viewLifetime);

	/*!
	* This method is used as a test stub for the hiding
	* spot finding routine. It uses the LAS to find hiding spots near
	* the player's current position
	*/
	static void testFindHidingSpots 
	(
		idVec3 hideFromLocation, 
		float hidingHeight,
		idBounds hideSearchBounds, 
		idEntity* p_ignoreEntity, 
		idAAS* p_aas
	);


};
