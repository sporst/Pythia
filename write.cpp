/*
* write.cpp - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#include "write.h"

#include <algorithm>

/**
* Used write the name elements of objects to a file.
**/
template<typename T>
class WriteName
{
	std::fstream& file_;
	
	public:
		WriteName(std::fstream& file) : file_(file) {}
		
		void operator()(T& x)
		{
			file_.seekp(x.nameoffset + 1);
			file_.write(x.name->c_str(), static_cast<unsigned int>(x.name->length()));
			if (!file_) throw new std::string("Error: Couldn't write file.");
		}
};

/**
* Stores the obfuscated data back to the file.
* @param filename Name of the file where data is written to.
* @param dfmresources Obfuscated DFM data
* @param vmtdir Obfuscated VMT data.
**/
void store(const std::string& filename, const DFMData& dfmresources, VMTDir& vmtdir, PeLib::PeFile32& pef)
{
    std::fstream file(filename.c_str(), std::ios::in | std::ios::out | std::ios::binary);
    
    PeLib::PeHeader32& peh = pef.peHeader();
    
	if (!file) throw new std::string("Error: Couldn't open file.");
	
	std::deque<VMT*> vmts;
	fill(vmtdir, vmts);

	for (std::deque<VMT*>::iterator Iter = vmts.begin(); Iter != vmts.end(); ++Iter)
	{
		file.seekp((*Iter)->nameoffset + 1);
		file.write((*Iter)->name->c_str(), static_cast<unsigned int>((*Iter)->name->length()));
		if (!file) throw new std::string("Error: Couldn't write file.");
		
		if ((*Iter)->vmtTypeInfo)
		{
			file.seekp(peh.vaToOffset((*Iter)->vmtTypeInfo) + 2);
			file.write((*Iter)->name->c_str(), static_cast<unsigned int>((*Iter)->name->length()));
			if (!file) throw new std::string("Error: Couldn't write file.");
		}
		
		std::for_each((*Iter)->typeinfo.begin(), (*Iter)->typeinfo.end(), WriteName<PropInfo>(file));
		std::for_each((*Iter)->fields.begin(), (*Iter)->fields.end(), WriteName<FieldInfo>(file));
		std::for_each((*Iter)->methods.begin(), (*Iter)->methods.end(), WriteName<MethodInfo>(file));
	}
	
	std::deque<DFMResource*> dfms;
	fill(dfmresources, dfms);
	
	for (std::deque<DFMResource*>::iterator Iter = dfms.begin(); Iter != dfms.end(); ++Iter)
	{
		DFMResource* dfm = *Iter;

		file.seekp(dfm->offset + 1);
		file.write(dfm->classname->c_str(), static_cast<unsigned int>(dfm->classname->length()));
		file.seekp(1, std::ios_base::cur);
		file.write(dfm->name->c_str(), static_cast<unsigned int>(dfm->name->length()));
		
		std::deque<DFMProperty> dfmps(dfm->properties.begin(), dfm->properties.end());
		
		while (dfmps.size())
		{
			DFMProperty property = dfmps[0];
			dfmps.pop_front();
			
			file.seekp(property.offset + 1);
			
			if (property.name.size())
			{
				for (unsigned int j=0;j<property.name.size() - 1;++j)
				{
					file.write(property.name[j]->c_str(), static_cast<unsigned int>(property.name[j]->length()));
					file.write(".", 1);
				}

				file.write(property.name.back()->c_str(), static_cast<unsigned int>(property.name.back()->length()));
			}
			
			if (property.value.size())
			{
				file.seekp(property.offset + propertyNameLength(property.name) + 3);
				
				if (property.type == DFM_BITMAP)
				{
					file.seekp(4, std::ios_base::cur);
				}
					
				for (unsigned int i=0;i<property.value.size() - 1;i++)
				{
					file.write(property.value[i]->c_str(), static_cast<unsigned int>(property.value[i]->length()));
					file.write(".", 1);
				}

				file.write(property.value.back()->c_str(), static_cast<unsigned int>(property.value.back()->length()));
			}
			
			std::copy(property.values.begin(), property.values.end(), std::back_inserter(dfmps));
		}
	}
}
