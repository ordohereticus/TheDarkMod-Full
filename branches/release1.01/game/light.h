/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3679 $
 * $Date: 2009-08-30 06:28:05 -0400 (Sun, 30 Aug 2009) $
 * $Author: tels $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_LIGHT_H__
#define __GAME_LIGHT_H__

/*
===============================================================================

  Generic light.

===============================================================================
*/
class CLightMaterial;

extern const idEventDef EV_Light_GetLightParm;
extern const idEventDef EV_Light_SetLightParm;
extern const idEventDef EV_Light_SetLightParms;

class idLight : public idEntity {
public:
	CLASS_PROTOTYPE( idLight );

	idLight();
	virtual ~idLight();

	void			Spawn( void );

	void			Save( idSaveGame *savefile ) const;					// archives object for save game file
	void			Restore( idRestoreGame *savefile );					// unarchives object from save game file

	virtual void	UpdateChangeableSpawnArgs( const idDict *source );
	virtual void	Think( void );
	virtual void	FreeLightDef( void );
	virtual bool	GetPhysicsToSoundTransform( idVec3 &origin, idMat3 &axis );
	void			Present( void );

	void			SaveState( idDict *args );
	virtual void	SetColor( float red, float green, float blue );
	virtual void	SetColor( const idVec4 &color );
	virtual void	GetColor( idVec3 &out ) const;
	virtual void	GetColor( idVec4 &out ) const;

	/**
	* Tels: idLight::GetLightOrigin returns the origin of the light in the world. This
	* is different from the physics origin, since the light can be offset.
	*/
	const idVec3 &	GetLightOrigin( void ) const { return renderLight.origin; }

	const idVec3 &	GetBaseColor( void ) const { return baseColor; }
	void			SetShader( const char *shadername );
	void			SetLightParm( int parmnum, float value );
	void			SetLightParms( float parm0, float parm1, float parm2, float parm3 );
	void			SetRadiusXYZ( const float x, const float y, const float z );
	void			SetRadius( const float radius );
	void			On( void );
	void			Off( void );
	void			Fade( const idVec4 &to, float fadeTime );
	void			FadeOut( float time );
	void			FadeIn( float time );
	void			FadeTo( idVec3 color, float time );
	void			Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );
	void			BecomeBroken( idEntity *activator );
	qhandle_t		GetLightDefHandle( void ) const { return lightDefHandle; }
	void			SetLightParent( idEntity *lparent ) { lightParent = lparent; }
	void			SetLightLevel( void );

	/**
	 * greebo: Returns the current lightlevel (currentlevel).
	 */
	int				GetLightLevel() const;

	/**
	 * Tels: return the current light radius.
	 */
	void			GetRadius( idVec3 &out ) const;

	virtual void	ShowEditingDialog( void );

	enum {
		EVENT_BECOMEBROKEN = idEntity::EVENT_MAXEVENTS,
		EVENT_MAXEVENTS
	};

	virtual void	ClientPredictionThink( void );
	virtual void	WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void	ReadFromSnapshot( const idBitMsgDelta &msg );
	virtual bool	ClientReceiveEvent( int event, int time, const idBitMsg &msg );

	/**	Returns a bounding box surrounding the light.
	 */
	idBounds		GetBounds();
	/**	Called to update m_renderTrigger after the render light is modified.
	 *	Only updates the render trigger if a thread is waiting for it.
	 */
	void			PresentRenderTrigger();

	/**
	 * This will return a grayscale value dependent on the value from the light.
	 * X and Y are the coordinates returned by calculating the position from the 
	 * player related to the light. The Z coordinate can be ignored. The distance
	 * is required when the light has no textures to calculate a falloff.
	 */
	float			GetDistanceColor(float distance, float x, float y);

	/**
	 * GetTextureIndex calculates the index into the texture based on the x/y coordinates
	 * given and returns the index.
	 */
	int				GetTextureIndex(float x, float y, int TextureWidth, int TextureHeight, int BytesPerPixel);

	/**
	 * Returns true if the light is a parallel light.
	 */
	inline bool		IsParallel(void) { return renderLight.parallel; };
	inline bool		IsPointlight(void) { return renderLight.pointLight; };
	bool			CastsShadow(void);

	/**
	 * GetLightCone returns the lightdata.
	 * If the light is a pointlight it will return an ellipsoid defining the light.
	 * In case of a projected light, the returned data is a cone.
	 * If the light is a projected light and uses the additional vectors for
	 * cut off cones, it will return true.
	 */
	bool GetLightCone(idVec3 &Origin, idVec3 &Axis, idVec3 &Center);
	bool GetLightCone(idVec3 &Origin, idVec3 &Target, idVec3 &Right, idVec3 &Up, idVec3 &Start, idVec3 &End);

