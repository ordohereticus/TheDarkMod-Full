/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 12:53:28 -0400 (Tue, 16 Oct 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/
/*!
* This is the interface for an n-tree structure which sorts and holds
* darkmod hiding spots.
*
* 
* 
*/
#ifndef DARKMOD_HIDING_SPOT_TREE
#define DARKMOD_HIDING_SPOT_TREE

/*!
// @structure darkModHidingSpot_t
// @author SophisticatedAZombie (DMH)
// This structure holds information about a hiding spot.
*/
typedef struct
{
	aasGoal_t goal;

	// The hiding spot characteristic bit flags
	// as defined by the darkModHidingSpotType enumeration
	int hidingSpotTypes;

	// The light quotient of the spot.
	float lightQuotient;
	
	// The hiding spot "hidingness" quality, without distance from the search
	// center factored in.
	float qualityWithoutDistanceFactor;

	// The hiding spot "hidingness" quality, from 0 to 1.0
	float quality;

} darkModHidingSpot_t;

//-------------------------------------------------------------------------

typedef struct tagDarkModHidingSpotNode
{
	// The unique spot id
	int id;

	// The spot
	darkModHidingSpot_t spot;

	// Next hiding spot
	tagDarkModHidingSpotNode* p_next;

} darkModHidingSpotNode;



/*---------------------------------------------------------------------------*/

typedef struct tagTDarkmodHidingSpotAreaNode
{
	// greebo: This is a unique ID to resolve pointers after map restore
	int id;

	unsigned int aasAreaIndex;
	unsigned int count;

	tagTDarkmodHidingSpotAreaNode* p_prevSibling;
	tagTDarkmodHidingSpotAreaNode* p_nextSibling;
	darkModHidingSpotNode* p_firstSpot;
	darkModHidingSpotNode* p_lastSpot;

	// Quality of the best spot in the area
	float bestSpotQuality;

	/* The extents*/
	idBounds bounds;

	
} TDarkmodHidingSpotAreaNode;

/*---------------------------------------------------------------------------*/

typedef unsigned long TDarkModHidingSpotTreeIterationHandle;

/*---------------------------------------------------------------------------*/

class CDarkmodHidingSpotTree
{
private:
	// The highest used area node id (0 on initialisiation)
	int maxAreaNodeId;

	// The highest used spot node id (0 on initialisation)
	int maxSpotNodeId;

protected:

	// The number of areas
	unsigned long numAreas;

	unsigned long numSpots;

	// The first area
	TDarkmodHidingSpotAreaNode* p_firstArea;
	TDarkmodHidingSpotAreaNode* p_lastArea;

	// Handles
	unsigned long lastIndex_indexRetrieval;
	TDarkModHidingSpotTreeIterationHandle lastAreaHandle_indexRetrieval;
	TDarkModHidingSpotTreeIterationHandle lastSpotHandle_indexRetrieval;

	// Mapping methods, retrieves node pointers for indices and vice versa
	int getAreaNodeId(TDarkmodHidingSpotAreaNode* area) const; // returns -1 for invalid pointer
	TDarkmodHidingSpotAreaNode* getAreaNode(int areaNodeId) const; // returns NULL for invalid Id

	int getSpotNodeId(darkModHidingSpotNode* spotNode) const; // Returns -1 for invalid pointer
	darkModHidingSpotNode* getSpotNode(int spotNodeId) const; // Returns NULL for invalid id

	void clearIndexRetrievalTracking();
	
	/*!
	* Gets the Nth spot from the tree, where N is a 0 based index.
	* This is a slow iteration from the beginning of the tree
	* and should only be used if the index being retrieved is less than the
	* previous index retrieved or if none has yet been retrieved
	* since the last time the tree was altered.
	*
	* @return Pointer to spot, NULL if index was out of bounds
	*/
	darkModHidingSpot_t* getNthSpotInternal
	(
		unsigned int index,
		idBounds& out_areaNodeBounds
	);

	/*!
	* This method sorts the area by hiding spot light internally
	*
	* @param inout_p_firstNode On input, pointer to the first point in the area
	*	On return, pointer to the first point in the list, sorted from
	*	greatest to least quality.
	*
	* @param numSpots The number of spots in the list passed in.  The number of
	*	spots is not changed by sorting, unless the algorithm is broken, in which
	*	case you should fix it yourself, damnit.

	*/
	void quicksortHidingSpotList
	(
		darkModHidingSpotNode*& inout_p_firstNode,
		unsigned long numSpots
	);

	/*!
	* This method sorts the given area list
	*
	* @param inout_p_firstNode On input, pointer to the first area
	*	On return, pointer to the first area in the list, sorted from
	*	greatest to least quality.
	*
	* @param numAreas The number of areas in the list passed in.  

	*/
	void quicksortAreaList
	(
		TDarkmodHidingSpotAreaNode*& inout_p_firstNode,
		unsigned long numAreas
	);

	/*!
	* This method breaks a node into 8 rough "octants" 
	* The original node is used as one of the octants, and
	* the other nodes created between this node and the next
	* original node in the  list
	*
	* @param in_p_areaNode The node that is to be sub-divided.
	*
	* @param out_numSubAreasWithPoints Passes back out the number
	*	of sub areas (up to 8) that had points.
	*
	* @param out_subAreas Passes back the pointers to the sub
	*	areas that had points (up to 8)
	*/
	bool subDivideArea
	(
		TDarkmodHidingSpotAreaNode* in_p_areaNode,
		unsigned int& out_numSubAreasWithPoints,
		TDarkmodHidingSpotAreaNode* out_p_subAreas[8]
	);

public:

	/*!
	* Constructor for an empty tree
	*/
	CDarkmodHidingSpotTree();

	/*!
	* Destroys all nodes and clears the
	* tree memory usage on destruction
	*/
	~CDarkmodHidingSpotTree();

	/*!
	* Destroys all nodes and clears the tree memory
	* usage.
	*/
	void clear();

	void Save( idSaveGame *savefile ) const;
	void Restore( idRestoreGame *savefile );

	/*!
	* Get the number of spots in the entire tree
	*/
	unsigned long getNumSpots()
	{
		return numSpots;
	}

	/*!
	* Gets the node for an area by area index
	*/
	TDarkmodHidingSpotAreaNode* getArea
	(
		unsigned int areaIndex
	);

	/*!
	* Inserts an area and returns the are node handle
	* for use in inserting hiding spots.
	*/
	TDarkmodHidingSpotAreaNode* insertArea
	(
		unsigned int areaIndex
	);

	/*!
	* Tests a spot for redundancy with spots already in
	* the same area.  Redundancy occurs if the distance
	* is less than the redundancy distance. 
	* If redundant, the existing point may be updated
	* to higher quality values if the new point was
	* of higher quality than the existing point.
	*
	* @return true if redundant
	* @return false if not redundant
	*/
	bool determineSpotRedundancy
	(
		TDarkmodHidingSpotAreaNode* p_areaNode,
		aasGoal_t goal,
		int hidingSpotTypes,
		float quality,
		float redundancyDistance
	);

	/*!
	* Inserts a hiding spot into a given area node.
	*
	* @param p_areaNode The area node to which the point should be added
	*
	* @param goal The goal descriptor of the point
	*
	* @param hidingSpotTypes The types of hiding spot that apply to the point
	*
	* @param quality The measure of hiding spot quality from 0.0 to 1.0
	*
	* @param redundancyDistance The distance between points for a unique point
	*	to be created. If the new point is within this distance from a point
	*	already in the area, the points will be combined.
	*	If this number is < 0.0, then no redundancy testing is done.
	*
	*/
	bool insertHidingSpot
	(
		TDarkmodHidingSpotAreaNode* p_areaNode,
		aasGoal_t goal,
		int hidingSpotTypes,
		float lightQuotient,
		float qualityWithoutDistanceFactor,
		float quality,
		float redundancyDistance
	);

	/*!
	* This method is intended to be used once all of the
	* nodes in the tree have been created and all the points
	* inserted.  It subdivides each area in the tree
	* into octants based on x/y/z midline partitions.
	* It does this over and over on any area which contains
	* more than the limiting number of points.
	*
	* @param maxPointsPerArea
	*
	* @return true on succes
	*
	* @return false on failure, structure may be corrupted due
	*	to failure to allocate new area nodes. This should only 
	*	happen if the computer critically runs out of memory 
	*	(time to shut down the application).
	*
	*/
	bool subDivideAreas
	(
		unsigned int maxPointsPerArea
	);

	/*! 
	* Starts an iteration of the areas in the tree
	*/
	TDarkmodHidingSpotAreaNode* getFirstArea
	(
		TDarkModHidingSpotTreeIterationHandle& out_iterationHandle
	);

	/*!
	* Moves forward in an itreation of the tree
	*/
	TDarkmodHidingSpotAreaNode* getNextArea
	(
		TDarkModHidingSpotTreeIterationHandle& inout_iterationHandle
	);

	/*!
	* Gets first hiding spot in area
	*/
	darkModHidingSpot_t* getFirstHidingSpotInArea
	(
		TDarkModHidingSpotTreeIterationHandle& inout_areaIterationHandle,
		TDarkModHidingSpotTreeIterationHandle& out_spotHandle
	);

	/*!
	* Gets next hiding spot in area
	*/
	darkModHidingSpot_t* getNextHidingSpotInArea
	(
		TDarkModHidingSpotTreeIterationHandle& inout_spotHandle
	);

	/*!
	* This speeds up requests for the Nth spot if N is >= the
	* last N requested and not by much (such as N+1)
	*
	* @param index The index of the spot to get
	*
	*/
	darkModHidingSpot_t* getNthSpot
	(
		unsigned int index
	);

	/*!
	* This speeds up requests for the Nth spot if N is >= the
	* last N requested and not by much (such as N+1)
	*
	* @param index The index of the spot to get.
	*
	* @param out_areaBounds The bounds of the sub-area containing
	*	the spot
	*
	*/
	darkModHidingSpot_t* getNthSpotWithAreaNodeBounds
	(
		unsigned int index,
		idBounds& out_areaNodeBounds
	);

	/*!
	* Attempts to split off one Nth of the tree (1/N) in a logical fashion
	* for sharing a search. The caller provides a tree which after the call
	* contains only the areas and spots removed from this tree.
	*
	* @param N The fraction 1/Nth of this tree will be moved to the other tree
	*	If N is 0 nothing is moved.
	* 
	* @param p_out_otherTree This is the tree to which the fraction of the tree
	*	will be moved.  The tree is cleared before any areas or spots are moved
	*	to it.
	*/
	bool getOneNth
	(
		unsigned int N,
		CDarkmodHidingSpotTree* p_out_otherTree
	);

	/*!
	* This copies this tree into another tree.
	*/
	bool copy
	(
		CDarkmodHidingSpotTree* p_out_otherTree
	);

	/*!
	* This method rescores the points and repriortizes the list based on a new search center.
	* This can be used if a new stimulus is detected while searching, to bias the search
	* toward the new stimulous.
	*
	*/
	bool sortForNewCenter
	(
		idVec3 center,
		float searchRadius
	);


};








#endif
