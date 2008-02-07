#include "stratagus.h"
#include "patch_manager.h"
#include "patch_type.h"
#include "patch.h"

#include <algorithm>


CPatchManager::CPatchManager()
{
}


CPatchManager::~CPatchManager()
{
	std::list<CPatch *>::iterator i;
	for (i = this->patches.begin(); i != this->patches.end(); ++i) {
		delete *i;
	}
	this->patches.clear();
}


CPatch *
CPatchManager::add(const CPatchType *type, int x, int y)
{
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
		if ((*i)->getX() <= x && x < (*i)->getX() + (*i)->getType()->getWidth() &&
				(*i)->getY() <= y && y < (*i)->getY() + (*i)->getType()->getHeight()) {
			return *i;
		}
	}
	return NULL;
}

