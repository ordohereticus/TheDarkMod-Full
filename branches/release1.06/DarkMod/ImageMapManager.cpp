// vim:ts=4:sw=4:cindent
/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4831 $
 * $Date: 2011-05-02 13:22:30 -0400 (Mon, 02 May 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

/*
	Copyright (C) 2011 Tels (Donated to The Dark Mod Team)

	ImageMapManager

	Image maps (usually grayscale images) are loaded only once and the can
	be shared between different SEED entities.

*/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ImageMapManager.cpp 4831 2011-05-02 17:22:30Z greebo $", init_version);

#include "ImageMapManager.h"

/*
===============
CImageMapManager::CImageMapManager
===============
*/
CImageMapManager::CImageMapManager( void ) {
	m_imageMaps.Clear();
	m_lastError = "";
}

CImageMapManager::~CImageMapManager()
{
	Shutdown();
}

/*
===============
CImageMapManager::Save
===============
*/
void CImageMapManager::Save( idSaveGame *savefile ) const {

	int num = m_imageMaps.Num();
	savefile->WriteInt( num );
	for (int i = 0; i < num; i++)
	{
		savefile->WriteString( m_imageMaps[i].name );
		savefile->WriteFloat( m_imageMaps[i].density );
		savefile->WriteUnsignedInt( m_imageMaps[i].users );
	}
}

/*
===============
CImageMapManager::Restore
===============
*/
void CImageMapManager::Restore( idRestoreGame *savefile ) {

	int num;

	// free old image maps
	Clear();

	savefile->ReadInt( num );
	m_imageMaps.Clear();
	m_imageMaps.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		m_imageMaps[i].img = NULL;

		savefile->ReadString( m_imageMaps[i].name );
		savefile->ReadFloat( m_imageMaps[i].density );
		savefile->ReadUnsignedInt( m_imageMaps[i].users );
		if (m_imageMaps[i].users > 0)
		{
			// need to load it again
			if (! LoadImage( &m_imageMaps[i] ) )
			{
				// TODO: Error happened
			}
		}
	}
}

/*
===============
CImageMapManager::Init
===============
*/
void CImageMapManager::Init( void ) {
	Clear();
}

/*
===============
CImageMapManager::Clear
===============
*/
void CImageMapManager::Clear( void ) {

	int num = m_imageMaps.Num();

	for (int i = 0; i < num; i++)
	{
		if (m_imageMaps[i].img)
		{
			m_imageMaps[i].img->Unload(true);
			delete m_imageMaps[i].img;
			m_imageMaps[i].img = NULL;
			m_imageMaps[i].users = 0;
		}
	}
	m_imageMaps.Clear();
	m_lastError = "";
}

/*
===============
CImageMapManager::Shutdown
===============
*/
void CImageMapManager::Shutdown( void ) {
	Clear();
}

/*
===============
CImageMapManager::GetImageMap - Create a new map entry by name

===============
*/
int CImageMapManager::GetImageMap( idStr name ) {
	imagemap_t map;
	// stgatilov: initialize if you don't wont to get 0xCCCCCCCC pointer=)
	map.img = NULL;

	// convert for example "spots" into "textures/seed/spots.png"

	// If the map name does not start with "textures/seed/", prepend it:
	idStr mapName = name;
	if (mapName.Length() < 15 || !mapName.Cmpn("textures/seed/", 14))
	{
		mapName = "textures/seed/" + name;
	}

	// try to find PNG, TGA or JPG version
	idFile *fl = NULL;
	if((fl = fileSystem->OpenFileRead(mapName)) == NULL)
	{
		//gameLocal.Warning("ImageMapManager: Could not find %s, trying .png next.\n", mapName.c_str() );
		idStr m = mapName + ".png";
		if((fl = fileSystem->OpenFileRead(m)) == NULL)
		{
			//gameLocal.Warning("ImageMapManager: Could not find %s, trying .tga next.\n", m.c_str() );
			// can't find PNG, try TGA
			m = mapName + ".tga";
			if((fl = fileSystem->OpenFileRead(m)) == NULL)
			{
				// gameLocal.Warning("ImageMapManager: Could not find %s, trying .jpg next.\n", m.c_str() );
				// can't find TGA, try JPG as last resort
				mapName += ".jpg";
				if((fl = fileSystem->OpenFileRead(mapName)) == NULL)
				{
					// not found at all
					m_lastError = "File not found.";
					return -1;
				}
			}
			else
			{
				// use TGA
				mapName += ".tga";
			}
		}
		else
		{
			// use PNG
			mapName += ".png";
		}
	}
	if (fl)
	{
		fileSystem->CloseFile(fl);
	}

	// TODO: If we have many different (>32?) maps, use a hash?
	int num = m_imageMaps.Num();
	for (int i = 0; i < num; i++)
	{
		if (m_imageMaps[i].name == mapName)
		{
			m_imageMaps[i].users++;
			return i + 1;
		}
	}

	// not found, create new entry
	map.users = 1;
	map.name = mapName;

	// error loading image
	if (!LoadImage( &map ))
	{
		gameLocal.Printf("ImageMapManager: Could not load %s: %s.\n", mapName.c_str(), m_lastError.c_str() );
		if (map.img)
		{
			delete map.img;
			map.img = NULL;
		}
		return -1;
	}
	gameLocal.Printf("ImageMapManager: Loaded '%s', average density is %0.04f.\n", mapName.c_str(), map.density );

	m_imageMaps.Append(map);

	// return new index
	return num + 1;
}


