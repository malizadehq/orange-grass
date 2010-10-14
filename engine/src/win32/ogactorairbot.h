/*
 *  OGActorAirbot.h
 *  OrangeGrass
 *
 *  Created by Viacheslav Bogdanov on 11.11.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef OGACTORAIRBOT_H_
#define OGACTORAIRBOT_H_

#include "ogactorbot.h"


class COGActorAirBot : public COGActorBot
{
public:
	COGActorAirBot();
	virtual ~COGActorAirBot();

	// Create actor.
	virtual bool Create (
		IOGActorParams* _pParams,
		const Vec3& _vPos,
		const Vec3& _vRot,
        const Vec3& _vScale);

	// Adding to actor manager event handler.
	virtual void OnAddedToManager ();

	// Update actor.
	virtual void Update (unsigned long _ElapsedTime);

protected:

    IOGEffect*  m_pExplosionEffect;
    IOGSgNode*  m_pExplosionNode;
};


#endif
