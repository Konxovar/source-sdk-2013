#include "cbase.h"
#include "ammodef.h"
#include "basecombatcharacter.h"
#include "in_buttons.h"
#include "tier0/memdbgon.h"
#define VORTZAP_AMMO_RATE 0.2
#define VORTZAP_BEAM_SPRITE "sprites/vortzap_player.vmt"
#define VORTZAP_ATTACK_RATE 0.19f
#define VORTZAP_MAX_ATTACKTIME 0.31f
#define NEXT_AMMO_REGENERATE 0.25f
#define	AMMO_REGENERATE_PENALTY		2.0f
#define AMMO_REGEN_RATE 0.25f
static int g_vortzapBeam;

class CWeaponVortzap : public CBaseCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponVortzap, CBaseCombatWeapon);
	CWeaponVortzap();
	void Spawn(void);
	void Precache(void);
	void PrimaryAttack(void);
	void WeaponIdle(void);
	void ItemPostFrame(void);
	void	UpdatePenaltyTime(void);
	void Regenerate(void);

	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo)
	{
		m_active = false;
		return BaseClass::Holster();
	}

	void EffectUpdate(void);
	void AddViewKick(void);
	DECLARE_SERVERCLASS();

protected:
	CNetworkVector(m_targetPosition);
	CNetworkVector(m_worldPosition);
	CNetworkVar(int, m_active);
	CNetworkVar(int, m_viewModelIndex);
	float m_flNextAttackTime;
	float m_TimeSinceRegenerate;
	float m_TimeSinceAttack;
	float m_flRegeneratePenalty;
	float m_flTimeTillCanRegen;
	float m_flNextZapBulletRegen;
	bool ZeroAmmoIsBecauseRanOut;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponVortzap, DT_WeaponVortzap)
SendPropVector(SENDINFO_NAME(m_targetPosition, m_targetPosition), -1, SPROP_COORD),
SendPropVector(SENDINFO_NAME(m_worldPosition, m_worldPosition), -1, SPROP_COORD),
SendPropInt(SENDINFO(m_active), 1, SPROP_UNSIGNED),
SendPropModelIndex(SENDINFO(m_viewModelIndex)),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_vortzap, CWeaponVortzap);
PRECACHE_WEAPON_REGISTER(weapon_vortzap);

BEGIN_DATADESC(CWeaponVortzap)
DEFINE_FIELD(m_active, FIELD_INTEGER),
DEFINE_FIELD(m_viewModelIndex, FIELD_INTEGER),
DEFINE_FIELD(m_targetPosition, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_worldPosition, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_flNextAttackTime, FIELD_TIME),
DEFINE_FIELD(m_TimeSinceRegenerate, FIELD_TIME),
DEFINE_FIELD(m_TimeSinceAttack, FIELD_TIME),
DEFINE_FIELD(m_flRegeneratePenalty, FIELD_FLOAT),
DEFINE_FIELD(m_flTimeTillCanRegen, FIELD_FLOAT),
DEFINE_FIELD(ZeroAmmoIsBecauseRanOut, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextZapBulletRegen, FIELD_BOOLEAN),


END_DATADESC()

CWeaponVortzap::CWeaponVortzap(void)
{
	m_active = false;
	m_targetPosition = vec3_origin;
	m_worldPosition = vec3_origin;
	m_flNextAttackTime = 0;
	m_flRegeneratePenalty = 0.0f;
	ZeroAmmoIsBecauseRanOut = 0;
	m_flNextZapBulletRegen = 0.0f;
}

void CWeaponVortzap::Spawn(void)
{
	BaseClass::Spawn();
	FallInit();
}

void CWeaponVortzap::Precache(void)
{
	BaseClass::Precache();
	g_vortzapBeam = PrecacheModel(VORTZAP_BEAM_SPRITE);
}

