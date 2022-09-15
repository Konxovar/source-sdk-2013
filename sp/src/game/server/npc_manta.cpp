//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: An alien which the vortigaunts use like a personal plane.
//=============================================================================//


//-----------------------------------------------------------------------------
// Generic NPC - purely for scripted sequence work.
//-----------------------------------------------------------------------------
#include	"cbase.h"
#include	"npcevent.h"
#include	"ai_basenpc.h"
#include	"ai_hull.h"
#include "ai_baseactor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// NPC's Anim Events Go Here
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_Manta : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CNPC_Manta, CAI_BaseActor);

	void	Spawn(void);
	void	Precache(void);
	Class_T Classify(void);
	void	HandleAnimEvent(animevent_t *pEvent);
	int		GetSoundInterests(void);
	void	SetupWithoutParent(void);
	void	PrescheduleThink(void);
};

LINK_ENTITY_TO_CLASS(npc_manta, CNPC_Manta);

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Manta::Classify(void)
{
	return	CLASS_PLAYER_ALLY;
}



//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_Manta::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_Manta::GetSoundInterests(void)
{
	return	NULL;
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_Manta::Spawn()
{
	// manta is allowed to use multiple models, because he appears in the pod.
	// He defaults to his normal model.
	char *szModel = (char *)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/manta.mdl";
		SetModelName(AllocPooledString(szModel));
	}

	Precache();
	SetModel(szModel);

	BaseClass::Spawn();

	SetHullType(HULL_LARGE);
	SetHullSizeNormal();


	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);
	SetBloodColor(BLOOD_COLOR_GREEN);
	m_iHealth = 8;
	m_flFieldOfView = 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;

	NPCInit();
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_Manta::Precache()
{
	PrecacheModel(STRING(GetModelName()));
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

/*
void CNPC_Manta::SetupWithoutParent(void)
{
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);

	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD);
	CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE);
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*
void CNPC_Manta::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	// Figure out if Eli has just been removed from his parent
	if (GetMoveType() == MOVETYPE_NONE && !GetMoveParent())
	{
		SetupWithoutParent();
		SetupVPhysicsHull();
	}
}
*/
//-----------------------------------------------------------------------------
// AI Schedules Specific to this NPC
//-----------------------------------------------------------------------------
