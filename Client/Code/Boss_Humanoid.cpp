#include "pch.h"
#include "../Header/Boss_Humanoid.h"
#include "Export_System.h"
#include "Export_Utility.h"

#include "../Header/UIMisterBullet.h"


CBoss_Humanoid::CBoss_Humanoid(LPDIRECT3DDEVICE9 _pGraphicDev)
	: CMonster(_pGraphicDev)
	, m_eCurState(CBoss_Humanoid::BOSS_SPAWN)
	, m_ePreState(CBoss_Humanoid::BOSS_SPAWN)
	, m_bIsDamaged(false)
	, m_fSpawnTimer(2.5f)
	, m_iBossKillCount(0)
{
	for (_int i = 0; i < CBoss_Humanoid::BOSS_END; ++i)
		m_pTextureCom[i] = nullptr;
}

CBoss_Humanoid::CBoss_Humanoid(LPDIRECT3DDEVICE9 _pGraphicDev, _vec3 _vecPos)
	: CMonster(_pGraphicDev)
	, m_eCurState(CBoss_Humanoid::BOSS_SPAWN)
	, m_ePreState(CBoss_Humanoid::BOSS_SPAWN)
	, m_bIsDamaged(false)
	, m_fSpawnTimer(2.5f)
	, m_iBossKillCount(0)
{
	for (_int i = 0; i < CBoss_Humanoid::BOSS_END; ++i)
		m_pTextureCom[i] = nullptr;

	m_vStartPos = _vecPos;
}

CBoss_Humanoid::~CBoss_Humanoid()
{
}

CBoss_Humanoid* CBoss_Humanoid::Create(LPDIRECT3DDEVICE9 _pGraphicDev)
{
	CBoss_Humanoid* pGameObject = new CBoss_Humanoid(_pGraphicDev);

	if (FAILED(pGameObject->Ready_GameObject()))
	{
		Safe_Release(pGameObject);
		MSG_BOX("Boss_Humanoid Create Failed");
		return nullptr;
	}

	return pGameObject;
}

CBoss_Humanoid* CBoss_Humanoid::Create(LPDIRECT3DDEVICE9 _pGraphicDev, _vec3 _vecPos)
{
	CBoss_Humanoid* pGameObject = new CBoss_Humanoid(_pGraphicDev, _vecPos);

	if (FAILED(pGameObject->Ready_GameObject()))
	{
		Safe_Release(pGameObject);
		MSG_BOX("Boss_Humanoid Create Failed");
		return nullptr;
	}

	return pGameObject;
}

HRESULT CBoss_Humanoid::Ready_GameObject()
{
	FAILED_CHECK_RETURN(Add_Component(), E_FAIL);

	m_pTransformCom->Set_Pos(2.f, 0.f, 5.f);

	_vec3 vDir = { 0.5f, 0.5f, 0.5f };

	m_pColliderCom->SetTransform(m_pTransformCom);
	m_pColliderCom->SetRadius(1.f);
	m_pColliderCom->SetLookDir(vDir);
	m_pColliderCom->SetShow(true);
	m_pColliderCom->SetActive(true);

	m_pHitBufferCom->SetvOffSet({ 0.f,0.f,0.f });
	m_pHeadHit->SetvOffSet({ 0.5f,0.5f,0.f });
	m_pCriticalHit->SetvOffSet({ -0.5f,0.5f,0.f });

	Set_Animation();

	return S_OK;
}

_int CBoss_Humanoid::Update_GameObject(const _float& _fTimeDelta)
{
	Attack(_fTimeDelta);

	_int iExit = Engine::CGameObject::Update_GameObject(_fTimeDelta);

	_matrix		matWorld, matView, matBill, matResult;
	m_pTransformCom->Get_WorldMatrix(&matWorld);

	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);

	D3DXMatrixIdentity(&matBill);

	matBill._11 = matView._11;
	matBill._13 = matView._13;
	matBill._31 = matView._31;
	matBill._33 = matView._33;

	D3DXMatrixInverse(&matBill, 0, &matBill);

	matResult = matBill * matWorld;

	m_pTransformCom->Set_WorldMatrix(&(matResult));

	Add_RenderGroup(RENDERID::RENDER_ALPHA, this);
	Engine::Add_Collider(m_pColliderCom);

	return iExit;
}

