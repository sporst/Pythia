/*
* helpers.cpp - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#include <iostream>
#include <string>

unsigned int g_recognizedVmts;

/**
* Prints an error message to stdout and terminates the program
* returning EXIT_FAILURE to the shell.
* @param error Error message to print.
**/
void die(const std::string& error)
{
	std::cout << error << std::endl;
	std::exit(EXIT_FAILURE);
}
