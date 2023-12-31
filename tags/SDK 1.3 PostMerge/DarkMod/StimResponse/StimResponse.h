/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 22:25:02 +0100 (Fr, 23 M�r 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/
#ifndef SR_STIMRESPONSE__H
#define SR_STIMRESPONSE__H

extern char *cStimType[];

// If default stims are to be added here, the static array in the CPP file
// also must be updated. USER and UNDEFINED are not to be added though, as
// they have special meanings.
typedef enum {
	ST_FROB,			// Frobbed
	ST_FIRE,			// Fire
	ST_WATER,			// Water
	ST_DAMAGE,			// damages target
	ST_SHIELD,			// protects against arrows or physical blows
	ST_HEALING,			// heals target
	ST_HOLY,			// holy is applied
	ST_MAGIC,			// Magic is being used
	ST_TOUCH,			// triggered if touched
	ST_KNOCKOUT,		// target is knocked out
	ST_KILL,			// target is killed
	ST_RESTORE,			// target is restored
	ST_LIGHT,			// triggered by light
	ST_SOUND,			// triggered by sound
	ST_VISUAL,			// visual contact
	ST_INVITE,			// can be used to trigger special behaviour (like a stool can invite an AI to sit down)
	ST_READ,			// Can be read
	ST_RANDOM,			// Random response is selected
	ST_TIMER,			// Timer trigger
	ST_COMMUNICATION,	// A communication stimulus (see CommunicationStim.h)
	ST_GAS,				// triggered by gas arrows
	ST_USER				= 1000,	// User defined types should use this as it's base
	ST_DEFAULT			= -1
} StimType;

typedef enum {
	SS_DISABLED,		// Stim is disabled and can not be triggered
	SS_ENABLED,			// Stim is enabled and waits for activation
	SS_DEFAULT
} StimState;

/**
 * CStimResponse is the baseclass for stims and responses
 */
class CStimResponse {
friend CStimResponseCollection;
protected:
	CStimResponse(idEntity *Owner, int Type);
	virtual ~CStimResponse(void);

public:
	void EnableSR(bool Enable = true);

public:
	/**
	 * Id for the stimulus that uniquely identifies a stim, so they can
	 * be associated to each other.
	 */
	int					m_StimTypeId;

	// This is only populated with the Id as used in the entity definition. We
	// store the name here to reference the script action key.
	idStr				m_StimTypeName;

	/**
	 * If set to true, then the stim can be removed from an entity. This is mistly needed
	 * for an external app lication later on, so that the defauls can not be accidently
	 * removed.
	 */
	bool				m_Removable;

	/**
	 * State for the stim/response.
	 */
	StimState			m_State;

	/**
	 * Default means that this is a stim which has been added as default to this entity.
	 * Thiw would also mainly be used for an editor.
	 */
	bool				m_Default;

	/**
	* Timestamp for stims/responses with finite duration after they're enabled (milliseconds)
	**/
	int						m_EnabledTimeStamp;

	/**
	* Stim or response duration after being enabled (in milliseconds).  
	* SR will automatically disable itself after this time.
	**/
	int						m_Duration;

	idEntity			*m_Owner;
};

#endif /* SR_STIMRESPONSE__H */
