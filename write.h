/*
* write.h - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#ifndef WRITE_H
#define WRITE_H

#include "DFMParser.h"
#include "VMTDir.h"

#include <string>

void store(const std::string& filename, const DFMData& dfmresources, VMTDir& vmtdir, PeLib::PeFile32& pef);

#endif
