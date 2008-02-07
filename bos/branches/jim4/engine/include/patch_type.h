#ifndef _PATCH_TYPE_H_
#define _PATCH_TYPE_H_


class CGraphic;


class CPatchType
{
public:
	CPatchType(const std::string &name, const CGraphic *graphic,
	           int width, int height, unsigned short *flags) :
		name(name), graphic(graphic), width(width), height(height)
	{
		this->flags = new unsigned short[this->width * this->height];
		memcpy(this->flags, flags, this->width * this->height * sizeof(unsigned short));
	}

	~CPatchType()
	{
		delete[] flags;
	}

	inline const CGraphic *getGraphic() const { return this->graphic; }
	inline int getWidth() const { return this->width; }
	inline int getHeight() const { return this->height; }

	unsigned short getFlag(int x, int y)
	{
		Assert(0 <= x && x < width && 0 <= y && y < height);
		return flags[y * width + x];
	}

private:
	std::string name;
	const CGraphic *graphic;
	int width;
	int height;
	unsigned short *flags;
};


#endif /* _PATCH_TYPE_H_ */

