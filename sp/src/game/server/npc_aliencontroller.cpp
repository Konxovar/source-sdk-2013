//========= Copyright CCD, No rights reserved. ============//
//
// Purpose: The Alien Controller, the Nihilanth's Generals. What does that make the Grunts? Colonels. And the Vortigaunts are Corporals. Corporal Vitairn, we meet at last.
//=============================================================================//


#include "cbase.h"
#include "game.h"
#include "npcevent.h"
#include "aliencontroller_energyball.h"
#include "ai_basenpc.h"
#include "ai_condition.h"
#include "ai_schedule.h"
#include "hl2_shareddefs.h"
#include "ai_hint.h"
#include "ai_motor.h"
#include "ai_navigator.h"
#include "ai_route.h"
#include "ai_memory.h"
#include "activitylist.h"
#include "gamerules.h"	
#include "npcevent.h"
#include "ai_interactions.h"
#include "ndebugoverlay.h"
#include "player.h"
#include "hl2_gamerules.h"
#include "gamerules.h"		// For g_pGameRules
#include "ammodef.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include	"ai_hull.h"
#include "ai_baseactor.h"
#include "ai_moveprobe.h"



#include "ai_senses.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar sk_controller_health("sk_controller_health", "45");
//
// Spawnflags.
//
#define SF_ALIENCONTROLLER_FLYING		16
#define ALIENCONTROLLER_TAKEOFF_SPEED 170
#define ALIENCONTROLLER_AIRSPEED			220 // FIXME: should be about 440, but I need to add acceleration


enum FlyState_t
{
	FlyState_Flying,
	FlyState_Floating,
	FlyState_Falling,
};




//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		ALNCNTRL_AE_SHOOTENERGYBLL		( 1 )
static int AE_ALIENCONTROLLER_TAKEOFF;
static int AE_ALIENCONTROLLER_FLY;
static int AE_ALIENCONTROLLER_FLOAT;

//
// Custom activities.
//
static int ACT_ALIENCONTROLLER_TAKEOFF;
static int ACT_ALIENCONTROLLER_FLY;
static int ACT_ALIENCONTROLLER_FLOAT;






//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_AlienController : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CNPC_AlienController, CAI_BaseActor);
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;


	void	Spawn(void);
	void	Precache(void);
	Class_T Classify(void);
	void	HandleAnimEvent(animevent_t *pEvent);
	int		GetSoundInterests(void);
	void	PrescheduleThink(void);

	int RangeAttack1Conditions(float flDot, float flDist);

	virtual void OnListened(void);

	virtual bool FValidateHintType(CAI_Hint *pHint);
	virtual Activity GetHintActivity(short sHintType, Activity HintsActivity);

	virtual void StartTask(const Task_t *pTask);
	virtual void RunTask(const Task_t *pTask);

	void	GatherConditions( void );

	virtual bool	IsCurTaskContinuousMove();

	virtual int		SelectSchedule( void );
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	int				SelectScheduleAttack();

	virtual int		TranslateSchedule( int scheduleType );

	//sounds

	void IdleSound(void);
	void PainSound(const CTakeDamageInfo &info);
	void AlertSound(void);
	void DeathSound(const CTakeDamageInfo &info);
	void AttackSound(void);
	void ShriekSound(void);


	void RunAI(void);
	bool FInViewCone(Vector pOrigin);



	virtual bool OverrideMove(float flInterval);

	NPC_STATE SelectIdealState(void);

	//
	// Alien Controller schedules.
	//
	enum
	{
		SCHED_ALIENCONTROLLER_IDLE_FLOAT = BaseClass::NEXT_SCHEDULE,

		//
		// Various levels of wanting to get away from something, selected
		// by current value of m_nMorale.
		//

		SCHED_ALIENCONTROLLER_FLOAT,
		SCHED_ALIENCONTROLLER_FLY,
		SCHED_ALIENCONTROLLER_FLY_FAIL,
		SCHED_ALIENCONTROLLER_FLYING, //This would be to SCHED_ALIENCONTROLLER_FLOAT what walking would be to standing still.

		SCHED_ALIENCONTROLLER_BARNACLED,

		SCHED_ALIENCONTROLLER_COMBAT_FAIL,

		SCHED_ALIENCONTROLLER_RANGE_ATTACK1,
		SCHED_ALIENCONTROLLER_TAKE_COVER1,
		SCHED_ALIENCONTROLLER_MOVE_TO_MELEE,

		SCHED_ALIENCONTROLLER_ESTABLISH_LINE_OF_FIRE,

		NEXT_SCHEDULE,
	};


	//
	// Alien Controller tasks.
	//
	enum
	{
		TASK_ALIENCONTROLLER_FIND_FLYTO_NODE = BaseClass::NEXT_TASK,
		//TASK_CROW_PREPARE_TO_FLY,
		TASK_ALIENCONTROLLER_TAKEOFF,
		//TASK_CROW_LAND,
		TASK_ALIENCONTROLLER_FLOAT,
		TASK_ALIENCONTROLLER_FLY,
		TASK_ALIENCONTROLLER_FLY_TO_HINT,
		TASK_ALIENCONTROLLER_PICK_RANDOM_GOAL,
		TASK_ALIENCONTROLLER_PICK_EVADE_GOAL,

		TASK_ALIENCONTROLLER_FALL_TO_GROUND,

		TASK_ALIENCONTROLLER_WAIT_FOR_BARNACLE_KILL,

		TASK_ALIENCONTROLLER_CHASE_ENEMY_CONTINUOUSLY,
	};


	//=========================================================
	// Alien Controller Conditions
	//=========================================================
	enum
	{
		COND_ALIENCONTROLLER_ALIENCONTROLLER_FLY = BaseClass::NEXT_CONDITION,
		COND_ALIENCONTROLLER_FORCED_FLY,
		COND_ALIENCONTROLLER_BARNACLED,
		NEXT_CONDITION,
	};

private:
	// Select the combat schedule
	int SelectCombatSchedule();

	// Chase the enemy, updating the target position as the player moves
	void StartTaskChaseEnemyContinuously( const Task_t *pTask );
	void RunTaskChaseEnemyContinuously( const Task_t *pTask );

protected:
	void SetFlyingState(FlyState_t eState);
	inline bool IsFlying(void) const { return GetNavType() == NAV_FLY; }

	void Takeoff(const Vector &vGoal);
	void FlapSound(void);

	void MoveAlienControllerFly(float flInterval);
	bool Probe(const Vector &vecMoveDir, float flSpeed, Vector &vecDeflect);

	float m_flFloatIdleMoveTime;

	float m_flEnemyDist;		// Distance to GetEnemy(), cached in GatherEnemyConditions.
	int m_nMorale;				// Used to determine which avoidance schedule to pick. Degrades as I pick avoidance schedules.

	bool m_bReachedMoveGoal;

