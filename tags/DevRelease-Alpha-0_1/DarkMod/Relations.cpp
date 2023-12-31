/******************************************************************************/
/*                                                                            */
/*         Dark Mod AI Relationships (C) by Chris Sarantos in USA 2005		  */
/*                          All rights reserved                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************
*
*	Class for storing and managing AI to Player and AI to AI relationships
*	See header file for complete description.
*
*****************************************************************************/

/******************************************************************************
 *
 * PROJECT: DarkMod
 * $Source$
 * $Revision: 96 $
 * $Date: 2005-03-30 13:16:20 -0500 (Wed, 30 Mar 2005) $
 * $Author: sparhawk $
 * $Name$
 *
 * $Log$
 * Revision 1.2  2005/03/30 18:16:20  sparhawk
 * CVS Header added
 *
 ******************************************************************************/

// TODO: Go thru game_local and call Save and Restore relations when game
// is saved or restored.

#pragma hdrstop

#pragma warning(disable : 4996)

#include "relations.h"
#include "matrixsq.h"
#include "../game/game_local.h"

static const int s_DefaultRelation = -1;

static const int s_DefaultSameTeamRel = 5;

CLASS_DECLARATION( idClass, CRelations )
END_CLASS

CRelations::CRelations( void )
{
	m_RelMat = new CMatrixSq<int>;
}

CRelations::~CRelations( void )
{
	Clear();
	delete m_RelMat;
}

CRelations &CRelations::operator=(const CRelations &in)
{
	if(!m_RelMat->IsCleared())
		Clear();

	m_RelMat->Copy( in.m_RelMat );

	return *this;
}

void CRelations::Clear( void )
{
	if( !m_RelMat->IsCleared() )
	{
		m_RelMat->Clear();
	}
}

bool CRelations::IsCleared( void )
{
	return m_RelMat->IsCleared();
}

int CRelations::Size( void )
{
	return m_RelMat->Dim();
}

int CRelations::GetRelNum(int i, int j)
{
	int *pval, returnval;

	// uncomment for debugging of relationship checks (happens rather frequently and spams the log file)
	// DM_LOG(LC_AI, LT_DEBUG).LogString("Checking relationship matrix for team %d towards team %d.\r", i, j);
	pval = m_RelMat->Get( i , j );

	if ( pval == NULL )
	{
		// uncomment for reporting errors when doing relationship checks
		//idLib::common->Warning( "Warning: Bad indices given for relationship matrix.  Assuming slight enemy relationship.\n" );
		//DM_LOG(LC_AI, LT_ERROR).LogString("Bad indices used to query relationship matrix: %d, col: %d.\r", i, j);
		
		returnval = s_DefaultRelation;
		goto Quit;
	}

	returnval = *pval;

Quit:
	return returnval;
}

int CRelations::GetRelType(int i, int j)
{
	int returnval, relNum;

	relNum = GetRelNum(i, j);

	if ( relNum<0 )
		returnval = E_ENEMY;

	else if ( relNum>0 )
		returnval = E_FRIEND;
	
	else if ( relNum == 0 )
		returnval = E_NEUTRAL;

	return returnval;
}

void CRelations::SetRel(int i, int j, int rel)
{
	m_RelMat->Set( i, j, rel );
}

void CRelations::ChangeRel( int i, int j, int offset)
{
	int entry, val;

	entry = GetRelNum( i, j );
	val = entry + offset;

	SetRel( i, j, val );
}

bool CRelations::IsFriend( int i, int j)
{
	if ( GetRelNum(i, j) > 0 )
		return true;
	else
		return false;
}

bool CRelations::IsEnemy( int i, int j)
{
	if ( GetRelNum(i, j) < 0 )
		return true;
	else
		return false;
}

bool CRelations::IsNeutral( int i, int j)
{
	if ( GetRelNum(i, j) == 0 )
		return true;
	else
		return false;
}

