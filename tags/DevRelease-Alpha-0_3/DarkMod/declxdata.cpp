/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 465 $
 * $Date: 2006-06-21 09:08:20 -0400 (Wed, 21 Jun 2006) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.5  2006/06/21 13:05:32  sparhawk
 * Added version tracking per cpp module
 *
 * Revision 1.4  2006/03/25 09:52:43  gildoran
 * Altered the parse functions for the decls I wrote to adhere to our coding standards.
 *
 * Revision 1.3  2006/03/25 08:13:45  gildoran
 * New update for declarations... Improved the documentation/etc for xdata decls, and added some basic code for tdm_matinfo decls.
 *
 * Revision 1.2  2006/03/23 14:13:38  gildoran
 * Added import command to xdata decls.
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Source$  $Revision: 465 $   $Date: 2006-06-21 09:08:20 -0400 (Wed, 21 Jun 2006) $", init_version);

#include "declxdata.h"

tdmDeclXData::tdmDeclXData()
{
}

tdmDeclXData::~tdmDeclXData()
{
	FreeData();
}

size_t tdmDeclXData::Size() const
{
	return sizeof(tdmDeclXData);
}

const char *tdmDeclXData::DefaultDefinition() const
{
	return "{}";
}

void tdmDeclXData::FreeData()
{
	m_data.Clear();
}

