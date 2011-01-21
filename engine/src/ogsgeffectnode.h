/*
 *  ogsgeffectnode.h
 *  OrangeGrass
 *
 *  Created by Viacheslav Bogdanov on 12.11.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef OGSGEFFECTNODE_H_
#define OGSGEFFECTNODE_H_

#include "Mathematics.h"
#include "IOGSgNode.h"
#include "IOGAnimationController.h"


class COGSgEffectNode : public IOGSgNode
{
	COGSgEffectNode ();

public:
	COGSgEffectNode (IOGEffect* _pRenderable, IOGPhysicalObject* _pPhysics);
	virtual ~COGSgEffectNode ();

	// get world transform.
	virtual const MATRIX& GetWorldTransform () const;

	// Get OBB
	virtual const IOGObb& GetOBB () const;

	// render.
	virtual void Render ();

	// update transform.
	virtual void Update (unsigned long _ElapsedTime);

	// get physics.
	virtual IOGPhysicalObject* GetPhysics ();

	// Get type of the renderable.
    virtual RenderableType GetRenderableType () const {return OG_RENDERABLE_EFFECT;}

	// get animation controller.
    virtual IOGAnimationController* GetAnimator () { return NULL; }

	// start animation.
    virtual void StartAnimation (const std::string& _Alias);

	// Get active point
    virtual bool GetActivePoint (Vec3& _point, const std::string& _Alias);

	// Get active state
	virtual bool IsActive () const { return m_bActive; }

	// Set active state
	virtual void Activate (bool _bActive);

	// Set billboard basis vectors.
	void SetBillboardVectors (const Vec3& _vLook, const Vec3& _vUp, const Vec3& _vRight);

protected:

    IOGEffect*              m_pRenderable;
    IOGPhysicalObject*      m_pPhysics;
    bool					m_bActive;
};

#endif
