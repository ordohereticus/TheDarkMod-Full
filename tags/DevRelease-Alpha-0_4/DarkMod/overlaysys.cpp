/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 655 $
 * $Date: 2006-12-13 13:54:05 -0500 (Wed, 13 Dec 2006) $
 * $Author: gildoran $
 *
 * $Log$
 * Revision 1.4  2006/12/13 18:54:05  gildoran
 * Added SDK function names to error messages to help with debugging.
 * Fixed a bug that allowed creation of duplicate handles.
 * Added OVERLAYS_INVALID_HANDLE to be used instead of OVERLAYS_MIN_HANDLE - 1
 *
 * Revision 1.3  2006/12/12 19:37:59  gildoran
 * Fixed dangling else bugs.
 *
 * Revision 1.2  2006/09/18 18:56:50  gildoran
 * Added getNextOverlay, and code to automatically set an overlay as interactive if the GUI is.
 *
 * Revision 1.1  2006/09/18 13:38:13  gildoran
 * Added the first version of a unified interface for GUIs.
 *
 *
 ***************************************************************************/


#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Source$  $Revision: 655 $   $Date: 2006-12-13 13:54:05 -0500 (Wed, 13 Dec 2006) $", init_version);

#include "../game/Game_local.h"

#include "overlaySys.h"

// I'm keeping these functions seperate from the overlaysys code,
// in case I end up needing to implement recycling.
static inline idUserInterface* newGui( const char* file ) {
	//return uiManager->FindGui( file, true, false, true );
	return uiManager->FindGui( file, true, true );
}
static inline void delGui( idUserInterface* gui ) {
	uiManager->DeAlloc( gui );
}

COverlaySys::COverlaySys() {
	m_lastUsedHandle = 0;
	m_lastUsedOverlay = NULL;
	m_updateOpaque = false;
	m_highestOpaque = NULL;
	m_updateInteractive = false;
	m_highestInteractive = NULL;
	m_nextHandle = OVERLAYS_MIN_HANDLE;
}

COverlaySys::~COverlaySys() {
	SOverlay* overlay;
	idLinkList<SOverlay>* oNode = m_overlays.NextNode();
	while ( oNode ) {
		overlay = oNode->Owner();
		oNode = oNode->NextNode();

		if ( !overlay->m_external )
			delGui( overlay->m_gui );
		delete overlay;
	}
}

void COverlaySys::Save( idSaveGame *savefile ) const {
	SOverlay* overlay;
	idLinkList<SOverlay>* oNode = m_overlays.NextNode();
	while ( oNode ) {

		overlay = oNode->Owner();
		savefile->WriteInt( overlay->m_handle );
		savefile->WriteInt( overlay->m_layer );
		savefile->WriteBool( overlay->m_external );
		savefile->WriteBool( overlay->m_opaque );
		savefile->WriteBool( overlay->m_interactive );
		if ( !overlay->m_external )
			savefile->WriteUserInterface( overlay->m_gui, false );

		oNode = oNode->NextNode();
	}
	savefile->WriteInt( OVERLAYS_INVALID_HANDLE );
}

void COverlaySys::Restore( idRestoreGame *savefile ) {
	int handle;
	SOverlay* overlay;

	savefile->ReadInt( handle );
	while ( handle >= OVERLAYS_MIN_HANDLE ) {
		overlay = new SOverlay;
		if ( !overlay ) {
			gameLocal.Error( "Unable to allocate SOverlay.\n" );
			goto Quit;
		}

		overlay->m_node.SetOwner( overlay );
		overlay->m_node.AddToEnd( m_overlays );
		overlay->m_handle = handle;
		savefile->ReadInt( overlay->m_layer );
		savefile->ReadBool( overlay->m_external );
		savefile->ReadBool( overlay->m_opaque );
		savefile->ReadBool( overlay->m_interactive );
		if ( overlay->m_external )
			overlay->m_gui = NULL; // I don't think there's a way to save/restore pointers to GUIs saved by other things.
		else
			savefile->ReadUserInterface( overlay->m_gui );

		savefile->ReadInt( handle );
	}

	m_updateOpaque = m_updateInteractive = true;

	Quit:
	return;
}

