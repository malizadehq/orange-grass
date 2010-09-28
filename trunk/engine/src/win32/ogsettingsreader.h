/*
 *  OGSettingsReader.h
 *  OrangeGrass
 *
 *  Created by Viacheslav Bogdanov on 11.11.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef OGSETTINGSREADER_H_
#define OGSETTINGSREADER_H_

#include "IOGSettingsReader.h"
#include "tinyxml.h"


struct COGSettingsSource : public IOGSettingsSource
{
	TiXmlDocument* pSource;
	TiXmlHandle* pDoc;
};


struct COGGroupNode : public IOGGroupNode
{
	TiXmlHandle* pNode;
	TiXmlElement* pElement;
};


class COGSettingsReader : public IOGSettingsReader
{
public:
	COGSettingsReader();
	virtual ~COGSettingsReader();

	// open settings source.
	virtual IOGSettingsSource* OpenSource (const std::string& _File);

	// read group node.
	virtual IOGGroupNode* OpenGroupNode (IOGSettingsSource* _pSource, IOGGroupNode* pParent, const std::string& _Alias);

	// close group node.
	virtual void CloseGroupNode (IOGGroupNode* _pNode);

	// read next group node.
	virtual IOGGroupNode* ReadNextNode (IOGGroupNode* _pNode);

	// read string parameter.
	virtual std::string ReadStringParam (IOGGroupNode* _pGroup, const std::string& _Alias);

	// read int parameter.
	virtual int ReadIntParam (const std::string& _Alias);

	// read float parameter.
	virtual float ReadFloatParam (const std::string& _Alias);

	// close source.
	virtual void CloseSource (IOGSettingsSource* _pSource);
};


#endif