private:

	float				m_flFloatTime;
	bool				m_bFloat;
	Vector				m_vLastStoredOrigin;
	float				m_flLastStuckCheck;

	float m_flNextFireTime;

	Vector				m_vDesiredTarget;
	Vector				m_vCurrentTarget;

	int   m_nCtrlrEnergyBallSprite;

	float m_nextAlnCntrlSoundTime;

};




//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------

BEGIN_DATADESC(CNPC_AlienController)

DEFINE_FIELD(m_flNextFireTime, FIELD_TIME),

END_DATADESC()

LINK_ENTITY_TO_CLASS(npc_aliencontroller, CNPC_AlienController);

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_AlienController::Classify(void)
{
	return	CLASS_ALIENCONTROLLER;
}


//-----------------------------------------------------------------------------
// Continuous movement tasks
//-----------------------------------------------------------------------------
bool CNPC_AlienController::IsCurTaskContinuousMove()
{
	const Task_t* pTask = GetTask();
	if ( pTask && (pTask->iTask == TASK_ALIENCONTROLLER_CHASE_ENEMY_CONTINUOUSLY) )
		return true;

	return BaseClass::IsCurTaskContinuousMove();
}

//-----------------------------------------------------------------------------
// Chase the enemy, updating the target position as the player moves
//-----------------------------------------------------------------------------
void CNPC_AlienController::StartTaskChaseEnemyContinuously( const Task_t *pTask )
{
	DevWarning( 1, "StartTaskChaseEnemyContinuously!!\n" );
	CBaseEntity *pEnemy = GetEnemy();
	if ( !pEnemy )
	{
		TaskFail( FAIL_NO_ENEMY );
		return;
	}

	// We're done once we get close enough
	if ( WorldSpaceCenter().DistToSqr( pEnemy->WorldSpaceCenter() ) <= pTask->flTaskData * pTask->flTaskData )
	{
		TaskComplete();
		return;
	}

	// TASK_GET_PATH_TO_ENEMY
	if ( IsUnreachable( pEnemy ) )
	{
		TaskFail(FAIL_NO_ROUTE);
		return;
	}

	if ( !GetNavigator()->SetGoal( GOALTYPE_ENEMY, AIN_NO_PATH_TASK_FAIL ) )
	{
		// no way to get there =( 
		DevWarning( 1, "GetPathToEnemy failed!!\n" );
		RememberUnreachable( pEnemy );
		TaskFail(FAIL_NO_ROUTE);
		return;
	}

	// NOTE: This is TaskRunPath here.
	if ( TranslateActivity( ACT_RUN ) != ACT_INVALID )
	{
		GetNavigator()->SetMovementActivity( ACT_RUN );
	}
	else
	{
		GetNavigator()->SetMovementActivity(ACT_WALK);
	}

	// Cover is void once I move
	Forget( bits_MEMORY_INCOVER );

	if (GetNavigator()->GetGoalType() == GOALTYPE_NONE)
	{
		TaskComplete();
		GetNavigator()->ClearGoal();		// Clear residual state
		return;
	}

	// No shooting delay when in this mode
	m_MoveAndShootOverlay.SetInitialDelay( 0.0 );

	if (!GetNavigator()->IsGoalActive())
	{
		SetIdealActivity( GetStoppedActivity() );
	}
	else
	{
		// Check validity of goal type
		ValidateNavGoal();
	}

	// set that we're probably going to stop before the goal
	GetNavigator()->SetArrivalDistance( pTask->flTaskData );
	m_vSavePosition = GetEnemy()->WorldSpaceCenter();
}

void CNPC_AlienController::RunTaskChaseEnemyContinuously( const Task_t *pTask )
{
	if (!GetNavigator()->IsGoalActive())
	{
		SetIdealActivity( GetStoppedActivity() );
	}
	else
	{
		// Check validity of goal type
		ValidateNavGoal();
	}

	CBaseEntity *pEnemy = GetEnemy();
	if ( !pEnemy )
	{
		TaskFail( FAIL_NO_ENEMY );
		return;
	}

	// We're done once we get close enough
	if ( WorldSpaceCenter().DistToSqr( pEnemy->WorldSpaceCenter() ) <= pTask->flTaskData * pTask->flTaskData )
	{
		GetNavigator()->StopMoving();
		TaskComplete();
		return;
	}

	// Recompute path if the enemy has moved too much
	if ( m_vSavePosition.DistToSqr( pEnemy->WorldSpaceCenter() ) < (pTask->flTaskData * pTask->flTaskData) )
		return;

	if ( IsUnreachable( pEnemy ) )
	{
		TaskFail(FAIL_NO_ROUTE);
		return;
	}

	if ( !GetNavigator()->RefindPathToGoal() )
	{
		TaskFail(FAIL_NO_ROUTE);
		return;
	}

	m_vSavePosition = pEnemy->WorldSpaceCenter();
}




//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_AlienController::GetSoundInterests(void)
{
	return	SOUND_WORLD | SOUND_COMBAT | SOUND_PLAYER | SOUND_DANGER | SOUND_PHYSICS_DANGER | SOUND_BULLET_IMPACT | SOUND_MOVE_AWAY;
}

//=========================================================
// OnListened - monsters dig through the active sound list for
// any sounds that may interest them. (smells, too!)
//=========================================================
void CNPC_AlienController::OnListened(void)
{
	AISoundIter_t iter;

	CSound *pCurrentSound;


	pCurrentSound = GetSenses()->GetFirstHeardSound(&iter);

	while (pCurrentSound)
	{
		pCurrentSound = GetSenses()->GetNextHeardSound(&iter);
	}

	BaseClass::OnListened();
}

//========================================================
// RunAI - overridden for controller because there are not things
// that need to be checked every think.
//========================================================

void CNPC_AlienController::RunAI(void)
{
	/*
	if (GetSchedule(SCHED_RANGE_ATTACK1))
	{
		SetActivity(ACT_RANGE_ATTACK1);
	}
	*/
	//This crashed the game.
	BaseClass::RunAI();
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_AlienController::Spawn()
{

	char *szModel = (char *)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/aliencontroller.mdl";
		SetModelName(AllocPooledString(szModel));
	}

	Precache();
	SetModel(szModel);

	BaseClass::Spawn();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_FLY);

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_FLY | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD | bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_SQUAD);


	AddEFlags(EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);
	SetBloodColor(BLOOD_COLOR_GREEN);
	m_iHealth = sk_controller_health.GetFloat();
	m_flFieldOfView = 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;

	m_flNextFireTime = gpGlobals->curtime;

	NPCInit();


	m_flDistTooFar = 784;
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_AlienController::Precache()
{
	PrecacheModel(STRING(GetModelName()));

	PrecacheModel("models/spitball_large.mdl");

	m_nCtrlrEnergyBallSprite = PrecacheModel("models/effects/combineball.mdl");// client side spittle.

	PrecacheScriptSound("NPC_CombineBall.Launch");
	PrecacheScriptSound("NPC_Bullsquid.Idle");
	PrecacheScriptSound("NPC_Bullsquid.Pain");
	PrecacheScriptSound("NPC_Bullsquid.Alert");
	PrecacheScriptSound("NPC_Bullsquid.Death");
	PrecacheScriptSound("NPC_Bullsquid.Growl");
	PrecacheScriptSound("NPC_Bullsquid.TailWhip");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_AlienController::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	if (GetMoveType() == MOVETYPE_NONE && !GetMoveParent())
	{
		SetupVPhysicsHull();
	}
}

