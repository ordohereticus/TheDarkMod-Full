/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5083 $
 * $Date: 2011-12-04 23:50:26 -0500 (Sun, 04 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAMETYPEINFO_H__
#define __GAMETYPEINFO_H__

/*
===================================================================================

	This file has been generated with the Type Info Generator v1.0 (c) 2004 id Software

===================================================================================
*/

typedef struct {
	const char * name;
	const char * type;
	const char * value;
} constantInfo_t;

typedef struct {
	const char * name;
	int value;
} enumValueInfo_t;

typedef struct {
	const char * typeName;
	const enumValueInfo_t * values;
} enumTypeInfo_t;

typedef struct {
	const char * type;
	const char * name;
	int offset;
	int size;
} classVariableInfo_t;

typedef struct {
	const char * typeName;
	const char * superType;
	int size;
	const classVariableInfo_t * variables;
} classTypeInfo_t;


static constantInfo_t constantInfo[] = {
	{ NULL, NULL, NULL }
};

static enumTypeInfo_t enumTypeInfo[] = {
	{ NULL, NULL }
};

static classTypeInfo_t classTypeInfo[] = {
	{ NULL, NULL, 0, NULL }
};

#endif /* !__GAMETYPEINFO_H__ */
