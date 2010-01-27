/*
* vmtdir.cpp - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#include "VMTDir.h"

#include <algorithm>

extern unsigned int g_recognizedVmts;

/**
* Adds a child VMT to a given parent VMT.
* @param parent The parent VMT.
* @param child The child VMT.
* @return Returns true if adding the child succeeded.
**/
bool addChild(VMT* parent, VMT* child)
{
	if (parent->vmtSelfPtr == child->parentvmt)
	{
		child->parent = parent;
		parent->children.push_back(child);
		return true;
	}
	
	return std::find_if(parent->children.begin(), parent->children.end(),
		std::bind2nd(std::ptr_fun(&addChild), child)) != parent->children.end();
}

/**
* Inserts a VMT into the VMT hierarchy. If it's position in the VMT can not be
* resolved the new element is added to the root element.
* @param vmt The VMT to insert.
**/
void insert(VMTDir& root, VMT* vmt)
{
	if (vmt->parentvmt)
	{
		if (std::find_if(root.begin(), root.end(),
				std::bind2nd(std::ptr_fun(&addChild), vmt)) != root.end())
		{
			return;
		}
	}

	root.push_back(vmt);
}

/**
* Reads the property information of a VMT type.
* @param vmt The current VMT.
* @param Pointer to beginning of property info block.
* @offset Offset of the property info block.
**/
void readTypeInfo(VMT* vmt, const unsigned char* t, unsigned int offset)
{
    const unsigned char* tptr = t + 1;

    unsigned char len = *tptr++;
    tptr += len + 4 + 4 + 2;
    offset += 1 + 1 + len + 4 + 4 + 2;
    
    len = *tptr++;
    tptr += len;
    offset += len + 1;

    unsigned short int properties = *(short int*)tptr;
    tptr+=2;
    offset += 2;
    
    PropInfo pi;

    for (unsigned int i=0;i<(unsigned int)properties;i++)
    {
        pi.PropType = *(unsigned int*)tptr;
        tptr+=4;
        pi.GetProc = *(unsigned int*)tptr;
        tptr+=4;
        pi.SetProc = *(unsigned int*)tptr;
        tptr+=4;
        pi.StoredProc = *(unsigned int*)tptr;
        tptr+=4;
        pi.Index = *(unsigned int*)tptr;
        tptr+=4;
        pi.Default = *(unsigned int*)tptr;
        tptr+=4;
        pi.NameIndex = *(unsigned int*)tptr;
        tptr+=2;
	    offset += 26;
		pi.name = new std::string(readPascalString<unsigned char>(tptr));
		pi.nameoffset = offset;
	    offset += 1 + static_cast<unsigned int>(pi.name->length());
		tptr += 1 + static_cast<unsigned int>(pi.name->length());

        vmt->typeinfo.push_back(pi);
    }
}

/**
* Reads the method information of a VMT type.
* @param vmt The current VMT.
* @param Pointer to beginning of method info block.
* @offset Offset of the method info block.
**/
void readMethodInfo(VMT* vmt, const unsigned char* t, unsigned int offset)
{
	const unsigned char* tptr = t;
	unsigned short int num = *(short int*)tptr;
	tptr+=2;
	offset += 2;

	for (unsigned int i=0;i<(unsigned int)num;i++)
	{
		MethodInfo mi;
		mi.id = *(short int*)tptr;
		tptr+=2;
		offset += 2;
		mi.va = *(unsigned int*)tptr;
		tptr+=4;
		offset += 4;
		mi.nameoffset = offset;
		mi.name = new std::string;
		*mi.name = readPascalString<unsigned char>(tptr);
		offset += 1 + static_cast<unsigned int>(mi.name->length());
		tptr += 1 + static_cast<unsigned int>(mi.name->length());
		vmt->methods.push_back(mi);
	}
}

void readFieldTable(VMT* vmt, const unsigned char* t, unsigned int offset)
{
	unsigned int fields = *(unsigned short*)t;
	const unsigned char* dataptr = t + 6;
	offset += 6;
	
	for (unsigned int i=0;i<fields;i++)
	{
		dataptr += 6;
		offset += 6;
		
		FieldInfo fi;
		fi.nameoffset = offset;
		fi.name = new std::string;
		*fi.name = readPascalString<unsigned char>(dataptr);
		dataptr += 1 + static_cast<unsigned int>(fi.name->length());
		offset += 1 + static_cast<unsigned int>(fi.name->length());
		vmt->fields.push_back(fi);
	}
}

