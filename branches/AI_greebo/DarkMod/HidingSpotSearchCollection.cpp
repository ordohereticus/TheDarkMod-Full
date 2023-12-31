/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1564 $
 * $Date: 2007-10-26 09:35:37 -0400 (Fri, 26 Oct 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: HidingSpotSearchCollection.cpp 1564 2007-10-26 13:35:37Z greebo $", init_version);

#include "./HidingSpotSearchCollection.h"

//---------------------------------------------------------------------------

#define MAX_NUM_HIDING_SPOT_SEARCHES 4

//--------------------------------------------------------------------

// Global instance
CHidingSpotSearchCollection HidingSpotSearchCollection;

//--------------------------------------------------------------------

// Constructor

CHidingSpotSearchCollection::CHidingSpotSearchCollection(void)
{
	numSearchesInUse = 0;
	p_firstSearch = NULL;
}

//--------------------------------------------------------------------

// Destructor

CHidingSpotSearchCollection::~CHidingSpotSearchCollection(void)
{
	clear();
}

//--------------------------------------------------------------------

void CHidingSpotSearchCollection::clear()
{
	// Destroy all searches
	TDarkmodHidingSpotSearchNode* p_cursor = p_firstSearch;
	TDarkmodHidingSpotSearchNode* p_temp;
	while (p_cursor != NULL)
	{
		p_temp = p_cursor->p_next;
		delete p_cursor;
		p_cursor = p_temp;
	}

	p_firstSearch = NULL;

	// No active searches any more
	numSearchesInUse = 0;

	// Done

}

void CHidingSpotSearchCollection::Save( idSaveGame *savefile ) const
{
	savefile->WriteUnsignedInt(numSearchesInUse);

	int searchesSaved = 0;
	TDarkmodHidingSpotSearchNode* p_cursor = p_firstSearch;
	while (p_cursor != NULL)
	{
		savefile->WriteInt(p_cursor->searchId);
		savefile->WriteInt(p_cursor->refCount);
		
		p_cursor->search.Save(savefile);

		searchesSaved++;
		p_cursor = p_cursor->p_next;
	}

	if (searchesSaved != numSearchesInUse)
	{
		DM_LOG(LC_AI, LT_ERROR).LogString("Error while saving collection: searchesSaved != numSearchesInUse\r");
	}
}

void CHidingSpotSearchCollection::Restore( idRestoreGame *savefile )
{
	clear();

	savefile->ReadUnsignedInt(numSearchesInUse);

	p_firstSearch = NULL;

	TDarkmodHidingSpotSearchNode* lastSearch = NULL;
	for (unsigned int i = 0; i < numSearchesInUse; i++)
	{
		TDarkmodHidingSpotSearchNode* curSearch = new TDarkmodHidingSpotSearchNode;

		if (p_firstSearch == NULL)
		{
			p_firstSearch = curSearch;
		}

		curSearch->p_prev = lastSearch;
		curSearch->p_next = NULL;

		if (curSearch->p_prev != NULL)
		{
			curSearch->p_prev->p_next = curSearch;
		}

		savefile->ReadInt(curSearch->searchId);
		savefile->ReadInt(curSearch->refCount);

		curSearch->search.Restore(savefile);

		lastSearch = curSearch;
	}
}

//--------------------------------------------------------------------

THidingSpotSearchHandle CHidingSpotSearchCollection::getUnusedSearch()
{
	if (numSearchesInUse >= MAX_NUM_HIDING_SPOT_SEARCHES)
	{
		return NULL_HIDING_SPOT_SEARCH_HANDLE;
	}
	else
	{
		TDarkmodHidingSpotSearchNode* p_node = new TDarkmodHidingSpotSearchNode;
		if (p_node == NULL)
		{
			return NULL_HIDING_SPOT_SEARCH_HANDLE;
		}

		// We are returning to somebody, so they have a reference
		p_node->refCount = 1;

		// greebo: Use the numSearchesInUse counter, this should give a nice unique id
		p_node->searchId = numSearchesInUse;

		p_node->p_prev = NULL;
		p_node->p_next = p_firstSearch;
		if (p_firstSearch != NULL)
		{
			p_firstSearch->p_prev = p_node;
		}
		p_firstSearch = p_node;

		// One more search in use
		numSearchesInUse ++;

		return (THidingSpotSearchHandle) p_node;
	}
	
}


//**********************************************************************
// Public
//**********************************************************************

int CHidingSpotSearchCollection::getSearchId(THidingSpotSearchHandle searchHandle)
{
	TDarkmodHidingSpotSearchNode* p_node = p_firstSearch;
	while (p_node != NULL)
	{
		if (static_cast<THidingSpotSearchHandle>(p_node) == searchHandle)
		{
			return p_node->searchId;
		}
		p_node = p_node->p_next;
	}

	// None found
	return -1;
}

THidingSpotSearchHandle CHidingSpotSearchCollection::getSearchHandle(int searchId)
{
	TDarkmodHidingSpotSearchNode* p_node = p_firstSearch;
	while (p_node != NULL)
	{
		if (p_node->searchId == searchId)
		{
			return static_cast<THidingSpotSearchHandle>(p_node);
		}
		p_node = p_node->p_next;
	}

	// None found
	return NULL_HIDING_SPOT_SEARCH_HANDLE;
}

darkModAASFindHidingSpots* CHidingSpotSearchCollection::getSearchByHandle
(
	THidingSpotSearchHandle searchHandle
)
{
	TDarkmodHidingSpotSearchNode* p_node = (TDarkmodHidingSpotSearchNode*) searchHandle;
	if (p_node != NULL)
	{
		if (p_node->refCount <= 0)
		{
			return NULL;
		}
		return &(p_node->search);
	}
	else
	{
		return NULL;
	}
}

//----------------------------------------------------------------------------------

darkModAASFindHidingSpots* CHidingSpotSearchCollection::getSearchAndReferenceCountByHandle
(
	THidingSpotSearchHandle searchHandle,
	unsigned int& out_refCount
)
{
	TDarkmodHidingSpotSearchNode* p_node = (TDarkmodHidingSpotSearchNode*) searchHandle;
	if (p_node != NULL)
	{
		if (p_node->refCount <= 0)
		{
			out_refCount = 0;
			return NULL;
		}

		out_refCount = p_node->refCount;
		return &(p_node->search);
	}
	else
	{
		return NULL;
	}
}


//------------------------------------------------------------------------------

void CHidingSpotSearchCollection::dereference (THidingSpotSearchHandle searchHandle)
{
	TDarkmodHidingSpotSearchNode* p_node = (TDarkmodHidingSpotSearchNode*) searchHandle;
	if (p_node != NULL)
	{
		// Decrement Refcount
		p_node->refCount --;

		if (p_node->refCount <= 0)
		{
			if (p_node->p_prev != NULL)
			{
				p_node->p_prev->p_next = p_node->p_next;
			}
			else
			{
				p_firstSearch = p_node->p_next;
			}

			if (p_node->p_next != NULL)
			{
				p_node->p_next->p_prev = p_node->p_prev;
			}

			delete p_node;
		
			// One less search
			numSearchesInUse --;
		}
	}
	
}

//------------------------------------------------------------------------------

THidingSpotSearchHandle CHidingSpotSearchCollection::findSearchByBounds 
(
	idBounds bounds,
	idBounds exclusionBounds
)
{
	TDarkmodHidingSpotSearchNode* p_node = p_firstSearch;
	while (p_node != NULL)
	{
		idBounds existingBounds = p_node->search.getSearchLimits();
		idBounds existingExclusionBounds = p_node->search.getSearchExclusionLimits();
		if (existingBounds.Compare (bounds, 50.0))
		{
			if (existingExclusionBounds.Compare (exclusionBounds, 50.0))
			{
				// Reuse this one
				p_node->refCount ++;
				return (THidingSpotSearchHandle) p_node;
			}
		}

		p_node = p_node->p_next;
	}

	// None found
	return NULL_HIDING_SPOT_SEARCH_HANDLE;

}

//------------------------------------------------------------------------------

THidingSpotSearchHandle CHidingSpotSearchCollection::getOrCreateSearch
(
	const idVec3 &hideFromPos, 
	idAAS* in_p_aas, 
	float in_hidingHeight,
	idBounds in_searchLimits, 
	idBounds in_searchExclusionLimits, 
	int in_hidingSpotTypesAllowed, 
	idEntity* in_p_ignoreEntity,
	int frameIndex,
	bool& out_b_searchCompleted
)
{
	/* Search with same bounds already? */
	THidingSpotSearchHandle hSearch = findSearchByBounds (in_searchLimits, in_searchExclusionLimits);
	if (hSearch != NULL_HIDING_SPOT_SEARCH_HANDLE)
	{
		darkModAASFindHidingSpots* p_search = getSearchByHandle(hSearch);
		out_b_searchCompleted = p_search->isSearchCompleted();
		return hSearch;
	}

	/* Make new search */
	hSearch = getUnusedSearch();
	if (hSearch == NULL_HIDING_SPOT_SEARCH_HANDLE)
	{
		return hSearch;
	}

	/* Initialize the search */
	darkModAASFindHidingSpots* p_search = getSearchByHandle(hSearch);
	p_search->initialize
	(
		hideFromPos, 
		//in_p_aas, 
		in_hidingHeight,
		in_searchLimits, 
		in_searchExclusionLimits,
		in_hidingSpotTypesAllowed, 
		in_p_ignoreEntity
	);

	// Start search
	DM_LOG(LC_AI, LT_DEBUG).LogString ("Starting search for hiding spots\r");
	bool b_moreProcessingToDo = p_search->startHidingSpotSearch
	(
		p_search->hidingSpotList,
		g_Global.m_maxNumHidingSpotPointTestsPerAIFrame,
		frameIndex
	);
	DM_LOG(LC_AI, LT_DEBUG).LogString ("First pass of hiding spot search found %d spots\r", p_search->hidingSpotList.getNumSpots());

	// Is search completed?
	out_b_searchCompleted = !b_moreProcessingToDo;

	// Search created, return search to caller
	return hSearch;
		

}