bool CRelations::SetFromArgs( idDict *args )
{
	idList<SEntryData> EntryList;
	idList<int> DiagsAdded;
	SEntryData EntryDat;
	int start, end, num(1), tempint, tempint2, maxrow(0), keylen;
	idStr tempKey, tempVal, row, col, val;

	bool hadSynError(false), hadLogicError(false);
	
	const idKeyValue *Entry = args->MatchPrefix( "rel ", NULL );
	while ( Entry ) 
	{
		tempKey = Entry->GetKey();
		tempVal = Entry->GetValue();

		keylen = tempKey.Length();
		
		// parse it
		start = 4;
		end = tempKey.FindChar(tempKey.c_str(), ',');
		end--;

		DM_LOG(LC_AI, LT_DEBUG).LogString("Relmat Parser: arg %d, start = %d, end(comma-1) = %d\r", num, start, end);

		if(end == -1)
		{
			hadSynError = true;
			goto Quit;
		}
		
		row = tempKey.Mid( start, (end - start + 1) );

		start = end+2;

		if( start > keylen )
		{
			hadSynError = true;
			goto Quit;
		}

		col = tempKey.Mid( start, keylen - start + 1 );

		val = args->GetString( tempKey.c_str(), "" );

		DM_LOG(LC_AI, LT_DEBUG).LogString("Relmat Parser: arg %d, row = %s, col = %s, val = %d\r", num, row.c_str(), col.c_str(), atoi(val.c_str()) );

		if( !row.IsNumeric() || !col.IsNumeric() || !val.IsNumeric() )
		{
			hadSynError = true;
			goto Quit;
		}

		//set up the Entry data
		EntryDat.row = atoi( row.c_str() );
		EntryDat.col = atoi( col.c_str() );
		EntryDat.val = atoi( val.c_str() );

		if (EntryDat.row > maxrow )
			maxrow = EntryDat.row;

		EntryList.Append(EntryDat);

		// Check for diagonal element of the ROW team, fill it in with
		// the default diagonal relation if it does not exist.

		if (args->FindKeyIndex( va("rel %d,%d", EntryDat.row, EntryDat.row) ) == -1
								&& DiagsAdded.FindIndex(EntryDat.row) == -1 )
		{
			tempint = EntryDat.col;
			tempint2 = EntryDat.val;

			EntryDat.col = EntryDat.row;
			EntryDat.val = s_DefaultSameTeamRel;

			EntryList.Append(EntryDat);
			DiagsAdded.Append(EntryDat.row);

			DM_LOG(LC_AI, LT_DEBUG).LogString("Relmat Parser: Added missing diagonal %d, %d\r", EntryDat.row, EntryDat.row );

			//set EntryDat back the way it was for further checks
			EntryDat.col = tempint;
			EntryDat.val = tempint2;
		}

		// Check for diagonal element of the COLUMN team, fill it in with
		// the default diagonal relation if it does not exist.

		if (args->FindKeyIndex( va("rel %d,%d", EntryDat.col, EntryDat.col) ) == -1
								&& DiagsAdded.FindIndex(EntryDat.col) == -1 )
		{
			tempint = EntryDat.row;
			tempint2 = EntryDat.val;
			
			if( EntryDat.col > maxrow )
				maxrow = EntryDat.col;

			EntryDat.row = EntryDat.col;
			EntryDat.val = s_DefaultSameTeamRel;
			EntryList.Append(EntryDat);
			DiagsAdded.Append(EntryDat.col);

			DM_LOG(LC_AI, LT_DEBUG).LogString("Relmat Parser: Added missing diagonal %d, %d\r", EntryDat.col, EntryDat.col );

			//set EntryDat back the way it was for further checks
			EntryDat.row = tempint;
			EntryDat.val = tempint2;
		}

		// Check for asymmetric element and append one with same val if
		// it does not exist.
		if ( args->FindKeyIndex( va("rel %d,%d", EntryDat.col, EntryDat.row) ) == -1 )
		{
			tempint = EntryDat.row;
			EntryDat.row = EntryDat.col;
			EntryDat.col = tempint;

			EntryList.Append(EntryDat);
			DM_LOG(LC_AI, LT_DEBUG).LogString("Relmat Parser: Added missing asymmetric element %d, %d\r", EntryDat.row, EntryDat.col );
		}

		Entry = args->MatchPrefix( "rel ", Entry );
		num++;
	}

	EntryList.Condense();

	maxrow++;
	if (EntryList.Num() != (maxrow*maxrow))
	{
		hadLogicError = true;
		goto Quit;
	}

	if ( !m_RelMat->Init(maxrow) )
	{
		hadLogicError = true;
		goto Quit;
	}

	for( int i=0; i<EntryList.Num(); i++ )
	{
		if ( !m_RelMat->Set(EntryList[i].row, EntryList[i].col, EntryList[i].val ) )
		{
			hadLogicError = true;
			goto Quit;
		}
	}

Quit:
	if(hadSynError)
	{
		DM_LOG(LC_AI, LT_ERROR).LogString("[AI Relations] Syntax error when parsing Worldspawn args to Relationship Manager (arg number %d from the top)\r", num);
		// make this a warning for now, so we can play maps without relationship matrices
		//idLib::common->Error("Syntax error when parsing Worldspawn args to Relationship Manager (arg number %d from the top)", num);
		idLib::common->Warning("[AI Relations] Syntax error when parsing Worldspawn args to Relationship Manager (arg number %d from the top)\r", num);
	}

	if(hadLogicError)
	{
		DM_LOG(LC_AI, LT_ERROR).LogString("[AI Relations] Logical error when parsing Worldspawn args to Relationship Manager (matrix indices are incorrect or missing)\r");
		DM_LOG(LC_AI, LT_ERROR).LogString("[AI Relations] (number of elements = %d, required elements = %d)\r", EntryList.Num(), (maxrow*maxrow));
		// make this a warning for now, so we can play maps without relationship matrices
		//idLib::common->Error("Logical error when parsing Worldspawn args to Relationship Manager (matrix indices are incorrect)");
		idLib::common->Warning("[AI Relations] Logical error when parsing Worldspawn args to Relationship Manager (matrix indices are incorrect or missing)\r");
	}

	return !(hadSynError || hadLogicError);
}