/**
* Reads a virtual method table.
* @param file Pointer to beginning of the Delphi file.
* @param offset File offset of the VMT.
* @param peh PeHeader of the Delphi file.
**/
VMT* readVMT(const unsigned char* file, unsigned int offset, PeLib::PeHeader32& peh)
{
	VMT* vmt = new VMT();
	
	const unsigned int* vmtptr = reinterpret_cast<const unsigned int*>(file + offset);
	vmt->vmtSelfPtr = *vmtptr++;
	vmt->vmtIntfTable = *vmtptr++;
	vmt->vmtAutoTable = *vmtptr++;
	vmt->vmtInitTable = *vmtptr++;
	vmt->vmtTypeInfo = *vmtptr++;
	vmt->vmtFieldTable = *vmtptr++;
	vmt->vmtMethodTable = *vmtptr++;
	vmt->vmtDynamicTable = *vmtptr++;
	vmt->vmtClassName = *vmtptr++;
	vmt->vmtInstanceSize = *vmtptr++;
	vmt->vmtParent = *vmtptr++;
	vmt->vmtSafeCallException = *vmtptr++;
	vmt->vmtAfterConstruction = *vmtptr++;
	vmt->vmtBeforeDestruction = *vmtptr++;
	vmt->vmtDispatch = *vmtptr++;
	vmt->vmtDefaultHandler = *vmtptr++;
	vmt->vmtNewInstance = *vmtptr++;
	vmt->vmtFreeInstance = *vmtptr++;
	vmt->vmtDestroy = *vmtptr++;
	
	unsigned int uiOffset = peh.rvaToOffset(vmt->vmtParent - peh.getImageBase());
	if (uiOffset == std::numeric_limits<unsigned int>::max())
	{
		vmt->parentvmt = 0;
		vmt->parent = 0;
	}
	else
	{
		vmt->parentvmt = *(unsigned int*)(file + uiOffset);
	}
	
	vmt->offset = offset;
	
	vmt->nameoffset = peh.rvaToOffset(vmt->vmtClassName - peh.getImageBase());

	if (vmt->nameoffset == std::numeric_limits<unsigned int>::max())
	{
		delete vmt;
		return 0;
	}
	
	const unsigned char* no = file + vmt->nameoffset;
	std::string name = readPascalString<unsigned char>(no);
	
	if (!verifyPascalString<ValidCharacter>(name))
	{
		delete vmt;
		return 0;
	}
	
//	std::cout << name << std::endl;

	vmt->name = new std::string;
	*vmt->name = name;
	
	return vmt;
}

/**
* After all VMTs were read this function needs to be called to re-build the
* VMT hierarchy. The type hierarchy can not be built on the fly because
* the VMTs are not read in the correct order to build a clean tree right away.
**/
void fix(VMTDir& root)
{
	std::vector<VMT*> notfound;

	for (std::vector<VMT*>::iterator Iter = root.begin(); Iter != root.end();)
	{
		if ((*Iter)->vmtParent != 0)
		{
			notfound.push_back(*Iter);
			Iter = root.erase(Iter);
		}
		else
		{
			++Iter;
		}
	}

	// This loop rebuilds the VMT hierarchy.
	while (true)
    {
		unsigned int size_before = static_cast<unsigned int>(notfound.size());

		for (size_t i=0;i<root.size();i++)
		{
			notfound.erase(std::remove_if(notfound.begin(), notfound.end(),
					std::bind1st(std::ptr_fun(&addChild), root[i])), notfound.end());
		}

		// Unable to resolve the correct positions of the VMTs in the hierarchy.
		// Dump them back into the root.
		if (size_before == notfound.size())
		{
			root.insert(root.end(), notfound.begin(), notfound.end());
			return;
		}
	}
}

class ReadExtraInfo
{
	private:
		const VMTDir& vmtdir_;
		unsigned char* file_;
		const PeLib::PeHeader32& peh_;
		
	public:
		ReadExtraInfo(const VMTDir& vmtdir, unsigned char* file, PeLib::PeHeader32& peh)
			: vmtdir_(vmtdir), file_(file), peh_(peh) {}
		
		void operator()(VMT* vmt)
		{
			if (vmt->vmtFieldTable)
			{
				unsigned int tioffset = peh_.rvaToOffset(vmt->vmtFieldTable - peh_.getImageBase());
				
				if (tioffset != std::numeric_limits<unsigned int>::max())
				{
					readFieldTable(vmt, file_ + tioffset, tioffset);
				}
			}
		
			if (vmt->vmtMethodTable)
			{
				unsigned int tioffset = peh_.rvaToOffset(vmt->vmtMethodTable - peh_.getImageBase());
				
				if (tioffset != std::numeric_limits<unsigned int>::max())
				{
					readMethodInfo(vmt, file_ + tioffset, tioffset);
				}
			}
			
			if (vmt->vmtTypeInfo)
			{
				unsigned int tioffset = peh_.rvaToOffset(vmt->vmtTypeInfo - peh_.getImageBase());
				if (tioffset != std::numeric_limits<unsigned int>::max())
				{
					readTypeInfo(vmt, file_ + tioffset, tioffset);
					
					for (unsigned int i=0;i<vmt->typeinfo.size();++i)
					{
					    vmt->typeinfo[i].typeoffset = peh_.rvaToOffset(vmt->typeinfo[i].PropType - peh_.getImageBase());
		
					    if (vmt->typeinfo[i].typeoffset != std::numeric_limits<unsigned int>::max())
					    {
							vmt->typeinfo[i].typeoffset += 5;
							const unsigned char* typeinfoaddr = file_ + vmt->typeinfo[i].typeoffset;
							std::string type = readPascalString<unsigned char>(typeinfoaddr);
							if (!verifyPascalString<ValidCharacter>(type))
							{
								continue;
							}
		
							VMT* typevmt = find<FindByName>(vmtdir_, type);
							if (typevmt)
							{
								vmt->typeinfo[i].type = typevmt->name;
							}
							else
							{
								vmt->typeinfo[i].type = new std::string(type);
							}
						}
					}
				}
			}
		}
};

