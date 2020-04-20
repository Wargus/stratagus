/*
 * Copyright 2016 Bruno Ribeiro
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hqx/HQ2x.hh>
#include <hqx/HQ3x.hh>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <ctime>


using std::ifstream;
using std::ofstream;
using std::string;


#pragma pack(push, 1)

struct BitmapHeader
{
    uint16_t bfType;
    uint32_t bfSize;
    uint32_t bfRes1;
    uint32_t bfOffBits;
};

struct DibHeader
{
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

#pragma pack(pop)


/**
 * @brief Saves an Windows Bitmap image (24 BPP).
 */
int main_saveBitmap(
	const uint32_t *data,
	uint32_t width,
	uint32_t height,
	const string &fileName )
{
	BitmapHeader bh;
	DibHeader dh;
	uint16_t suffix;
	uint32_t zero = 0;
	const uint32_t *ptr;

	ofstream output(fileName.c_str(), std::ios_base::binary);
	if (!output.good()) return -1;

	suffix = ((width + 3) & ~0x03) - width;

	dh.biSize          = sizeof(DibHeader);
	dh.biWidth         = width;
	dh.biHeight        = height;
	dh.biPlanes        = 1;
	dh.biBitCount      = 24;
	dh.biCompression   = 0;
	dh.biSizeImage     = (uint16_t) ( (width*3+suffix)*height );
	dh.biXPelsPerMeter = 0x2E23;
	dh.biYPelsPerMeter = dh.biXPelsPerMeter;
	dh.biClrUsed       = 0;
	dh.biClrImportant  = 0;

	bh.bfType    = 0x4D42;
	bh.bfSize    = dh.biSizeImage + 0x0036;
	bh.bfRes1    = 0;
	bh.bfOffBits = 0x0036;
	output.write( (char*) &bh, sizeof(BitmapHeader) );
	output.write( (char*) &dh, sizeof(DibHeader) );

	ptr = data + (width * height);
	for (uint32_t i = 0; i < height; i++)
	{
		ptr -= width;

		for (uint32_t j = 0; j < width; ++j)
			output.write( (char*) (ptr + j), 3 );

		if (suffix > 0)
			output.write( (char*) &zero, suffix );
	}

  output.close();

  return 0;
}


/**
 * @brief Loads an Windows Bitmap image (24 BPP).
 */
int main_loadBitmap(
	const string &fileName,
	uint32_t *&data,
	uint16_t &width,
	uint16_t &height )
{
	BitmapHeader bh;
	DibHeader dh;
	uint16_t  suffix;
	uint32_t zero = 0;
	uint32_t *ptr;
	uint16_t bits;

	ifstream input(fileName.c_str(), std::ios_base::binary);
	if (!input.good()) return -1;

	input.read( (char*) &bh, sizeof(BitmapHeader) );
	if (bh.bfType != 0x4D42) return -1;
	input.read( (char*) &dh.biSize, sizeof(uint32_t) );
	if (dh.biSize != 40) return -1;

	input.read( (char*) &dh.biWidth, sizeof(DibHeader) - sizeof(uint32_t) );
	width  = dh.biWidth;
	height = dh.biHeight;
	if (dh.biBitCount != 24) return -1;

	suffix = ((width + 3) & ~0x03) - width;
	ptr = data = new uint32_t[width * height]();
	ptr += width * height;
	for (uint32_t i = 0; i < height; i++)
	{
		ptr -= width;

		for (uint32_t j = 0; j < width; ++j)
		{
			input.read( (char*) (ptr + j), 3 );
			*(ptr + j) |= 0xFF000000;
		}

		if (suffix > 0)
			input.read( (char*) &zero, suffix );
	}

	input.close();
	return 0;
}


int main(int argc, char **argv )
{
	uint32_t factor = 2;

	if (argc != 2 && argc != 3) return 1;
	if (argc == 3) factor = atoi(argv[2]);

	// loads the input image
	uint16_t width, height;
	uint32_t *image = NULL;
	if ( main_loadBitmap(argv[1], image, width, height) != 0) return 1;
	std::cout << "Resizing '" << argv[1] << "' [" << width << "x" << height << "] by " <<
		factor << 'x' << std::endl;

	clock_t t = clock();

	// resize the input image using the given scale factor
	uint32_t outputSize = (width * factor) * (height * factor);
	uint32_t *output = new uint32_t[outputSize]();
	HQx *scale;
	if (factor == 2)
		scale = new HQ2x();
	else
		scale = new HQ3x();
	scale->resize(image, width, height, output);
	delete scale;

	t = clock() - t;
	std::cout << "Processing time: " << t / (CLOCKS_PER_SEC / 1000) << " ms" << std::endl;

	// saves the resized image
	if ( main_saveBitmap(output, width * factor, height * factor, "output.bmp") != 0 ) return 1;

	delete[] image;
	delete[] output;
}