// Note Our coding standards require using gotos in this sort of code.
bool tdmDeclXData::Parse( const char *text, const int textLength )
{
	// Only set to true if we have successfully parsed the decl.
	bool		successfulParse = false;

	idLexer		src;
	idToken		tKey;
	idToken		tVal;

	src.LoadMemory( text, textLength, GetFileName(), GetLineNum() );
	src.SetFlags( DECL_LEXER_FLAGS & ~LEXFL_NOSTRINGESCAPECHARS );

	bool		precache = false;
	idStr		value;
	idDict		importKeys;
	const tdmDeclXData *xd;
	const idDict *importData;
	const idKeyValue *kv;
	const idKeyValue *kv2;
	int i;

	// Skip until the opening brace. I don't trust using the
	// skipUntilString function, since it could be fooled by
	// a string containing a brace.
	do {
		if ( !src.ReadToken( &tKey ) ) {
			goto Quit;
		}
	} while ( tKey.type != TT_PUNCTUATION || tKey.subtype != P_BRACEOPEN );
	//src.SkipUntilString( "{" );

	while (1)
	{
		// If there's an EOF, fail to load.
		if ( !src.ReadToken( &tKey ) ) {
			src.Warning( "Unclosed xdata decl." );
			goto Quit;
		}

		// Quit upon encountering the closing brace.
		if ( tKey.type == TT_PUNCTUATION && tKey.subtype == P_BRACECLOSE) {
			break;
		}

		if ( tKey.type == TT_STRING ) {

			if ( !src.ReadToken( &tVal ) ||
				 tVal.type != TT_PUNCTUATION ||
				 tVal.subtype != P_COLON ) {
				src.Warning( "Abandoned key: %s", tKey.c_str() );
				goto Quit;
			}

			// We're parsing a key/value pair.
			if ( !src.ReadToken( &tVal ) ) {
				src.Warning("Unexpected EOF in key:value pair.");
				goto Quit;
			}

			if ( tVal.type == TT_STRING ) {

				// Set the key:value pair.
				m_data.Set( tKey.c_str(), tVal.c_str() );

			} else if ( tVal.type == TT_PUNCTUATION && tVal.subtype == P_BRACEOPEN ) {

				value = "";

				while (1) {
					if ( !src.ReadToken( &tVal ) ) {
						src.Warning("EOF encountered inside value block.");
						goto Quit;
					}

					if ( tVal.type == TT_PUNCTUATION && tVal.subtype == P_BRACECLOSE ) {
						break;
					}

					if ( tVal.type != TT_STRING ) {
						src.Warning( "Non-string encountered in value block: %s", tVal.c_str() );
						goto Quit;
					}

					value += tVal + "\n";
				}

				// Set the key:value pair.
				m_data.Set( tKey.c_str(), value.c_str() );

			} else {
				src.Warning( "Invalid value: %s", tVal.c_str() );
				goto Quit;
			}

		} else if ( tKey.type == TT_NAME ) {

			if ( tKey.Icmp("precache") == 0 ) {
				precache = true;
			} else if ( tKey.Icmp("import") == 0 ) {

				if ( !src.ReadToken( &tKey ) )
				{
					src.Warning("Unexpected EOF in import statement.");
					goto Quit;
				}

				if ( tKey.type == TT_PUNCTUATION && tKey.subtype == P_BRACEOPEN ) {

					// Initialize the list of keys to copy over.
					importKeys.Clear();

					while (1) {

						if ( !src.ReadToken( &tKey ) ) {
							src.Warning("Unexpected EOF in import block.");
							goto Quit;
						}

						if ( tKey.type == TT_PUNCTUATION && tKey.subtype == P_BRACECLOSE ) {
							break;
						}

						if ( tKey.type != TT_STRING ) {
							src.Warning( "Invalid source key: %s", tKey.c_str() );
							goto Quit;
						}

						if ( !src.ReadToken( &tVal ) ) {
							src.Warning("Unexpected EOF in import block.");
							goto Quit;
						}

						if ( tVal.type == TT_PUNCTUATION && tVal.subtype == P_POINTERREF ) {

							if ( !src.ReadToken( &tVal ) ) {
								src.Warning("Unexpected EOF in import block.");
								goto Quit;
							}

							if ( tVal.type != TT_STRING ) {
								src.Warning( "Invalid target key: %s", tVal.c_str() );
								goto Quit;
							}

							importKeys.Set( tKey.c_str(), tVal.c_str() );

						} else {

							// We accidently read too far.
							src.UnreadToken( &tVal );

							importKeys.Set( tKey.c_str(), tKey.c_str() );
						}

					}

					if ( !src.ReadToken( &tKey ) ||
						 tKey.type != TT_NAME ||
						 tKey.Icmp("from") != 0 ) {
						src.Warning( "Missing from statement.", tKey.c_str() );
						goto Quit;
					}

					if ( !src.ReadToken( &tKey ) ||
						 tKey.type != TT_STRING ) {
						src.Warning( "Invalid xdata for importation." );
						goto Quit;
					}

					xd = static_cast< const tdmDeclXData* >( declManager->FindType( DECL_XDATA, tKey.c_str(), false ) );
					if ( xd != NULL ) {

						importData = &(xd->m_data);

						i = importKeys.GetNumKeyVals();
						while (i--) {
							kv = importKeys.GetKeyVal(i);
							kv2 = importData->FindKey( kv->GetKey() );
							m_data.Set( kv->GetValue(), kv2->GetValue() );
						}

					} else {
						src.Warning( "Unable to load xdata for importation: %s", tKey.c_str() );
						goto Quit;
					}

				} else if ( tKey.type == TT_STRING ) {

					xd = static_cast< const tdmDeclXData* >( declManager->FindType( DECL_XDATA, tKey.c_str(), false ) );
					if ( xd != NULL ) {
						m_data.Copy( xd->m_data );
					} else {
						src.Warning( "Unable to load xdata for importation: %s", tKey.c_str() );
						goto Quit;
					}

				} else {
					src.Warning("Syntax error immediately after import statement.");
					goto Quit;
				}

			} else {
				src.Warning( "Unrecognized command: %s", tKey.c_str() );
				goto Quit;
			}

		}
	}

	if (precache) {
		gameLocal.CacheDictionaryMedia( &m_data );
	}

	successfulParse = true;

	Quit:
	if (!successfulParse) {
		MakeDefault();
	}
	return successfulParse;
}