void COverlaySys::drawOverlays() {
	idUserInterface* gui;
	idLinkList<SOverlay>* oNode = findOpaque();
	if ( !oNode )
		oNode = m_overlays.NextNode();
	while ( oNode ) {
		gui = oNode->Owner()->m_gui;
		if ( gui )
			gui->Redraw( gameLocal.realClientTime );
		oNode = oNode->NextNode();
	}
}

bool COverlaySys::isOpaque() {
	return findOpaque() != NULL;
}

int COverlaySys::getNextOverlay( int handle ) {
	int retHandle = OVERLAYS_INVALID_HANDLE;

	idLinkList<SOverlay> *oNode;
	SOverlay *overlay = findOverlay( handle, false );
	if ( overlay )
		oNode = overlay->m_node.NextNode();
	else if ( handle == OVERLAYS_INVALID_HANDLE )
		oNode = m_overlays.NextNode();
	else {
		gameLocal.Warning( "getNextOverlay: Non-existant GUI handle: %d\n", handle );
		goto Quit;
	}

	if ( oNode ) {
		m_lastUsedOverlay = oNode->Owner();
		m_lastUsedHandle  = m_lastUsedOverlay->m_handle;
		retHandle = m_lastUsedHandle;
	}

	Quit:
	return retHandle;
}

int COverlaySys::createOverlay( int layer, int handle ) {
	assert( handle >= OVERLAYS_MIN_HANDLE || handle == OVERLAYS_INVALID_HANDLE );

	int retHandle = OVERLAYS_INVALID_HANDLE;

	// Find a valid handle for our overlay.
	idLinkList<SOverlay>* oNode;
	if ( handle == OVERLAYS_INVALID_HANDLE ) {
		// Any handle will do.

		bool foundHandle = false;

		// Iterate through all available handles until we come back to where we started.
		handle = m_nextHandle;
		do {

			// Check if the overlay has the handle.
			oNode = m_overlays.NextNode();			
			while ( oNode ) {
				if ( oNode->Owner()->m_handle == handle )
					break;
				oNode = oNode->NextNode();
			}
			// If we went all the way through the loop, the handle doesn't already exist.
			if ( !oNode ) {
				foundHandle = true;
				break;
			}

			handle++;
			if ( handle < OVERLAYS_MIN_HANDLE )
				handle = OVERLAYS_MIN_HANDLE;
		} while ( handle != m_nextHandle );

		if ( !foundHandle ) {
			gameLocal.Warning( "No more handles available.\n" );
			goto Quit;
		}

		m_nextHandle = handle + 1;
		if ( m_nextHandle < OVERLAYS_MIN_HANDLE )
			m_nextHandle = OVERLAYS_MIN_HANDLE;

	} else {
		// There's a specific handle that is desired.

		oNode = m_overlays.NextNode();
		while ( oNode ) {
			// If the handle is unavailable, don't create anything.
			if ( oNode->Owner()->m_handle == handle )
				goto Quit;
			oNode = oNode->NextNode();
		}

	}

	// At this point, 'handle' is the handle our overlay will have.

	// Find the position after which we will insert our overlay.
	idLinkList<SOverlay>* position = &m_overlays; // The position after which we will insert the new overlay
	oNode = m_overlays.NextNode();
	while ( oNode ) {
		if ( layer >= oNode->Owner()->m_layer )
			position = oNode;
		oNode = oNode->NextNode();
	}

	SOverlay* overlay = new SOverlay;
	if ( !overlay )
	{
		gameLocal.Warning( "Unable to allocate overlay.\n" );
		goto Quit;
	}

	overlay->m_node.SetOwner( overlay );
	overlay->m_node.InsertAfter( *position );
	overlay->m_gui = NULL;
	overlay->m_handle = handle;
	overlay->m_layer = layer;
	overlay->m_external = true;
	overlay->m_opaque = false;
	overlay->m_interactive = false;

	retHandle = handle;

	Quit:
	return retHandle;
}

