/*
* DFMParser.cpp - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#include "DFMParser.h"
#include "helpers.h"

#include <exception>

/**
* Determines whether a class is used as a top-level element in the DFM tree.
* This is necessary because these elements can not yet be crypted.
* @param dfmres The DFM tree.
* @param name The name of a class.
* @return True if the class is used as a top level element in the DFM tree.
**/
bool isTopElement(const DFMData& dfmres, const std::string& name)
{
	for (unsigned int i=0;i<dfmres.size();++i)
	{
		if (*dfmres[i]->classname == name)
		{
			return true;
		}
	}
	
	return false;
}

/**
* Skips over the data of a property because in most cases the data is not
* important for obfuscation.
* If the property type is of the type that's possibly a method name the
* data is read into a string and stored in the property.
* @param dataptr Pointer to the first byte of the data.
* @param property Current property.
* @return Number of bytes that were skipped.
**/
unsigned int skipData(unsigned char*& dataptr, DFMData& dfmres, DFMResource* res, DFMProperty& property, unsigned int offset)
{
	property.type = *dataptr++;
	unsigned int skip = 1;

	switch(property.type)
	{
		case DFM_ARRAY:	// Array
			while (*dataptr)
			{
				DFMProperty p;
				p.offset = offset + skip - 1;
				skip += skipData(dataptr, dfmres, res, p, offset + skip);
				property.values.push_back(p);
			}
			++dataptr;
			++skip;
			break;
		case DFM_BYTE:	// Byte
			++dataptr;
			++skip;
			break;
		case DFM_WORD:	// Word
			dataptr += 2;
			skip += 2;
			break;
		case DFM_DWORD:	// Dword
			dataptr += 4;
			skip += 4;
			break;
		case DFM_DOUBLE:	// Not sure if it's really a Double value
			dataptr += 10;
			skip += 10;
			break;
		case DFM_STRING:	// String
		case DFM_ENUM:
			property.value.push_back(new std::string(readPascalString<unsigned char>(dataptr)));
			dataptr += 1 + static_cast<unsigned int>(property.value.back()->length());
			skip += 1 + static_cast<unsigned int>(property.value.back()->length());
			break;
		case DFM_VARIANT:
		case DFM_BOOLEAN_FALSE:	// Boolean value "false"
		case DFM_BOOLEAN_TRUE:	// Boolean value "true"
			break;
		case DFM_BITMAP:	// Bitmap
			{
				unsigned int size = *(unsigned int*)dataptr;
				dataptr += 4;
				std::string type = readPascalString<unsigned char>((const unsigned char*)dataptr);
				if (verifyPascalString<ValidCharacter>(type))
				{
					property.value.push_back(new std::string(type));
				}
				
				dataptr += size;
				skip += 4 + size;
				break;
			}
		case DFM_SET:	// Set
			while (*dataptr)
			{
				std::string value = readPascalString<unsigned char>(dataptr);
				skip += 1 + static_cast<unsigned int>(value.length());
				dataptr += 1 + static_cast<unsigned int>(value.length());
			}
			
			++dataptr;
			++skip;
			break;
		case DFM_NIL:
			break;
		case DFM_RECORD: // Record

			// Each iteration of this loop reads one item of the record.
			while (*dataptr)
			{
				++dataptr;
				++skip;

				// Each iteration of this loop reads one property of the item.
				while (*dataptr && *dataptr != 1)
				{
					DFMProperty prop;
					prop.offset = offset + skip;
					std::string propertyname = readPascalString<unsigned char>(dataptr);
					prop.name.push_back(new std::string(propertyname));

					skip += 1 + propertyNameLength(prop.name);
					dataptr += 1 + propertyNameLength(prop.name);
					skip += skipData(dataptr, dfmres, res, prop, offset + skip);
					property.values.push_back(prop);
				}
				// *dataptr is 0 (end of record) or 1 (new item)
				++dataptr;
				++skip;
			}
			++dataptr;
			++skip;
			break;
		case DFM_UNICODE_STRING: // Unicode String
			skip += 4 + 2 * *(unsigned int*)dataptr;
			dataptr += 4 + 2 * *(unsigned int*)dataptr;
			break;
		case DFM_LONGSTRING:	// LongString
		case DFM_LONGSTRING2:
			{
//			assert(property.type != 0x0C);
				std::string value = readPascalString<unsigned int>(dataptr);
				skip += 4 + static_cast<unsigned int>(value.length());
				dataptr += 4 + static_cast<unsigned int>(value.length());
				break;
			}
		default:
			throw std::string("Error: Cannot recognize resource type " + toHexString(property.type));
	}
	
	return skip;
}

