/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 739 $
 * $Date: 2007-01-19 20:37:41 -0500 (Fri, 19 Jan 2007) $
 * $Author: thelvyn $
 *
 * $Log$
 * Revision 1.3  2007/01/20 01:37:41  thelvyn
 * Implemented Ctrl, Shift and Alt key detection.
 * Right , Left supported for all. Also generic dont care if left or right functions.
 * Testing is in place in playerview.cpp
 * I reused #ifdef MOUSETEST as I still have the mouse code in there as well.
 * You can what if any buttons are detected. Mouse L, R, M and for keyboard Left, Right or both of Ctrl, Shift and Alt
 *
 * Revision 1.2  2005/11/11 22:17:26  sparhawk
 * SDK 1.3 Merge
 *
 * Revision 1.1.1.1  2004/10/30 15:52:35  sparhawk
 * Initial release
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __LIB_H__
#define __LIB_H__


/*
===============================================================================

	idLib contains stateless support classes and concrete types. Some classes
	do have static variables, but such variables are initialized once and
	read-only after initialization (they do not maintain a modifiable state).

	The interface pointers idSys, idCommon, idCVarSystem and idFileSystem
	should be set before using idLib. The pointers stored here should not
	be used by any part of the engine except for idLib.

	The frameNumber should be continuously set to the number of the current
	frame if frame base memory logging is required.

===============================================================================
*/

class idLib {
public:
	static class idSys *		sys;
	static class idCommon *		common;
	static class idCVarSystem *	cvarSystem;
	static class idFileSystem *	fileSystem;
	static int					frameNumber;

	static void					Init( void );
	static void					ShutDown( void );
};


/*
===============================================================================

	Types and defines used throughout the engine.

===============================================================================
*/

typedef unsigned char			byte;		// 8 bits
typedef unsigned short			word;		// 16 bits
typedef unsigned int			dword;		// 32 bits
typedef unsigned int			uint;
typedef unsigned long			ulong;

typedef int						qhandle_t;

class idFile;
class idVec3;
class idVec4;

#ifndef NULL
#define NULL					((void *)0)
#endif

#ifndef BIT
#define BIT( num )				( 1 << ( num ) )
#endif

/* Bit operations */
#define BITCHK(flag,bit) (((flag)&(bit))==(bit)) /* if all the bits are set */
#define BITANY(flag,bit) ((flag)&(bit)) /* if any bit is set */
#define BITSET(flag,bit) flag|=(bit)
#define BITCLR(flag,bit) flag&=(~(bit))
#define BITFLIP(flag,bit) if (BIT( (flag),(bit) )) BITCLR( (flag),(bit) ); else BITSET( (flag),(bit) )


#define	MAX_STRING_CHARS		1024		// max length of a string

// maximum world size
#define MAX_WORLD_COORD			( 128 * 1024 )
#define MIN_WORLD_COORD			( -128 * 1024 )
#define MAX_WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

// basic colors
extern	idVec4 colorBlack;
extern	idVec4 colorWhite;
extern	idVec4 colorRed;
extern	idVec4 colorGreen;
extern	idVec4 colorBlue;
extern	idVec4 colorYellow;
extern	idVec4 colorMagenta;
extern	idVec4 colorCyan;
extern	idVec4 colorOrange;
extern	idVec4 colorPurple;
extern	idVec4 colorPink;
extern	idVec4 colorBrown;
extern	idVec4 colorLtGrey;
extern	idVec4 colorMdGrey;
extern	idVec4 colorDkGrey;

// packs color floats in the range [0,1] into an integer
dword	PackColor( const idVec3 &color );
void	UnpackColor( const dword color, idVec3 &unpackedColor );
dword	PackColor( const idVec4 &color );
void	UnpackColor( const dword color, idVec4 &unpackedColor );

// little/big endian conversion
short	BigShort( short l );
short	LittleShort( short l );
int		BigLong( int l );
int		LittleLong( int l );
float	BigFloat( float l );
float	LittleFloat( float l );
void	BigRevBytes( void *bp, int elsize, int elcount );
void	LittleRevBytes( void *bp, int elsize, int elcount );
void	LittleBitField( void *bp, int elsize );
void	Swap_Init( void );

bool	Swap_IsBigEndian( void );

// for base64
void	SixtetsForInt( byte *out, int src);
int		IntForSixtets( byte *in );


#ifdef _DEBUG
void AssertFailed( const char *file, int line, const char *expression );
#undef assert
#define assert( X )		if ( X ) { } else AssertFailed( __FILE__, __LINE__, #X )
#endif

class idException {
public:
	char error[MAX_STRING_CHARS];

	idException( const char *text = "" ) { strcpy( error, text ); }
};


/*
===============================================================================

	idLib headers.

===============================================================================
*/

// memory management and arrays
#include "Heap.h"
#include "containers/List.h"

// math
#include "math/Simd.h"
#include "math/Math.h"
#include "math/Random.h"
#include "math/Complex.h"
#include "math/Vector.h"
#include "math/Matrix.h"
#include "math/Angles.h"
#include "math/Quat.h"
#include "math/Rotation.h"
#include "math/Plane.h"
#include "math/Pluecker.h"
#include "math/Polynomial.h"
#include "math/Extrapolate.h"
#include "math/Interpolate.h"
#include "math/Curve.h"
#include "math/Ode.h"
#include "math/Lcp.h"

// bounding volumes
#include "bv/Sphere.h"
#include "bv/Bounds.h"
#include "bv/Box.h"
#include "bv/Frustum.h"

// geometry
#include "geometry/DrawVert.h"
#include "geometry/JointTransform.h"
#include "geometry/Winding.h"
#include "geometry/Winding2D.h"
#include "geometry/Surface.h"
#include "geometry/Surface_Patch.h"
#include "geometry/Surface_Polytope.h"
#include "geometry/Surface_SweptSpline.h"
#include "geometry/TraceModel.h"

// text manipulation
#include "Str.h"
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "Base64.h"
#include "CmdArgs.h"

// containers
#include "containers/BTree.h"
#include "containers/BinSearch.h"
#include "containers/HashIndex.h"
#include "containers/HashTable.h"
#include "containers/StaticList.h"
#include "containers/LinkList.h"
#include "containers/Hierarchy.h"
#include "containers/Queue.h"
#include "containers/Stack.h"
#include "containers/StrList.h"
#include "containers/StrPool.h"
#include "containers/VectorSet.h"
#include "containers/PlaneSet.h"

// hashing
#include "hashing/CRC8.h"
#include "hashing/CRC16.h"
#include "hashing/CRC32.h"
#include "hashing/Honeyman.h"
#include "hashing/MD4.h"
#include "hashing/MD5.h"

// misc
#include "Dict.h"
#include "LangDict.h"
#include "BitMsg.h"
#include "MapFile.h"
#include "Timer.h"

#endif	/* !__LIB_H__ */
