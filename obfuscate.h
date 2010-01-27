/*
* obfuscate.h - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#ifndef OBFUSCATE_H
#define OBFUSCATE_H

#include "VMTDir.h"
#include "DFMParser.h"

void obfuscate(DFMData& dfmres, VMTDir& vmtdir);

#endif
