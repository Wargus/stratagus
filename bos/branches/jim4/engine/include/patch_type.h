#ifndef _PATCH_TYPE_H_
#define _PATCH_TYPE_H_


#include "video.h"


class CPatchType
{
public:
	/**
	**  Patch type constructor
	*/
	CPatchType(const std::string &name, const std::string &file,
	           int tileWidth, int tileHeight, unsigned short *flags) :
		name(name), file(file), graphic(NULL), tileWidth(tileWidth), tileHeight(tileHeight)
	{
		this->flags = new unsigned short[this->tileWidth * this->tileHeight];
		memcpy(this->flags, flags, this->tileWidth * this->tileHeight * sizeof(unsigned short));
	}

	/**
	**  Patch type destructor
	*/
	~CPatchType()
	{
		CGraphic::Free(this->graphic);
		delete[] this->flags;
	}

	/**
	**  Load the patch type
	*/
	void load()
	{
		if (!this->graphic) {
			this->graphic = CGraphic::New(this->file);
			this->graphic->Load();
		}
	}

	/**
	**  Clean the allocated memory
	*/
	void clean()
	{
		CGraphic::Free(this->graphic);
		this->graphic = NULL;
	}

	/**
	**  Get the name
	*/
	inline const std::string &getName() const { return this->name; }

	/**
	**  Get the file name
	*/
	inline const std::string &getFile() const { return this->file; }

	/**
	**  Get the graphic
	*/
	inline const CGraphic *getGraphic() const { return this->graphic; }

	/**
	**  Get the tile width of the patch
	*/
	inline int getTileWidth() const { return this->tileWidth; }

	/**
	**  Get the tile height of the patch
	*/
	inline int getTileHeight() const { return this->tileHeight; }

	/**
	**  Get the tile flag at a tile location
	*/
	unsigned short getFlag(int x, int y)
	{
		Assert(0 <= x && x < this->tileWidth && 0 <= y && y < this->tileHeight);
		return flags[y * this->tileWidth + x];
	}

	/**
	**  Set the tile flag at a tile location
	*/
	void setFlag(int x, int y, unsigned short flag)
	{
		Assert(0 <= x && x < this->tileWidth && 0 <= y && y < this->tileHeight);
		flags[y * this->tileWidth + x] = flag;
	}

	/**
	**  Get all of the tile flags
	*/
	inline unsigned short *getFlags()
	{
		return this->flags;
	}

private:
	std::string name;
	std::string file;
	CGraphic *graphic;
	int tileWidth;
	int tileHeight;
	unsigned short *flags;
};


#endif /* _PATCH_TYPE_H_ */

