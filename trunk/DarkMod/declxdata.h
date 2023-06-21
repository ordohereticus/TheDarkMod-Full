/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 375 $
 * $Date: 2006-03-23 09:13:51 -0500 (Thu, 23 Mar 2006) $
 * $Author: gildoran $
 *
 * $Log$
 * Revision 1.2  2006/03/23 14:13:38  gildoran
 * Added import command to xdata decls.
 *
 *
 *
 ***************************************************************************/

#ifndef __DARKMOD_DECLXDATA_H__
#define __DARKMOD_DECLXDATA_H__

class tdmDeclXData : public idDecl
{
public:
	tdmDeclXData();
	~tdmDeclXData();

	virtual size_t			Size( void ) const;
	virtual const char *	DefaultDefinition( void ) const;
	virtual void			FreeData( void );
	virtual bool			Parse( const char *text, const int textLength );

	idDict m_data;

};

#endif /* __DARKMOD_DECLXDATA_H__ */
