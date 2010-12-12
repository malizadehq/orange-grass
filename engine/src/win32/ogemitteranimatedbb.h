/*
 *  OGEmitterAnimatedBB.h
 *  OrangeGrass
 *
 *  Created by Viacheslav Bogdanov on 11.11.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef OGEMITTERANIMATEDBB_H_
#define OGEMITTERANIMATEDBB_H_

#include "ogemitter.h"
#include <vector>


class COGEmitterAnimatedBB : public COGEmitter
{
public:
	COGEmitterAnimatedBB();
	virtual ~COGEmitterAnimatedBB();

	// Initialize emitter.
	virtual void Init(IOGGroupNode* _pNode);

	// Update.
	virtual void Update (unsigned long _ElapsedTime);

	// Render.
	virtual void Render (const MATRIX& _mWorld, const Vec3& _vLook, const Vec3& _vUp, const Vec3& _vRight);

	// Start.
	virtual void Start ();

	// Stop.
	virtual void Stop ();

	// Get effect type.
    virtual OGEmitterType GetType() const { return s_Type; }

	// Get effect type.
    virtual const std::string& GetAlias() const { return s_Alias; }

protected:

	struct ParticleFormat
	{
        float   frame;
	    float	scale;
        float   angle;
	    Vec3	offset;
	    BBVert	pVertices[4];
	};

protected:

    std::vector<IOGMapping*>    m_Frames;
    ParticleFormat				m_BB;

	std::string     m_Texture;
	unsigned int	m_MappingStartId;
	unsigned int	m_MappingFinishId;
	Vec4			m_color;
	float			m_fFrameInc;
	float			m_fInitialScale;
	float			m_fScaleInc;
	float			m_fRotateInc;
	float			m_fInitialAngleMin;
	float			m_fInitialAngleMax;

public:

    static std::string     s_Alias;
    static OGEmitterType   s_Type;
};


#endif