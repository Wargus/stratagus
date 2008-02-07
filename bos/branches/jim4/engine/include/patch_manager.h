#ifndef _PATCH_MANAGER_H_
#define _PATCH_MANAGER_H_


#include <list>

class CPatchType;
class CPatch;


class CPatchManager
{
public:
	CPatchManager();
	~CPatchManager();

	CPatch *add(const CPatchType *type, int x, int y);

	void moveToTop(CPatch *patch);
	void moveToBottom(CPatch *patch);

	CPatch *getPatch(int x, int y) const;

private:
	std::list<CPatch *> patches;
};


#endif /* _PATCH_MANAGER_H_ */

