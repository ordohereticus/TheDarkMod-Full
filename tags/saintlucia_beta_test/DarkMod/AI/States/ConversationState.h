/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2820 $
 * $Date: 2008-09-12 13:51:08 -0400 (Fri, 12 Sep 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_CONVERSATION_STATE_H__
#define __AI_CONVERSATION_STATE_H__

#include "State.h"
#include "../Conversation/ConversationCommand.h"

namespace ai
{

#define STATE_CONVERSATION "Conversation"

class ConversationState :
	public State
{
public:
	enum ExecutionState
	{
		ENotReady = 0,		// not ready yet (try next frame)
		EReady,				// ready for starting
		EExecuting,			// executing, but ready for new commands
		EBusy,				// execution in progress, can't handle new commands
		ENumExecutionStates,// invalid index
	};

private:
	// The conversation index
	int _conversation;

	// The execution state
	ExecutionState _state;

	// The conversation command type
	ConversationCommand::Type _commandType;

	int _finishTime;

	ConversationState();

public:
	// Get the name of this state
	virtual const idStr& GetName() const;

	// This is called when the state is first attached to the AI's Mind.
	virtual void Init(idAI* owner);

	// Gets called each time the mind is thinking
	virtual void Think(idAI* owner);

	// Incoming events issued by the Subsystems
	virtual void OnSubsystemTaskFinished(idAI* owner, SubsystemId subSystem);

	// Sets the conversation this state should handle
	void SetConversation(int index);

	/**
	 * greebo: Processes the given command belonging to the given conversation.
	 * The command's state variable is updated after this call and corresponds 
	 * to the execution state of this AI.
	 */
	void ProcessCommand(ConversationCommand& command);

	// Starts execution of the given command, returns FALSE on failure
	void StartCommand(ConversationCommand& command, Conversation& conversation);

	// Handles the given command, returns FALSE on failure
	void Execute(ConversationCommand& command, Conversation& conversation);

	// Returns the current conversation command execution state
	ConversationState::ExecutionState GetExecutionState();

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Override base class method
	virtual bool CheckAlertLevel(idAI* owner);

	static StatePtr CreateInstance();

private:
	// Plays the given sound (shader) and returns the length in msecs
	int Talk(idAI* owner, const idStr& soundName);

	// Returns true if the conversation can be started
	bool CheckConversationPrerequisites();

	// Returns the execution state of the given actor of the given conversation
	ExecutionState GetConversationStateOfActor(idAI* ai);

	// Private helper for debug output
	void DrawDebugOutput(idAI* owner);
};
typedef boost::shared_ptr<ConversationState> ConversationStatePtr;

} // namespace ai

#endif /* __AI_CONVERSATION_STATE_H__ */