void CRelations::Save( idSaveGame *save ) const
{
	DM_LOG(LC_AI, LT_DEBUG).LogString("Saving Relationship Matrix data\r");
	m_RelMat->SaveMatrixSq( save );
}

void CRelations::Restore( idRestoreGame *save )
{
	DM_LOG(LC_AI, LT_DEBUG).LogString("Loading Relationship Matrix data from save\r");
	m_RelMat->RestoreMatrixSq( save );

	CopyThisToGlobal();
}

void CRelations::CopyThisToGlobal( void )
{
	g_globalRelations = *this;
}

void CRelations::DebugPrintMat( void )
{
	int i, row, col, val;
	
	idLib::common->Printf("Printing Relations Matrix with %d elements:\n", m_RelMat->NumFilled() );
	DM_LOG(LC_AI, LT_DEBUG).LogString("DebugPrintMat: m_RelMat::IsCleared = %d\r", m_RelMat->IsCleared() );
	if( m_RelMat->IsCleared() )
	{
		idLib::common->Printf("Relations Matrix is Empty.\n");
		goto Quit;
	}

	for(i=0; i < m_RelMat->NumFilled(); i++)
	{
		row = i / m_RelMat->Dim();
		col = i % m_RelMat->Dim();
		val = *m_RelMat->Get1d(i);
		idLib::common->Printf("[Relations Matrix] Element %d,%d: %d\n", row, col, val );
	}

Quit:
	return;
}