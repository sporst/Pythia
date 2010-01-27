/*
* DFMParser.h - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#ifndef DFMPARSER_H
#define DFMPARSER_H

#include <PeLib.h>

enum {	DFM_VARIANT = 0,
		DFM_ARRAY = 1,
		DFM_BYTE = 2,
		DFM_WORD = 3,
		DFM_DWORD = 4,
		DFM_DOUBLE = 5,
		DFM_ENUM = 6,
		DFM_STRING = 7,
		DFM_BOOLEAN_FALSE = 8,
		DFM_BOOLEAN_TRUE = 9,
		DFM_BITMAP = 10,
		DFM_SET = 11,
		DFM_LONGSTRING2 = 12,
		DFM_NIL = 13,
		DFM_RECORD = 14,
		DFM_UNICODE_STRING = 0x12,
		DFM_LONGSTRING = 0x14
	};

/**
* A DFM property consists of a name and a value. For simplicity's sake
* the file offset of the property and whether it was already obfuscated are
* also stored.
**/
struct DFMProperty
{
	unsigned int type;
	unsigned int offset;
	std::vector<std::string*> name;
	std::vector<std::string*> value;
	std::vector<DFMProperty> values;
};

/**
* A DFM resource consists of a class name, an object name and a number
* of properties. For simplicity's sake the file offset of resource
* is also stored.
**/
struct DFMResource
{
       unsigned int offset;
       std::string* name;
       std::string* classname;
       DFMResource* parent;
       
       std::vector<DFMProperty> properties;
       std::vector<DFMResource*> children;
       
       DFMResource() : name(0), classname(0), parent(0) {}
};

typedef std::vector<DFMResource*> DFMData;

void readDFMResources(PeLib::PeFile32& pefile, DFMData& dfmresources);
bool isTopElement(const DFMData& dfmres, const std::string& name);

#endif