void COverlaySys::destroyOverlay( int handle ) {
	SOverlay* overlay = findOverlay( handle, false );
	if ( overlay == NULL )
	{
		gameLocal.Warning( "destroyOverlay: Non-existant GUI handle: %d\n", handle );
		goto Quit;
	}

	// If the last-used cache points to the overlay, clear it.
	if ( m_lastUsedOverlay == overlay ) {
		m_lastUsedHandle = OVERLAYS_INVALID_HANDLE;
		m_lastUsedOverlay = NULL; // not necessary, but perhaps safer
	}

	// If this was an opaque overlay, we need to update opacity.
	if ( overlay->m_opaque )
		m_updateOpaque = true;

	// If this was an interactive overlay, we need to update interactivity.
	if ( overlay->m_interactive )
		m_updateInteractive = true;

	if ( !overlay->m_external )
		delGui( overlay->m_gui );
	delete overlay;

	Quit:
	return;
}

/// Returns whether or not an overlay exists.
bool COverlaySys::exists( int handle ) {
	return findOverlay( handle ) != NULL;
}

/// Sets the overlay's GUI to an external GUI.
void COverlaySys::setGui( int handle, idUserInterface* gui ) {
	SOverlay* overlay = findOverlay( handle );
	if ( overlay ) {
		if ( !overlay->m_external ) {
			delGui( overlay->m_gui );
			overlay->m_external = true;
		}

		overlay->m_gui = gui;
	} else {
		gameLocal.Warning( "setGui: Non-existant GUI handle: %d\n", handle );
	}
}

/// Sets the overlay's GUI to an internal, unique one.
bool COverlaySys::setGui( int handle, const char* file ) {
	bool retVal = false;
	SOverlay* overlay = findOverlay( handle );
	if ( overlay ) {

		idUserInterface* gui = newGui( file );
		if ( gui ) {

			if ( !overlay->m_external )
				delGui( overlay->m_gui );
			overlay->m_gui = gui;
			overlay->m_external = false;
			retVal = true;
		} else {
			gameLocal.Warning( "Unable to load gui: %s\n", file );
		}

	} else {
		gameLocal.Warning( "setGui: Non-existant GUI handle: %d\n", handle );
	}
	return retVal;
}

idUserInterface* COverlaySys::getGui( int handle ) {
	SOverlay* overlay = findOverlay( handle );
	if ( overlay )
		return overlay->m_gui;
	else {
		gameLocal.Warning( "getGui: Non-existant GUI handle: %d\n", handle );
		return NULL;
	}
}

void COverlaySys::setLayer( int handle, int layer ) {
	SOverlay* overlay = findOverlay( handle );
	if ( overlay ) {

		// Find the new spot for the overlay.
		idLinkList<SOverlay>* position = &m_overlays; // insert the new overlay AFTER this position
		idLinkList<SOverlay>* oNode = m_overlays.NextNode();
		while ( oNode ) {
			if ( layer >= oNode->Owner()->m_layer )
				position = oNode;
			oNode = oNode->NextNode();
		}

		// Did we actually change positions?
		if ( position->Next() != overlay && position != &overlay->m_node ) {
			overlay->m_node.InsertAfter( *position );
			overlay->m_layer = layer;

			// If this is an opaque overlay, we need to update opacity.
			if ( overlay->m_opaque )
				m_updateOpaque = true;

			// If this is an interactive overlay, we need to update interactivity.
			if ( overlay->m_interactive )
				m_updateInteractive = true;
		}

	} else {
		gameLocal.Warning( "setLayer: Non-existant GUI handle: %d\n", handle );
	}
}

