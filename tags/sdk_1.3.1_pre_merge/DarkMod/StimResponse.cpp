/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 870 $
 * $Date: 2007-03-27 10:21:59 -0400 (Tue, 27 Mar 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/
/******************************************************************************/
/*                                                                            */
/*         StimResponse (C) by Gerhard W. Gruber in Germany 2005              */
/*                          All rights reserved                               */
/*                                                                            */
/******************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: StimResponse.cpp 870 2007-03-27 14:21:59Z greebo $", init_version);

#include "DarkModGlobals.h"
#include "StimResponse.h"
#include "AIComm_StimResponse.h"

// If default stims are to be added, this array also must be
// updated. USER and UNDEFINED are not to be added though, as
// they have special meanings. This array allows us to reuse
// the name in the key in the entity definition for the 
// predefined stim/responses instead of their numerical values.
char *cStimType[] = {
	"STIM_FROB",
	"STIM_FIRE",
	"STIM_WATER",
	"STIM_DAMAGE",
	"STIM_SHIELD",
	"STIM_HEALING",
	"STIM_HOLY",
	"STIM_MAGIC",
	"STIM_TOUCH",
	"STIM_KNOCKOUT",
	"STIM_KILL",
	"STIM_RESTORE",
	"STIM_LIGHT",
	"STIM_SOUND",
	"STIM_VISUAL",
	"STIM_INVITE",
	"STIM_READ",
	"STIM_RANDOM",
	"STIM_TIMER",
	"STIM_COMMUNICATION",
	"STIM_GAS",
	NULL
};

CStimResponseCollection::CStimResponseCollection(void)
{
}

CStimResponseCollection::~CStimResponseCollection(void)
{
}

CStim* CStimResponseCollection::createStim(idEntity* p_owner, StimType type)
{
		CStim* pRet;

		if (type == ST_COMMUNICATION)
		{
			DM_LOG(LC_STIM_RESPONSE, LT_DEBUG).LogString ("Creating CAIComm_Stim");
			pRet = new CAIComm_Stim (p_owner, type);
		}
		else
		{
			DM_LOG(LC_STIM_RESPONSE, LT_DEBUG).LogString ("Creating CStim");
			pRet = new CStim(p_owner, type);
		}

		return pRet;
}

CResponse* CStimResponseCollection::createResponse(idEntity* p_owner, StimType type)
{
		CResponse* pRet;

		if (type == ST_COMMUNICATION)
		{
			DM_LOG(LC_STIM_RESPONSE, LT_DEBUG).LogString ("Creating CAIComm_Response");
			pRet = new CAIComm_Response (p_owner, type);
		}
		else
		{
			DM_LOG(LC_STIM_RESPONSE, LT_DEBUG).LogString ("Creating CResponse");
			pRet = new CResponse(p_owner, type);
		}

		// Optimization: Set contents to include CONTENTS_RESPONSE
		p_owner->GetPhysics()->SetContents( p_owner->GetPhysics()->GetContents() | CONTENTS_RESPONSE );

		return pRet;
}

CStim *CStimResponseCollection::AddStim(idEntity *Owner, int Type, float fRadius, bool bRemovable, bool bDefault)
{
	CStim *pRet = NULL;
	int i, n;

	n = m_Stim.Num();
	for(i = 0; i < n; i++)
	{
		if(m_Stim[i]->m_StimTypeId == Type)
		{

			DM_LOG(LC_STIM_RESPONSE, LT_DEBUG).LogString ("Stim of that type is already in collection, returning it");
			pRet = m_Stim[i];
			break;
		}
	}

	if(pRet == NULL)
	{
		// Create either type specific descended class, or the default base class
		pRet = createStim(Owner, (StimType) Type);
		m_Stim.Append(pRet);
	}

	if(pRet != NULL)
	{
		pRet->m_Default = bDefault;
		pRet->m_Removable = bRemovable;
		pRet->m_Radius = fRadius;
		pRet->m_State = SS_DISABLED;

		AddEntityToList((idList<void *>	&)gameLocal.m_StimEntity, Owner); 
	}

	return pRet;
}

CResponse *CStimResponseCollection::AddResponse(idEntity *Owner, int Type, bool bRemovable, bool bDefault)
{
	CResponse *pRet = NULL;
	int i, n;

	n = m_Response.Num();
	for(i = 0; i < n; i++)
	{
		if(m_Response[i]->m_StimTypeId == Type)
		{
			DM_LOG(LC_STIM_RESPONSE, LT_DEBUG).LogString ("Response of that type is already in collection, returning it");
			pRet = m_Response[i];
			break;
		}
	}

	if(pRet == NULL)
	{
		pRet = createResponse (Owner, (StimType) Type);
		m_Response.Append(pRet);
	}

	if(pRet != NULL)
	{
		pRet->m_Default = bDefault;
		pRet->m_Removable = bRemovable;

		AddEntityToList((idList<void *>	&)gameLocal.m_RespEntity, Owner); 
	}

	// Optimization: Update clip contents to include contents_response

	return pRet;
}

CStim *CStimResponseCollection::AddStim(CStim *s)
{
	CStim *pRet = NULL;
	int i, n;

	if(s == NULL)
		goto Quit;

	n = m_Stim.Num();
	for(i = 0; i < n; i++)
	{
		if(m_Stim[i]->m_StimTypeId == s->m_StimTypeId)
		{
			pRet = m_Stim[i];
			break;
		}
	}

	if(pRet == NULL)
	{
		pRet = s;
		m_Stim.Append(pRet);

		AddEntityToList((idList<void *>	&)gameLocal.m_StimEntity, s->m_Owner); 
	}

Quit:
	return pRet;
}

CResponse *CStimResponseCollection::AddResponse(CResponse *r)
{
	CResponse *pRet = NULL;
	int i, n;

	if(r == NULL)
		goto Quit;

	n = m_Response.Num();
	for(i = 0; i < n; i++)
	{
		if(m_Response[i]->m_StimTypeId == r->m_StimTypeId)
		{
			pRet = m_Response[i];
			break;
		}
	}

	if(pRet == NULL)
	{
		pRet = r;
		m_Response.Append(pRet);

		AddEntityToList((idList<void *>	&)gameLocal.m_RespEntity, r->m_Owner);
	}

Quit:
	return pRet;
}


int CStimResponseCollection::RemoveStim(int Type)
{
	CStim *pRet = NULL;
	int i, n;

	n = m_Stim.Num();
	for(i = 0; i < n; i++)
	{
		if(m_Stim[i]->m_StimTypeId == Type)
		{
			pRet = m_Stim[i];
			if(pRet->m_Removable == true)
			{
				m_Stim.RemoveIndex(i);
				delete pRet;
			}
			break;
		}
	}

	return m_Stim.Num();
}

int CStimResponseCollection::RemoveResponse(int Type)
{
	idEntity *owner = NULL;
	CResponse *pRet = NULL;
	int i, n;

	n = m_Response.Num();
	for(i = 0; i < n; i++)
	{
		if(m_Response[i]->m_StimTypeId == Type)
		{
			pRet = m_Response[i];
			if(pRet->m_Removable == true)
			{
				owner = pRet->m_Owner;
				m_Response.RemoveIndex(i);
				delete pRet;
			}
			break;
		}
	}

	// Remove the CONTENTS_RESPONSE flag if no more responses
	if( m_Response.Num() <= 0 && owner != NULL )
		owner->GetPhysics()->SetContents( owner->GetPhysics()->GetContents() & ~CONTENTS_RESPONSE );

	return m_Response.Num();
}

int CStimResponseCollection::RemoveStim(CStim *s)
{
	CStim *pRet = NULL;
	int i, n;

	n = m_Stim.Num();
	for(i = 0; i < n; i++)
	{
		if(m_Stim[i] == s)
		{
			pRet = m_Stim[i];
			if(pRet->m_Removable == true)
			{
				m_Stim.RemoveIndex(i);
				delete pRet;
			}
			break;
		}
	}

	return m_Stim.Num();
}

int CStimResponseCollection::RemoveResponse(CResponse *r)
{
	CResponse *pRet = NULL;
	int i, n;

	n = m_Response.Num();
	for(i = 0; i < n; i++)
	{
		if(m_Response[i] == r)
		{
			pRet = m_Response[i];
			if(pRet->m_Removable == true)
			{
				m_Response.RemoveIndex(i);
				delete pRet;
			}
			break;
		}
	}

	return m_Response.Num();
}


void CStimResponseCollection::AddEntityToList(idList<void *> &oList, void *e)
{
	bool add = true;
	int i, n;

	n = oList.Num();
	for(i = 0; i < n; i++)
	{
		if(oList[i] == e)
		{
			add = false;
			break;
		}
	}

	if(add == true)
		oList.Append(e);
}

CStimResponse *CStimResponseCollection::GetStimResponse(int StimType, bool Stim)
{
	CStimResponse *rc = NULL, *sr;
	int i, n;
	idList<CStimResponse *> *oList = (Stim == true) ? (idList<CStimResponse *> *)&m_Stim : (idList<CStimResponse *> *)&m_Response;

	n = oList->Num();
	for(i = 0; i < n; i++)
	{
		sr = oList->operator[](i);
		if(sr->m_StimTypeId == StimType)
		{
			rc = sr;
			break;
		}
	}

	return rc;
}

CStim *CStimResponseCollection::GetStim(int StimType)
{
	return (CStim *)GetStimResponse(StimType, true);
}

CResponse *CStimResponseCollection::GetResponse(int StimType)
{
	return (CResponse *)GetStimResponse(StimType, false);
}

bool CStimResponseCollection::ParseSpawnArg(const idDict *args, idEntity *Owner, const char sr_class, int Counter)
{
	bool rc = false;
	idStr str;
	idStr name;
	CStim *stim = NULL;
	CResponse *resp = NULL;
	CStimResponse *sr = NULL;
	float Radius = 0.0f;
	StimState state( SS_DISABLED );

	// Check if the entity contains either a stim or a response.
	if(sr_class != 'S' && sr_class != 'R')
	{
		DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Invalid sr_class value [%s]\r", str.c_str());
		goto Quit;
	}

	// Get the id of the stim/response type so we know what sub-class to create
	sprintf(name, "sr_type_%u", Counter);
	args->GetString(name, "-1", str);

	// This is invalid as an entity definition
	if(str == "-1")
	{
		sr = NULL;
		goto Quit;
	}

	// If the first character is alphanumeric, we check if it 
	// is a known id and convert it.
	StimType typeOfStim = ST_DEFAULT;

	if((str[0] >= 'a' && str[0] <= 'z')
		|| (str[0] >= 'A' && str[0] <= 'Z'))
	{
		int i = 0;
		bool found = false;

		while(cStimType[i] != NULL)
		{
			if(str == cStimType[i])
			{
				typeOfStim = (StimType) i;
				found = true;
				break;
			}
			i++;
		}

		// If the string hasn't been found, we have an invalid id.
		if(found == false)
		{
			DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Invalid sr_type id [%s]\r", str.c_str());
			sr = NULL;
			goto Quit;
		}
	}
	else if(str[0] >= '0' && str[0] <= '9') // Is it numeric?
	{	
		typeOfStim = (StimType) atol(str.c_str());
	}
	else		// neither a character nor a number, thus it is invalid.
	{
		DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Invalid sr_type id [%s]\r", str.c_str());
		sr = NULL;
		goto Quit;
	}


	if(sr_class == 'S')
	{
		stim = createStim (Owner, typeOfStim);
		sr = stim;
	}
	else if (sr_class == 'R')
	{
		resp = createResponse (Owner, typeOfStim);
		sr = resp;
	}

	// Set stim response type
	sr->m_StimTypeId = typeOfStim;

	// Set stim response name string
	sr->m_StimTypeName = str;

	// Read stim response state from the def file
	sprintf(name, "sr_state_%u", Counter);
	args->GetInt(name, "1", (int &)state);
	
	if( args->GetBool(name, "1") )
		sr->EnableSR(true);
	else
		sr->EnableSR(false);

	// A stim also may have a radius
	if(sr_class == 'S')
	{
		sprintf(name, "sr_radius_%u", Counter);
		args->GetFloat(name, "0.0", Radius);
		stim->m_Radius = Radius;

		sprintf(name, "sr_use_bounds_%u", Counter);
		stim->m_bUseEntBounds = args->GetBool(name, "0");

		// set up time interleaving so the stim isn't fired every frame
		sprintf(name, "sr_time_interval_%u", Counter);
		stim->m_TimeInterleave = args->GetInt(name, "0");

		// userfriendly stim duration time
		sprintf(name, "sr_duration_%u", Counter);
		stim->m_Duration = args->GetInt(name, "0");

		sprintf(name, "sr_magnitude_%u", Counter);
		stim->m_Magnitude = args->GetFloat(name, "1.0");

		// Check if we have a timer on this stim.
		CreateTimer(args, stim, Counter);
	}
	else	// this is only for responses
	{
		// Get the name of the script function for processing the response
		name = "sr_script_" + str;
		args->GetString(name, "", str);
		if(str == "")
		{
			DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("No scriptfunction specified for stim/response id [%s]\r", sr->m_StimTypeName.c_str());
			// greebo: Disabled this to allow responses without script being added (they can have effects to handle the response)
			//delete sr;
			//sr = NULL;
			//goto Quit;
		}

		resp->m_ScriptFunction = str;

		// Try to identify the ResponseEffect spawnargs
		int effectIdx = 1;
		while (effectIdx > 0) {
			// Try to find a string like "sr_effect_2_1"
			sprintf(name, "sr_effect_%u_%u", Counter, effectIdx);
			args->GetString(name, "", str);

			if (str == "")
			{
				// Set the index to negative values to end the loop
				effectIdx = -1;
			}
			else {
				// Assemble the postfix of this effect for later key/value lookup
				// This is passed to the effect script eventually
				idStr effectPostfix;
				sprintf(effectPostfix, "%u_%u", Counter, effectIdx);

				DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Adding response effect\r");
				resp->addResponseEffect(str, effectPostfix);
				effectIdx++;
			}
		}
	}

	rc = true;

Quit:
	if(sr != NULL)
	{
		if(stim != NULL)
		{
			DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Stim %08lX added to collection for %08lX\r", stim, Owner);
			AddStim(stim);
			stim->m_State = state;
		}

		if(resp != NULL)
		{
			DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Response %08lX added to collection for %08lX\r", resp, Owner);
			AddResponse(resp);
		}
	}

	return rc;
}

void CStimResponseCollection::ParseSpawnArgsToStimResponse(const idDict *args, idEntity *Owner)
{
	idStr str;
	idStr name;
	int i;
	char sr_class;

	if(Owner == NULL)
	{
		DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Owner set to NULL is not allowed!\r");
		goto Quit;
	}

	i = 1;
	while(i != 0)
	{
		sprintf(name, "sr_class_%u", i);
		DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Looking for %s\r", name.c_str());
		if(!args->GetString(name, "X", str))
			goto Quit;

		sr_class = str[0];
		if(ParseSpawnArg(args, Owner, sr_class, i) == false)
			goto Quit;

		i++;
	}

Quit:
	return;
}


void CStimResponseCollection::CreateTimer(const idDict *args, CStim *stim, int Counter)
{
	idStr str;
	int n, Milliseconds(0);
	CStimResponseTimer *t = NULL;


	t = stim->GetTimer();

	args->GetInt( va("sr_timer_reload_%u",Counter) , "-1", n);
	t->m_Reload = n;

	args->GetString( va("sr_timer_type_%u",Counter), "RELOAD", str);
	if(str.Cmp("RELOAD") == 0)
		t->m_Type = CStimResponseTimer::SRTT_RELOAD;
	else
		t->m_Type = CStimResponseTimer::SRTT_SINGLESHOT;

	args->GetString( va("sr_timer_time_%u",Counter), "0:0:0:0", str );
    TimerValue val;
	val = CStimResponseTimer::ParseTimeString( str );
	
	// if timer is actually set
	if( val.Hour || val.Minute || val.Second || val.Millisecond )
	{
		// TODO: Return a bool here so that the outer function knows not to add this to m_Stim in the collection?

		stim->CreateTimer();
		t->SetTimer(0,0,0, Milliseconds);
		
		// timer starts on map startup by default, otherwise wait for start
		if( !(args->GetBool( va("sr_timer_waitforstart_%u",Counter), "0" )) )
			t->Start(sys->GetClockTicks());
	}
}

bool CStimResponseCollection::HasStim( void )
{
	return (m_Stim.Num() > 0);
}

bool CStimResponseCollection::HasResponse( void )
{
	return (m_Response.Num() > 0);
}
	bool			HasResponse( void );

/********************************************************************/
/*                    CStimResponse                                 */
/********************************************************************/
CStimResponse::CStimResponse(idEntity *Owner, int Type)
{
	m_StimTypeId = Type;
	m_Owner = Owner;
	m_State = SS_DISABLED;
	m_Removable = true;
	m_Default = false;
	m_Duration = 0;
	m_EnabledTimeStamp = 0;
}

CStimResponse::~CStimResponse(void)
{
}

void CStimResponse::EnableSR(bool bEnable)
{
	if(bEnable == true)
	{
		m_State = SS_ENABLED;
		m_EnabledTimeStamp = gameLocal.time;
	}
	else
		m_State = SS_DISABLED;
}

/********************************************************************/
/*                     CStim                                        */
/********************************************************************/
CStim::CStim(idEntity *e, int Type)
: CStimResponse(e, Type)
{
	m_bUseEntBounds = false;
	m_TimeInterleave = 0;
	m_TimeInterleaveStamp = 0;
	m_Radius = 0.0;
	m_Magnitude = 0.0;
	m_Chance = 1.0;
	m_ChanceTimer = false;
	m_MaxResponses = 0;
	m_CurResponses = 0;
	m_ApplyTimer = 0;
	m_ApplyTimerVal = 0;
}

CStim::~CStim(void)
{
	gameLocal.m_StimTimer.Remove(this);
}

void CStim::AddResponseIgnore(idEntity *e)
{
	if(CheckResponseIgnore(e) != true)
		m_ResponseIgnore.Append(e);
}

void CStim::RemoveResponseIgnore(idEntity *e)
{
	m_ResponseIgnore.Remove(e);
}

bool CStim::CheckResponseIgnore(idEntity *e)
{
	bool rc = false;
	int i, n;

	n = m_ResponseIgnore.Num();
	for(i = 0; i < n; i++)
	{
		if(m_ResponseIgnore[i] == e)
		{
			rc = true;
			break;
		}
	}

	return rc;
}


CStimResponseTimer *CStim::CreateTimer(void)
{
	gameLocal.m_StimTimer.AddUnique(this);
	m_Timer.SetTicks(sys->ClockTicksPerSecond());

	return(&m_Timer);
}

void CStim::RemoveTimer(void)
{
	gameLocal.m_StimTimer.Remove(this);
}

void CStim::PostFired (int numResponses)
{
}



/********************************************************************/
/*                   CResponse                                      */
/********************************************************************/
CResponse::CResponse(idEntity *e, int Type)
: CStimResponse(e, Type)
{
	m_FollowUp = NULL;
	m_ScriptFunction = NULL;
	m_MinDamage = 0.0f;
	m_MaxDamage = 0;
	m_Chance = 1.0f;
}

CResponse::~CResponse(void)
{
	// Remove all the allocated response effects from the heap
	for (int i = 0; i < m_ResponseEffects.Num(); i++)
		delete m_ResponseEffects[i];
}

void CResponse::TriggerResponse(idEntity *StimEnt)
{
	DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("CResponse::TriggerResponse \r");

	DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Response for Id %s triggered (Action: %s)\r", m_StimTypeName.c_str(), m_ScriptFunction.c_str());

	const function_t *pScriptFkt = m_Owner->scriptObject.GetFunction(m_ScriptFunction.c_str());
	if(pScriptFkt == NULL)
	{
		DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Action: %s not found in local space, checking for global.\r", m_ScriptFunction.c_str());
		pScriptFkt = gameLocal.program.FindFunction(m_ScriptFunction.c_str());
	}

	if(pScriptFkt)
	{
		DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Running ResponseScript\r");
		idThread *pThread = new idThread(pScriptFkt);
		int n = pThread->GetThreadNum();
		pThread->CallFunctionArgs(pScriptFkt, true, "eef", m_Owner, StimEnt, n);
		pThread->DelayedStart(0);
	}
	else
	{
		DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("ResponseActionScript not found! [%s]\r", m_ScriptFunction.c_str());
	}

	DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Cycling through Response Effects\r");
	DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Available Response Effects: %u\r", m_ResponseEffects.Num());
	for (int i = 0; i < m_ResponseEffects.Num(); i++) {
		m_ResponseEffects[i]->runScript(m_Owner, StimEnt);
	}

	// Continue the chain if we have a followup response to be triggered.
	if(m_FollowUp != NULL)
	{
		DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Followup: %08lX\r", m_FollowUp);
		m_FollowUp->TriggerResponse(StimEnt);
	}
}

void CResponse::SetResponseAction(idStr const &action)
{
	m_ScriptFunction = action;
}

CResponseEffect* CResponse::addResponseEffect(const idStr& effectEntityDef, const idStr& effectPostfix)
{
	CResponseEffect* returnValue = NULL;
	
	DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("\nSeeking EffectEntity [%s]\n", effectEntityDef.c_str());
	// Try to locate the specified entity definition
	const idDict* dict = gameLocal.FindEntityDefDict(effectEntityDef.c_str());
	if (dict != NULL) 
	{
		gameLocal.Printf("EffectEntityDef found, looking for script!\r");
		
		idStr scriptStr = dict->GetString("script");
		gameLocal.Printf("Script value is %s.\n", scriptStr.c_str());

		const function_t* scriptFunc = gameLocal.program.FindFunction(scriptStr.c_str());
		if (scriptFunc != NULL)
		{
			DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Script Function found: [%s]\r", scriptStr.c_str());
		}
		// Allocate a new effect object
		CResponseEffect* newEffect = new CResponseEffect(dict, scriptFunc, effectPostfix);
		
		// Add the item to the list
		m_ResponseEffects.Append(newEffect);

		returnValue = newEffect;
	}
	else
	{
		// Entity not found, emit a warning
		gameLocal.Printf("Warning: EffectEntityDef not found: %s.\r", effectEntityDef.c_str());
	}

	DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Items in the list: %u\r", m_ResponseEffects.Num());
	
	return returnValue;
}


/********************************************************************/
/*                 CResponseEffect                                  */
/********************************************************************/

void CResponseEffect::runScript(idEntity* owner, idEntity* stimEntity) {
	if (_scriptFunction == NULL) return;

	DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Running ResponseEffect Script...\r");
	idThread *pThread = new idThread(_scriptFunction);
	int n = pThread->GetThreadNum();
	pThread->CallFunctionArgs(_scriptFunction, true, "eesf", owner, stimEntity, _effectPostfix.c_str(), n);
	pThread->DelayedStart(0);
}

/********************************************************************/
/*                 CStimResponseTimer                               */
/********************************************************************/
CStimResponseTimer::CStimResponseTimer()
{
	m_Type = SRTT_SINGLESHOT;
	m_State = SRTS_DISABLED;
	m_Reload = 0;
	m_ReloadVal = 0;
	m_Timer.Flags = TIMER_UNDEFINED;
	m_TimerVal.Flags = TIMER_UNDEFINED;
	m_LastTick = 0.0;
	m_Ticker = 0.0;
	m_TicksPerSecond = 0.0;
	m_TicksPerMilliSecond = 0.0;
}

CStimResponseTimer::~CStimResponseTimer(void)
{
}

void CStimResponseTimer::SetTicks(double const &TicksPerSecond)
{
	m_TicksPerSecond = TicksPerSecond;
	m_TicksPerMilliSecond = TicksPerSecond/1000.0;
}

TimerValue CStimResponseTimer::ParseTimeString(idStr &str)
{
	TimerValue v;
	int i, h, m, s, ms;

	v.Flags = TIMER_UNDEFINED;

	if(str.Length() == 0)
		goto Quit;

	if(str[2] != ':' && str[5]  != ':')
	{
		DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Invalid timer string [%s]\r", str.c_str());
		goto Quit;
	}

	h = m = s = ms = 0;
	for(i = 0; i < 4; i++)
	{
		switch(i)
		{
			case 0:
			{
				h = atoi(str.c_str());
				if(!(h >= 0 && h <= 23))
				{
					DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Invalid hour string [%s]\r", str.c_str());
					goto Quit;
				}
			}
			break;

			case 1:
			{
				m = atoi(str.c_str());
				if(!(m >= 0 && m <= 59))
				{
					DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Invalid minute string [%s]\r", str.c_str());
					goto Quit;
				}
			}
			break;

			case 2:
			{
				s = atoi(str.c_str());
				if(!(s >= 0 && s <= 59))
				{
					DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Invalid second string [%s]\r", str.c_str());
					goto Quit;
				}
			}
			break;

			case 3:
			{
				ms = atoi(str.c_str());
				if(!(ms >= 0 && ms <= 999))
				{
					DM_LOG(LC_STIM_RESPONSE, LT_ERROR)LOGSTRING("Invalid millisecond string [%s]\r", str.c_str());
					goto Quit;
				}
			}
			break;
		}
	}

	v.Hour = h;
	v.Minute = m;
	v.Second = s;
	v.Millisecond = ms;

//	v = SetHours(h) + SetMinutes(m) + SetSeconds(s);

Quit:
	return v;
}

void CStimResponseTimer::SetReload(int Reload)
{
	m_Reload = Reload;
	m_ReloadVal = Reload;
}

void CStimResponseTimer::SetTimer(int Hour, int Minute, int Second, int Millisecond)
{
//	m_Timer = SetHours(Hour) |  SetMinutes(Minute) | SetSeconds(Seconds) | SetMSeconds(Milisecond);
	m_TimerVal.Hour = Hour;
	m_TimerVal.Minute = Minute;
	m_TimerVal.Second = Second;
	m_TimerVal.Millisecond = Millisecond;
	memset(&m_Timer, 0, sizeof(TimerValue));
}

void CStimResponseTimer::Stop(void)
{
	SetState(SRTS_DISABLED);
}

void CStimResponseTimer::Start(double const &t)
{
	m_LastTick = t;
	SetState(SRTS_RUNNING);
}

void CStimResponseTimer::Restart(double const &t)
{
	// Switch to the next timer cycle if reloading is still possible or 
	// reloading is ignored.
	m_Ticker = t;

	if(m_Reload > 0 || m_Reload == -1)
	{
		memset(&m_Timer, 0, sizeof(TimerValue));
		m_Reload--;
		Start(t);
	}
	else
		Stop();
}

void CStimResponseTimer::Reset(void)
{
	memset(&m_Timer, 0, sizeof(TimerValue));
	m_Reload = m_ReloadVal;
}

void CStimResponseTimer::SetState(TimerState State)
{
	m_State = State;
}

void CStimResponseTimer::GetTimerValueDiff(TimerValue const &A, TimerValue const &B, TimerValue &rc) const
{
}


int CStimResponseTimer::Tick(double const &t)
{
	int rc = -1;

	DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("this: %08lX %s\r", this, __FUNCTION__);

	if(m_State != SRTS_RUNNING)
		goto Quit;

	rc = 0;

	// We don't really care for an overrun of the ticckcounter. If 
	// it really happens, the worst thing would be that a particular
	// timer object would take longer to complete, because for this
	// one cycle, the tick would become negative and thus would subtract
	// the value instead of adding it. In the next cylce, everything 
	// should work again though, since we always store the current
	// value to remember it for the next cycle.
	double tick = t - m_LastTick;

	// If the overrun happened, we just ignore this tick. It's the easiest
	// thing to do and the safest.
	if(tick < 0.0)
		goto Quit;

	m_Ticker =+ tick;

	// It could be possible that one haertbeat took longer than one millisecond.
	// In this case we loop and advance the timer for each millisecond that 
	// expired. The expectation is, that this will not happen most of the time
	// of if it does, then mostly on slow machines.
	while(m_Ticker > m_TicksPerMilliSecond)
	{
		DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Millisecs triggered: %f/%f\r", m_TicksPerMilliSecond, m_Ticker);

		m_Ticker -= m_TicksPerMilliSecond;
		m_Timer.Millisecond++;
		if(m_Timer.Millisecond > 999)
		{
			m_Timer.Millisecond = 0;
			m_Timer.Second++;

			if(m_Timer.Second > 59)
			{
				m_Timer.Second = 0;
				m_Timer.Minute++;

				if(m_Timer.Minute > 59)
				{
					m_Timer.Minute = 0;
					m_Timer.Hour++;
				}
			}
		}

		// Now check if the timer already expired.
		if(m_Timer.Millisecond >= m_TimerVal.Millisecond)
		{
			if(m_Timer.Second >= m_TimerVal.Second)
			{
				if(m_Timer.Minute >= m_TimerVal.Minute)
				{
					if(m_Timer.Hour >= m_TimerVal.Hour)
					{
						rc++;
						if(m_Type == SRTT_SINGLESHOT)
							Stop();
						else
							Restart(t);

						break;
					}
				}
			}
		}
	}


Quit:
	m_LastTick = t;

	return rc;
}

