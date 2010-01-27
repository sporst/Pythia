/*
* collisions.h - Proof of concept for a Delphi Obfuscator
*
* Copyright (c) 2005 - 2007 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
*/

#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "VMTDir.h"
#include "DFMParser.h"

void checkStringCollisions(DFMData& dfmres, const VMTDir& vmtdir);

// TODO: Add some way to stop traversal of directories.

/**
* This class is used to search for components with a given name.
**/
class ComponentFinder : public DFMVisitor
{
	private:
		/**
		* The name of the component to search.
		**/
		std::string searchString;

		/**
		* Flag that indicates whether the search was successful.
		**/
		bool foundString;

		/**
		* The found resource (or 0).
		**/
		const DFMResource* res;

	protected:
		/**
		* Called for each DFMResource in the DFM directory.
		**/
		virtual void resourceCallback(DFMResource* res);

		/**
		* Called for each DFMProperty in the DFM directory;
		**/
		virtual void propertyCallback(DFMProperty& property);

	public:
		/**
		* Creates a new ComponentFinder object.
		**/
		ComponentFinder(const std::string& searchString);

		/**
		* Flag that indicates whether the search was succesful.
		**/
		bool found() const;

		/**
		* Found resource or 0.
		**/
		const DFMResource* foundResource() const;
};

/**
* This class is used to find string collisions (see checkStringCollisions for
* a description).
**/
class CollisionVisitor : public DFMVisitor
{
	private:
		/**
		* The DFM directory to traverse.
		**/
		DFMData& dfmData;

		/**
		* The VMT Directory that's synchronized with the DFMDirectory.
		**/
		const VMTDir& vmtDir;

		/**
		* The last traversed resource.
		**/
		const DFMResource* lastResource;

	protected:
		/**
		* Called for each DFMResource in the DFM directory.
		**/
		virtual void resourceCallback(DFMResource* res);

		/**
		* Called for each DFMProperty in the DFM directory;
		**/
		virtual void propertyCallback(DFMProperty& property);

	public:

		/**
		* Creates a new CollisionVisitor object.
		*
		* @param dfmData A DFM directory.
		* @param vmtDir A VMT directory.
		**/
		CollisionVisitor(DFMData& dfmData, const VMTDir& vmtDir);
};

#endif