int COverlaySys::getLayer( int handle ) {
	SOverlay* overlay = findOverlay( handle );
	if ( overlay ) {
		return overlay->m_layer;
	} else {
		gameLocal.Warning( "getLayer: Non-existant GUI handle: %d\n", handle );
		return 0;
	}
}

bool COverlaySys::isExternal( int handle ) {
	SOverlay* overlay = findOverlay( handle );
	if ( overlay ) {
		return overlay->m_external;
	} else {
		gameLocal.Warning( "isExternal: Non-existant GUI handle: %d\n", handle );
		return false;
	}
}

void COverlaySys::setOpaque( int handle, bool isOpaque ) {
	SOverlay* overlay = findOverlay( handle );
	if ( overlay ) {
		if ( overlay->m_opaque != isOpaque ) {
			overlay->m_opaque = isOpaque;
			m_updateOpaque = true;
		}
	} else {
		gameLocal.Warning( "setOpaque: Non-existant GUI handle: %d\n", handle );
	}
}

bool COverlaySys::isOpaque( int handle ) {
	SOverlay* overlay = findOverlay( handle );
	if ( overlay ) {
		return overlay->m_opaque;
	} else {
		gameLocal.Warning( "isOpaque: Non-existant GUI handle: %d\n", handle );
		return false;
	}
}

void COverlaySys::setInteractive( int handle, bool isInteractive ) {
	SOverlay* overlay = findOverlay( handle );
	if ( overlay ) {
		if ( overlay->m_interactive != isInteractive ) {
			overlay->m_interactive = isInteractive;
			m_updateInteractive = true;
		}
	} else {
		gameLocal.Warning( "setInteractive: Non-existant GUI handle: %d\n", handle );
	}
}

bool COverlaySys::isInteractive( int handle ) {
	SOverlay* overlay = findOverlay( handle );
	if ( overlay ) {
		return overlay->m_interactive;
	} else {
		gameLocal.Warning( "isInteractive: Non-existant GUI handle: %d\n", handle );
		return false;
	}
}

SOverlay* COverlaySys::findOverlay( int handle, bool updateCache ) {
	SOverlay* retVal = NULL;

	if ( handle < OVERLAYS_MIN_HANDLE )
		goto Quit;

	// Are we looking for the same handle as last time?
	if ( handle == m_lastUsedHandle ) {
		retVal = m_lastUsedOverlay;
		goto Quit;
	}

	idLinkList<SOverlay>* oNode = m_overlays.NextNode();
	while ( oNode ) {

		// Did we find the handle we're looking for?
		if ( handle == oNode->Owner()->m_handle ) {

			retVal = oNode->Owner();

			if ( updateCache ) {
				m_lastUsedHandle = handle;
				m_lastUsedOverlay = retVal;
			}

			goto Quit;
		}

		oNode = oNode->NextNode();
	}

	Quit:
	return retVal;
}

idLinkList<SOverlay>* COverlaySys::findOpaque() {
	if ( m_updateOpaque ) {
		// Find the highest opaque overlay.
		idLinkList<SOverlay>* oNode = m_overlays.PrevNode();
		while ( oNode ) {
			if ( oNode->Owner()->m_opaque )
				break;
			oNode = oNode->PrevNode();
		}
		m_highestOpaque = oNode;
		m_updateOpaque = false;
	}
	return m_highestOpaque;
}

idUserInterface* COverlaySys::findInteractive() {
	if ( m_updateInteractive ) {
		// Find the highest interactive overlay.
		idLinkList<SOverlay>* oNode = m_overlays.PrevNode();
		while ( oNode ) {
			if ( oNode->Owner()->m_interactive )
				break;
			oNode = oNode->PrevNode();
		}
		m_highestInteractive = oNode ? oNode->Owner()->m_gui : NULL;
		m_updateInteractive = false;
	}
	return m_highestInteractive;
}


