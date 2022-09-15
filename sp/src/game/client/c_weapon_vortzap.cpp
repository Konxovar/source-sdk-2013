#include "cbase.h"
#include "beamdraw.h"
#include "c_weapon__stubs.h"
#include "clienteffectprecachesystem.h"
#include "tier0/memdbgon.h"

CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectVortzap)
CLIENTEFFECT_MATERIAL("sprites/vortzap_player")
CLIENTEFFECT_REGISTER_END()

#define HAND_LEFT	0
#define HAND_RIGHT	1


static const char *PLAYER_LEFT_CLAW = "leftclaw";
static const char *PLAYER_RIGHT_CLAW = "rightclaw";

class C_VortzapBeam : public CDefaultClientRenderable
{
public:
	C_VortzapBeam();

	void Update(C_BaseEntity *pOwner);

	matrix3x4_t z;
	const matrix3x4_t& RenderableToWorldTransform() { return z; }

	virtual const Vector& GetRenderOrigin(void) { return m_worldPosition; }
	virtual const QAngle& GetRenderAngles(void) { return vec3_angle; }
	virtual bool ShouldDraw(void) { return true; }
	virtual bool IsTransparent(void) { return true; }
	virtual bool ShouldReceiveProjectedTextures(int flags) { return false; }
	virtual int DrawModel(int flags);
	virtual void GetRenderBounds(Vector& mins, Vector& maxs)
	{
		mins.Init(-32, -32, -32);
		maxs.Init(32, 32, 32);
	}

	C_BaseEntity *m_pOwner;
	Vector m_targetPosition;
	Vector m_worldPosition;
	int m_active;
	int m_viewModelIndex;


	int				m_iLeftHandAttachment;
	int				m_iRightHandAttachment;

};

class C_WeaponVortzap : public C_BaseCombatWeapon
{
	DECLARE_CLASS(C_WeaponVortzap, C_BaseCombatWeapon)
public:
	C_WeaponVortzap() {}
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	
	void OnDataChanged(DataUpdateType_t updateType)
	{
		BaseClass::OnDataChanged(updateType);
		m_beam.Update(this);
	}
private:
	C_WeaponVortzap(const C_WeaponVortzap&);
	C_VortzapBeam m_beam;
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_vortzap, C_WeaponVortzap);
IMPLEMENT_CLIENTCLASS_DT(C_WeaponVortzap, DT_WeaponVortzap, CWeaponVortzap)
RecvPropVector(RECVINFO_NAME(m_beam.m_targetPosition, m_targetPosition)),
RecvPropVector(RECVINFO_NAME(m_beam.m_worldPosition, m_worldPosition)),
RecvPropInt(RECVINFO_NAME(m_beam.m_active, m_active)),
RecvPropInt(RECVINFO_NAME(m_beam.m_viewModelIndex, m_viewModelIndex)),
END_RECV_TABLE()

C_VortzapBeam::C_VortzapBeam()
{
	m_pOwner = NULL;
	m_hRenderHandle = INVALID_CLIENT_RENDER_HANDLE;
	m_iLeftHandAttachment = LookupAttachment(PLAYER_LEFT_CLAW);
	m_iRightHandAttachment = LookupAttachment(PLAYER_RIGHT_CLAW);
}

void C_VortzapBeam::Update(C_BaseEntity *pOwner)
{
	m_pOwner = pOwner;
	if (m_active)
	{
		if (m_hRenderHandle == INVALID_CLIENT_RENDER_HANDLE)
			ClientLeafSystem()->AddRenderable(this, RENDER_GROUP_TRANSLUCENT_ENTITY);
		else
			ClientLeafSystem()->RemoveRenderable(m_hRenderHandle);
	}
	else if (!m_active && m_hRenderHandle != INVALID_CLIENT_RENDER_HANDLE)
	{
		ClientLeafSystem()->RemoveRenderable(m_hRenderHandle);
		m_hRenderHandle = INVALID_CLIENT_RENDER_HANDLE;
	}
}

int C_VortzapBeam::DrawModel(int flags)
{
	Vector points[3];
	QAngle tmpAngle;
	if (!m_active)
		return 0;

	C_BaseEntity *pEnt = cl_entitylist->GetEnt(m_viewModelIndex);
	if (!pEnt)
		return 0;

	Vector vecHandPos;
	QAngle vecHandAngle;
	pEnt->GetAttachment(1, points[0], vecHandAngle);

	points[1] = 0.5 * (m_targetPosition + points[0]);
	points[1].z += 4 * sin(gpGlobals->curtime * 11) + 5 * cos(gpGlobals->curtime * 13);

	points[2] = m_worldPosition;

	IMaterial *pMat = materials->FindMaterial("sprites/vortzap_player", TEXTURE_GROUP_CLIENT_EFFECTS);
	Vector color;
	color.Init(1, 1, 1);

	float scrollOffset = gpGlobals->curtime - (int)gpGlobals->curtime;

	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(pMat);

	DrawBeamQuadratic(points[0], points[1], points[2], 13, color, scrollOffset, 1);

	return 1;
}