/**
* Searches through an entire file and tries to find valid VMTs.
* @param pefile The file to be read.
* @param vmtdir All found VMTs will be stored here.
**/
void readVMTs(PeLib::PeFile32& pefile, VMTDir& vmtdir)
{
    std::ifstream file(pefile.getFileName().c_str(), std::ios::binary);
    
    if (!file)
    {
		die("Error: Couldn't open file " + pefile.getFileName() + ".");
	}
    
    // Read the entire file.
	unsigned int fs = PeLib::fileSize(pefile.getFileName());
	std::vector<unsigned char> v(fs);
	file.read(reinterpret_cast<char*>(&v[0]), fs);
	
	PeLib::PeHeader32& peh = pefile.peHeader();
	
	for (unsigned int i=0;i<fs;i+=4) // All VMTs are DWORD-aligned
	{
		PeLib::dword d = *(PeLib::dword*)(&v[i]);
		PeLib::dword o = pefile.peHeader().rvaToOffset(d - pefile.peHeader().getImageBase());

		if (o == std::numeric_limits<PeLib::dword>::max()) continue;

		if (d >= pefile.peHeader().getImageBase() && o >= 0x200 && i >= 0x200)
		{
			if (o == i + 76)
			{
				if (VMT* vmt = readVMT(&v[0], i, peh))
				{
					vmt->offset = i;
					insert(vmtdir, vmt);
					++g_recognizedVmts;
				}
			}
		}
	}
	
	fix(vmtdir);
	
	std::deque<VMT*> vmts;
	fill(vmtdir, vmts);
	std::for_each(vmts.begin(), vmts.end(), ReadExtraInfo(vmtdir, &v[0], peh));
}

VMT* handleCollections(VMT* vmt, const VMTDir& vmtdir)
{
	if (vmt && vmt->parent && vmt->parent->name &&
		(*vmt->parent->name == "TCollection"
			|| *vmt->parent->name == "TOwnedCollection"
			|| *vmt->parent->name == "TActionClientsCollection")
		)
	{
		if (*vmt->name == "TdfsStatusPanels") return find<FindByName>(vmtdir, "TdfsStatusPanel");
		else if (*vmt->name == "TCoolBands") return find<FindByName>(vmtdir, "TCoolBand");
		else if (*vmt->name == "TStatusPanels") return find<FindByName>(vmtdir, "TStatusPanel");
		else if (*vmt->name == "THeaderSections") return find<FindByName>(vmtdir, "THeaderSection");
		else if (*vmt->name == "TListColumns") return find<FindByName>(vmtdir, "TListColumn");
		else if (*vmt->name == "TmxStatusPanels") return find<FindByName>(vmtdir, "TmxStatusPanel");
		else if (*vmt->name == "TActionBars") return find<FindByName>(vmtdir, "TActionBarItem");
		else if (*vmt->name == "TActionClients") return find<FindByName>(vmtdir, "TActionClientItem");
		else if (*vmt->name == "TDBGridColumns") return find<FindByName>(vmtdir, "TDBGridColumns");
		else if (*vmt->name == "TDBGridColumnsEh") return find<FindByName>(vmtdir, "TDBGridColumnsEh");
		else if (*vmt->name == "TAggregates") return find<FindByName>(vmtdir, "TAggregates");
		else if (*vmt->name == "TDBSumCollection") return find<FindByName>(vmtdir, "TDBSumCollection");
		else if (*vmt->name == "TParameters") return find<FindByName>(vmtdir, "TParameter");
		else throw std::string("Error: Unknown collection " + *vmt->name);
	}
	
	return vmt;
}

std::string* getAttributeType(const VMT* vmt, const std::string& name, const VMTDir& vmtdir)
{
	for (unsigned int i=0;i<vmt->typeinfo.size();++i)
	{
		if (*vmt->typeinfo[i].name == name)
		{
			return vmt->typeinfo[i].type;
		}
	}

	if (vmt->parent) return getAttributeType(handleCollections(vmt->parent, vmtdir), name, vmtdir);
	
	return 0;
}