void CWeaponVortzap::EffectUpdate(void)
{
	m_active = true;
	Vector start, angles, forward, right, up;
	trace_t tr;

	WeaponSound(SINGLE);

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	m_viewModelIndex = pOwner->entindex();
	CBaseViewModel *vm = pOwner->GetViewModel();
	if (vm)
		m_viewModelIndex = vm->entindex();

	pOwner->EyeVectors(&forward, &right, &up);
	start = pOwner->Weapon_ShootPosition();
	Vector aimDir = pOwner->GetAutoaimVector(AUTOAIM_2DEGREES);
	Vector end = start + forward * 4096;

	VectorVectors(aimDir, right, up);

	UTIL_TraceLine(start, end, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);
	end = m_worldPosition = tr.endpos;

	m_targetPosition = start + forward * 25;

	ClearMultiDamage();
	CBaseEntity *pHit = tr.m_pEnt;

	if (m_flNextAttackTime < gpGlobals->curtime)
	{
		if (pHit != NULL)
		{
			CTakeDamageInfo dmgInfo(this, pOwner, 25, DMG_SHOCK);
			CalculateBulletDamageForce(&dmgInfo, m_iPrimaryAmmoType, aimDir, tr.endpos);
			pHit->DispatchTraceAttack(dmgInfo, aimDir, &tr);
		}

		if (tr.DidHitWorld() && !(tr.surface.flags & SURF_SKY))
		{
			UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");
			UTIL_DecalTrace(&tr, "RedGlowFade");
			CPVSFilter filter(tr.endpos);
			te->GaussExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);
		}

		ApplyMultiDamage();
		UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactGauss");

		Vector recoilForce = pOwner->BodyDirection2D() * -(2 * 10.0f);
		recoilForce[0.5] += -5.0f;

		pOwner->ApplyAbsVelocityImpulse(recoilForce);
		AddViewKick();
		m_flNextAttackTime = gpGlobals->curtime + VORTZAP_ATTACK_RATE;
	}
}

void CWeaponVortzap::AddViewKick(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	QAngle viewPunch;
	viewPunch.x = random->RandomFloat(-0.4f, 0.4f);
	viewPunch.y = random->RandomFloat(-0.4f, 0.4f);
	viewPunch.z = 0;

	pOwner->ViewPunch(viewPunch);
}

void CWeaponVortzap::PrimaryAttack(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	m_flTimeTillCanRegen = 2.0f;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		m_active = false;
		return;
	}
	else
	{
		if (!m_active)
		{
			SendWeaponAnim(ACT_VM_PRIMARYATTACK);
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_active = true;
		}
		else
		{
			EffectUpdate();
		}
	}
	m_TimeSinceRegenerate = gpGlobals->curtime + NEXT_AMMO_REGENERATE;
	m_TimeSinceAttack = gpGlobals->curtime;
	if (m_flNextPrimaryAttack < gpGlobals->curtime)
	{
		pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
//		WeaponSound(SINGLE);
		AddViewKick();
		m_flNextPrimaryAttack = gpGlobals->curtime + VORTZAP_AMMO_RATE;
	}
}
void CWeaponVortzap::UpdatePenaltyTime(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Check our penalty time decay
	if (((pOwner->m_nButtons & IN_ATTACK) == false) && (m_flNextPrimaryAttack < gpGlobals->curtime))
	{
		m_flRegeneratePenalty -= gpGlobals->frametime;
		m_flRegeneratePenalty = clamp(m_flRegeneratePenalty, 0.0f, AMMO_REGENERATE_PENALTY);
	}
}

void CWeaponVortzap::WeaponIdle(void)
{
	SendWeaponAnim(ACT_VM_IDLE);

	if (m_active)
		m_active = false;
	if (ZeroAmmoIsBecauseRanOut = 1 && m_TimeSinceRegenerate < gpGlobals->curtime)
	{
		Regenerate();
	}
	if (m_flNextZapBulletRegen < gpGlobals->curtime)
	{
		m_flNextZapBulletRegen = gpGlobals->curtime + AMMO_REGEN_RATE;
	}
}

void CWeaponVortzap::Regenerate(void)
{
//	SendWeaponAnim(ACT_VM_IDLE);
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	
	m_flRegeneratePenalty = 2.0;
	
	if (!(pOwner->m_nButtons & IN_ATTACK && m_TimeSinceAttack < gpGlobals->curtime))
	{
		m_flTimeTillCanRegen = m_flTimeTillCanRegen - 0.25;
	}
	if (ZeroAmmoIsBecauseRanOut = 1 && m_TimeSinceRegenerate < gpGlobals->curtime && m_flTimeTillCanRegen <= 0 && (m_flNextZapBulletRegen < gpGlobals->curtime))
	{
		pOwner->GiveAmmo(1, "GaussEnergy");
		m_flRegeneratePenalty = m_flRegeneratePenalty - 0.1;
	}


	if ((m_flRegeneratePenalty = 0) && m_TimeSinceRegenerate < gpGlobals->curtime)
	{
		ZeroAmmoIsBecauseRanOut = 0;
	}
}

void CWeaponVortzap::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 || (pOwner->m_afButtonReleased & IN_ATTACK))
	{
		m_active = false;
		WeaponIdle();
		return;
	}
	if (pOwner->m_nButtons & IN_ATTACK)
	{
		ZeroAmmoIsBecauseRanOut = 0;
	}


	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		ZeroAmmoIsBecauseRanOut = 1;


	if (pOwner->m_nButtons & IN_ATTACK && m_flRegeneratePenalty <= 0)
	{
		PrimaryAttack();
	}
	else
	{
		WeaponIdle();
		return;
	}
}
