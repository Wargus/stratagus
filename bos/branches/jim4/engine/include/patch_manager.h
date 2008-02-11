#ifndef _PATCH_MANAGER_H_
#define _PATCH_MANAGER_H_


#include <list>
#include <map>

class CPatchType;
class CPatch;


class CPatchManager
{
public:
	/**
	**  Patch manager constructor
	*/
	CPatchManager();

	/**
	**  Patch manager destructor
	*/
	~CPatchManager();

	/**
	**  Create a new patch of type typeName at location x, y
	*/
	CPatch *add(const std::string &typeName, int x, int y);

	/**
	**  Move a patch to the top
	*/
	void moveToTop(CPatch *patch);

	/**
	**  Move a patch to the bottom
	*/
	void moveToBottom(CPatch *patch);

	/**
	**  Get the patch at location x, y
	*/
	CPatch *getPatch(int x, int y) const;

	std::list<CPatch *> getPatches();

	/**
	**  Load the patches used in the map
	*/
	void load();

	/**
	**  Clear the patches used in the map
	*/
	void clear();

	/**
	**  Define a new patch type.
	**  Types should be created only once and last for the duration of the game.
	*/
	CPatchType *newPatchType(const std::string &name, const std::string &file,
		int tileWidth, int tileHeight, int *flags);

private:
	std::list<CPatch *> patches;
	std::map<std::string, CPatchType *> patchTypesMap;
};


#endif /* _PATCH_MANAGER_H_ */

