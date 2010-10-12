/*
 *  ogactormanager.mm
 *  OrangeGrass
 *
 *  Created by Viacheslav Bogdanov on 12.11.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "OrangeGrass.h"
#include "ogactormanager.h"
#include "ogactorstatic.h"
#include "ogactorlandbot.h"
#include "ogactorairbot.h"
#include "ogactorplayer.h"
#include "ogactorplasmamissile.h"
#include "ogactormissile.h"
#include "IOGMath.h"
#include <algorithm>


COGActorManager::COGActorManager ()	: m_pPlayersActor(NULL)
{
}


COGActorManager::~COGActorManager ()
{
	Clear();
}


// Clear actors manager
void COGActorManager::Clear ()
{
	m_pPlayersActor = NULL;
    std::list<IOGActor*>::iterator iter = m_ActorsList.begin();
    for (; iter != m_ActorsList.end(); ++iter)
    {
		OG_SAFE_DELETE((*iter));
	}
	m_ActorsList.clear();
}


// Create actor
IOGActor* COGActorManager::CreateActor (
	const std::string& _Alias,
	const Vec3& _vPos,
	const Vec3& _vRot,
    const Vec3& _vScale)
{
	IOGActorParams* pParams = GetActorParamsMgr()->GetParams(_Alias);
	if (!pParams)
	{
		OG_LOG_ERROR("Trying to create actor of unknown alias %s", _Alias.c_str());
		return NULL;
	}

	switch (pParams->type)
	{
	case OG_ACTOR_STATIC:
		{
			COGActorStatic* pActor = new COGActorStatic();
			if (pActor->Create(pParams, _vPos, _vRot, _vScale) == false)
			{
				OG_SAFE_DELETE(pActor);
				return NULL;
			}
			return pActor;
		}
		break;

	case OG_ACTOR_LANDBOT:
		{
			COGActorLandBot* pActor = new COGActorLandBot();
			if (pActor->Create(pParams, _vPos, _vRot, _vScale) == false)
			{
				OG_SAFE_DELETE(pActor);
				return NULL;
			}
			return pActor;
		}
		break;

	case OG_ACTOR_AIRBOT:
		{
			COGActorAirBot* pActor = new COGActorAirBot();
			if (pActor->Create(pParams, _vPos, _vRot, _vScale) == false)
			{
				OG_SAFE_DELETE(pActor);
				return NULL;
			}
			return pActor;
		}
		break;

	case OG_ACTOR_PLAYER:
		{
			COGActorPlayer* pActor = new COGActorPlayer();
			if (pActor->Create(pParams, _vPos, _vRot, _vScale) == false)
			{
				OG_SAFE_DELETE(pActor);
				return NULL;
			}
			return pActor;
		}
		break;

	case OG_ACTOR_PLASMAMISSILE:
		{
			COGActorPlasmaMissile* pActor = new COGActorPlasmaMissile();
			if (pActor->Create(pParams, _vPos, _vRot, _vScale) == false)
			{
				OG_SAFE_DELETE(pActor);
				return NULL;
			}
			return pActor;
		}
		break;

	case OG_ACTOR_MISSILE:
		{
			COGActorMissile* pActor = new COGActorMissile();
			if (pActor->Create(pParams, _vPos, _vRot, _vScale) == false)
			{
				OG_SAFE_DELETE(pActor);
				return NULL;
			}
			return pActor;
		}
		break;

	case OG_ACTOR_NONE:
	default:
		OG_LOG_ERROR("Trying to create actor of unknown type %d from model %s", pParams->type, _Alias.c_str());
		break;
	}

	return NULL;
}


// Add actor to the list.
void COGActorManager::AddActor (IOGActor* _pActor)
{
	if (_pActor->GetType() == OG_ACTOR_PLAYER)
	{
		m_pPlayersActor = _pActor;
	}

    _pActor->OnAddedToManager();
	m_ActorsList.push_back(_pActor);
}


// Destroy actor and remove from list.
void COGActorManager::DestroyActor (IOGActor* _pActor)
{
	if (_pActor == m_pPlayersActor)
	{
		m_pPlayersActor = NULL;
	}

	std::list<IOGActor*>::iterator iter = std::find(m_ActorsList.begin(), m_ActorsList.end(), _pActor);
	if (iter != m_ActorsList.end())
	{
		OG_SAFE_DELETE((*iter));
		m_ActorsList.erase(iter);
	}
}


// Update actors.
void COGActorManager::Update (unsigned long _ElapsedTime)
{
    std::list<IOGActor*>::iterator iter = m_ActorsList.begin();
    for (; iter != m_ActorsList.end(); ++iter)
    {
		(*iter)->Update(_ElapsedTime);
    }
}


// Get nearest intersected actor.
IOGActor* COGActorManager::GetNearestIntersectedActor (
    const Vec3& _RayStart, 
    const Vec3& _RayDir )
{
    std::list<IOGActor*> IntersectedList;
    std::list<IOGActor*>::iterator iter = m_ActorsList.begin();
    for (; iter != m_ActorsList.end(); ++iter)
    {
        if ((*iter)->CheckIntersection(_RayStart, _RayDir))
            IntersectedList.push_back(*iter);
    }
    if (IntersectedList.empty())
        return NULL;

    if (IntersectedList.size() == 1)
        return *IntersectedList.begin();

    std::list<IOGActor*>::iterator inters_iter = IntersectedList.begin();
    IOGActor* pNearest = *inters_iter;
    float fDist = Dist3D(_RayStart, pNearest->GetSgNode()->GetOBB().m_vCenter);
    ++inters_iter;
    for (; inters_iter != IntersectedList.end(); ++inters_iter)
    {
        IOGActor* pCur = *inters_iter;
        float curDist = Dist3D(_RayStart, pCur->GetSgNode()->GetOBB().m_vCenter);
        if (curDist < fDist)
        {
            fDist = curDist;
            pNearest = pCur;
        }
    }
    return pNearest;
}


// Get actors list.
const std::list<IOGActor*>& COGActorManager::GetActorsList () const
{
    return m_ActorsList;
}


// Get player's actor.
IOGActor* COGActorManager::GetPlayersActor ()
{
	return m_pPlayersActor;
}
