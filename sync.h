/*
* sync.h - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#ifndef SYNC_H
#define SYNC_H

#include "DFMParser.h"
#include "VMTDir.h"
#include "helpers.h"

#include <string>

void synchronize(DFMData& dfmres, const VMTDir& vmtdir);
void synchronizeProperties(DFMResource* dfm, const std::string& classname, DFMProperty& property, const VMTDir& vmtdir, const DFMData& dfmres);

/**
* Used to synchronize the names of objects in the DFM tree with the
* names of fields in the VMT tree.
**/
class SynchronizeName
{
	private:
		const VMTDir& vmtdir_;
		
	public:
		SynchronizeName(const VMTDir& vmtdir) : vmtdir_(vmtdir) {}
		
		void operator()(DFMResource* dfm)
		{
			if (dfm && dfm->parent)
			{
				DFMResource* parent = dfm;
				
				VMT* vmt = 0;
				
				std::string* str = 0;

				if (dfm)
				   parent = parent->parent;
				
				do
				{
                    vmt = find<FindByName>(vmtdir_, *parent->classname);
                    
                    parent = parent->parent;
                    
                    if (vmt)
                    {
                       str = getVMTAttribute(vmt->fields, *dfm->name);
                       
                       if (str)
                          break;
                    }
                }
                while (parent);
				
				if (!str)
				   return;
				
				delete dfm->name;
				dfm->name = str;
			}
		}
};

/**
* Used to synchronize the class names of objects in the DFM tree with the
* names of classes in the VMT tree.
**/
class SynchronizeClassName
{
	private:
		const VMTDir& vmtdir_;
		
	public:
		SynchronizeClassName(const VMTDir& vmtdir) : vmtdir_(vmtdir) {}
		
		void operator()(DFMResource* dfm)
		{
			VMT* vmt = find<FindByName>(vmtdir_, *dfm->classname);
			if (!vmt) return;

			delete dfm->classname;
			dfm->classname = vmt->name;
		}
};

/**
* Used to synchronize the names and values in the DFM tree with elements
* of the VMT tree.
**/
class SynchronizeProperties
{
	private:
		const VMTDir& vmtdir_;
		const DFMData& dfmres_;
		
	public:
		SynchronizeProperties(const VMTDir& vmtdir, const DFMData& dfmres)
			: vmtdir_(vmtdir), dfmres_(dfmres) {}
			
		void operator()(DFMResource* dfm)
		{
			for (unsigned int i=0;i<dfm->properties.size();++i)
			{
				synchronizeProperties(dfm, *dfm->classname, dfm->properties[i], vmtdir_, dfmres_);
			}
		}
};

#endif
