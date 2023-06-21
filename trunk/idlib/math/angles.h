/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 2 $
 * $Date: 2004-10-30 11:52:07 -0400 (Sat, 30 Oct 2004) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:35  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __MATH_ANGLES_H__
#define __MATH_ANGLES_H__

/*
===============================================================================

	Euler angles

===============================================================================
*/

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

class idVec3;
class idQuat;
class idRotation;
class idMat3;
class idMat4;

class idAngles {
public:
	float			pitch;
	float			yaw;
	float			roll;

					idAngles( void );
					idAngles( float pitch, float yaw, float roll );
					explicit idAngles( const idVec3 &v );

	void 			Set( float pitch, float yaw, float roll );
	idAngles &		Zero( void );

	float			operator[]( int index ) const;
	float &			operator[]( int index );
	idAngles		operator-() const;			// negate angles, in general not the inverse rotation
	idAngles &		operator=( const idAngles &a );
	idAngles		operator+( const idAngles &a ) const;
	idAngles &		operator+=( const idAngles &a );
	idAngles		operator-( const idAngles &a ) const;
	idAngles &		operator-=( const idAngles &a );
	idAngles		operator*( const float a ) const;
	idAngles &		operator*=( const float a );
	idAngles		operator/( const float a ) const;
	idAngles &		operator/=( const float a );

	friend idAngles	operator*( const float a, const idAngles &b );

	bool			Compare( const idAngles &a ) const;							// exact compare, no epsilon
	bool			Compare( const idAngles &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const idAngles &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const idAngles &a ) const;						// exact compare, no epsilon

	idAngles &		Normalize360( void );	// normalizes 'this'
	idAngles &		Normalize180( void );	// normalizes 'this'

	void			Clamp( const idAngles &min, const idAngles &max );

	int				GetDimension( void ) const;

	void			ToVectors( idVec3 *forward, idVec3 *right = NULL, idVec3 *up = NULL ) const;
	idVec3			ToForward( void ) const;
	idQuat			ToQuat( void ) const;
	idRotation		ToRotation( void ) const;
	idMat3			ToMat3( void ) const;
	idMat4			ToMat4( void ) const;
	idVec3			ToAngularVelocity( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;
};

extern idAngles ang_zero;

ID_INLINE idAngles::idAngles( void ) {
}

ID_INLINE idAngles::idAngles( float pitch, float yaw, float roll ) {
	this->pitch = pitch;
	this->yaw	= yaw;
	this->roll	= roll;
}

ID_INLINE idAngles::idAngles( const idVec3 &v ) {
	this->pitch = v[0];
	this->yaw	= v[1];
	this->roll	= v[2];
}

ID_INLINE void idAngles::Set( float pitch, float yaw, float roll ) {
	this->pitch = pitch;
	this->yaw	= yaw;
	this->roll	= roll;
}

ID_INLINE idAngles &idAngles::Zero( void ) {
	pitch = yaw = roll = 0.0f;
	return *this;
}

ID_INLINE float idAngles::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &pitch )[ index ];
}

ID_INLINE float &idAngles::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &pitch )[ index ];
}

ID_INLINE idAngles idAngles::operator-() const {
	return idAngles( -pitch, -yaw, -roll );
}

ID_INLINE idAngles &idAngles::operator=( const idAngles &a ) {
	pitch	= a.pitch;
	yaw		= a.yaw;
	roll	= a.roll;
	return *this;
}

ID_INLINE idAngles idAngles::operator+( const idAngles &a ) const {
	return idAngles( pitch + a.pitch, yaw + a.yaw, roll + a.roll );
}

ID_INLINE idAngles& idAngles::operator+=( const idAngles &a ) {
	pitch	+= a.pitch;
	yaw		+= a.yaw;
	roll	+= a.roll;

	return *this;
}

ID_INLINE idAngles idAngles::operator-( const idAngles &a ) const {
	return idAngles( pitch - a.pitch, yaw - a.yaw, roll - a.roll );
}

ID_INLINE idAngles& idAngles::operator-=( const idAngles &a ) {
	pitch	-= a.pitch;
	yaw		-= a.yaw;
	roll	-= a.roll;

	return *this;
}

ID_INLINE idAngles idAngles::operator*( const float a ) const {
	return idAngles( pitch * a, yaw * a, roll * a );
}

ID_INLINE idAngles& idAngles::operator*=( float a ) {
	pitch	*= a;
	yaw		*= a;
	roll	*= a;
	return *this;
}

ID_INLINE idAngles idAngles::operator/( const float a ) const {
	float inva = 1.0f / a;
	return idAngles( pitch * inva, yaw * inva, roll * inva );
}

ID_INLINE idAngles& idAngles::operator/=( float a ) {
	float inva = 1.0f / a;
	pitch	*= inva;
	yaw		*= inva;
	roll	*= inva;
	return *this;
}

ID_INLINE idAngles operator*( const float a, const idAngles &b ) {
	return idAngles( a * b.pitch, a * b.yaw, a * b.roll );
}

ID_INLINE bool idAngles::Compare( const idAngles &a ) const {
	return ( ( a.pitch == pitch ) && ( a.yaw == yaw ) && ( a.roll == roll ) );
}

ID_INLINE bool idAngles::Compare( const idAngles &a, const float epsilon ) const {
	if ( idMath::Fabs( pitch - a.pitch ) > epsilon ) {
		return false;
	}
			
	if ( idMath::Fabs( yaw - a.yaw ) > epsilon ) {
		return false;
	}

	if ( idMath::Fabs( roll - a.roll ) > epsilon ) {
		return false;
	}

	return true;
}

ID_INLINE bool idAngles::operator==( const idAngles &a ) const {
	return Compare( a );
}

ID_INLINE bool idAngles::operator!=( const idAngles &a ) const {
	return !Compare( a );
}

ID_INLINE void idAngles::Clamp( const idAngles &min, const idAngles &max ) {
	if ( pitch < min.pitch ) {
		pitch = min.pitch;
	} else if ( pitch > max.pitch ) {
		pitch = max.pitch;
	}
	if ( yaw < min.yaw ) {
		yaw = min.yaw;
	} else if ( yaw > max.yaw ) {
		yaw = max.yaw;
	}
	if ( roll < min.roll ) {
		roll = min.roll;
	} else if ( roll > max.roll ) {
		roll = max.roll;
	}
}

ID_INLINE int idAngles::GetDimension( void ) const {
	return 3;
}

ID_INLINE const float *idAngles::ToFloatPtr( void ) const {
	return &pitch;
}

ID_INLINE float *idAngles::ToFloatPtr( void ) {
	return &pitch;
}

#endif /* !__MATH_ANGLES_H__ */