private:
	renderLight_t	renderLight;				// light presented to the renderer
	idVec3			localLightOrigin;			// light origin relative to the physics origin
	idMat3			localLightAxis;				// light axis relative to physics axis
	qhandle_t		lightDefHandle;				// handle to renderer light def
	int				levels;
	int				currentLevel;
	idVec3			baseColor;
	bool			breakOnTrigger;
	int				count;
	int				triggercount;
	idEntity *		lightParent;
	idVec4			fadeFrom;
	idVec4			fadeTo;
	int				fadeStart;
	int				fadeEnd;
	bool			soundWasPlaying;

private:
	void			PresentLightDefChange( void );
	void			PresentModelDefChange( void );

	void			Event_SetShader( const char *shadername );
	void			Event_GetLightParm( int parmnum );
	void			Event_SetLightParm( int parmnum, float value );
	void			Event_SetLightParms( float parm0, float parm1, float parm2, float parm3 );
	void			Event_SetRadiusXYZ( float x, float y, float z );
	void			Event_SetRadius( float radius );
	void			Event_Hide( void );
	void			Event_Show( void );
	void			Event_On( void );
	void			Event_Off( void );
	void			Event_ToggleOnOff( idEntity *activator );
	void			Event_SetSoundHandles( void );
	void			Event_FadeOut( float time );
	void			Event_FadeIn( float time );
	/**
	* Tels: Allows the color of the light to fade over to a new value over a time period.
	*/
	void			Event_FadeToLight( idVec3 &color, float time );
	/**
	* Allows script to get and set the light origin separate from model origin.
	* Used to achieve moving lights with a stationary model
	**/
	void			Event_GetLightOrigin( void );
	void			Event_SetLightOrigin( idVec3 &pos );

	/**
	* Allows us to get the light level, and tell if the light is on or off.
	* "On" light levels are > 0.0
	*/
	void			Event_GetLightLevel();


	/*
	*	Add the light to the appropriate LAS area
	*/
	void			Event_AddToLAS();

	/**	Returns 1 if the light is in PVS.
	 *	Doesn't take into account vis-area optimizations for shadowcasting lights.
	 */
	void			Event_InPVS();


	/**
	 * Texturename for the falloff image
	 */
	const char		*m_MaterialName;

	/**
	 * Pointer to the material that is used for this light. This pointer
	 * is only loaded once. If the material needs to change dynamically
	 * for a light, the m_FalloffImage must be set to the new material name
	 * and m_LightMaterial must be set to NULL, to force the reload, next
	 * time the light should use a new material.
	 */
	CLightMaterial	*m_LightMaterial;

public:

	/**
	 * Each light also gets the maxlightradius, which determines which value
	 * is the maximum radius for that particular light,
	 */
	float			m_MaxLightRadius;

	/*!
	* Darkmod LAS
	* The area the light is in, assigned by The Dark Mod Lighting Awareness System (LAS)
	*/
	int LASAreaIndex;

};

#endif /* !__GAME_LIGHT_H__ */
