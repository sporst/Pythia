/*
* main.cpp - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#include "DFMParser.h"
#include "VmtDir.h"
#include "helpers.h"
#include "obfuscate.h"
#include "write.h"
#include "sync.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <map>
#include <iomanip>
#include <PeLib.h>

extern unsigned int g_recognizedVmts;

void printUsage()
{
	std::cout << "Usage: pythia.exe [options] file\n\n";
	std::cout << "Options:\n";
	std::cout << "  -i    Prints information about the file (does not modify the file)\n";
	std::cout << "  -c    Show changes (Prints the obfuscated strings)\n";
}

void printStats()
{
	std::cout << "Recognized VMTs: " << g_recognizedVmts << "\n\n";
}

void printVMT(const VMT* vmt, std::string pad = "")
{
     std::cout << pad << "Name: " << *vmt->name << "\n";
     std::cout << pad << "Offset: 0x" << std::uppercase << std::hex << vmt->offset << "\n";
     std::cout << pad << "Properties: " << std::dec << vmt->typeinfo.size() << "\n";
     
     for (unsigned int i=0;i<vmt->typeinfo.size();i++)
     {
         std::cout << pad << "  " << *vmt->typeinfo[i].type << " " << *vmt->typeinfo[i].name << "\n";
         std::cout << pad << "    GetProc: " << std::hex << vmt->typeinfo[i].GetProc << "\n";
         std::cout << pad << "    SetProc: " << std::hex << vmt->typeinfo[i].SetProc << "\n";
         std::cout << pad << "    StoredProc: " << std::hex << vmt->typeinfo[i].StoredProc << "\n";
     }
     
     std::cout << pad << "Methods: " << std::dec << vmt->methods.size() << "\n";
     
     for (unsigned int i=0;i<vmt->methods.size();i++)
     {
         std::cout << pad << "  Name: " << *vmt->methods[i].name << " ( 0x" << std::hex << vmt->methods[i].va << " )\n";
     }
     
     std::cout << pad << "Fields: " << std::dec << vmt->fields.size() << "\n";
     
     for (unsigned int i=0;i<vmt->fields.size();i++)
     {
         std::cout << pad << "  Name: " << *vmt->fields[i].name << "\n";
     }
     
     std::cout << "\n";
     
     for (unsigned int i=0;i<vmt->children.size();i++)
         printVMT(vmt->children[i], pad + "  ");
}

void printDfm(DFMResource* dfm, std::string pad = "")
{
     std::cout << pad << *dfm->classname << " " << *dfm->name << "\n";
     
     std::cout << pad << "Properties: " << std::dec << dfm->properties.size() << "\n";
     
     for (unsigned int i=0;i<dfm->properties.size();i++)
     {
          for (unsigned int j=0;j<dfm->properties[i].name.size();j++)
          {
              std::cout << pad << "  " << *dfm->properties[i].name[j] << "\n";
          }
          
//          for (unsigned int j=0;j<dfm->properties[i].value.size();j++)
//          {
//              std::cout << pad << "  " << *dfm->properties[i].value[j] << "\n";
//          }
     }
     
     std::cout << "\n";
     
     for (unsigned int i=0;i<dfm->children.size();i++)
         printDfm(dfm->children[i], pad + "  ");
}

bool printInformation = false;
bool showChanges = false;
	
int main(int argc, char *argv[])
{
	std::cout << "Pythia 1.1 - Author: Sebastian Porst (webmaster@the-interweb.com)\n\n";
	
	srand(static_cast<unsigned int>(time(0)));
	
	if (argc < 2)
	{
		printUsage();
		return 1;
	}
	
	for (int i=1;i<argc - 1;i++)
	{
        if (!strcmp(argv[i], "-i"))
           printInformation = true;
           
        if (!strcmp(argv[i], "-c"))
           showChanges = true;
    }
    
    if ( printInformation && showChanges )
    {
         die("-i and -c are mutually exclusive");
    }
	
    std::string filename = argv[argc - 1];
    
    PeLib::PeFile32 pefile(filename);
    
    if (pefile.readMzHeader() == 0 && pefile.readPeHeader() == 0 && pefile.readResourceDirectory() == 0)
    {
	    VMTDir vmtdir;
		
		readVMTs(pefile, vmtdir);
		
		printStats();
		
		if ( printInformation )
		{
			for (std::vector<VMT*>::iterator Iter = vmtdir.begin(); Iter != vmtdir.end(); ++Iter)
			{
				printVMT(*Iter);
			}
//             std::for_each(vmtdir.begin(), vmtdir.end(), printVMT);
        }
		
	    DFMData dfmresources;
	    
	    try
	    {
		    readDFMResources(pefile, dfmresources);
		    
		    if ( printInformation )
		    {
                 std::cout << "Recognized DFMs\n\n";
             
			for (std::vector<DFMResource*>::iterator Iter = dfmresources.begin(); Iter != dfmresources.end(); ++Iter)
			{
				printDfm(*Iter);
			}

//                 std::for_each(dfmresources.begin(), dfmresources.end(), printDfm);
            }
            else
		    {
    			synchronize(dfmresources, vmtdir);
    			obfuscate(dfmresources, vmtdir);
    			store(filename, dfmresources, vmtdir, pefile);
            }
		}
		catch(const std::string& e)
		{
			die(e);
		}
		
		if ( !printInformation )
		{
  	    	std::cout << "Everything seems to have worked. Try to start the obfuscated file now." << std::endl;
        }
		
	    return EXIT_SUCCESS;
	}
	else
	{
		die("Error: File does not seem to be a valid Delphi file.");
	}
}
