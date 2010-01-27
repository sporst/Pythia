/*
* vmtdir.h - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#ifndef VMTPARSER_H
#define VMTPARSER_H

#include "helpers.h"

#include <PeLib.h>

template<typename T>
bool cmpncs(T s1, T s2);

/**
* Stores the property info of a VMT.
**/
struct PropInfo
{
    unsigned int PropType;
    unsigned int GetProc;
    unsigned int SetProc;
    unsigned int StoredProc;
    int Index;
    int Default;
    short int NameIndex;
    
    std::string* name;
    unsigned int nameoffset;
    
    std::string* type;
    unsigned int typeoffset;
    
    PropInfo()
    {
		name = 0;
		type = 0;
	}
};

/**
* Stores the method info of a VMT.
**/
struct MethodInfo
{
    unsigned short int id;
    unsigned int va;
    std::string* name;
	unsigned int nameoffset;
	
	MethodInfo()
	{
		name = 0;
	}
};

struct FieldInfo
{
	unsigned int nameoffset;
	std::string* name;
	
	FieldInfo()
	{
		name = 0;
	}
};

/**
* Stores information about a virtual method table.
**/
struct VMT
{
	unsigned int offset;
	unsigned int nameoffset;
	std::string* name;
    unsigned int parentvmt;
	
    VMT* parent;
    std::vector<VMT*> children;
    std::vector<PropInfo> typeinfo;
    std::vector<MethodInfo> methods;
    std::vector<FieldInfo> fields;
	
	unsigned int vmtSelfPtr;
	unsigned int vmtIntfTable;
	unsigned int vmtAutoTable;
	unsigned int vmtInitTable;
	unsigned int vmtTypeInfo;
	unsigned int vmtFieldTable;
	unsigned int vmtMethodTable;
	unsigned int vmtDynamicTable;
	unsigned int vmtClassName;
	unsigned int vmtInstanceSize;
	unsigned int vmtParent;
	unsigned int vmtSafeCallException;
	unsigned int vmtAfterConstruction;
	unsigned int vmtBeforeDestruction;
	unsigned int vmtDispatch;
	unsigned int vmtDefaultHandler;
	unsigned int vmtNewInstance;
	unsigned int vmtFreeInstance;
	unsigned int vmtDestroy;
	
	VMT()
	{
		name = 0;
		parent = 0;
	}
	
	~VMT()
	{
		for (unsigned int i=0;i<children.size();i++)
		{
			delete children[i];
		}
	}
};

typedef std::vector<VMT*> VMTDir;

void readVMTs(PeLib::PeFile32& pefile, VMTDir& vmtparser);
VMT* handleCollections(VMT* vmt, const VMTDir& vmtdir);
std::string* getAttributeType(const VMT* vmt, const std::string& name, const VMTDir& vmtdir);

/**
* Used when searching VMTs by method name.
**/
struct FindByMethodName
{
	static VMT* find(VMT* vmt, const std::string& methodname)
	{
		for (unsigned int i=0;i<vmt->methods.size();i++)
		{
			if (cmpncs(*vmt->methods[i].name, methodname))
			{
				return vmt;
			}
		}
		return 0;
	}
};

/**
* Used when searching VMTs by property name.
**/
struct FindByPropertyName
{
	static VMT* find(VMT* vmt, const std::string& propertyname)
	{
		for (unsigned int i=0;i<vmt->typeinfo.size();i++)
		{
			if (*vmt->typeinfo[i].name == propertyname)
			{
				return vmt;
			}
		}
		return 0;
	}
};

/**
* Used when searching VMTs by field name.
**/
struct FindByFieldName
{
	static VMT* find(VMT* vmt, const std::string& fieldname)
	{
		for (unsigned int i=0;i<vmt->fields.size();i++)
		{
			if (*vmt->fields[i].name == fieldname)
			{
				return vmt;
			}
		}
		return 0;
	}
};

/**
* Searches upwards through the VMT hierarchy for a VMT with a given value.
* @param vmt The VMT where the search begins.
* @param value The value to search for.
* @return The VMT with the given value. If no such VMT was found the
*         return value is 0.
**/
template<typename T>
VMT* findVMT(VMT* vmt, const std::string& value)
{
	VMT* vmt2 = T::find(vmt, value);
	if (vmt2) return vmt2;
	else if (vmt->parent) return findVMT<T>(vmt->parent, value);
	else return 0;
}

/**
* Searches through a vector and returns the found element.
* @param v The vector.
* @param value The string value of the element to search for.
* @return The found element or 0 if no element was found.
**/
template<typename T>
std::string* getVMTAttribute(const std::vector<T>& v, const std::string& value)
{
	for (unsigned int i=0;i<v.size();++i)
	{
//        std::cout << *v[i].name << std::endl;
		if (*v[i].name == value)
		{
			return v[i].name;
		}
	}
	
	return 0;
}

#endif
