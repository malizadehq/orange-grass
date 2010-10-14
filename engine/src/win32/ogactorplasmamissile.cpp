/*
 *  ogactorplasmamissile.cpp
 *  OrangeGrass
 *
 *  Created by Viacheslav Bogdanov on 12.11.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "OrangeGrass.h"
#include "ogactorplasmamissile.h"
#include "IOGMath.h"


COGActorPlasmaMissile::COGActorPlasmaMissile()
{
    m_pHeadEffect = NULL;
    m_pOwner = NULL;
}


COGActorPlasmaMissile::~COGActorPlasmaMissile()
{
	if (m_pNode)
	{
        if (m_bAdded)
		    GetSceneGraph()->RemoveNode(m_pNode);
        else
            OG_SAFE_DELETE(m_pNode);
		m_pNode = NULL;
	}
    if (m_pPhysicalObject)
    {
        if (m_bAdded)
            GetPhysics()->RemoveObject(m_pPhysicalObject);
        else
            OG_SAFE_DELETE(m_pPhysicalObject);
        m_pPhysicalObject = NULL;
    }
}


// Create actor.
bool COGActorPlasmaMissile::Create (IOGActorParams* _pParams,
                                    const Vec3& _vPos,
                                    const Vec3& _vRot,
                                    const Vec3& _vScale)
{
	m_pParams = _pParams;

	if (m_pParams->type == OG_ACTOR_NONE)
	{
		OG_LOG_ERROR("Creating COGActorPlasmaMissile from model %s failed, actor type is OG_ACTOR_NONE", _pParams->model_alias.c_str());
		return false;
	}

	m_pHeadEffect = GetEffectsManager()->CreateEffect(OG_EFFECT_PLASMA);
	if (!m_pHeadEffect)
	{
		OG_LOG_ERROR("Creating COGActorPlasmaMissile failed, cannot get effect %s", _pParams->model_alias.c_str());
		return false;
	}
	
    m_pPhysicalObject = GetPhysics()->CreateObject(&m_pParams->physics, m_pHeadEffect->GetAABB(), this);
    if (!m_pPhysicalObject)
	{
		OG_LOG_ERROR("Creating COGActorPlasmaMissile failed, cannot create physical object");
        return false;
	}
	m_pPhysicalObject->SetWorldTransform(_vPos, _vRot, _vScale);

	m_pNode = GetSceneGraph()->CreateNode(m_pHeadEffect, m_pPhysicalObject);
	if (!m_pNode)
	{
		OG_LOG_ERROR("Creating COGActorPlasmaMissile failed, cannot create SG node");
		return false;
	}

	m_FlightWorker.Create(this);
    Activate(false);

	return true;
}


// Adding to actor manager event handler.
void COGActorPlasmaMissile::OnAddedToManager ()
{
    GetPhysics()->AddObject(m_pPhysicalObject);
    GetSceneGraph()->AddEffectNode(m_pNode);
    m_bAdded = true;
}


// Update actor.
void COGActorPlasmaMissile::Update (unsigned long _ElapsedTime)
{
	if (m_FlightWorker.IsActive())
	{
		m_FlightWorker.Update(_ElapsedTime);
		if (m_FlightWorker.IsFinished())
		{
			Activate(false);
		}
	}
}


// Set active state
void COGActorPlasmaMissile::Activate (bool _bActive)
{
	m_bActive = _bActive;

	m_FlightWorker.Reset();
	m_FlightWorker.Activate(m_bActive);
	if (m_bActive)
	{
		m_FlightWorker.Reset();
		m_FlightWorker.Activate(true);
		m_pHeadEffect->Start();
		m_pHeadEffect->SetDirection(Vec3(0,0,1));
	}
	else
	{
		m_FlightWorker.Activate(false);
		m_pHeadEffect->Stop();
	}
}


// Set owner.
void COGActorPlasmaMissile::SetOwner (IOGActor* _pOwner, const Vec3& _vLaunchOffset)
{
    m_pOwner = _pOwner;
    m_vLaunchOffset = _vLaunchOffset;
}


// Fire to target.
void COGActorPlasmaMissile::Fire (const Vec3& _vTarget)
{
    Vec3 vPos = m_pOwner->GetPhysicalObject()->GetPosition() + m_vLaunchOffset;
    Vec3 vRot = m_pOwner->GetPhysicalObject()->GetRotation();
	m_pPhysicalObject->SetWorldTransform(vPos, vRot, Vec3(1,1,1));
    Activate(true);
}
