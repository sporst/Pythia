/*
* helpers.h - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#ifndef HELPERS_H
#define HELPERS_H

#include <sstream>
#include <string>
#include <set>
#include <deque>
#include <cctype>

#include "VMTDir.h"

/// Prints an error message and terminates the program.
void die(const std::string& error);

/**
* Performs non-case sensitive string comparison.
* @param s1 The first string.
* @param s2 The second string.
* @return Indicates whether or not the strings were equal.
**/
template<typename T>
bool cmpncs(T s1, T s2)
{
	std::transform(s1.begin(), s1.end(), s1.begin(), (int(*)(int)) toupper);
	std::transform(s2.begin(), s2.end(), s2.begin(), (int(*)(int)) toupper);
	
	return s1 == s2;
}

/**
* Returns the complete length of a property name.
* @param name The name.
* @return The length of the name.
**/
template<typename T>
unsigned int propertyNameLength(const T& name)
{
	// Names of n segments are separated by n - 1 period characters.
	unsigned int ret = name.size() ? static_cast<unsigned int>(name.size()) - 1 : 0;
	
	for (size_t i=0;i<name.size();++i)
	{
		ret += static_cast<unsigned int>(name[i]->length());
	}
	
	return ret;
}

/**
* Used to determine whether or not a character is a character that can
* appear in a Pascal string.
**/
struct ValidCharacter
{
	static bool isInvalid(unsigned char c)
	{
		return std::isalnum(c) == false && c != '.' && c != '_';
	}
};

/**
* Determines whether a string only contains valid characters.
* @param str The string to check.
* @return True if the string contains only valid characters.
**/
template<typename T>
bool verifyPascalString(const std::string& str)
{
	return std::find_if(str.begin(), str.end(), T::isInvalid) == str.end();
}

/**
* Reads a Pascal-style string from a buffer.
* @param beg Pointer to the beginning of a P-String.
* @return The data of the P-String.
**/
template<typename T>
std::string readPascalString(const unsigned char* beg)
{
	std::stringstream ss;
	T len = *(T*)beg;
    beg += sizeof(T);
	for (unsigned int i=0;i<len;i++)
	{
		ss << *beg++;
	}
	
	return ss.str();
}

/**
* Converts the parameter to a hex-string.
* @param x Value to be converted.
* @return A string that contains the hex-value of the parameter.
**/
template<typename T>
std::string toHexString(T x)
{
	std::stringstream ss;
	ss << std::hex << std::uppercase << x;
	return ss.str();
}

/**
* Produces a random letter or number.
**/
struct RandomCharacterGenerator
{
	char operator()()
	{
		unsigned int randValue = rand() % 62;
			
		if (randValue <= 9) return '0' + randValue;
		else if (randValue <=  35) return 'A' + randValue - 10;
		else return 'a' + randValue - 36;
	}
};

/**
* Generates a random string of the given size. It's guaranteed that the first
* character of the string is a letter.
* @param maxsize Size of the string.
* @return A random string 
**/
template<typename T>
std::string randomString(unsigned int size)
{
	if (!size) return "";
	
	std::string ret(size, (char)('A' + rand() % 26));
	
	std::generate(ret.begin() + 1, ret.end(), T());
	
	return ret;
}

/**
* Makes sure that all generated random strings are unique.
* @param Size of the random string to generate.
* @return A random string of the given size.
**/
template<typename T>
std::string uniqueString(unsigned int size)
{
	if (size == 0) return "";
	
	static std::set<std::string> strings;
	
	// Make 20 attempts to generate a unique string.
	for (unsigned int i=0;i<20;++i)
	{
		std::string retstr = randomString<T>(size);

		if (strings.find(retstr) == strings.end())
		{
			strings.insert(retstr);
			return retstr;
		}
	}
	
	throw std::string("Error: Cannot create enough unique strings.");
}
#include <iostream>
template<typename T>
struct FindByName
{
	static bool compare(T val, const std::string& classname)
	{
//  std::cout << "Comparing to " << *val->name << std::endl;
		return *val->name == classname;
	}
};

/**
* Searches through a container.
* @param cont The container.
* @param value The value to find.
* @param The value if the search was succesful. 0 otherwise.
**/
template<template <typename T> class SearchModus, typename Container>
typename Container::value_type find(const Container& cont, const std::string& value)
{
	typedef typename Container::value_type Element;
	
	std::deque<Element> vals(cont.begin(), cont.end());

	while (vals.size())
	{
		Element val = vals[0];
		vals.pop_front();
		
		if (SearchModus<Element>::compare(val, value)) return val;
		
		std::copy(val->children.begin(), val->children.end(), std::back_inserter(vals));
	}
	
	return 0;
}

/**
* Fills a container with the elements of another container and all child-elements
* of these elements.
* @param src Source container.
* @param dest Destination container.
**/
template<typename SourceContainer, typename DestContainer>
void fill(const SourceContainer& src, DestContainer& dest)
{
	typedef typename DestContainer::value_type Element;
	
	dest.assign(src.begin(), src.end());

	unsigned int counter = 0;
	
	while (counter != dest.size())
	{
		Element elem = dest[counter++];
		std::copy(elem->children.begin(), elem->children.end(), std::back_inserter(dest));
	}
}

template<typename T>
std::string* getValue(const T& vals, const std::string& val, bool ignoreCase = false)
{
	for (unsigned int i=0;i<vals.size();++i)
	{
		if (!ignoreCase && *vals[i].name == val) return vals[i].name;
		else if (ignoreCase && cmpncs(*vals[i].name, val)) return vals[i].name;
	}
	
	return 0;
}

template<class T>
void splitName(T& name)
{
	if (!name.size()) return;
	std::string value = *name[0];
	if (value.find(".") == std::string::npos) return;

	delete name[0];
	name.clear();

	while (value.find(".") != std::string::npos)
	{
		unsigned int position = static_cast<unsigned int>(value.find("."));
		name.push_back(new std::string(value.substr(0, position)));
		value = value.substr(position + 1);
	}

	name.push_back(new std::string(value));
}

#endif
