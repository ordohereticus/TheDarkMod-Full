/***************************************************************************

 *

 * PROJECT: The Dark Mod

 * $Source$

 * $Revision: 723 $

 * $Date: 2007-01-16 22:45:38 -0500 (Tue, 16 Jan 2007) $

 * $Author: thelvyn $

 *

 * $Log$
 * Revision 1.3  2007/01/17 03:44:24  thelvyn
 * *** empty log message ***
 *
 * Revision 1.2  2005/11/10 19:21:04  sparhawk
 * SDK 1.3 Merge
 *

 * Revision 1.1.1.1  2004/10/30 15:52:34  sparhawk

 * Initial release

 *

 ***************************************************************************/



// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __DECLFX_H__
#define __DECLFX_H__

/*
===============================================================================

	idDeclFX

===============================================================================
*/

enum { 
	FX_LIGHT,
	FX_PARTICLE,
	FX_DECAL,
	FX_MODEL,
	FX_SOUND,
	FX_SHAKE,
	FX_ATTACHLIGHT,
	FX_ATTACHENTITY,
	FX_LAUNCH,
	FX_SHOCKWAVE
};

//
// single fx structure
//
typedef struct {
	int						type;
	int						sibling;

	idStr					data;
	idStr					name;
	idStr					fire;

	float					delay;
	float					duration;
	float					restart;
	float					size;
	float					fadeInTime;
	float					fadeOutTime;
	float					shakeTime;
	float					shakeAmplitude;
	float					shakeDistance;
	float					shakeImpulse;
	float					lightRadius;
	float					rotate;
	float					random1;
	float					random2;

	idVec3					lightColor;
	idVec3					offset;
	idMat3					axis;

	bool					soundStarted;
	bool					shakeStarted;
	bool					shakeFalloff;
	bool					shakeIgnoreMaster;
	bool					bindParticles;
	bool					explicitAxis;
	bool					noshadows;
	bool					particleTrackVelocity;
	bool					trackOrigin;
} idFXSingleAction;

//
// grouped fx structures
//
class idDeclFX : public idDecl {
public:
	virtual size_t			Size( void ) const;
	virtual const char *	DefaultDefinition( void ) const;
	virtual bool			Parse( const char *text, const int textLength );
	virtual void			FreeData( void );
	virtual void			Print( void ) const;
	virtual void			List( void ) const;

	idList<idFXSingleAction>events;
	idStr					joint;

private:
	void					ParseSingleFXAction( idLexer &src, idFXSingleAction& FXAction );
};

#endif /* !__DECLFX_H__ */
