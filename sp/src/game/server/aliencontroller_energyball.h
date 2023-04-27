//========= Copyright CCD, No rights reserved. ============//
//
// Purpose:		Projectile shot by the Alien Controller 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	ENERGYBALL_H
#define	ENERGYBALL_H

#include "basegrenade_shared.h"

class CParticleSystem;

enum EnergyBallSize_e
{
	EB_SMALL,
	EB_MEDIUM,
	EB_LARGE,
};

#define BALL_GRAVITY 100

class CEnergyBall : public CBaseGrenade
{
	DECLARE_CLASS(CEnergyBall, CBaseGrenade);

public:
	CEnergyBall(void);

	virtual void		Spawn(void);
	virtual void		Precache(void);
	virtual void		Event_Killed(const CTakeDamageInfo &info);

	virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }

	void 				GrenadeSpitTouch(CBaseEntity *pOther);
	void				SetEnrgyBallSize(int nSize);
	void				Detonate(void);
	void				Think(void);

private:
	DECLARE_DATADESC();

	void	InitBallSound(void);

	CHandle< CParticleSystem >	m_hEnergyBallEffect;
	CSoundPatch		*m_pHissSound;
	bool			m_bPlaySound;
};

#endif	//GRENADESPIT_H
