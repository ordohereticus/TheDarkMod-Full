/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2342 $
 * $Date: 2008-05-15 14:43:06 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/
#ifndef _MULTI_STATE_MOVER_POSITION_H_
#define _MULTI_STATE_MOVER_POSITION_H_

class CMultiStateMover;
class CMultiStateMoverButton;

/**
 * greebo: A MultiStateMoverPosition is an entity carrying information
 * about a possible MultiStateMover position (doh!).
 *
 * It can represent a floor of an elevator, for instance. The MultiStateMover
 * itself must target a set of two or more Position entities at spawn time.
 */
class CMultiStateMoverPosition : 
	public idEntity
{
	// The list of targetted obstacle entities
	idList< idEntityPtr<idFuncAASObstacle> > aasObstacleEntities;

	// The associated mover
	idEntityPtr<CMultiStateMover> mover;

public:
	CLASS_PROTOTYPE( CMultiStateMoverPosition );

	void	Spawn();

	void	Save(idSaveGame *savefile) const;
	void	Restore(idRestoreGame *savefile);

	// Sets the associated mover entity (called by the mover itself at spawn)
	void	SetMover(CMultiStateMover* newMover);

	/**
	 * greebo: Returns the button entity which can be used to fetch the elevator to this position.
	 * 
	 * @returns: NULL if no suitable button found.
	 */
	CMultiStateMoverButton*	GetFetchButton();

	/** 
	 * greebo: Returns the button entity which can be used to move the associated elevator to 
	 * the given <toPosition>. Used by AI to find out which button to press when stanind on
	 * an elevator.
	 *
	 * @returns: the button entity or NULL if nothing found.
	 */
	CMultiStateMoverButton*	GetRideButton(CMultiStateMoverPosition* toPosition);

	// greebo: These two events are called when the mulitstate mover leaves/arrives the position
	virtual void	OnMultistateMoverArrive(CMultiStateMover* mover);
	virtual void	OnMultistateMoverLeave(CMultiStateMover* mover);

private:
	// Find all AAS func entities after spawn
	void	Event_PostSpawn();

	/**
	 * Calls the mover event script whose name is contained in the given spawnArg.
	 * The script thread is immediately executed, if the function exists. 
	 *
	 * @spawnArg: The spawnarg containing the script function name (e.g. "call_on_leave")
	 */
	void RunMoverEventScript(const idStr& spawnArg, CMultiStateMover* mover);
};

/**
 * greebo: This is the info structure which gets inserted into
 * a local list in the MultiStateMover itself.
 */
struct MoverPositionInfo 
{
	// The name of the position
	idStr name;

	// The position entity
	idEntityPtr<CMultiStateMoverPosition> positionEnt;
};

#endif /* _MULTI_STATE_MOVER_POSITION_H_ */
