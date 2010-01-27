/*
* sync.cpp - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#include "sync.h"

#include <algorithm>
#include <cassert>

/**
* Synchronizes a DFM tree with a VMT tree.
* @param dfmres DFM tree.
* @param vmtdir VMT tree.
**/
void synchronize(DFMData& dfmres, const VMTDir& vmtdir)
{
	std::vector<std::string*> objectnames;

	std::deque<DFMResource*> dfms;
	fill(dfmres, dfms);

	std::for_each(dfms.begin(), dfms.end(), SynchronizeName(vmtdir));
	std::for_each(dfms.begin(), dfms.end(), SynchronizeClassName(vmtdir));
	std::for_each(dfms.begin(), dfms.end(), SynchronizeProperties(vmtdir, dfmres));
}

/**
* Synchronizes one element of a DFM tree with an element of a VMT tree.
* @param classname Class of the element the property belongs to.
* @param value Name of value of a property.
* @param vmtdir VMT tree.
* @param dfmres DFM tree.
**/
void synchronizePropertyValue(const std::string& classname, std::vector<std::string*>& value, const VMTDir& vmtdir, const DFMData& dfmres)
{
	splitName(value);
	
	VMT* vmt = handleCollections(find<FindByName>(vmtdir, classname), vmtdir);
	if (!vmt) return;

	for (unsigned int i=0;i<value.size();++i)
	{
		if (!value[i]->size()) continue;
		
		VMT* vmt2;
		if (vmt2 = findVMT<FindByPropertyName>(vmt, *value[i]))
		{
			std::string* y = getVMTAttribute(vmt2->typeinfo, *value[i]);
			if (!y) break;
			delete value[i];
			value[i] = y;

			std::string* x = getAttributeType(vmt2, *value[i], vmtdir);
			if (!x) break;

			// Special handling. Figure out a better way.
			if (*x == "TCustomActionBarColorMap")
			{
				x = new std::string("TXPColorMap");
			}

			vmt2 = find<FindByName>(vmtdir, *x);
			if (!vmt2) break;
		}
		else if (vmt2 = findVMT<FindByMethodName>(vmt, *value[i]))
		{
			std::string* x = getValue(vmt2->methods, *value[i], true);
			delete value[i];
			value[i] = x;
		}
		else if (vmt2 = findVMT<FindByFieldName>(vmt, *value[i]))
		{
			std::string* x = getValue(vmt2->fields, *value[i]);
			delete value[i];
			value[i] = x;
		}
		else if (DFMResource* res = find<FindByName>(dfmres, *value[i]))
		{
			delete value[i];
			value[i] = res->name;
			vmt2 = find<FindByName>(vmtdir, *res->classname);
		}
		else if (vmt2 = find<FindByName>(vmtdir, *value[i]))
		{
			// This last if-branch is required for bitmaps.
			// value[i] should be TBitmap or TIcon here.
			delete value[i];
			value[i] = vmt2->name;
		}
		else
		{
//			std::cout << "Couldn't find " << *value[i] << std::endl;
		}
		
		vmt = vmt2;
		
		if (!vmt) break;
	}
}

/**
* Synchronizes the properties of a DFM tree with elements of a VMT tree.
**/
void synchronizeProperties(DFMResource* dfm, const std::string& classname, DFMProperty& property, const VMTDir& vmtdir, const DFMData& dfmres)
{
	while (dfm && dfm->parent) dfm = dfm->parent;
	
	// Order is important.
	synchronizePropertyValue(*dfm->classname, property.value, vmtdir, dfmres);
	synchronizePropertyValue(classname, property.name, vmtdir, dfmres);

	for (unsigned int i=0;i<property.values.size();++i)
	{
		if (property.values[i].name.size() == 0) continue;
		
		VMT* vmt = handleCollections(find<FindByName>(vmtdir, classname), vmtdir);
		if (!vmt) continue;
		std::string* x = getAttributeType(vmt, *property.name.front(), vmtdir);
		if (!x) continue;
		vmt = find<FindByName>(vmtdir, *x);
		if (!vmt) continue;
		synchronizeProperties(dfm, *vmt->name, property.values[i], vmtdir, dfmres);
	}
}
