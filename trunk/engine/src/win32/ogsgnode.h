/*
 *  ogsgnode.h
 *  OrangeGrass
 *
 *  Created by Viacheslav Bogdanov on 12.11.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef OGSGNODE_H_
#define OGSGNODE_H_

#include "Mathematics.h"
#include "IOGSgNode.h"
#include "IOGAnimationController.h"


class COGSgNode : public IOGSgNode
{
	COGSgNode ();

public:
	COGSgNode (IOGRenderable* _pRenderable, IOGPhysicalObject* _pPhysics);
	virtual ~COGSgNode ();

	// get world transform.
	virtual const MATRIX& GetWorldTransform () const;

	// Get OBB
	virtual const IOGObb& GetOBB () const;

	// render.
	virtual void Render ();

	// update transform.
	virtual void Update (unsigned long _ElapsedTime);

	// get renderable.
	virtual IOGRenderable* GetRenderable ();

	// get physics.
	virtual IOGPhysicalObject* GetPhysics ();

	// get animation controller.
    virtual IOGAnimationController* GetAnimator () { return m_pAnimator;}

	// start animation.
    virtual void StartAnimation (const std::string& _Alias);

	// Get active state
	virtual bool IsActive () const { return m_bActive; }

	// Set active state
	virtual void Activate (bool _bActive);

protected:

    bool					m_bActive;
    IOGRenderable*          m_pRenderable;
    IOGPhysicalObject*      m_pPhysics;
    IOGAnimationController* m_pAnimator;
    unsigned int            m_AnimFrame;
};

#endif