void CBoss_Humanoid::LateUpdate_GameObject()
{
	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO::INFO_POS, &vPos);

	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

	CGameObject::Compute_ViewZ(&vPos);

	Change_State(); 

	State_Check();

	Engine::CGameObject::LateUpdate_GameObject();
}

void CBoss_Humanoid::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_WorldMatrix());
	//Jonghan Monster Change Start

	//m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	m_pAnimatorCom->Render_Animator();
	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	//m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void CBoss_Humanoid::Damaged_By_Player(const DAMAGED_STATE& _eDamagedState, const _float& _fAttackDamage)
{
	Changing_State(CBoss_Humanoid::BOSS_DAMAGED);
	CComponent* pComponent = Engine::Get_Component(COMPONENTID::ID_DYNAMIC, L"Layer_Effect", L"EffectExecutionBlood", L"Com_EffectSecond");
	if (pComponent)
		static_cast<CEffect*>(pComponent)->Operate_Effect();

	static_cast<CUIMisterBullet*>(Engine::Get_ListUI(UITYPE::UI_MISTERBULLET)->front())->Add_Count();
	++m_iBossKillCount;

	if (4 < m_iBossKillCount)
	{
		// Need to Interaction with SceneChange to Go Boss_Robot
	}
}

void CBoss_Humanoid::Change_State()
{
	if (m_pAnimatorCom->GetCurrAnim()->GetFinish())
	{
		switch (m_eCurState)
		{
		case CBoss_Humanoid::BOSS_SPAWN:
			m_eCurState = CBoss_Humanoid::BOSS_ATTACK;
			break;
		case CBoss_Humanoid::BOSS_ATTACK:
			m_eCurState = CBoss_Humanoid::BOSS_SPAWN;
			break;
		}
		m_pAnimatorCom->GetCurrAnim()->SetFinish(false);

	}
}

_vec3 CBoss_Humanoid::Calculate_NextPos(const _float& _fMinX, const _float& _fMaxX, const _float& _fMinY, const _float& _fMaxY)
{
	_float x, y, z(25.f);
	x = _float(_int(rand()) % _int(_fMaxX - _fMinX)) + _fMinX;
	y = _float(_int(rand()) % _int(_fMaxY - _fMinY)) + _fMinY;

	_vec3 vPos = { x,y,z }; // we need Position.x & Position.y, Need Not Position.z

	return vPos;
}

void CBoss_Humanoid::Damaged_ReAction()
{
	m_pAnimatorCom->Toggle_Pause();
	m_bIsDamaged = true;
	m_iBossKillCount++;
	if (5 <= m_iBossKillCount)
	{
		//Change Scene or Do Something to change Boss_Robot
	}
}

