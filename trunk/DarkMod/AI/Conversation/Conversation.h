/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2679 $
 * $Date: 2008-07-17 12:21:20 -0400 (Thu, 17 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_CONVERSATION_H__
#define __AI_CONVERSATION_H__

#include "../../../idlib/precompiled.h"

#include "ConversationCommand.h"

namespace ai {

/**
 * greebo: This class encapsulates a single conversation between
 * two or more characters in the game.
 */
class Conversation
{
	// The name of this conversation
	idStr _name;

	// whether this conversation has errors or not
	bool _isValid;

	float _talkDistance;

	// All actors participating in this conversation
	idStringList _actors;

	// The list of commands this conversation consists of (this is the actual "script")
	idList<ConversationCommandPtr> _commands;

	// The current conversation command index
	int _currentCommand;

	// Counter to tell how often this conversation has been played
	int _playCount;

public:
	Conversation();

	// Construct a conversation using the given spawnargs, using the given index
	Conversation(const idDict& spawnArgs, int index);

	/**
	 * greebo: Returns TRUE if this conversation is valid.
	 * Use this to check whether the construction of this class from given
	 * spawnargs has been successful.
	 */
	bool IsValid();

	// Returns the name of this conversation
	const idStr& GetName() const;

	// Returns the number of times this conversation has been (partially) played 
	int GetPlayCount();

	/**
	 * greebo: Returns TRUE if this conversation can be played. This basically means
	 * that all actors participating in this conversation are conscious.
	 */
	bool CheckConditions();

	/**
	 * greebo: Starts this conversation. Note that you should have called
	 * CheckConditions() beforehand (with a positive result, of course).
	 */
	void Start();

	/**
	 * greebo: This is the "think" routine for conversations.
	 *
	 * @returns: TRUE if the process was successful, FALSE on error.
	 */
	bool Process();

	// Returns TRUE if the conversation has no more commands to execute
	bool IsDone();

	// Gets the actor with the given index/name
	idAI* GetActor(int index);
	idAI* GetActor(const idStr& name);

	// Save/Restore routines
	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);

private:
	// Helper function to parse the properties from the spawnargs
	void InitFromSpawnArgs(const idDict& dict, int index);
};
typedef boost::shared_ptr<Conversation> ConversationPtr;

} // namespace ai

#endif /* __AI_CONVERSATION_H__ */
