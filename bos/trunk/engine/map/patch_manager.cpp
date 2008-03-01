#include "stratagus.h"
#include "patch_manager.h"
#include "patch_type.h"
#include "patch.h"
#include "iolib.h"

#include <algorithm>


CPatchManager::CPatchManager()
{
}


CPatchManager::~CPatchManager()
{
	this->clear();
}


CPatch *
CPatchManager::add(const std::string &typeName, int x, int y)
{
	CPatchType *type = this->patchTypesMap[typeName];
	Assert(type);
	if (!type) {
		fprintf(stderr, "Patch not found: %s\n", typeName.c_str());
		return NULL;
	}

	CPatch *patch = new CPatch(type, x, y);
	this->patches.push_back(patch);
	return patch;
}


void
CPatchManager::moveToTop(CPatch *patch)
{
	this->patches.remove(patch);
	this->patches.push_back(patch);
}


void
CPatchManager::moveToBottom(CPatch *patch)
{
	this->patches.remove(patch);
	this->patches.push_front(patch);
}


CPatch *
CPatchManager::getPatch(int x, int y) const
{
	std::list<CPatch *>::const_reverse_iterator i;
	for (i = this->patches.rbegin(); i != this->patches.rend(); ++i) {
		if ((*i)->getX() <= x && x < (*i)->getX() + (*i)->getType()->getTileWidth() &&
				(*i)->getY() <= y && y < (*i)->getY() + (*i)->getType()->getTileHeight()) {
			return *i;
		}
	}
	return NULL;
}

std::list<CPatch *>
CPatchManager::getPatches() const
{
	return this->patches;
}

std::vector<std::string>
CPatchManager::getPatchTypeNames() const
{
	std::vector<std::string> names;
	std::map<std::string, CPatchType *>::const_iterator i;

	for (i = this->patchTypesMap.begin(); i != this->patchTypesMap.end(); ++i) {
		names.push_back(i->second->getName());
	}

	return names;
}


void
CPatchManager::load()
{
	std::list<CPatch *>::iterator i;
	for (i = this->patches.begin(); i != this->patches.end(); ++i) {
		(*i)->getType()->load();
	}
}

void
CPatchManager::clear()
{
	std::list<CPatch *>::iterator i;
	for (i = this->patches.begin(); i != this->patches.end(); ++i) {
		(*i)->getType()->clean();
		delete *i;
	}
	this->patches.clear();
}



CPatchType *
CPatchManager::newPatchType(const std::string &name, const std::string &file,
	int tileWidth, int tileHeight, int *flags)
{
	unsigned short *newFlags = new unsigned short[tileWidth * tileHeight];
	for (int i = 0; i < tileWidth * tileHeight; ++i) {
		newFlags[i] = flags[i];
	}

	CPatchType *patchType = newPatchType(name, file, tileWidth, tileHeight, newFlags);

	delete[] newFlags;
	return patchType;
}

CPatchType *
CPatchManager::newPatchType(const std::string &name, const std::string &file,
	int tileWidth, int tileHeight, unsigned short *flags)
{
	Assert(this->patchTypesMap[name] == NULL);
	if (this->patchTypesMap[name] != NULL) {
		fprintf(stderr, "Patch type already exists: %s\n", name.c_str());
		return this->patchTypesMap[name];
	}

	CPatchType *patchType = new CPatchType(name, file, tileWidth, tileHeight, flags);
	this->patchTypesMap[name] = patchType;

	return patchType;
}

CPatchType *
CPatchManager::getPatchType(const std::string &name)
{
	return this->patchTypesMap[name];
}

void
CPatchManager::savePatchType(CFile *file, CPatchType *patchType)
{
	file->printf("patchType(\"%s\", \"%s\", %d, %d, {\n",
		patchType->getName().c_str(), patchType->getGraphic()->File.c_str(),
		patchType->getTileWidth(), patchType->getTileHeight());

	for (int j = 0; j < patchType->getTileHeight(); ++j) {
		for (int i = 0; i < patchType->getTileWidth(); ++i) {
			file->printf(" 0x%04x,", patchType->getFlag(i, j));
		}
		file->printf("\n");
	}

	file->printf("})\n");
}
