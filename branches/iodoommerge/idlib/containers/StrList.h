/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5122 $ (Revision of last commit) 
 $Date: 2011-12-11 14:47:31 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#ifndef __STRLIST_H__
#define __STRLIST_H__

/*
===============================================================================

	idStrList

===============================================================================
*/

typedef idList<idStr> idStrList;
typedef idList<idStr*> idStrPtrList;
typedef idStr *idStrPtr;

/*
================
idListSortCompare<idStrPtr>

Compares two pointers to strings. Used to sort a list of string pointers alphabetically in idList<idStr>::Sort.
================
*/
template<>
ID_INLINE int idListSortCompare<idStrPtr>( const idStrPtr *a, const idStrPtr *b ) {
	return ( *a )->Icmp( **b );
}

/*
================
idStrList::Sort

Sorts the list of strings alphabetically. Creates a list of pointers to the actual strings and sorts the
pointer list. Then copies the strings into another list using the ordered list of pointers.
================
*/
template<>
ID_INLINE void idStrList::Sort( cmp_t *compare ) {
	int i;

	if ( !num ) {
		return;
	}

	idList<idStr>		other;
	idList<idStrPtr>	pointerList;

	pointerList.SetNum( num );
	for( i = 0; i < num; i++ ) {
		pointerList[ i ] = &( *this )[ i ];
	}

	pointerList.Sort();

	other.SetNum( num );
	other.SetGranularity( granularity );
	for( i = 0; i < other.Num(); i++ ) {
		other[ i ] = *pointerList[ i ];
	}

	this->Swap( other );
}

/*
================
idStrList::SortSubSection

Sorts a subsection of the list of strings alphabetically.
================
*/
template<>
ID_INLINE void idStrList::SortSubSection( int startIndex, int endIndex, cmp_t *compare ) {
	int i, s;

	if ( !num ) {
		return;
	}
	if ( startIndex < 0 ) {
		startIndex = 0;
	}
	if ( endIndex >= num ) {
		endIndex = num - 1;
	}
	if ( startIndex >= endIndex ) {
		return;
	}

	idList<idStr>		other;
	idList<idStrPtr>	pointerList;

	s = endIndex - startIndex + 1;
	other.SetNum( s );
	pointerList.SetNum( s );
	for( i = 0; i < s; i++ ) {
		other[ i ] = ( *this )[ startIndex + i ];
		pointerList[ i ] = &other[ i ];
	}

	pointerList.Sort();

	for( i = 0; i < s; i++ ) {
		(*this)[ startIndex + i ] = *pointerList[ i ];
	}
}

/*
================
idStrList::Size
================
*/
template<>
ID_INLINE size_t idStrList::Size( void ) const {
	size_t s;
	int i;

	s = sizeof( *this );
	for( i = 0; i < Num(); i++ ) {
		s += ( *this )[ i ].Size();
	}

	return s;
}

/*
===============================================================================

	idStrList path sorting

===============================================================================
*/

/*
================
idListSortComparePaths

Compares two pointers to strings. Used to sort a list of string pointers alphabetically in idList<idStr>::Sort.
================
*/
template<class idStrPtr>
ID_INLINE int idListSortComparePaths( const idStrPtr *a, const idStrPtr *b ) {
	return ( *a )->IcmpPath( **b );
}

/*
================
idStrListSortPaths

Sorts the list of path strings alphabetically and makes sure folders come first.
================
*/
ID_INLINE void idStrListSortPaths( idStrList &list ) {
	int i;

	if ( !list.Num() ) {
		return;
	}

	idList<idStr>		other;
	idList<idStrPtr>	pointerList;

	pointerList.SetNum( list.Num() );
	for( i = 0; i < list.Num(); i++ ) {
		pointerList[ i ] = &list[ i ];
	}

	pointerList.Sort( idListSortComparePaths<idStrPtr> );

	other.SetNum( list.Num() );
	other.SetGranularity( list.GetGranularity() );
	for( i = 0; i < other.Num(); i++ ) {
		other[ i ] = *pointerList[ i ];
	}

	list.Swap( other );
}

#endif /* !__STRLIST_H__ */
