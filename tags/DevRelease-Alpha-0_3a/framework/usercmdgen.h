/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 570 $
 * $Date: 2006-09-12 15:56:37 -0400 (Tue, 12 Sep 2006) $
 * $Author: gildoran $
 *
 * $Log$
 * Revision 1.7  2006/09/12 19:56:37  gildoran
 * Forgot to upload usercmdgen.h
 *
 * Revision 1.6  2006/08/14 01:08:42  ishtvan
 * added impulse51 for dropping
 *
 * Revision 1.5  2006/04/03 02:04:04  gildoran
 * Added some code for an inventory prototype.
 *
 * Revision 1.4  2005/11/10 19:21:04  sparhawk
 * SDK 1.3 Merge
 *
 * Revision 1.3  2004/11/28 09:15:24  sparhawk
 * SDK V2 merge
 *
 * Revision 1.2  2004/11/14 00:44:01  sparhawk
 * Added USE/Frob Key.
 *
 * Revision 1.1.1.1  2004/10/30 15:52:34  sparhawk
 * Initial release
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __USERCMDGEN_H__
#define __USERCMDGEN_H__

/*
===============================================================================

	Samples a set of user commands from player input.

===============================================================================
*/

const int USERCMD_HZ			= 60;			// 60 frames per second
const int USERCMD_MSEC			= 1000 / USERCMD_HZ;

// usercmd_t->button bits
const int BUTTON_ATTACK			= BIT(0);
const int BUTTON_RUN			= BIT(1);
const int BUTTON_ZOOM			= BIT(2);
const int BUTTON_SCORES			= BIT(3);
const int BUTTON_MLOOK			= BIT(4);
const int BUTTON_5				= BIT(5);
const int BUTTON_6				= BIT(6);
const int BUTTON_7				= BIT(7);

// usercmd_t->impulse commands
const int IMPULSE_0				= 0;			// weap 0
const int IMPULSE_1				= 1;			// weap 1
const int IMPULSE_2				= 2;			// weap 2
const int IMPULSE_3				= 3;			// weap 3
const int IMPULSE_4				= 4;			// weap 4
const int IMPULSE_5				= 5;			// weap 5
const int IMPULSE_6				= 6;			// weap 6
const int IMPULSE_7				= 7;			// weap 7
const int IMPULSE_8				= 8;			// weap 8
const int IMPULSE_9				= 9;			// weap 9
const int IMPULSE_10			= 10;			// weap 10
const int IMPULSE_11			= 11;			// weap 11
const int IMPULSE_12			= 12;			// weap 12
const int IMPULSE_13			= 13;			// weap reload
const int IMPULSE_14			= 14;			// weap next
const int IMPULSE_15			= 15;			// weap prev
const int IMPULSE_16			= 16;			// <unused>
const int IMPULSE_17			= 17;			// ready to play ( toggles ui_ready )
const int IMPULSE_18			= 18;			// center view
const int IMPULSE_19			= 19;			// show PDA/INV/MAP
const int IMPULSE_20			= 20;			// toggle team ( toggles ui_team )
const int IMPULSE_21			= 21;			// <unused>
const int IMPULSE_22			= 22;			// spectate
const int IMPULSE_23			= 23;			// <unused>
const int IMPULSE_24			= 24;			// <unused>
const int IMPULSE_25			= 25;			// <unused>
const int IMPULSE_26			= 26;			// <unused>
const int IMPULSE_27			= 27;			// <unused>
const int IMPULSE_28			= 28;			// vote yes
const int IMPULSE_29			= 29;			// vote no
const int IMPULSE_40			= 40;			// use vehicle
const int IMPULSE_41			= 41;			// TDM Use/Frob key
const int IMPULSE_42			= 42;			// Inventory prev (unused)
const int IMPULSE_43			= 43;			// Inventory next (unused)
const int IMPULSE_44			= 44;			// Lean forward
const int IMPULSE_45			= 45;			// Lean left
const int IMPULSE_46			= 46;			// Lean right
const int IMPULSE_47			= 47;			// Inventory prev item
const int IMPULSE_48			= 48;			// Inventory next item
const int IMPULSE_49			= 49;			// Inventory prev group
const int IMPULSE_50			= 50;			// Inventory next group
const int IMPULSE_51			= 51;			// Inventory use item
const int IMPULSE_52			= 52;			// Inventory drop item

// usercmd_t->flags
const int UCF_IMPULSE_SEQUENCE	= 0x0001;		// toggled every time an impulse command is sent

class usercmd_t {
public:
	int			gameFrame;						// frame number
	int			gameTime;						// game time
	int			duplicateCount;					// duplication count for networking
	byte		buttons;						// buttons
	signed char	forwardmove;					// forward/backward movement
	signed char	rightmove;						// left/right movement
	signed char	upmove;							// up/down movement
	short		angles[3];						// view angles
	short		mx;								// mouse delta x
	short		my;								// mouse delta y
	signed char impulse;						// impulse command
	byte		flags;							// additional flags
	int			sequence;						// just for debugging

public:
	void		ByteSwap();						// on big endian systems, byte swap the shorts and ints
	bool		operator==( const usercmd_t &rhs ) const;
};

typedef enum {
	INHIBIT_SESSION = 0,
	INHIBIT_ASYNC
} inhibit_t;

const int MAX_BUFFERED_USERCMD = 64;

class idUsercmdGen {
public:
	virtual				~idUsercmdGen( void ) {}

	// Sets up all the cvars and console commands.
	virtual	void		Init( void ) = 0;

	// Prepares for a new map.
	virtual void		InitForNewMap( void ) = 0;

	// Shut down.
	virtual void		Shutdown( void ) = 0;

	// Clears all key states and face straight.
	virtual	void		Clear( void ) = 0;

	// Clears view angles.
	virtual void		ClearAngles( void ) = 0;

	// When the console is down or the menu is up, only emit default usercmd, so the player isn't moving around.
	// Each subsystem (session and game) may want an inhibit will OR the requests.
	virtual void		InhibitUsercmd( inhibit_t subsystem, bool inhibit ) = 0;

	// Returns a buffered command for the given game tic.
	virtual usercmd_t	TicCmd( int ticNumber ) = 0;

	// Called async at regular intervals.
	virtual	void		UsercmdInterrupt( void ) = 0;

	// Set a value that can safely be referenced by UsercmdInterrupt() for each key binding.
	virtual	int			CommandStringUsercmdData( const char *cmdString ) = 0;

	// Returns the number of user commands.
	virtual int			GetNumUserCommands( void ) = 0;

	// Returns the name of a user command via index.
	virtual const char *GetUserCommandName( int index ) = 0;

	// Continuously modified, never reset. For full screen guis.
	virtual void		MouseState( int *x, int *y, int *button, bool *down ) = 0;

	// Directly sample a usercmd.
	virtual usercmd_t	GetDirectUsercmd( void ) = 0;
};

extern idUsercmdGen	*usercmdGen;

#endif /* !__USERCMDGEN_H__ */