HRESULT CBoss_Humanoid::Add_Component()
{
	CComponent* pComponent = NULL;

	pComponent = m_pHitBufferCom = dynamic_cast<CRcCol*>(Engine::Clone_Proto(L"Proto_HitBufferCom"));
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_STATIC].insert({ L"Com_HitBufferCom", pComponent });
	pComponent->SetOwner(*this);

	pComponent = m_pHeadHit = dynamic_cast<CRcCol*>(Engine::Clone_Proto(L"Proto_HitBufferCom"));
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_STATIC].insert({ L"Com_HeadHit", pComponent });
	pComponent->SetOwner(*this);

	pComponent = m_pCriticalHit = dynamic_cast<CRcCol*>(Engine::Clone_Proto(L"Proto_HitBufferCom"));
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_STATIC].insert({ L"Com_CriticalHit", pComponent });
	pComponent->SetOwner(*this);

	pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(Engine::Clone_Proto(L"Proto_RcTex"));
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_STATIC].insert({ L"Com_Buffer", pComponent });

	pComponent = m_pTextureCom[CBoss_Humanoid::BOSS_SPAWN] = dynamic_cast<CTexture*>(Engine::Clone_Proto(L"Proto_BossHumanoidSpawnTex")); //Drone coming with AttackMotion
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_STATIC].insert({ L"Com_SpawnTexture", pComponent });

	pComponent = m_pTextureCom[CBoss_Humanoid::BOSS_ATTACK] = dynamic_cast<CTexture*>(Engine::Clone_Proto(L"Proto_BossHumanoidAttackTex"));
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_STATIC].insert({ L"Com_AttackTexture", pComponent });

	pComponent = m_pCalculatorCom = dynamic_cast<CCalculator*>(Engine::Clone_Proto(L"Proto_Calculator"));
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_DYNAMIC].insert({ L"Com_Calculator", pComponent });

	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(Engine::Clone_Proto(L"Proto_Transform"));
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	pComponent = m_pColliderCom = dynamic_cast<CCollider*>(Engine::Clone_Proto(L"Proto_Collider"));
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_DYNAMIC].insert({ L"Com_Collider", pComponent });
	pComponent->SetOwner(*this);

	pComponent = m_pAnimatorCom = dynamic_cast<CAnimator*>(Engine::Clone_Proto(L"Proto_Animator"));
	NULL_CHECK_RETURN(pComponent, E_FAIL);
	m_mapComponent[(_uint)COMPONENTID::ID_DYNAMIC].insert({ L"Com_Animator", pComponent });

	return S_OK;
}

void CBoss_Humanoid::State_Check()
{
	if (m_eCurState != m_ePreState)
	{
		_vec3 vPos;
		switch (m_eCurState)
		{
		case CBoss_Humanoid::BOSS_SPAWN:
			vPos = Calculate_NextPos(10.f, 20.f, 10.f, 20.f); // We need to Know WindowsTexture X Y Z
			m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
			m_pAnimatorCom->PlayAnimation(L"Spawn", false);
			break;
		case CBoss_Humanoid::BOSS_ATTACK:
			m_pAnimatorCom->PlayAnimation(L"Attack", false);
			break;
		case CBoss_Humanoid::BOSS_DAMAGED:
			Damaged_ReAction();
			break;
		}

		m_ePreState = m_eCurState;
	}
}

void CBoss_Humanoid::Set_Animation()
{
	m_pAnimatorCom->CreateAnimation(L"Spawn", m_pTextureCom[BOSS_SPAWN], 13.f);
	m_pAnimatorCom->CreateAnimation(L"Attack", m_pTextureCom[BOSS_ATTACK], 9.f);

	m_pAnimatorCom->PlayAnimation(L"Spawn", false);
}

void CBoss_Humanoid::Attack(const _float& _fTimeDelta)
{
	if (Engine::Key_Press(DIK_SPACE)) {
		Damaged_By_Player();
	}
	if (CBoss_Humanoid::BOSS_DAMAGED == m_eCurState)
	{
		if (0.f >= m_fSpawnTimer)
		{
			m_bIsDamaged = false;
			m_pAnimatorCom->Toggle_Pause();
			m_eCurState = CBoss_Humanoid::BOSS_SPAWN;
			m_fSpawnTimer = 2.5f;
		}
		else
			m_fSpawnTimer -= _fTimeDelta;
	}
	else if (CBoss_Humanoid::BOSS_ATTACK == m_eCurState)
	{
		_vec3 vPos, vPlayerPos, vDir;
		m_pTransformCom->Get_Info(INFO::INFO_POS, &vPos);
		if (nullptr == m_pPlayerTransformCom)
		{
			m_pPlayerTransformCom = dynamic_cast<Engine::CTransform*>
				(Engine::Get_Component(COMPONENTID::ID_DYNAMIC, L"Layer_Player", L"Player", L"Com_Body_Transform"));
			NULL_CHECK(m_pPlayerTransformCom, -1);
		}

		m_pPlayerTransformCom->Get_Info(INFO::INFO_POS, &vPlayerPos);

		vDir = vPlayerPos - vPos;
	}
}

void CBoss_Humanoid::Free()
{
	CMonster::Free();
}
