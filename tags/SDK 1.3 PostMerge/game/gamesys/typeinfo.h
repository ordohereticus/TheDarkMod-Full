/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 17:25:02 -0400 (Fri, 23 Mar 2007) $
 * $Author: sparhawk $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __SYS_TYPEINFO_H__
#define __SYS_TYPEINFO_H__

/*
===================================================================================

	Game Type Info

===================================================================================
*/

const char *	GetTypeVariableName( const char *typeName, int offset );

void			PrintType( const void *typePtr, const char *typeName );
void			WriteTypeToFile( idFile *fp, const void *typePtr, const char *typeName );
void			InitTypeVariables( const void *typePtr, const char *typeName, int value );

void			ListTypeInfo_f( const idCmdArgs &args );

void			WriteGameState_f( const idCmdArgs &args );
void			CompareGameState_f( const idCmdArgs &args );
void			TestSaveGame_f( const idCmdArgs &args );

#endif /* !__SYS_TYPEINFO_H__ */
