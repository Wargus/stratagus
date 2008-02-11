#ifndef _PATCH_H_
#define _PATCH_H_


class CPatchType;


class CPatch
{
public:
	/**
	**  Patch constructor
	*/
	CPatch(CPatchType *type, int x, int y) :
		type(type), x(x), y(y)
	{
	}

	/**
	**  Patch destructor
	*/
	~CPatch()
	{
	}

	/**
	**  Get the patch type
	*/
	inline CPatchType *getType() { return this->type; }

	/**
	**  Set the position of the patch
	*/
	inline void setPos(int x, int y) { this->x = x; this->y = y; }

	/**
	**  Set the X position of the patch
	*/
	inline void setX(int x) { this->x = x; }
	/**
	**  Get the X position of the patch
	*/
	inline int getX() const { return this->x; }

	/**
	**  Set the Y position of the patch
	*/
	inline void setY(int y) { this->y = y; }
	/**
	**  Get the Y position of the patch
	*/
	inline int getY() const { return this->y; }

private:
	CPatchType *type;
	int x;
	int y;
};


#endif /* _PATCH_H_ */

