#ifndef _PATCH_H_
#define _PATCH_H_


class CPatchType;


class CPatch
{
public:
	CPatch(const CPatchType *type, int x, int y) :
		type(type), x(x), y(y)
	{
	}

	~CPatch()
	{
	}

	inline const CPatchType *getType() const { return this->type; }
	inline void setPos(int x, int y) { this->x = x; this->y = y; }
	inline void setX(int x) { this->x = x; }
	inline int getX() const { return this->x; }
	inline void setY(int y) { this->y = y; }
	inline int getY() const { return this->y; }

private:
	const CPatchType *type;
	int x;
	int y;
};


#endif /* _PATCH_H_ */

