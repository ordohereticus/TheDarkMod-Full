/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5171 $ (Revision of last commit) 
 $Date: 2012-01-07 03:08:06 -0500 (Sat, 07 Jan 2012) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_engine.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: GEModifierGroup.cpp 5171 2012-01-07 08:08:06Z greebo $");

#include "GEModifierGroup.h"

rvGEModifierGroup::rvGEModifierGroup ( ) :
	rvGEModifier ( "Group", NULL )
{	
}

rvGEModifierGroup::~rvGEModifierGroup ( )
{
	int i;
	
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		delete mModifiers[i];
	}
	
	mModifiers.Clear ( );
}

bool rvGEModifierGroup::Append ( rvGEModifier* mod )
{
	// All modifiers must be the same type
	assert ( !mModifiers.Num() || !idStr::Icmp ( mod->GetName ( ), mModifiers[0]->GetName ( ) ) );

	if ( !mModifiers.Num ( ) )
	{
		mName = mod->GetName ( );
	}

	mModifiers.Append ( mod );
	return true;
}

bool rvGEModifierGroup::IsValid ( void )
{
	int i;
	
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		if ( !mModifiers[i]->IsValid ( ) )
		{
			return false;
		}
	}
	
	return true;
}

bool rvGEModifierGroup::Apply ( void )
{
	int i;
	
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		mModifiers[i]->Apply ( );
	}
	
	return true;
}

bool rvGEModifierGroup::Undo ( void )
{
	int i;
	
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		mModifiers[i]->Undo ( );
	}
	
	return true;
}

bool rvGEModifierGroup::CanMerge ( rvGEModifier* mergebase )
{
	rvGEModifierGroup*	merge = (rvGEModifierGroup*) mergebase;
	int					i;
			
	if ( mModifiers.Num() != merge->mModifiers.Num ( ) )
	{
		return false;
	}
			
	// Double check the merge is possible
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		if ( mModifiers[i]->GetWindow() != merge->mModifiers[i]->GetWindow() )
		{
			return false;
		}
		
		if ( idStr::Icmp ( mModifiers[i]->GetName ( ), merge->mModifiers[i]->GetName ( ) ) )
		{
			return false;
		}

		if ( !mModifiers[i]->CanMerge ( merge->mModifiers[i] ) )
		{
			return false;
		}
	}

	return true;
}

bool rvGEModifierGroup::Merge ( rvGEModifier* mergebase )
{
	rvGEModifierGroup*	merge = (rvGEModifierGroup*) mergebase;
	int					i;
	
	// Double check the merge is possible
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		mModifiers[i]->Merge ( merge->mModifiers[i] );
	}
	
	return true;
}
