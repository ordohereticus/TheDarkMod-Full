// vim:ts=4:sw=4:cindent
/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4471 $
 * $Date: 2011-01-24 10:40:30 -0500 (Mon, 24 Jan 2011) $
 * $Author: tels $
 *
 ***************************************************************************/

// Copyright (C) 2011 Tels (Donated to The Dark Mod)

#ifndef __DARKMOD_IMAGEMAPMANAGER_H__
#define __DARKMOD_IMAGEMAPMANAGER_H__

// to get CImage
#include "../DarkMod/CImage.h"

/*
===============================================================================

  ImageMap Manager - manages image maps used by the SEEDsystem

  This class is a singleton and initiated/destroyed from gameLocal.

  Image maps (usually grayscale images) are loaded only once and the can
  be shared between different SEED entities.

===============================================================================
*/

// Defines data for one image map
typedef struct {
	idStr				name;		//!< the filename from where the image was loaded
	CImage*				img;		//!< The CImage object that loads and contains the actual data
	float				density;	//!< average density (0..1.0)
	unsigned int		users;		//!< How many objects currently use this image? Data can only be freed if users == 0.
} imagemap_t;

class CImageMapManager {
public:

						CImageMapManager( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	/**
	* Called by gameLocal.
	*/
	void				Init ( void );
	void				Shutdown ( void );
	void				Clear ( void );

	/**
	* Given a name of the image map (either full filename, or partial name), tries to
    * load the corrosponding file and returns the index of the map for later access.
	* Returns -1 in case of errors (like file not found).
	* This will increment the users count on the map.
	*/
	int					GetImageMap( idStr mapname );

	/**
	* Given the id of a formerly loaded map, returns a ptr to the given raw image data.
	* If the map was previously freed, or never loaded, returns NULL.
	*/
	unsigned char*		GetMapData( const unsigned int id );

	/**
	* Returns the width in pixels of the image map. Returns 0 in case of error.
	*/
	unsigned int		GetMapWidth( const unsigned int id );

	/**
	* Returns the width in pixels of the image map. Returns 0 in case of error.
	*/
	unsigned int		GetMapHeight( const unsigned int id );

	/**
	* Returns the byte-per-pixel of the image map. Returns 0 in case of error.
	*/
	unsigned int		GetMapBpp( const unsigned int id );

	/**
	* Returns the filename of the map.
	*/
	const char *		GetMapName( const unsigned int id );

	/**
	* Returns the average density of the entire image (0..1.0).
	*/
	float				GetMapDensity( const unsigned int id );

	/**
	* Given the id of a formerly loaded map, returns a value between 0 and 255 for the
	* position on the map. xr and yr run from 0 .. 1.0f;
	*/
	unsigned int		GetMapDataAt( const unsigned int id, const float xr, const float yr);

	/**
	* Decrements the users count on this map. If "users == 0", the map data can be
	* freed later. Return 0 for success, -1 for error (map not found or users already == 0)
	*/
	bool				UnregisterMap( const unsigned int id );

	/**
	* For all maps where users == 0, the map data can be freed.
	*/
	void				FreeUnusedMaps( void );

	const char *		GetLastError( void );

private:

	/**
	* Checks that the given map handle is valid. Returns ptr to imagemap_t or NULL.
	*/
	imagemap_t*			GetMap( unsigned int handle );

	/**
	* Loads the image from disk, makes sure BPP is 1, and computes the average
	* density. Returns false if the image could not be loaded.
	*/
	bool 				LoadImage( imagemap_t* map );

	/**
	* Assure that the image was allocated and loaded, then return a ptr to it.
	*/
	CImage*				GetImage( unsigned int handle );

	/** List of loaded image maps */
	idList< imagemap_t >	m_imageMaps;

	/** Human readable message of the last error that occured */
	idStr					m_lastError;

};

#endif /* !__DARKMOD_MODELGENERATOR_H__ */

