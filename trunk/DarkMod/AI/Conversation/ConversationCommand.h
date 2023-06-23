/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2678 $
 * $Date: 2008-07-17 11:11:36 -0400 (Thu, 17 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_CONVERSATION_COMMAND_H__
#define __AI_CONVERSATION_COMMAND_H__

#include "../../../idlib/precompiled.h"

#include <boost/shared_ptr.hpp>

namespace ai {

class Conversation;

class ConversationCommand
{
public:
	// These are the various command types
	enum Type
	{
		EWaitSeconds = 0,
		EWaitForTrigger,
		EWaitForActor,
		EWalkToPosition,
		EWalkToEntity,
		EStopMove,
		ETalk,
		EPlayAnimOnce,
		EPlayAnimCycle,
		EActivateTarget,
		ELookAtActor,
		ELookAtPosition,
		ELookAtEntity,
		ETurnToActor,
		ETurnToPosition,
		ETurnToEntity,
		EAttackActor,
		EAttackEntity,
		ENumCommands,
	};

	// The string representations of the above (keep in sync please)
	static const char* const TypeNames[ENumCommands];

	// Each command can have several states
	enum State
	{
		ENotStartedYet = 0,
		EExecuting,
		EFinished,
		EAborted,
		ENumStates, // invalid index
	};

private:
	// The type of this command
	Type _type;

	// The state of this command
	State _state;

	// The index of the actor who is supposed to execute this command
	// Note that the index as specified in the spawnargs is decreased by 1 to
	// match the ones in the idList.
	int _actor;

	// Argument list
	idStringList _arguments;

public:
	// Default constructor
	ConversationCommand();

	// Returns the type of this conversation command
	Type GetType();

	// The execution state of this command
	State GetState();

	// Returns the actor index of this command
	int GetActor();

	// Returns the number of arguments
	int GetNumArguments();

	// Returns the given argument (starting with index 0) or "" if the argument doesn't exist
	idStr GetArgument(int index);

	/**
	 * Execute this command (i.e. tell the actor what to do)
	 *
	 * @conversation: the "parent" conversation we're belonging to, needed to get the actors.
	 *
	 * @returns: the execution state of this command.
	 */
	State Execute(Conversation& conversation);

	/**
	 * greebo: Parses the command parameters from the given idDict.
	 * The prefix is something along the lines "conv_2_cmd_3_" and is
	 * used to find all spawnargs relevant for this command.
	 *
	 * Returns TRUE if the parse process succeeded, FALSE otherwise.
	 */
	bool Parse(const idDict& dict, const idStr& prefix);

	// Save/Restore routines
	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);

private:
	// Tries to convert the string representation of the command into a Type enum value
	static Type GetType(const idStr& cmdString);
};
typedef boost::shared_ptr<ConversationCommand> ConversationCommandPtr;

} // namespace ai

#endif /* __AI_CONVERSATION_COMMAND_H__ */