/**
* Given the id of a formerly loaded map, returns a ptr to the given raw image data.
* If the map was previously freed, or never loaded, loads the map again. Returns
* NULL in case the map cannot be loaded (file not found).
*/
unsigned char* CImageMapManager::GetMapData( const unsigned int id )
{
	CImage*	img = GetImage( id );

	if (img)
	{
		return img->GetImageData();
	}

	// some error happened
	return NULL;
}

/**
* Returns the width in pixels of the image map.
*/
unsigned int CImageMapManager::GetMapWidth( const unsigned int id )
{
	CImage*	img = GetImage( id );

	if (img)
	{
		return img->m_Width;
	}

	// some error happened
	return 0;
}

/**
* Returns the width in pixels of the image map.
*/
unsigned int CImageMapManager::GetMapHeight( const unsigned int id ) {
	CImage*	img = GetImage( id );

	if (img)
	{
		return img->m_Height;
	}

	// some error happened
	return 0;
}

/**
* Returns the bytes-per-pixel of the image map.
*/
unsigned int CImageMapManager::GetMapBpp( const unsigned int id ) {
	CImage*	img = GetImage( id );

	if (img)
	{
		return img->m_Bpp;
	}

	// some error happened
	return 0;
}

/**
* Returns the filename of the map.
*/
const char* CImageMapManager::GetMapName( const unsigned int id ) {
	imagemap_t* map = GetMap(id);

	// error?
	if (!map)
	{
		return NULL;
	}

	return map->name.c_str();
}

/**
* Returns the average density of the entire image (0..1.0).
*/
float CImageMapManager::GetMapDensity( const unsigned int id ) {
	imagemap_t* map = GetMap(id);

	// error?
	if (!map)
	{
		return 0.0f;
	}

	return map->density;
}


/**
* Given the id of a formerly loaded map, returns a value between 0 and 255 for the
* position on the map. xr and yr run from 0 .. 1.0.
*/
unsigned int CImageMapManager::GetMapDataAt( const unsigned int id, const float xr, const float yr) {
	CImage*	img = GetImage( id );
	if (!img)
	{
		return 0;
	}

	int x = img->m_Width * xr;
	int y = img->m_Height * yr;
	if (x < 0 || y < 0 || x > img->m_Width || y > img->m_Height)
	{
		m_lastError = "X or Y out of range.";
		return 0;
	}
    unsigned char *imgData = img->GetImageData();

	if (imgData)
	{
		return imgData[ x + y * img->m_Width ];
	}

	m_lastError = "Can't access image data.";
	return 0;
}
	
/**
* Decrements the users count on this map. If "users == 0", the map data can be
* freed later. Return true for success, false for error (map not found or users
* already == 0)
*/
bool CImageMapManager::UnregisterMap( const unsigned int id ) {
	imagemap_t* map = GetMap(id);

	// error?
	if (!map)
	{
		return false;
	}

	// not registered?
	if ( map->users == 0)
	{
		return false;
	}

	// unregister one user
	map->users --;

	return true;
}

/**
* For all maps where users == 0, the map data can be freed.
*/
void CImageMapManager::FreeUnusedMaps( void ) {
	int num = m_imageMaps.Num();

	for (int i = 0; i < num; i++)
	{
		if (m_imageMaps[i].users == 0 && m_imageMaps[i].img)
		{
			m_imageMaps[i].img->Unload(true);
			delete m_imageMaps[i].img;
			m_imageMaps[i].img = NULL;
		}
	}
	return;
}

const char* CImageMapManager::GetLastError( void ) {
	return m_lastError.c_str();
}

/* ************************ Private methods ****************************** */

/**
* Loads the image from disk, makes sure BPP is 1, and computes the average
* density.
*/
bool CImageMapManager::LoadImage( imagemap_t* map ) {
	if (!map)
	{
		return false;
	}
	// already loaded
	if (map->img)
	{
		return true;
	}

	// map data not yet loaded, load it now
	map->img = new CImage();
	// error allocating?
	if (!map->img)
	{
		m_lastError = "Could not allocate new CImage()";
		return false;
	}

	map->img->SetDefaultImageType(CImage::AUTO_DETECT);
	map->img->LoadImageFromVfs( map->name );
	map->img->InitImageInfo();

	if (map->img->m_Bpp != 1)
	{
		m_lastError = "Bytes per pixel is not 1.";
		gameLocal.Error("ImageMapManager: Bytes per pixel must be 1 but is %i for %s!\n", map->img->m_Bpp, map->name.c_str() );
	}

	map->density = 0.0f;
	unsigned char* imgData = map->img->GetImageData();
	if (!imgData)
	{
		return false;
	}

	// Compute an average density for the image map, so the SEED can use it
	double fImgDensity = 0.0f;

	int w = map->img->m_Width;
	int h = map->img->m_Height;
	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			fImgDensity += (double)imgData[ w * y + x ];	// 0 .. 255
		}
	}

	// divide the sum by (w*h) (pixel count)
	fImgDensity /= (double)(w * h * 256.0f);

	map->density = (float)fImgDensity;

	return true;
}

/**
* Checks that the given map handle is valid. Returns ptr to imagemap_t or NULL.
*/
imagemap_t*	CImageMapManager::GetMap( unsigned int handle )
{
	if (handle > 0 && handle <= (unsigned int)m_imageMaps.Num())
	{
		return &m_imageMaps[handle - 1];
	}

	m_lastError = "Invalid handle.";
	return NULL;
}

/**
* Assure that the image was allocated and loaded, then return a ptr to it.
*/
CImage*	CImageMapManager::GetImage( unsigned int handle )
{
	if (handle > 0 && handle <= (unsigned int)m_imageMaps.Num())
	{
		handle --;
		if ( LoadImage( &m_imageMaps[handle] ) )
		{
			return m_imageMaps[handle].img;
		}

	}
	
	return NULL;
}

