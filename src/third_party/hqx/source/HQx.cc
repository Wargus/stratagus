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

#include <hqx/HQx.hh>
#include <cstdlib>


static const uint32_t AMASK = 0xFF000000;
static const uint32_t YMASK = 0x00FF0000;
static const uint32_t UMASK = 0x0000FF00;
static const uint32_t VMASK = 0x000000FF;

HQx::HQx()
{
	// nothing to do
}


HQx::~HQx()
{
	// nothing to do
}


uint32_t HQx::ARGBtoAYUV(
	uint32_t value )
{
    uint32_t A, R, G, B, Y, U, V;

    A = value >> 24;
    R = (value >> 16) & 0xFF;
    G = (value >> 8) & 0xFF;
    B = value & 0xFF;

    Y = (uint32_t) ( 0.299 * R + 0.587 * G + 0.114 * B);
    U = (uint32_t) (-0.169 * R - 0.331 * G +   0.5 * B) + 128;
    V = (uint32_t) (   0.5 * R - 0.419 * G - 0.081 * B) + 128;
    return (A << 24) + (Y << 16) + (U << 8) + V;
}


#if (1)

/*
 * Use this function for sharper images (good for cartoon style, used by DOSBOX)
 */

bool HQx::isDifferent(
	uint32_t color1,
	uint32_t color2,
	uint32_t trY,
	uint32_t trU,
	uint32_t trV,
	uint32_t trA )
{
	color1 = ARGBtoAYUV(color1);
	color2 = ARGBtoAYUV(color2);

	uint32_t value;

	value = abs(int(color1 & YMASK) - int(color2 & YMASK));
	if (value > trY) return true;

	value = abs(int(color1 & UMASK) - int(color2 & UMASK));
	if (value > trU) return true;

	value = abs(int(color1 & VMASK) - int(color2 & VMASK));
	if (value > trV) return true;

	value = abs(int(color1 & AMASK) - int(color2 & AMASK));
	if (value > trA) return true;

	return false;
}

#else

/*
 * Use this function for smoothed images (good for complex graphics)
 */

bool HQx::isDifferent(
	uint32_t color1,
	uint32_t color2,
	uint32_t trY,
	uint32_t trU,
	uint32_t trV,
	uint32_t trA )
{
	uint32_t yuv1 = ARGBtoAYUV(color1);
	uint32_t yuv2 = ARGBtoAYUV(color2);

	return abs(int(yuv1 & YMASK) - int(yuv2 & YMASK)) > trY ||
		   abs(int(yuv1 & UMASK) - int(yuv2 & UMASK)) > trU ||
		   abs(int(yuv1 & VMASK) - int(yuv2 & VMASK)) > trV ||
		   abs(int(yuv1 & AMASK) - int(yuv2 & AMASK)) > trA;
}

#endif