//-----------------------------------------------------------------------------
// AI Schedules Specific to this NPC
//-----------------------------------------------------------------------------


//Attack Sound


//=========================================================
// IdleSound 
//=========================================================
#define ALNCTRL_ATTN_IDLE	(float)1.5
void CNPC_AlienController::IdleSound(void)
{
	EmitSound("NPC_Bullsquid.Idle");
}

//=========================================================
// PainSound 
//=========================================================
void CNPC_AlienController::PainSound(const CTakeDamageInfo &info)
{
	EmitSound("NPC_Bullsquid.Pain");
}

//=========================================================
// AlertSound
//=========================================================
void CNPC_AlienController::AlertSound(void)
{
	EmitSound("NPC_Bullsquid.Alert");
}

//=========================================================
// DeathSound
//=========================================================
void CNPC_AlienController::DeathSound(const CTakeDamageInfo &info)
{
	EmitSound("NPC_Bullsquid.Death");
}

//=========================================================
// AttackSound
//=========================================================
void CNPC_AlienController::AttackSound(void)
{
	EmitSound("NPC_CombineBall.Launch");
}

//=========================================================
// ShriekSound
//=========================================================
void CNPC_AlienController::ShriekSound(void)
{
	if (gpGlobals->curtime >= m_nextAlnCntrlSoundTime)
	{
		EmitSound("NPC_Bullsquid.Growl");
		m_nextAlnCntrlSoundTime = gpGlobals->curtime + random->RandomInt(1.5, 3.0);
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_AlienController::GatherConditions( void )
{	
	ClearCondition( COND_ALIENCONTROLLER_ALIENCONTROLLER_FLY );
	ClearCondition( COND_ALIENCONTROLLER_BARNACLED );

	BaseClass::GatherConditions();
}

//-----------------------------------------------------------------------------
// Select attack schedules
//-----------------------------------------------------------------------------
int CNPC_AlienController::SelectScheduleAttack()
{
	// Kick attack?
	if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
	{
		DevWarning( 1, "SCHED_MELEE_ATTACK1!!\n" );
		return SCHED_MELEE_ATTACK1;
	}

	// If I'm fighting a combine turret (it's been hacked to attack me), I can't really
	// hurt it with bullets, so become grenade happy.
	/*if ( GetEnemy() && GetEnemy()->Classify() == CLASS_COMBINE && FClassnameIs(GetEnemy(), "npc_turret_floor") )
	{
		
	}*/

	// Can I shoot?
	if ( HasCondition(COND_CAN_RANGE_ATTACK1) )
	{
		DevWarning( 1, "SCHED_RANGE_ATTACK1!!\n" );
		return SCHED_RANGE_ATTACK1;

		// Throw a grenade if not allowed to engage with weapon.
		/*if ( CanGrenadeEnemy() )
		{
				return SCHED_RANGE_ATTACK2;
		}*/

		/*DesireCrouch();
		return SCHED_TAKE_COVER_FROM_ENEMY;*/
	}

	if (HasCondition(COND_WEAPON_SIGHT_OCCLUDED))
	{
		DevWarning( 1, "COND_WEAPON_SIGHT_OCCLUDED!!\n" );
		// If they are hiding behind something that we can destroy, start shooting at it.
		CBaseEntity *pBlocker = GetEnemyOccluder();
		if ( pBlocker && pBlocker->GetHealth() > 0 )
		{
			return SCHED_SHOOT_ENEMY_COVER;
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_AlienController::TranslateSchedule( int scheduleType ) 
{
	switch( scheduleType )
	{
	case SCHED_FAIL_ESTABLISH_LINE_OF_FIRE:
		{
			if( !IsCrouching() )
			{
				if( GetEnemy() && CouldShootIfCrouching( GetEnemy() ) )
				{
					Crouch();
					return SCHED_COMBAT_FACE;
				}
			}

			if( HasCondition( COND_SEE_ENEMY ) )
			{
				return TranslateSchedule( SCHED_TAKE_COVER_FROM_ENEMY );
			}
		}
		break;
	case SCHED_ESTABLISH_LINE_OF_FIRE:
		{
			// always assume standing
			// Stand();

			return SCHED_ALIENCONTROLLER_ESTABLISH_LINE_OF_FIRE;
		}
		break;
	/*case SCHED_HIDE_AND_RELOAD:
		{
			// stand up, just in case
			// Stand();
			// DesireStand();
			
			return SCHED_ALIENCONTROLLER_HIDE_AND_RELOAD;
		}
		break;*/
	case SCHED_RANGE_ATTACK1:
		{
			// always assume standing
			//Stand();

			return SCHED_ALIENCONTROLLER_RANGE_ATTACK1;
		}
	/*case SCHED_FAIL:
		{
			if ( GetEnemy() != NULL )
			{
				return SCHED_ALIENCONTROLLER_COMBAT_FAIL;
			}
			return SCHED_FAIL;
		}*/
	}

	return BaseClass::TranslateSchedule( scheduleType );
}


//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_AlienController::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	default:
		BaseClass::HandleAnimEvent(pEvent);

	case ALNCNTRL_AE_SHOOTENERGYBLL:
	{
		if (GetSchedule(SCHED_RANGE_ATTACK1))
		{
			Vector vFirePos;

			GetAttachment("FirePoint", vFirePos);

			if(GetEnemy() == NULL) {
				DevWarning( 1, "ALNCNTRL_AE_SHOOTENERGYBLL empty Enemy!!\n" );
				return;
			}

			Vector			vTarget = GetEnemy()->GetAbsOrigin();
			Vector			vToss;
			CBaseEntity*	pBlocker;
			float flGravity = BALL_GRAVITY;
			ThrowLimit(vFirePos, vTarget, flGravity, 3, Vector(0, 0, 0), Vector(0, 0, 0), GetEnemy(), &vToss, &pBlocker);
			
			CEnergyBall *pGrenade = (CEnergyBall*)CreateNoSpawn("aliencontroller_energyball", vFirePos, vec3_angle, this);
			//pGrenade->KeyValue( "velocity", vToss );

			pGrenade->Spawn();
			pGrenade->SetThrower(this);
			pGrenade->SetOwnerEntity(this);
			pGrenade->SetEnrgyBallSize(EB_MEDIUM);
			pGrenade->SetAbsVelocity(vToss);

			// Tumble through the air
			pGrenade->SetLocalAngularVelocity(
			QAngle(
			random->RandomFloat(-100, -500),
			random->RandomFloat(-100, -500),
			random->RandomFloat(-100, -500)
			)
			);
			
			AttackSound();

			CPVSFilter filter(vFirePos);
			te->SpriteSpray(filter, 0.0,
				&vFirePos, &vToss, m_nCtrlrEnergyBallSprite, 5, 10, 15);
		}
	}

	break;
	}

	if (pEvent->event == AE_ALIENCONTROLLER_TAKEOFF)
	{
		if (GetNavigator()->GetPath()->GetCurWaypoint())
		{
			Takeoff(GetNavigator()->GetCurWaypointPos());
		}
		return;
	}

	if (pEvent->event == AE_ALIENCONTROLLER_FLOAT)
	{
		//
		// Start floating.
		//
		SetActivity(ACT_HOVER);

		m_bFloat = false;
		m_flFloatTime = gpGlobals->curtime + random->RandomFloat(3, 5);

		return;


		return;
	}

	if (pEvent->event == AE_ALIENCONTROLLER_FLY)
	{
		//
		// Start flying.
		//
		SetActivity(ACT_FLY);

		m_bFloat = false;
		m_flFloatTime = gpGlobals->curtime + random->RandomFloat(3, 5);

		return;
	}
}


//=========================================================
// RangeAttack1Conditions
//=========================================================
int CNPC_AlienController::RangeAttack1Conditions ( float flDot, float flDist )
{
	if ( flDist < 85)
	{
		return COND_TOO_CLOSE_TO_ATTACK;
	}
	else if (flDist > 784)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.5)
	{
		return COND_NOT_FACING_ATTACK;
	}

	if(gpGlobals->curtime < m_flNextFireTime) {
		// can't attack right now
		return(COND_NONE);
	}

	// not moving, so fire again pretty soon.
	m_flNextFireTime = gpGlobals->curtime + 1.5;
	return COND_CAN_RANGE_ATTACK1;
}

bool CNPC_AlienController::OverrideMove(float flInterval)
{
	if (GetNavigator()->GetPath()->CurWaypointNavType() == NAV_FLY && GetNavigator()->GetNavType() != NAV_FLY)
	{
		SetNavType(NAV_FLY);
	}

	if (IsFlying())
	{
		if (GetNavigator()->GetPath()->GetCurWaypoint())
		{
			if (m_flLastStuckCheck <= gpGlobals->curtime)
			{
				if (m_vLastStoredOrigin == GetAbsOrigin())
				{
					if (GetAbsVelocity() == vec3_origin)
					{
						float flDamage = m_iHealth;

						CTakeDamageInfo	dmgInfo(this, this, flDamage, DMG_GENERIC);
						GuessDamageForce(&dmgInfo, vec3_origin - Vector(0, 0, 0.1), GetAbsOrigin());
						TakeDamage(dmgInfo);

						return false;
					}
					else
					{
						m_vLastStoredOrigin = GetAbsOrigin();
					}
				}
				else
				{
					m_vLastStoredOrigin = GetAbsOrigin();
				}

				m_flLastStuckCheck = gpGlobals->curtime + 1.0f;
			}

			if (m_bReachedMoveGoal)
			{
				if (HasCondition(COND_CAN_RANGE_ATTACK1))
				{
					SetSchedule(SCHED_RANGE_ATTACK1);
				}
				else
				{
					SetIdealActivity((Activity)ACT_FLY);
					SetFlyingState(FlyState_Floating);
				}
				TaskMovementComplete();
			}
			else
			{
				SetIdealActivity(ACT_FLY);
				MoveAlienControllerFly(flInterval);
			}

		}
		
		else if (!GetTask() || GetTask()->iTask == TASK_WAIT_FOR_MOVEMENT)
		{
			SetSchedule(SCHED_ALIENCONTROLLER_IDLE_FLOAT);
			SetFlyingState(FlyState_Floating);
			SetIdealActivity(ACT_HOVER);
		}
		
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Performs a takeoff. Called via an animation event at the moment
//			our feet leave the ground.
// Input  : pGoalEnt - The entity that we are going to fly toward.
//-----------------------------------------------------------------------------
void CNPC_AlienController::Takeoff(const Vector &vGoal)
{
	if (vGoal != vec3_origin)
	{
		//
		// Lift us off ground so engine doesn't instantly reset FL_ONGROUND.
		//
		UTIL_SetOrigin(this, GetAbsOrigin() + Vector(0, 0, 1));

		//
		// Fly straight at the goal entity at our maximum airspeed.
		//
		Vector vecMoveDir = vGoal - GetAbsOrigin();
		VectorNormalize(vecMoveDir);

		// FIXME: pitch over time

		SetFlyingState(FlyState_Flying);

		QAngle angles;
		VectorAngles(vecMoveDir, angles);
		SetAbsAngles(angles);

		SetAbsVelocity(vecMoveDir * ALIENCONTROLLER_TAKEOFF_SPEED);
	}
}
//-----------------------------------------------------------------------------
// Purpose: Handles all flight movement.
// Input  : flInterval - Seconds to simulate.
//-----------------------------------------------------------------------------

void CNPC_AlienController::MoveAlienControllerFly(float flInterval)
{
	//
	// Bound interval so we don't get ludicrous motion when debugging
	// or when framerate drops catastrophically.  
	//
	if (flInterval > 1.0)
	{
		flInterval = 1.0;
	}

	//
	// Determine the goal of our movement.
	//
	Vector vecMoveGoal = GetAbsOrigin();

	if (GetNavigator()->IsGoalActive())
	{
		vecMoveGoal = GetNavigator()->GetCurWaypointPos();

		if (GetNavigator()->CurWaypointIsGoal() == false)
		{
			AI_ProgressFlyPathParams_t params(MASK_NPCSOLID);
			params.bTrySimplify = false;

			GetNavigator()->ProgressFlyPath(params); // ignore result, crow handles completion directly

			// Fly towards the hint.
			if (GetNavigator()->GetPath()->GetCurWaypoint())
			{
				vecMoveGoal = GetNavigator()->GetCurWaypointPos();
			}
		}
	}
	else
	{
		// No movement goal.
		vecMoveGoal = GetAbsOrigin();
		SetAbsVelocity(vec3_origin);
		return;
	}

	Vector vecMoveDir = (vecMoveGoal - GetAbsOrigin());
	Vector vForward;
	AngleVectors(GetAbsAngles(), &vForward);

	//
	// Fly towards the movement goal.
	//
	float flDistance = (vecMoveGoal - GetAbsOrigin()).Length();

	if (vecMoveGoal != m_vDesiredTarget)
	{
		m_vDesiredTarget = vecMoveGoal;
	}
	else
	{
		m_vCurrentTarget = (m_vDesiredTarget - GetAbsOrigin());
		VectorNormalize(m_vCurrentTarget);
	}

	float flLerpMod = 0.25f;

	if (flDistance <= 256.0f)
	{
		flLerpMod = 1.0f - (flDistance / 256.0f);
	}


	VectorLerp(vForward, m_vCurrentTarget, flLerpMod, vForward);


	if (flDistance < ALIENCONTROLLER_AIRSPEED * flInterval)
	{
		if (GetNavigator()->IsGoalActive())
		{
			if (GetNavigator()->CurWaypointIsGoal())
			{
				m_bReachedMoveGoal = true;
			}
			else
			{
				GetNavigator()->AdvancePath();
			}
		}
		else
			m_bReachedMoveGoal = true;
	}

	if (GetHintNode())
	{
		AIMoveTrace_t moveTrace;
		GetMoveProbe()->MoveLimit(NAV_FLY, GetAbsOrigin(), GetNavigator()->GetCurWaypointPos(), MASK_NPCSOLID, GetNavTargetEntity(), &moveTrace);

		//See if it succeeded
		if (IsMoveBlocked(moveTrace.fStatus))
		{
			Vector vNodePos = vecMoveGoal;
			GetHintNode()->GetPosition(this, &vNodePos);

			GetNavigator()->SetGoal(vNodePos);
		}
	}
	else if (HasCondition(COND_CAN_RANGE_ATTACK1))
	{
		SetSchedule(SCHED_RANGE_ATTACK1);
	}

	//
	// Look to see if we are going to hit anything.
	//
	VectorNormalize(vForward);
	Vector vecDeflect;
	if (Probe(vForward, ALIENCONTROLLER_AIRSPEED * flInterval, vecDeflect))
	{
		vForward = vecDeflect;
		VectorNormalize(vForward);
	}

	SetAbsVelocity(vForward * ALIENCONTROLLER_AIRSPEED);

	if (GetAbsVelocity().Length() > 0 && GetNavigator()->CurWaypointIsGoal() && flDistance < ALIENCONTROLLER_AIRSPEED)
	{
		SetIdealActivity((Activity)ACT_ALIENCONTROLLER_FLOAT);
	}


	//Bank and set angles.
	Vector vRight;
	QAngle vRollAngle;

	VectorAngles(vForward, vRollAngle);
	vRollAngle.z = 0;

	AngleVectors(vRollAngle, NULL, &vRight, NULL);

	float flRoll = DotProduct(vRight, vecMoveDir) * 45;
	flRoll = clamp(flRoll, -45, 45);

	vRollAngle[ROLL] = flRoll;
	SetAbsAngles(vRollAngle);
}


//-----------------------------------------------------------------------------
// Purpose: Looks ahead to see if we are going to hit something. If we are, a
//			recommended avoidance path is returned.
// Input  : vecMoveDir - 
//			flSpeed - 
//			vecDeflect - 
// Output : Returns true if we hit something and need to deflect our course,
//			false if all is well.
//-----------------------------------------------------------------------------
bool CNPC_AlienController::Probe(const Vector &vecMoveDir, float flSpeed, Vector &vecDeflect)
{
	//
	// Look 1/2 second ahead.
	//
	trace_t tr;
	AI_TraceHull(GetAbsOrigin(), GetAbsOrigin() + vecMoveDir * flSpeed, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, TVECOLLISION_GROUP_ALIENCONTROLLER, &tr);
	if (tr.fraction < 1.0f)
	{
		//
		// If we hit something, deflect flight path parallel to surface hit.
		//
		Vector vecUp;
		CrossProduct(vecMoveDir, tr.plane.normal, vecUp);
		CrossProduct(tr.plane.normal, vecUp, vecDeflect);
		VectorNormalize(vecDeflect);
		return true;
	}

	vecDeflect = vec3_origin;
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Switches between flying mode and floating mode.
//-----------------------------------------------------------------------------
void CNPC_AlienController::SetFlyingState(FlyState_t eState)
{
	if (eState == FlyState_Flying)
	{
		// Flying
		SetGroundEntity(NULL);
		AddFlag(FL_FLY);
		SetNavType(NAV_FLY);
		CapabilitiesRemove(bits_CAP_MOVE_GROUND);
		CapabilitiesAdd(bits_CAP_MOVE_FLY);
		SetMoveType(MOVETYPE_FLY);
		m_vLastStoredOrigin = GetAbsOrigin();
		m_flLastStuckCheck = gpGlobals->curtime + 3.0f;
		m_flFloatIdleMoveTime = gpGlobals->curtime + random->RandomFloat(5.0f, 10.0f);
	}
	else if (eState == FlyState_Floating)
	{
		// Floating
		QAngle angles = GetAbsAngles();
		angles[PITCH] = 0.0f;
		angles[ROLL] = 0.0f;
		SetAbsAngles(angles);

		AddFlag(FL_FLY);
		SetNavType(NAV_FLY);
		SetMoveType(MOVETYPE_FLY);
		if (HasCondition(COND_CAN_RANGE_ATTACK1))
		{
			SetSchedule(SCHED_RANGE_ATTACK1);
		}
		m_vLastStoredOrigin = vec3_origin;
		m_flLastStuckCheck = gpGlobals->curtime + 3.0f;
		m_flFloatIdleMoveTime = gpGlobals->curtime + random->RandomFloat(5.0f, 10.0f);
	}
	else
	{
		// Falling
		SetNavType(NAV_FLY);
		SetMoveType(MOVETYPE_FLYGRAVITY);
		m_flFloatIdleMoveTime = gpGlobals->curtime + random->RandomFloat(5.0f, 10.0f);
	}
}

bool CNPC_AlienController::FValidateHintType(CAI_Hint *pHint)
{
	return(pHint->HintType() == HINT_ALIENCONTROLLER_FLYTO_POINT);
}


//-----------------------------------------------------------------------------
// Purpose: Returns the activity for the given hint type.
// Input  : sHintType - 
//-----------------------------------------------------------------------------
Activity CNPC_AlienController::GetHintActivity(short sHintType, Activity HintsActivity)
{
	if (sHintType == HINT_ALIENCONTROLLER_FLYTO_POINT)
	{
		return ACT_FLY;
	}

	return BaseClass::GetHintActivity(sHintType, HintsActivity);
}



//-----------------------------------------------------------------------------
// Select the combat schedule
//-----------------------------------------------------------------------------
int CNPC_AlienController::SelectCombatSchedule()
{
	DevWarning( 1, "SelectCombatSchedule!!\n" );
	// -----------
	// dead enemy
	// -----------
	if ( HasCondition( COND_ENEMY_DEAD ) )
	{
		// call base class, all code to handle dead enemies is centralized there.
		return SCHED_NONE;
	}

	// -----------
	// new enemy
	// -----------
	if ( HasCondition( COND_NEW_ENEMY ) )
	{
		CBaseEntity *pEnemy = GetEnemy();
		bool bFirstContact = false;
		float flTimeSinceFirstSeen = gpGlobals->curtime - GetEnemies()->FirstTimeSeen( pEnemy );

		if( flTimeSinceFirstSeen < 3.0f )
			bFirstContact = true;

		if ( pEnemy )
		{
			if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
			{
				return SCHED_RANGE_ATTACK1;
			}

			/*if( HasCondition( COND_SEE_ENEMY ) && CanGrenadeEnemy() )
			{
				return SCHED_RANGE_ATTACK2;
			}*/
			
			return SCHED_ESTABLISH_LINE_OF_FIRE;
		}
	}

	// ---------------------
	// no ammo
	// ---------------------
	/*if ( ( HasCondition ( COND_NO_PRIMARY_AMMO ) || HasCondition ( COND_LOW_PRIMARY_AMMO ) ) && !HasCondition( COND_CAN_MELEE_ATTACK1) )
	{
		return SCHED_HIDE_AND_RELOAD;
	}*/

	// ----------------------
	// LIGHT DAMAGE
	// ----------------------
	if ( HasCondition( COND_LIGHT_DAMAGE ) )
	{
		return SCHED_TAKE_COVER_FROM_ENEMY;
	}

	// If I'm scared of this enemy run away
	if ( IRelationType( GetEnemy() ) == D_FR )
	{
		if (HasCondition( COND_SEE_ENEMY )	|| 
			HasCondition( COND_SEE_FEAR )	|| 
			HasCondition( COND_LIGHT_DAMAGE ) || 
			HasCondition( COND_HEAVY_DAMAGE ))
		{
			FearSound();
			//ClearCommandGoal();
			return SCHED_RUN_FROM_ENEMY;
		}

		// If I've seen the enemy recently, cower. Ignore the time for unforgettable enemies.
		AI_EnemyInfo_t *pMemory = GetEnemies()->Find( GetEnemy() );
		if ( (pMemory && pMemory->bUnforgettable) || (GetEnemyLastTimeSeen() > (gpGlobals->curtime - 5.0)) )
		{
			// If we're facing him, just look ready. Otherwise, face him.
			if ( FInAimCone( GetEnemy()->EyePosition() ) )
				return SCHED_COMBAT_STAND;

			return SCHED_FEAR_FACE;
		}
	}

	int attackSchedule = SelectScheduleAttack();
	if ( attackSchedule != SCHED_NONE )
		return attackSchedule;

	if (HasCondition(COND_ENEMY_OCCLUDED))
	{
		// stand up, just in case
		Stand();
		DesireStand();

		if( GetEnemy() && !(GetEnemy()->GetFlags() & FL_NOTARGET) )
		{
			// Charge in and break the enemy's cover!
			return SCHED_ESTABLISH_LINE_OF_FIRE;
		}

		// If I'm a long, long way away, establish a LOF anyway. Once I get there I'll
		// start respecting the squad slots again.
		float flDistSq = GetEnemy()->WorldSpaceCenter().DistToSqr( WorldSpaceCenter() );
		if ( flDistSq > Square(3000) )
			return SCHED_ESTABLISH_LINE_OF_FIRE;

		// Otherwise tuck in.
		//Remember( bits_MEMORY_INCOVER );
		//return SCHED_ALIENCONTROLLER_WAIT_IN_COVER;
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the best new schedule for this NPC based on current conditions.
//-----------------------------------------------------------------------------
int CNPC_AlienController::SelectSchedule(void)
{
	if (HasCondition(COND_ALIENCONTROLLER_BARNACLED))
	{
		DevWarning( 1, "SCHED_ALIENCONTROLLER_BARNACLED!!\n" );
		// Caught by a barnacle!
		return SCHED_ALIENCONTROLLER_BARNACLED;
	}

	/*
	//
	// If we got damaged a bunch, and it was bad, fly away.
	//
	if (HasCondition(COND_REPEATED_DAMAGE) && HasCondition(COND_HEAVY_DAMAGE))
	{
		return SCHED_ALIENCONTROLLER_FLY;
	}
	*/


	switch (m_NPCState)
	{
	case NPC_STATE_IDLE:
		{
			//
			// If we're flying, just find somewhere to fly to.
			//
			if (IsFlying())
			{
				DevWarning( 1, "SCHED_ALIENCONTROLLER_IDLE_FLOAT!!\n" );
				return SCHED_ALIENCONTROLLER_IDLE_FLOAT;
			}
		}
	case NPC_STATE_ALERT:
			DevWarning( 1, "NPC_STATE_ALERT!!\n" );
	case NPC_STATE_COMBAT:
		{
			int nSched = SelectCombatSchedule();
			if ( nSched != SCHED_NONE )
				return nSched;
		}
		/*
		{
			// dead enemy
			if (HasCondition(COND_ENEMY_DEAD))
			{
				// call base class, all code to handle dead enemies is centralized there.
				return BaseClass::SelectSchedule();
			}

			if (HasCondition(COND_NEW_ENEMY))
			{

				return SCHED_WAKE_ANGRY;
			}

			if (HasCondition(COND_CAN_RANGE_ATTACK1))
			{
				return SCHED_RANGE_ATTACK1;
			}

			return SCHED_CHASE_ENEMY; //CHECK IF THIS RUINS ANYTHING

			break;
		}*/
	}
	
	DevWarning( 1, "BaseClass::SelectSchedule!!\n" );
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_AlienController::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	if( failedSchedule == SCHED_ALIENCONTROLLER_TAKE_COVER1 )
	{
		// This eases the effects of an unfortunate bug that usually plagues shotgunners. Since their rate of fire is low,
		// they spend relatively long periods of time without an attack squad slot. If you corner a shotgunner, usually 
		// the other memebers of the squad will hog all of the attack slots and pick schedules to move to establish line of
		// fire. During this time, the shotgunner is prevented from attacking. If he also cannot find cover (the fallback case)
		// he will stand around like an idiot, right in front of you. Instead of this, we have him run up to you for a melee attack.
		return SCHED_ALIENCONTROLLER_MOVE_TO_MELEE;
	}

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//=========================================================
// FInViewCone - returns true is the passed vector is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
bool CNPC_AlienController::FInViewCone(Vector pOrigin)
{
	Vector los = (pOrigin - GetAbsOrigin());

	// do this in 2D
	los.z = 0;
	VectorNormalize(los);

	Vector facingDir = EyeDirection2D();

	float flDot = DotProduct(los, facingDir);

	if (flDot > m_flFieldOfView)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CNPC_AlienController::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
		//
		// This task enables us to build a path that requires flight.
		//
		//		case TASK_CROW_PREPARE_TO_FLY:
		//		{
		//			SetFlyingState( FlyState_Flying );
		//			TaskComplete();
		//			break;
		//		}
	case TASK_ALIENCONTROLLER_CHASE_ENEMY_CONTINUOUSLY:
		StartTaskChaseEnemyContinuously( pTask );
		break;

	case TASK_RANGE_ATTACK1:
	{
		AutoMovement();

		Vector vecEnemyLKP = GetEnemyLKP();
		if (!FInAimCone(vecEnemyLKP))
		{
			GetMotor()->SetIdealYawToTargetAndUpdate(vecEnemyLKP, AI_KEEP_YAW_SPEED);
		}
		else
		{
			GetMotor()->SetIdealYawAndUpdate(GetMotor()->GetIdealYaw(), AI_KEEP_YAW_SPEED);
		}
		if (GetEnemy())
		{
			ShriekSound();

			BaseClass::StartTask(pTask);
		}
		break;
	}
	case TASK_ALIENCONTROLLER_TAKEOFF:
	{
		if (random->RandomInt(1, 4) == 1)
		{
			AlertSound();
		}

		SetIdealActivity((Activity)ACT_ALIENCONTROLLER_TAKEOFF);
		break;
	}

	case TASK_ALIENCONTROLLER_PICK_EVADE_GOAL:
	{
		if (GetEnemy() != NULL)
		{
			//
			// Get our enemy's position in x/y.
			//
			Vector vecEnemyOrigin = GetEnemy()->GetAbsOrigin();
			vecEnemyOrigin.z = GetAbsOrigin().z;

			//
			// Pick a hop goal a random distance along a vector away from our enemy.
			//
			m_vSavePosition = GetAbsOrigin() - vecEnemyOrigin;
			VectorNormalize(m_vSavePosition);
			m_vSavePosition = GetAbsOrigin() + m_vSavePosition * (32 + random->RandomInt(0, 32));

			GetMotor()->SetIdealYawToTarget(m_vSavePosition);
			TaskComplete();
		}
		else
		{
			TaskFail("No enemy");
		}
		break;
	}

	case TASK_ALIENCONTROLLER_FALL_TO_GROUND:
	{
		SetFlyingState(FlyState_Falling);
		break;
	}

	case TASK_FIND_HINTNODE:
	{
		if (GetGoalEnt())
		{
			TaskComplete();
			return;
		}
		// Overloaded because we search over a greater distance.
		if (!GetHintNode())
		{
			SetHintNode(CAI_HintManager::FindHint(this, HINT_CROW_FLYTO_POINT, bits_HINT_NODE_NEAREST | bits_HINT_NODE_USE_GROUP, 10000));
		}

		if (GetHintNode())
		{
			TaskComplete();
		}
		else
		{
			TaskFail(FAIL_NO_HINT_NODE);
		}
		break;
	}

	case TASK_GET_PATH_TO_HINTNODE:
	{
		//How did this happen?!
		if (GetGoalEnt() == this)
		{
			SetGoalEnt(NULL);
		}

		if (GetGoalEnt())
		{
			SetFlyingState(FlyState_Flying);
			StartTargetHandling(GetGoalEnt());

			m_bReachedMoveGoal = false;
			TaskComplete();
			SetHintNode(NULL);
			return;
		}

		if (GetHintNode())
		{
			Vector vHintPos;
			GetHintNode()->GetPosition(this, &vHintPos);

			SetNavType(NAV_FLY);
			CapabilitiesAdd(bits_CAP_MOVE_FLY);
			// @HACKHACK: Force allow triangulation. Too many HL2 maps were relying on this feature WRT fly nodes (toml 8/1/2007)
			NPC_STATE state = GetState();
			m_NPCState = NPC_STATE_SCRIPT;
			bool bFoundPath = GetNavigator()->SetGoal(vHintPos);
			m_NPCState = state;
			if (!bFoundPath)
			{
				GetHintNode()->DisableForSeconds(.3);
				SetHintNode(NULL);
			}
		}

		if (GetHintNode())
		{
			m_bReachedMoveGoal = false;
			TaskComplete();
		}
		else
		{
			TaskFail(FAIL_NO_ROUTE);
		}
		break;
	}

	//
	// We have failed to fly normally. Pick a random "up" direction and fly that way.
	//
	case TASK_ALIENCONTROLLER_FLY:
	{
		float flYaw = UTIL_AngleMod(random->RandomInt(-180, 180));

		Vector vecNewVelocity(cos(DEG2RAD(flYaw)), sin(DEG2RAD(flYaw)), random->RandomFloat(0.1f, 0.5f));
		vecNewVelocity *= ALIENCONTROLLER_AIRSPEED;
		SetAbsVelocity(vecNewVelocity);

		SetIdealActivity(ACT_FLY);

		m_bFloat = false;
		m_flFloatTime = gpGlobals->curtime + random->RandomFloat(2, 5);

		break;
	}

	case TASK_ALIENCONTROLLER_PICK_RANDOM_GOAL:
	{
		m_vSavePosition = GetLocalOrigin() + Vector(random->RandomFloat(-48.0f, 48.0f), random->RandomFloat(-48.0f, 48.0f), 0);
		TaskComplete();
		break;
	}

	case TASK_ALIENCONTROLLER_FLOAT:
	{
		float flYaw = UTIL_AngleMod(random->RandomInt(-180, 180));

		Vector vecNewVelocity(cos(DEG2RAD(flYaw)), sin(DEG2RAD(flYaw)), random->RandomFloat(0.1f, 0.5f));
		vecNewVelocity *= ALIENCONTROLLER_AIRSPEED;
		SetAbsVelocity(vecNewVelocity);

		
		SetIdealActivity(ACT_HOVER);

		m_flFloatTime = gpGlobals->curtime + random->RandomFloat(2, 5);

		break;
	}

	case TASK_ALIENCONTROLLER_WAIT_FOR_BARNACLE_KILL:
	{
		break;
	}

	default:
	{
		BaseClass::StartTask(pTask);
	}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CNPC_AlienController::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_ALIENCONTROLLER_TAKEOFF:
	{
		if (GetNavigator()->IsGoalActive())
		{
			GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() + GetNavigator()->GetCurWaypointPos(), AI_KEEP_YAW_SPEED);
		}
		else
			TaskFail(FAIL_NO_ROUTE);

		if (IsActivityFinished())
		{
			TaskComplete();
			SetIdealActivity(ACT_FLY);

			m_bFloat = false;
			m_flFloatTime = gpGlobals->curtime + random->RandomFloat(2, 5);
		}

		break;
	}

	case TASK_ALIENCONTROLLER_FLOAT:
	{
		SetIdealActivity(ACT_HOVER);

		break;
	}

	//
	// Face the direction we are flying.
	//
	case TASK_ALIENCONTROLLER_FLY:
	{
		GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() + GetAbsVelocity(), AI_KEEP_YAW_SPEED);

		break;
	}

	case TASK_ALIENCONTROLLER_FALL_TO_GROUND:
	{
		if (GetFlags() & FL_ONGROUND)
		{
			SetFlyingState(FlyState_Floating);
			TaskComplete();
		}
		break;
	}

	case TASK_ALIENCONTROLLER_WAIT_FOR_BARNACLE_KILL:
	{
		if (m_flNextFlinchTime < gpGlobals->curtime)
		{
			m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat(0.5f, 2.0f);
			// dvs: TODO: squirm
			// dvs: TODO: spawn feathers
			EmitSound("NPC_Crow.Squawk");
		}
		break;
	}

	default:
	{
		CAI_BaseNPC::RunTask(pTask);
	}
	}
}


NPC_STATE CNPC_AlienController::SelectIdealState(void)
{
	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch (m_NPCState)
	{
	case NPC_STATE_COMBAT:
	{
		// COMBAT goes to ALERT upon death of enemy
		if (GetEnemy() != NULL && (HasCondition(COND_LIGHT_DAMAGE)))
		{
			SetEnemy(NULL);
			return NPC_STATE_ALERT;
		}
		break;
	}
	}

	return BaseClass::SelectIdealState();
}



AI_BEGIN_CUSTOM_NPC(npc_aliencontroller, CNPC_AlienController)

DECLARE_TASK(TASK_ALIENCONTROLLER_FIND_FLYTO_NODE)
//DECLARE_TASK( TASK_CROW_PREPARE_TO_FLY )
DECLARE_TASK(TASK_ALIENCONTROLLER_TAKEOFF)
DECLARE_TASK(TASK_ALIENCONTROLLER_FLY)
DECLARE_TASK(TASK_ALIENCONTROLLER_PICK_RANDOM_GOAL)
DECLARE_TASK(TASK_ALIENCONTROLLER_FLOAT)
DECLARE_TASK(TASK_ALIENCONTROLLER_PICK_EVADE_GOAL)
DECLARE_TASK(TASK_ALIENCONTROLLER_WAIT_FOR_BARNACLE_KILL)
DECLARE_TASK( TASK_ALIENCONTROLLER_CHASE_ENEMY_CONTINUOUSLY )

// experiment
DECLARE_TASK(TASK_ALIENCONTROLLER_FALL_TO_GROUND)

DECLARE_ACTIVITY(ACT_ALIENCONTROLLER_TAKEOFF)
DECLARE_ACTIVITY(ACT_ALIENCONTROLLER_FLOAT)
DECLARE_ACTIVITY(ACT_ALIENCONTROLLER_FLY)

DECLARE_ANIMEVENT(AE_ALIENCONTROLLER_FLOAT)
DECLARE_ANIMEVENT(AE_ALIENCONTROLLER_FLY)
DECLARE_ANIMEVENT(AE_ALIENCONTROLLER_TAKEOFF)


DECLARE_CONDITION(COND_ALIENCONTROLLER_BARNACLED)
DECLARE_CONDITION(COND_ALIENCONTROLLER_FORCED_FLY)

//=========================================================
DEFINE_SCHEDULE
(
SCHED_ALIENCONTROLLER_IDLE_FLOAT,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_ALIENCONTROLLER_FLOAT"
"		TASK_ALIENCONTROLLER_PICK_RANDOM_GOAL		0"
"		TASK_GET_PATH_TO_SAVEPOSITION	0"
"		TASK_WALK_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_WAIT_PVS					0"
"		"
"	Interrupts"
"		COND_ALIENCONTROLLER_FORCED_FLY"
"		COND_PROVOKED"
"		COND_NEW_ENEMY"
"		COND_HEAVY_DAMAGE"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
"		COND_HEAR_COMBAT"
)

//=========================================================
DEFINE_SCHEDULE
(
SCHED_ALIENCONTROLLER_FLOAT,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_ALIENCONTROLLER_IDLE_FLOAT"
"		TASK_STOP_MOVING				0"
"		TASK_WAIT_PVS					0"
"		TASK_ALIENCONTROLLER_TAKEOFF				0"
//"		SCHED_ALIENCONTROLLER_FLY					0"
 "		TASK_SET_SCHEDULE					SCHEDULE:SCHED_ALIENCONTROLLER_FLY"
"		"
"	Interrupts"
"		COND_ALIENCONTROLLER_FORCED_FLY"
"		COND_PROVOKED"
"		COND_NEW_ENEMY"
"		COND_HEAVY_DAMAGE"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
"		COND_HEAR_COMBAT"
)

//=========================================================
DEFINE_SCHEDULE
(
SCHED_ALIENCONTROLLER_FLY,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_ALIENCONTROLLER_FLY_FAIL"
"		TASK_STOP_MOVING				0"
"		TASK_ALIENCONTROLLER_TAKEOFF				0"
//"		SCHED_ALIENCONTROLLER_FLY					0"
"	"
"	Interrupts"
"		COND_PROVOKED"
"		COND_NEW_ENEMY"
"		COND_HEAVY_DAMAGE"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
"		COND_HEAR_COMBAT"
)

//=========================================================
DEFINE_SCHEDULE
(
SCHED_ALIENCONTROLLER_FLY_FAIL,

"	Tasks"
"		TASK_ALIENCONTROLLER_FALL_TO_GROUND		0"
"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_ALIENCONTROLLER_IDLE_FLOAT"
"	"
"	Interrupts"
)

//=========================================================
// Controller is in the clutches of a barnacle
DEFINE_SCHEDULE
(
SCHED_ALIENCONTROLLER_BARNACLED,

"	Tasks"
"		TASK_STOP_MOVING						0"
"		TASK_SET_ACTIVITY						ACTIVITY:ACT_HOVER"
"		TASK_ALIENCONTROLLER_WAIT_FOR_BARNACLE_KILL		0"

"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_ALIENCONTROLLER_RANGE_ATTACK1,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_FACE_ENEMY					0"
"		TASK_ANNOUNCE_ATTACK			1"	// 1 = primary attack
"		TASK_WAIT_RANDOM				0.3"
"		TASK_RANGE_ATTACK1				0"
""
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_HEAVY_DAMAGE"
"		COND_LIGHT_DAMAGE"
"		COND_LOW_PRIMARY_AMMO"
"		COND_NO_PRIMARY_AMMO"
"		COND_WEAPON_BLOCKED_BY_FRIEND"
"		COND_TOO_CLOSE_TO_ATTACK"
"		COND_GIVE_WAY"
"		COND_HEAR_DANGER"
"		COND_HEAR_MOVE_AWAY"
//"		COND_COMBINE_NO_FIRE"
""
// Enemy_Occluded				Don't interrupt on this.  Means
//								comibine will fire where player was after
//								he has moved for a little while.  Good effect!!
// WEAPON_SIGHT_OCCLUDED		Don't block on this! Looks better for railings, etc.
)

DEFINE_SCHEDULE 
(
SCHED_ALIENCONTROLLER_ESTABLISH_LINE_OF_FIRE,

"	Tasks "
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_FAIL_ESTABLISH_LINE_OF_FIRE"
"		TASK_SET_TOLERANCE_DISTANCE		48"
"		TASK_GET_PATH_TO_ENEMY_LKP_LOS	0"
//"		TASK_SPEAK_SENTENCE				1"
"		TASK_RUN_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
"	"
"	Interrupts "
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
//"		COND_CAN_RANGE_ATTACK1"
//"		COND_CAN_RANGE_ATTACK2"
"		COND_CAN_MELEE_ATTACK1"
"		COND_CAN_MELEE_ATTACK2"
"		COND_HEAR_DANGER"
"		COND_HEAR_MOVE_AWAY"
"		COND_HEAVY_DAMAGE"
)

AI_END_CUSTOM_NPC()
