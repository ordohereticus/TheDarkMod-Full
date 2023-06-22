/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 50 $
 * $Date: 2004-11-28 13:35:17 -0500 (Sun, 28 Nov 2004) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.1  2004/11/28 18:30:46  sparhawk
 * SDK V2 merge
 *
 *
 ***************************************************************************/

#ifndef __MAYA_MAIN_H__
#define __MAYA_MAIN_H__

/*
==============================================================

	Maya Import

==============================================================
*/


typedef bool ( *exporterDLLEntry_t )( int version, idCommon *common, idSys *sys );
typedef const char *( *exporterInterface_t )( const char *ospath, const char *commandline );
typedef void ( *exporterShutdown_t )( void );

#endif /* !__MAYA_MAIN_H__ */