/**
* Reads all properties of a DFM resource.
* @param dataptr Pointer to the first byte of the resource (after the 'TPF0' signature).
* @param offset File offset of that first byte.
* @param dfmresources DFM resources to which the found resource will be added.
* @param isroot Indicates whether the current resource is the root resource or not.
* @return Returns the offset where the resource ends.
**/
unsigned int parseDFMResource(unsigned char*& dataptr, unsigned int offset, unsigned int maxoffset, DFMData& dfmresources, DFMResource* parent)
{
	if (offset > maxoffset) throw new std::string("Error: Failure when reading DFM data.");
	
	// Skip leading Fx bytes.
	if (*dataptr == 0xF1 || *dataptr == 0xF4)
	{
		++dataptr;
		++offset;
	}
	else if (*dataptr == 0xF2)
	{
		dataptr += 3;
		offset += 3;
	}

	// Read the current resource.
	DFMResource* dfmres = new DFMResource();

	dfmres->offset = offset;
	dfmres->classname = new std::string(readPascalString<unsigned char>(dataptr));
	offset += 1 + static_cast<unsigned int>(dfmres->classname->length());
	dataptr += 1 + static_cast<unsigned int>(dfmres->classname->length());

	dfmres->name = new std::string(readPascalString<unsigned char>(dataptr));
	offset += 1 + static_cast<unsigned int>(dfmres->name->length());
	dataptr += 1 + static_cast<unsigned int>(dfmres->name->length());
	
	dfmres->parent = parent;
	if (parent) parent->children.push_back(dfmres);
	else dfmresources.push_back(dfmres);

	while (*dataptr)
	{
		// Read all properties of the current resource.
		DFMProperty property;
			
		property.offset = offset;
		std::string* name = new std::string(readPascalString<unsigned char>(dataptr));
		property.name.push_back(name);

		// The order of the next two lines can not be changed.
		offset += 1 + propertyNameLength(property.name);
		dataptr += 1 + propertyNameLength(property.name);
		offset += skipData(dataptr, dfmresources, dfmres, property, offset);
		dfmres->properties.push_back(property);
	}
		
	++dataptr;
	++offset;
	
	// Are there sub-resources in the current resource?
	while (*dataptr)
	{
		offset = parseDFMResource(dataptr, offset, maxoffset, dfmresources, dfmres);
	}
		
	++dataptr;
	++offset;
	
	return offset;
}

/**
* Reads all DFM resources from a file.
* @param pefile PEFile to be read.
* @param dfmresources All recognized DFM resources will be stored here.
**/
void readDFMResources(PeLib::PeFile32& pefile, DFMData& dfmresources)
{
	PeLib::ResourceDirectory& resdir = pefile.resDir();
	
	unsigned int numberOfResources = resdir.getNumberOfResources(PeLib::PELIB_RT_RCDATA);
	unsigned int resourceGroupPosition = resdir.resourceTypeIdToIndex(PeLib::PELIB_RT_RCDATA);
	
	for (unsigned int i=0;i<numberOfResources;i++)
	{
		// Get the data of each resource.
		std::vector<PeLib::byte> resourceData;
		resdir.getResourceDataByIndex(resourceGroupPosition, i, resourceData);
		
		// 0x30465054 = "DFM "
   	    if (resourceData.size() >= 4 && *(unsigned int*)(&resourceData[0]) == 0x30465054)
		{
			PeLib::ResourceNode* root = resdir.getRoot();
			
			PeLib::ResourceNode* currNode = static_cast<PeLib::ResourceNode*>(root->getChild(resourceGroupPosition));
			currNode = static_cast<PeLib::ResourceNode*>(currNode->getChild(i));
			PeLib::ResourceLeaf* currLeaf = static_cast<PeLib::ResourceLeaf*>(currNode->getChild(0));

			unsigned char* data = &resourceData[4]; // Skip the "TPF0" identifier.
			unsigned int offset = pefile.peHeader().rvaToOffset(currLeaf->getOffsetToData() + 4);
			parseDFMResource(data, offset, offset + resourceData.size() - 4, dfmresources, 0);
		}
	}
}


