//     ____                _       __
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  )
// /_____/\____/____/     |__/|__/\__,_/_/  /____/
//
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name minimap.cpp - The patch manager. */
//
//      (c) Copyright 2008 by Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"
#include "patch_manager.h"
#include "patch_type.h"
#include "patch.h"
#include "iolib.h"
#include "map.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/


CPatchManager::CPatchManager() :
	loadedAll(false)
{
}


CPatchManager::~CPatchManager()
{
	std::map<std::string, CPatchType *>::iterator i;

	for (i = this->patchTypesMap.begin(); i != this->patchTypesMap.end(); ++i) {
		delete i->second;
	}
}


void
CPatchManager::updateMapFlags(int x1, int y1, int x2, int y2)
{
	x1 = std::max(x1, 0);
	y1 = std::max(y1, 0);
	x2 = std::min(x2, Map.Info.MapWidth - 1);
	y2 = std::min(y2, Map.Info.MapHeight - 1);

	for (int j = y1; j <= y2; ++j) {
		for (int i = x1; i <= x2; ++i) {
			int offsetX, offsetY;
			CPatch *patch = this->getPatch(i, j, &offsetX, &offsetY);
			if (patch) {
				Map.Field(i, j)->Flags = patch->getType()->getFlag(offsetX, offsetY);
			}
		}
	}
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

	updateMapFlags(x, y, x + type->getTileWidth() - 1, y + type->getTileHeight() - 1);

	return patch;
}

void
CPatchManager::remove(CPatch *patch)
{
	this->patches.remove(patch);
	this->removedPatches.push_back(patch);

	updateMapFlags(patch->getX(), patch->getY(),
		patch->getX() + patch->getType()->getTileWidth(),
		patch->getY() + patch->getType()->getTileHeight());
}

void
CPatchManager::move(CPatch *patch, int x, int y)
{
	if (x != patch->getX() || y != patch->getY()) {
		int x1, y1, x2, y2;

		if (x < patch->getX()) {
			x1 = x;
			x2 = patch->getX() + patch->getType()->getTileWidth() - 1;
		} else {
			x1 = patch->getX();
			x2 = x + patch->getType()->getTileWidth() - 1;
		}
		if (y < patch->getY()) {
			y1 = y;
			y2 = patch->getY() + patch->getType()->getTileHeight() - 1;
		} else {
			y1 = patch->getY();
			y2 = y + patch->getType()->getTileHeight() - 1;
		}

		patch->setPos(x, y);

		updateMapFlags(x1, y1, x2, y2);
	}
}

void
CPatchManager::moveToTop(CPatch *patch)
{
	this->patches.remove(patch);
	this->patches.push_back(patch);

	updateMapFlags(patch->getX(), patch->getY(),
		patch->getX() + patch->getType()->getTileWidth() - 1,
		patch->getY() + patch->getType()->getTileHeight() - 1);
}


void
CPatchManager::moveToBottom(CPatch *patch)
{
	this->patches.remove(patch);
	this->patches.push_front(patch);

	updateMapFlags(patch->getX(), patch->getY(),
		patch->getX() + patch->getType()->getTileWidth() - 1,
		patch->getY() + patch->getType()->getTileHeight() - 1);
}


CPatch *
CPatchManager::getPatch(int x, int y, int *xOffset, int *yOffset) const
{
	std::list<CPatch *>::const_reverse_iterator i;
	std::list<CPatch *>::const_reverse_iterator rend;

	// Search the patches from top to bottom
	rend = this->patches.rend();
	for (i = this->patches.rbegin(); i != rend; ++i) {
		int patchX = (*i)->getX();
		int patchY = (*i)->getY();
		CPatchType *patchType = (*i)->getType();

		// See if the patch is at location x,y
		if (patchX <= x && x < patchX + patchType->getTileWidth() &&
				patchY <= y && y < patchY + patchType->getTileHeight()) {
			int xPatchOffset = x - patchX;
			int yPatchOffset = y - patchY;
			unsigned short flag = patchType->getFlag(xPatchOffset, yPatchOffset);

			// Make sure the patch tile isn't transparent
			if (!(flag & MapFieldTransparent)) {
				if (xOffset != NULL && yOffset != NULL) {
					*xOffset = xPatchOffset;
					*yOffset = yPatchOffset;
				}

				return *i;
			}
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
CPatchManager::loadAll()
{
	std::map<std::string, CPatchType *>::const_iterator i;

	for (i = this->patchTypesMap.begin(); i != this->patchTypesMap.end(); ++i) {
		i->second->load();
	}

	loadedAll = true;
}

static void clearPatches(std::list<CPatch *> &patches)
{
	std::list<CPatch *>::iterator i;
	for (i = patches.begin(); i != patches.end(); ++i) {
		(*i)->getType()->clean();
		delete *i;
	}
	patches.clear();
}

void
CPatchManager::clear()
{
	clearPatches(this->patches);
	clearPatches(this->removedPatches);

	if (loadedAll) {
		std::map<std::string, CPatchType *>::const_iterator i;

		for (i = this->patchTypesMap.begin(); i != this->patchTypesMap.end(); ++i) {
			i->second->clean();
		}
	}
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
	// Loading a game might redefine a patch, just ignore it
	if (this->patchTypesMap[name] != NULL) {
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

std::string
CPatchManager::savePatches(bool patchesOnly) const
{
	std::map<std::string, bool> patchTypeSaved;
	std::list<CPatch *>::const_iterator i;
	std::ostringstream ostr;

	for (i = this->patches.begin(); i != this->patches.end(); ++i) {
		const std::string &name = (*i)->getType()->getName();

		if (!patchesOnly && !patchTypeSaved[name]) {
			ostr << this->savePatchType((*i)->getType());
			patchTypeSaved[name] = true;
		}

		ostr << "patch(\"" << name << "\", "
		     << (*i)->getX() << ", " << (*i)->getY() << ")\n";
	}

	return ostr.str();
}

std::string
CPatchManager::savePatchType(CPatchType *patchType) const
{
	std::ostringstream ostr;

	ostr << "patchType(\"" << patchType->getName() << "\", \""
	     << patchType->getGraphic()->File << "\", "
	     << patchType->getTileWidth() << ", "
	     << patchType->getTileHeight() << ", {\n";

	for (int j = 0; j < patchType->getTileHeight(); ++j) {
		for (int i = 0; i < patchType->getTileWidth(); ++i) {
			std::ostringstream flag;
			flag << " 0x"
			     << std::hex << std::setw(4) << std::setfill('0')
				 << patchType->getFlag(i, j);
			ostr << flag.str() << ",";
		}
		ostr << "\n";
	}

	ostr << "})\n";

	return ostr.str();
}

//@}
