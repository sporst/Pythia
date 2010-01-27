/*
* obfuscate.cpp - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#include "obfuscate.h"
#include "helpers.h"

#include <algorithm>

/**
* Replaces the name element of object x with a random string.
* @param x The object with the name element.
**/
template<typename T>
void obfuscate(T& x)
{
     extern bool showChanges;
     
     std::string newvalue = uniqueString<RandomCharacterGenerator>(static_cast<unsigned int>(x.name->length()));
     
     if ( showChanges )
     {
          std::cout << *x.name << " -> " << newvalue << "\n";
     }
     
     *x.name = newvalue;
}

/**
* Obfuscates the DFM and VMT data of a Delphi file.
* @param dfmres The DFM data of an entire Delphi file.
* @param vmtdir The VMT data of an entire Delphi file.
**/
void obfuscate(DFMData& dfmres, VMTDir& vmtdir)
{
	std::deque<VMT*> vmts;
	fill(vmtdir, vmts);
	
    extern bool showChanges;
    
    if ( showChanges )
    {
         std::cout << "Obfuscated strings: \n\n";
    }

	for (std::deque<VMT*>::iterator Iter = vmts.begin(); Iter != vmts.end(); ++Iter)
	{
		if (!isTopElement(dfmres, *(*Iter)->name)) obfuscate<VMT>(**Iter);

		std::for_each((*Iter)->typeinfo.begin(), (*Iter)->typeinfo.end(), obfuscate<PropInfo>);
		std::for_each((*Iter)->fields.begin(), (*Iter)->fields.end(), obfuscate<FieldInfo>);
		std::for_each((*Iter)->methods.begin(), (*Iter)->methods.end(), obfuscate<MethodInfo>);
	}

	for (unsigned int i=0;i<dfmres.size();++i)
	{
		obfuscate<DFMResource>(*dfmres[i]);
	}
	
    if ( showChanges )
    {
         std::cout << "\n";
    }
}


