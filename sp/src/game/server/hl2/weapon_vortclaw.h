//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_VORTCLAW_H
#define WEAPON_VORTCLAW_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_vortclaw.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

#define	VORTCLAW_RANGE	75.0f
#define	VORTCLAW_REFIRE	0.4f

//-----------------------------------------------------------------------------
// CWeaponVortclaw
//-----------------------------------------------------------------------------

class CWeaponVortclaw : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponVortclaw, CBaseHLBludgeonWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponVortclaw();

	float		GetRange( void )		{	return	VORTCLAW_RANGE;	}
	float		GetFireRate( void )		{	return	VORTCLAW_REFIRE;	}

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );

	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
	void		SecondaryAttack( void )	{	return;	}

	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
};

#endif // WEAPON_VORTCLAW_H
