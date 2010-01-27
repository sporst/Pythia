#include "collisions.h"
#include "helpers.h"
#include "sync.h"

/**
* Creates a new ComponentFinder object.
*
* @param searchString String to search for.
**/
ComponentFinder::ComponentFinder(const std::string& searchString)
	: searchString(searchString), foundString(false)
{
}

/**
* Updates the states of the object if the resource was found.
**/
void ComponentFinder::resourceCallback(DFMResource* res)
{
	if (*res->name == searchString)
	{
		foundString = true;
		res = res;
	}
}

/**
* Unused
**/
void ComponentFinder::propertyCallback(DFMProperty& property)
{
}

/**
* Returns a flag that indicates whether the search was succesful or not.
*
* @return A flag that indicates whether the search was succesful or not.
**/
bool ComponentFinder::found() const { return foundString; }

/**
* Returns the located resource if the search was succesful.
*
* @return The located resource or 0.
**/
const DFMResource* ComponentFinder::foundResource() const { return res; }

/**
* Creates a new CollisionVisitor object.
**/
CollisionVisitor::CollisionVisitor(DFMData& dfmData, const VMTDir& vmtDir)
	: dfmData(dfmData), vmtDir(vmtDir), lastResource(0)
{
}

void CollisionVisitor::resourceCallback(DFMResource* res)
{
	// Keep track of what resource is currently traversed.
	lastResource = res;
}

void CollisionVisitor::propertyCallback(DFMProperty& property)
{
	// At this point we're trying to find the property values which also
	// appear as component names. These values are necessarily Strings.
	// The type of these values is not limited to DFM_STRING though.

	extern bool verboseMode;

	if (property.value.size() == 0)
	{
		// If there's no value, then the value can't possible be the name
		// of a component.
		return;
	}

	VERBOSE_PRINT("Collision check: " << concatStringVector(property.value) 
			<< " in resource " << *lastResource->classname << " " << *lastResource->name);

	if (property.type != DFM_ENUM
		&& property.type != DFM_STRING
		&& property.type != DFM_LONGSTRING
		&& property.type != DFM_LONGSTRING2)
	{
		// If it's none of those types, the value can't possibly be the name
		// of a component.
		return;
	}

	// Create a deep copy of the property name vector.
	std::vector<std::string*> value = deepCopy(property.name);

	// Find the VCL type of the property.
	const std::string* type = findType(*lastResource->classname, value, vmtDir, dfmData);

	if (!type)
	{
		std::cout << "Error: Can't find type of property " << *value[0] << std::endl;
		return;
	}

	VERBOSE_PRINT("Type found: " << *type);

	if (
		*type != "String" &&
		*type != "WideString" &&
		*type != "TCaption" &&
		*type != "TFontName"
	)
	{
		// TODO: Add TFontName?

		// Only these VCL types can cause conflicts.
		return;
	}
	
	// At this point we have the value of a String property. Now
	// we need to check if a component with this name exists.
	// TODO: Method names, Property Names, Field Names, ...

	// TODO: value[0] should be value[all] ?
	ComponentFinder finder(*property.value[0]);
	finder.visit(dfmData);

	if (!finder.found())
	{
		// We're in the clear here. No conflict exists for that property.
		return;
	}

	// A conflict exists. Mark this property value as "Do not obfuscate"
	property.obfuscateValue = false;

	VERBOSE_PRINT("String conflict: " << concatStringVector(property.name) << " has value \"" << *property.value[0] << "\". A component has the same name.");
}

/**
* String properties that have the same value as the name of a component should
* not be obfuscated. It's assumed that the value of the string is a coincidence
* and does not reference the component's name.
*
* This function searches for these situations and marks the found string values
* with a flag that says that the values should not be obfuscated.
*
* @param dfmres A DFM Resource.
* @param VMTDir A VMT directory.
**/
void checkStringCollisions(DFMData& dfmres, const VMTDir& vmtdir)
{
	// TODO: Don't just check component names but property names, method
	// names and field names too.
	CollisionVisitor(dfmres, vmtdir).visit(dfmres);
}

