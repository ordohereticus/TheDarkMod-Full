/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4798 $
 * $Date: 2011-04-17 04:09:52 -0400 (Sun, 17 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ObjectiveComponent.cpp 4798 2011-04-17 08:09:52Z greebo $", init_version);

#include "ObjectiveComponent.h"

/**
* Add new component type names here.  Must be in exact same order as EComponentType
* enum, defined in the header
**/
const char* gCompTypeName[COMP_COUNT] =
{
	"kill",
	"ko",
	"ai_find_item",
	"ai_find_body",
	"alert",
	"destroy",
	"item",
	"pickpocket",
	"location",
	"custom",
	"custom_clocked",
	"info_location",
	"distance",
	"readable_opened",
	"readable_closed",
	"readable_page_reached"
};

CObjectiveComponent::CObjectiveComponent( void )
{
	m_bNotted = false;
	m_bState = false;
	m_EventCount = 0;
	m_bPlayerResponsibleOnly = true;
	m_bReversible = true;
	m_bLatched = false;
	m_Type = COMP_ITEM;
	m_SpecMethod[0] = SPEC_NONE;
	m_SpecMethod[1] = SPEC_NONE;
	m_Args.Clear();

	m_ClockInterval = 1000;
	m_TimeStamp = 0;

	m_Index[0] = 0;
	m_Index[1] = 0;
}

CObjectiveComponent::~CObjectiveComponent( void )
{
	m_SpecVal[0].Clear();
	m_SpecVal[1].Clear();

	m_Args.Clear();
}

bool CObjectiveComponent::SetState( bool bState )
{
	bool bReturnVal(false);

	if( m_bNotted )
		bState = !bState;

	if(bState != m_bState)
	{
		// state has changed, check for latching to see if it can change
		if( !m_bReversible )
		{
			if( !m_bLatched )
			{
				m_bLatched = true;
				m_bState = bState;
				bReturnVal = true;
			}
		}
		else
		{
			m_bState = bState;
			bReturnVal = true;
		}
	}

	return bReturnVal;
}

void CObjectiveComponent::Save( idSaveGame *savefile ) const
{
	savefile->WriteInt( m_Index[0] );
	savefile->WriteInt( m_Index[1] );
	savefile->WriteBool( m_bNotted );
	savefile->WriteInt( m_Type );
	savefile->WriteInt( m_SpecMethod[0] );
	savefile->WriteInt( m_SpecMethod[1] );
	savefile->WriteString( m_SpecVal[0] );
	savefile->WriteString( m_SpecVal[1] );
	savefile->WriteBool( m_bState );
	savefile->WriteInt( m_EventCount );
	savefile->WriteBool( m_bPlayerResponsibleOnly );
	savefile->WriteBool( m_bLatched );

	savefile->WriteInt( m_Args.Num() );
	for( int j=0; j<m_Args.Num(); j++ )
		savefile->WriteString( m_Args[j] );

	savefile->WriteInt( m_ClockInterval );
	savefile->WriteInt( m_TimeStamp );
	savefile->WriteBool( m_bReversible );
}

void CObjectiveComponent::Restore( idRestoreGame *savefile )
{
	int num(0), tempInt(0);

	savefile->ReadInt( m_Index[0] );
	savefile->ReadInt( m_Index[1] );
	savefile->ReadBool( m_bNotted );
	savefile->ReadInt( tempInt );
	m_Type = (EComponentType) tempInt;
	savefile->ReadInt( tempInt );
	m_SpecMethod[0] = (ESpecificationMethod) tempInt;
	savefile->ReadInt( tempInt );
	m_SpecMethod[1] = (ESpecificationMethod) tempInt;
	savefile->ReadString( m_SpecVal[0] );
	savefile->ReadString( m_SpecVal[1] );
	savefile->ReadBool( m_bState );
	savefile->ReadInt( m_EventCount );
	savefile->ReadBool( m_bPlayerResponsibleOnly );
	savefile->ReadBool( m_bLatched );

	savefile->ReadInt( num );
	m_Args.SetNum( num );
	for( int j=0; j<num; j++ )
		savefile->ReadString( m_Args[j] );

	savefile->ReadInt( m_ClockInterval );
	savefile->ReadInt( m_TimeStamp );
	savefile->ReadBool( m_bReversible );
}
