//========= Not Copyrighted Cursed Cookie Developments, No rights reserved. ============//
//
// Purpose:		For effects like reloading n' stuff
//
// 
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
/*
#define VORTIGAUNT_BEAM_ALL		-1
#define	VORTIGAUNT_RELOAD_BEAM		0

#define HAND_LEFT	0
#define HAND_RIGHT	1
#define HAND_BOTH	2


#define VORTZAP_RELOADGLOW_SPRITE "sprites/lgtning.vmt"

static const char *VORTIGAUNT_LEFT_CLAW = "leftclaw";
static const char *VORTIGAUNT_RIGHT_CLAW = "rightclaw";

void			StartHandGlow(int beamType, int nHand);
void			EndHandGlow(int beamType = VORTIGAUNT_BEAM_ALL);

#define	VORTZAP_RELOAD_GLOW_TIME		1.2f		// How long does glow last
#define	VORTZAP_GLOWFADE_TIME			0.5			// How long does it fade




#define VORTFX_RELOADGLOW		0	// Beam that damages the target

int ACT_VORTZAP_RELOAD;



class CVortReload : public CEffectData
{
	DECLARE_CLASS(CVortReload, CEffectData);

public:
	void	Precache(void);
	// ------------
	// Beams
	// ------------
	inline bool		InAttackSequence(void);
	void			ClearBeams(void);
	void			ArmBeam(int beamType, int nHand);
	void			ZapBeam(int nHand);
	int				m_nLightningSprite;

	// ---------------
	//  Glow
	// ----------------
	void			ClearHandGlow(void);
	float			m_fGlowAge;
	float			m_fGlowChangeTime;
	bool			m_bGlowTurningOn;
	int				m_nCurGlowIndex;

	CHandle<CVortigauntEffectRecharge>	m_hHandEffect[2];

	void			StartHandGlow(int beamType, int nHand);
	void			EndHandGlow(int beamType = VORTIGAUNT_BEAM_ALL);
	void			MaintainGlows(void);

private:
	int				m_iLeftHandAttachment;
	int				m_iRightHandAttachment;

public:
	
	DECLARE_DATADESC();

};
BEGIN_DATADESC(CVortReload)

DEFINE_ARRAY(m_hHandEffect, FIELD_EHANDLE, 2),

DEFINE_FIELD(m_nLightningSprite, FIELD_INTEGER),
DEFINE_FIELD(m_fGlowAge, FIELD_FLOAT),
DEFINE_FIELD(m_fGlowChangeTime, FIELD_TIME),
DEFINE_FIELD(m_bGlowTurningOn, FIELD_BOOLEAN),
DEFINE_FIELD(m_nCurGlowIndex, FIELD_INTEGER),m_iRightHandAttachment
DEFINE_FIELD(m_iLeftHandAttachment, FIELD_INTEGER),
DEFINE_FIELD(m_iRightHandAttachment, FIELD_INTEGER),

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: Make the glow
//-----------------------------------------------------------------------------
void ReloadZap(const CEffectData &data)
{

		StartHandGlow(VORTIGAUNT_RELOAD_BEAM, HAND_LEFT);
		StartHandGlow(VORTIGAUNT_RELOAD_BEAM, HAND_RIGHT);

}

void CVortReload::StartHandGlow(int beamType, int nHand)
{
	// We need this because there's a rare case where a scene can interrupt and turn off our hand glows, but are then
	// turned back on in the same frame due to how animations are applied and anim events are executed after the AI frame.
	if (m_fGlowChangeTime > gpGlobals->curtime)
		return;

	switch (beamType)
	{
	case VORTIGAUNT_RELOAD_BEAM:
	{
		// Validate the hand's range
		if (nHand >= ARRAYSIZE(m_hHandEffect))
			return;

		// Start up
		if (m_hHandEffect[nHand] == NULL)
		{
			// Create the token if it doesn't already exist
			m_hHandEffect[nHand] = CVortReload::CreateEffectReload(GetAbsOrigin(), this, NULL);
			if (m_hHandEffect[nHand] == NULL)
				return;
		}

		// Stomp our settings
		m_hHandEffect[nHand]->SetParent(this, (nHand == HAND_LEFT) ? m_iLeftHandAttachment : m_iRightHandAttachment);
		m_hHandEffect[nHand]->SetMoveType(MOVETYPE_NONE);
		m_hHandEffect[nHand]->SetLocalOrigin(Vector(8.0f, 4.0f, 0.0f));
	}
	break;

	case VORTIGAUNT_BEAM_ALL:
		Assert(0);
		break;
	}
}

DECLARE_CLIENT_EFFECT("ReloadZap", ReloadZap);

CVortReload *CVortReload::CreateEffectReload(const Vector &vecOrigin, CEffectData *pOwner, CEffectData *pTarget)
{
	CVortReload *pToken = (CVortReload *)CreateEntityByName("vort_effect_dispel");
	if (pToken == NULL)
		return NULL;

	// Set up our internal data
	UTIL_SetOrigin(pToken, vecOrigin);
	pToken->SetOwnerEntity(pOwner);
	pToken->Spawn();

	return pToken;
}


class CVortReload : public CEffectData
{
	DECLARE_CLASS(CVortReload, CEffectData);

public:

	static CVortReload *CreateEffectReload(const Vector &vecOrigin, CEffectData *pOwner, CEffectData *pTarget);

	CVortReload(void);

	void StartHandGlow(int beamType, int nHand);

	virtual void	Spawn(void);

	void	FadeAndDie(void);

private:

	CNetworkVar(bool, m_bFadeOut);


	DECLARE_DATADESC();
};


class CVortigauntEffectRecharge : public CEffectData
{
	DECLARE_CLASS(CVortigauntEffectRecharge, CEffectData);

public:

	static CVortigauntEffectRecharge *CreateEffectDispel(const Vector &vecOrigin, CEffectData *pOwner, CEffectData *pTarget);

	CVortigauntEffectRecharge(void);

	virtual void	Spawn(void);

	void	FadeAndDie(void);

private:

	CNetworkVar(bool, m_bFadeOut);

	DECLARE_DATADESC();
};
*/