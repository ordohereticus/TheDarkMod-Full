/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 698 $
 * $Date: 2007-01-02 23:16:06 -0500 (Tue, 02 Jan 2007) $
 * $Author: ishtvan $
 *
 * $Log$
 * Revision 1.2  2007/01/03 04:16:06  ishtvan
 * added optional impulse modifier argument to ClipTranslationalPush
 *
 * Revision 1.1.1.1  2004/10/30 15:52:34  sparhawk
 * Initial release
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __PUSH_H__
#define __PUSH_H__

/*
===============================================================================

  Allows physics objects to be pushed geometrically.

===============================================================================
*/

#define PUSHFL_ONLYMOVEABLE			1		// only push moveable entities
#define PUSHFL_NOGROUNDENTITIES		2		// don't push entities the clip model rests upon
#define PUSHFL_CLIP					4		// also clip against all non-moveable entities
#define PUSHFL_CRUSH				8		// kill blocking entities
#define PUSHFL_APPLYIMPULSE			16		// apply impulse to pushed entities

//#define NEW_PUSH

class idPush {
public:
					// Try to push other entities by moving the given entity.
					// If results.fraction < 1.0 the move was blocked by results.c.entityNum
					// Returns total mass of all pushed entities.
	float			ClipTranslationalPush( trace_t &results, idEntity *pusher, const int flags,
											const idVec3 &newOrigin, const idVec3 &move, float ImpulseMod = 1.0f );

	float			ClipRotationalPush( trace_t &results, idEntity *pusher, const int flags,
											const idMat3 &newAxis, const idRotation &rotation );

	float			ClipPush( trace_t &results, idEntity *pusher, const int flags,
											const idVec3 &oldOrigin, const idMat3 &oldAxis,
												idVec3 &newOrigin, idMat3 &newAxis );

					// initialize saving the positions of entities being pushed
	void			InitSavingPushedEntityPositions( void );
					// move all pushed entities back to their previous position
	void			RestorePushedEntityPositions( void );
					// returns the number of pushed entities
	int				GetNumPushedEntities( void ) const { return numPushed; }
					// get the ith pushed entity
	idEntity *		GetPushedEntity( int i ) const { assert( i >= 0 && i < numPushed ); return pushed[i].ent; }

private:
	struct pushed_s {
		idEntity *	ent;					// pushed entity
		idAngles	deltaViewAngles;		// actor delta view angles
	}				pushed[MAX_GENTITIES];	// pushed entities
	int				numPushed;				// number of pushed entities

	struct pushedGroup_s {
		idEntity *	ent;
		float		fraction;
		bool		groundContact;
		bool		test;
	}				pushedGroup[MAX_GENTITIES];
	int				pushedGroupSize;

private:
	void			SaveEntityPosition( idEntity *ent );
	bool			RotateEntityToAxial( idEntity *ent, idVec3 rotationPoint );
#ifdef NEW_PUSH
	bool			CanPushEntity( idEntity *ent, idEntity *pusher, idEntity *initialPusher, const int flags );
	void			AddEntityToPushedGroup( idEntity *ent, float fraction, bool groundContact );
	bool			IsFullyPushed( idEntity *ent );
	bool			ClipTranslationAgainstPusher( trace_t &results, idEntity *ent, idEntity *pusher, const idVec3 &translation );
	int				GetPushableEntitiesForTranslation( idEntity *pusher, idEntity *initialPusher, const int flags,
											const idVec3 &translation, idEntity *entityList[], int maxEntities );
	bool			ClipRotationAgainstPusher( trace_t &results, idEntity *ent, idEntity *pusher, const idRotation &rotation );
	int				GetPushableEntitiesForRotation( idEntity *pusher, idEntity *initialPusher, const int flags,
											const idRotation &rotation, idEntity *entityList[], int maxEntities );
#else
	void			ClipEntityRotation( trace_t &trace, const idEntity *ent, const idClipModel *clipModel,
										idClipModel *skip, const idRotation &rotation );
	void			ClipEntityTranslation( trace_t &trace, const idEntity *ent, const idClipModel *clipModel,
										idClipModel *skip, const idVec3 &translation );
	int				TryTranslatePushEntity( trace_t &results, idEntity *check, idClipModel *clipModel, const int flags,
												const idVec3 &newOrigin, const idVec3 &move );
	int				TryRotatePushEntity( trace_t &results, idEntity *check, idClipModel *clipModel, const int flags,
												const idMat3 &newAxis, const idRotation &rotation );
	int				DiscardEntities( idEntity *entityList[], int numEntities, int flags, idEntity *pusher );
#endif
};

#endif /* !__PUSH_H__ */
