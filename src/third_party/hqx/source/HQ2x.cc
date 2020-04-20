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
#include <iostream>



HQ2x::HQ2x()
{
	// nothing to do
}


HQ2x::~HQ2x()
{
	// nothing to do
}


uint32_t *HQ2x::resize(
	const uint32_t *image,
	uint32_t width,
	uint32_t height,
	uint32_t *output,
	uint32_t trY,
	uint32_t trU,
	uint32_t trV,
	uint32_t trA,
	bool wrapX,
	bool wrapY ) const
{
	int lineSize = width * 2;

	int previous, next;
	uint32_t w[9];

	trY <<= 16;
	trU <<= 8;
	trA <<= 24;

	// iterates between the lines
	for (uint32_t row = 0; row < height; row++)
	{
		/*
		 * Note: this function uses a 3x3 sliding window over the original image.
		 *
		 *   +----+----+----+
		 *   |    |    |    |
		 *   | w0 | w1 | w2 |
		 *   +----+----+----+
		 *   |    |    |    |
		 *   | w3 | w4 | w5 |
		 *   +----+----+----+
		 *   |    |    |    |
		 *   | w6 | w7 | w8 |
		 *   +----+----+----+
		 */

		// adjusts the previous and next line pointers
		if (row > 0)
			previous = -width;
		else
		{
			if (wrapY)
				previous = width * (height - 1);
			else
				previous = 0;
		}
		if (row < height - 1)
			next = width;
		else
		{
			if (wrapY)
				next = -(width * (height - 1));
			else
				next = 0;
		}

		// iterates between the columns
		for (uint32_t col = 0; col < width; col++)
		{
			w[1] = *(image + previous);
			w[4] = *image;
			w[7] = *(image + next);

			if (col > 0)
			{
				w[0] = *(image + previous - 1);
				w[3] = *(image - 1);
				w[6] = *(image + next - 1);
			}
			else
			{
				if (wrapX)
				{
					w[0] = *(image + previous + width - 1);
					w[3] = *(image + width - 1);
					w[6] = *(image + next + width - 1);
				}
				else
				{
					w[0] = w[1];
					w[3] = w[4];
					w[6] = w[7];
				}
			}

			if (col < width - 1)
			{
				w[2] = *(image + previous + 1);
				w[5] = *(image + 1);
				w[8] = *(image + next + 1);
			}
			else
			{
				if (wrapX)
				{
					w[2] = *(image + previous - width + 1);
					w[5] = *(image - width + 1);
					w[8] = *(image + next - width + 1);
				}
				else
				{
					w[2] = w[1];
					w[5] = w[4];
					w[8] = w[7];
				}
			}

			int pattern = 0;

			// computes the pattern to be used considering the neighbor pixels
			for (int k = 0, flag = 1; k < 9; k++)
			{
				// ignores the central pixel
				if (k == 4) continue;

				if (w[k] != w[4])
					if (HQx::isDifferent(w[4], w[k], trY, trU, trV, trA)) pattern |= flag;
				flag <<= 1;
			}

			switch (pattern)
			{
				case 0:
				case 1:
				case 4:
				case 32:
				case 128:
				case 5:
				case 132:
				case 160:
				case 33:
				case 129:
				case 36:
				case 133:
				case 164:
				case 161:
				case 37:
				case 165:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 2:
				case 34:
				case 130:
				case 162:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 16:
				case 17:
				case 48:
				case 49:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 64:
				case 65:
				case 68:
				case 69:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_8_5_2_1_1
					break;
				case 8:
				case 12:
				case 136:
				case 140:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 3:
				case 35:
				case 131:
				case 163:
					MIX_00_4_3_3_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 6:
				case 38:
				case 134:
				case 166:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_5_3_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 20:
				case 21:
				case 52:
				case 53:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_3_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 144:
				case 145:
				case 176:
				case 177:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_7_3_1
					break;
				case 192:
				case 193:
				case 196:
				case 197:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_5_3_1
					break;
				case 96:
				case 97:
				case 100:
				case 101:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_3_3_1
					MIX_11_4_8_5_2_1_1
					break;
				case 40:
				case 44:
				case 168:
				case 172:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_7_3_1
					MIX_11_4_5_7_2_1_1
					break;
				case 9:
				case 13:
				case 137:
				case 141:
					MIX_00_4_1_3_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 18:
				case 50:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_7_3_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 80:
				case 81:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_6_3_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 72:
				case 76:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_5_2_1_1
					break;
				case 10:
				case 138:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_5_2_1_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 66:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_8_5_2_1_1
					break;
				case 24:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 7:
				case 39:
				case 135:
					MIX_00_4_3_3_1
					MIX_01_4_5_3_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 148:
				case 149:
				case 180:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_3_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_7_3_1
					break;
				case 224:
				case 228:
				case 225:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_3_3_1
					MIX_11_4_5_3_1
					break;
				case 41:
				case 169:
				case 45:
					MIX_00_4_1_3_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_7_3_1
					MIX_11_4_5_7_2_1_1
					break;
				case 22:
				case 54:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_7_3_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 208:
				case 209:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_6_3_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 104:
				case 108:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_5_2_1_1
					break;
				case 11:
				case 139:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_5_2_1_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 19:
				case 51:
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
					MIX_00_4_3_3_1
					MIX_01_4_2_3_1
					}
					else
					{
					MIX_00_4_1_3_5_2_1
					MIX_01_4_1_5_2_3_3
					}
					MIX_10_4_7_3_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 146:
				case 178:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
					MIX_01_4_2_3_1
					MIX_11_4_7_3_1
					}
					else
					{
					MIX_01_4_1_5_2_3_3
					MIX_11_4_5_7_5_2_1
					}
					MIX_10_4_7_3_2_1_1
					break;
				case 84:
				case 85:
					MIX_00_4_3_1_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
					MIX_01_4_1_3_1
					MIX_11_4_8_3_1
					}
					else
					{
					MIX_01_4_5_1_5_2_1
					MIX_11_4_5_7_2_3_3
					}
					MIX_10_4_6_3_2_1_1
					break;
				case 112:
				case 113:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_2_1_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
					MIX_10_4_3_3_1
					MIX_11_4_8_3_1
					}
					else
					{
					MIX_10_4_7_3_5_2_1
					MIX_11_4_5_7_2_3_3
					}
					break;
				case 200:
				case 204:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
					MIX_10_4_6_3_1
					MIX_11_4_5_3_1
					}
					else
					{
					MIX_10_4_7_3_2_3_3
					MIX_11_4_7_5_5_2_1
					}
					break;
				case 73:
				case 77:
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
					MIX_00_4_1_3_1
					MIX_10_4_6_3_1
					}
					else
					{
					MIX_00_4_3_1_5_2_1
					MIX_10_4_7_3_2_3_3
					}
					MIX_01_4_1_5_2_1_1
					MIX_11_4_8_5_2_1_1
					break;
				case 42:
				case 170:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
					MIX_00_4_0_3_1
					MIX_10_4_7_3_1
					}
					else
					{
					MIX_00_4_3_1_2_3_3
					MIX_10_4_3_7_5_2_1
					}
					MIX_01_4_2_5_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 14:
				case 142:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
					MIX_00_4_0_3_1
					MIX_01_4_5_3_1
					}
					else
					{
					MIX_00_4_3_1_2_3_3
					MIX_01_4_1_5_5_2_1
					}
					MIX_10_4_6_7_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 67:
					MIX_00_4_3_3_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_8_5_2_1_1
					break;
				case 70:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_5_3_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_8_5_2_1_1
					break;
				case 28:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_3_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 152:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_7_3_1
					break;
				case 194:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_5_3_1
					break;
				case 98:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_3_3_1
					MIX_11_4_8_5_2_1_1
					break;
				case 56:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_7_3_1
					MIX_11_4_8_7_2_1_1
					break;
				case 25:
					MIX_00_4_1_3_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 26:
				case 31:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_6_7_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 82:
				case 214:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_6_3_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 88:
				case 248:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_2_1_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 74:
				case 107:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_5_2_1_1
					break;
				case 27:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_3_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 86:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_6_3_2_1_1
					MIX_11_4_8_3_1
					break;
				case 216:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_6_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 106:
					MIX_00_4_0_3_1
					MIX_01_4_2_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_5_2_1_1
					break;
				case 30:
					MIX_00_4_0_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_6_7_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 210:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_2_3_1
					MIX_10_4_6_3_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 120:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_2_1_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_3_1
					break;
				case 75:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_5_2_1_1
					MIX_10_4_6_3_1
					MIX_11_4_8_5_2_1_1
					break;
				case 29:
					MIX_00_4_1_3_1
					MIX_01_4_1_3_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 198:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_5_3_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_5_3_1
					break;
				case 184:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_7_3_1
					MIX_11_4_7_3_1
					break;
				case 99:
					MIX_00_4_3_3_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_3_3_1
					MIX_11_4_8_5_2_1_1
					break;
				case 57:
					MIX_00_4_1_3_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_7_3_1
					MIX_11_4_8_7_2_1_1
					break;
				case 71:
					MIX_00_4_3_3_1
					MIX_01_4_5_3_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_8_5_2_1_1
					break;
				case 156:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_3_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_7_3_1
					break;
				case 226:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_3_3_1
					MIX_11_4_5_3_1
					break;
				case 60:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_3_1
					MIX_10_4_7_3_1
					MIX_11_4_8_7_2_1_1
					break;
				case 195:
					MIX_00_4_3_3_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_5_3_1
					break;
				case 102:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_5_3_1
					MIX_10_4_3_3_1
					MIX_11_4_8_5_2_1_1
					break;
				case 153:
					MIX_00_4_1_3_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_7_3_1
					break;
				case 58:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					MIX_10_4_7_3_1
					MIX_11_4_8_7_2_1_1
					break;
				case 83:
					MIX_00_4_3_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					MIX_10_4_6_3_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 92:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 202:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					MIX_01_4_2_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					MIX_11_4_5_3_1
					break;
				case 78:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					MIX_01_4_5_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					MIX_11_4_8_5_2_1_1
					break;
				case 154:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					MIX_10_4_6_7_2_1_1
					MIX_11_4_7_3_1
					break;
				case 114:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					MIX_10_4_3_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 89:
					MIX_00_4_1_3_1
					MIX_01_4_2_1_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 90:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 55:
				case 23:
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
					MIX_00_4_3_3_1
					MIX_01_4
					}
					else
					{
					MIX_00_4_1_3_5_2_1
					MIX_01_4_1_5_2_3_3
					}
					MIX_10_4_7_3_2_1_1
					MIX_11_4_8_7_2_1_1
					break;
				case 182:
				case 150:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
					MIX_01_4
					MIX_11_4_7_3_1
					}
					else
					{
					MIX_01_4_1_5_2_3_3
					MIX_11_4_5_7_5_2_1
					}
					MIX_10_4_7_3_2_1_1
					break;
				case 213:
				case 212:
					MIX_00_4_3_1_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
					MIX_01_4_1_3_1
					MIX_11_4
					}
					else
					{
					MIX_01_4_5_1_5_2_1
					MIX_11_4_5_7_2_3_3
					}
					MIX_10_4_6_3_2_1_1
					break;
				case 241:
				case 240:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_2_1_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
					MIX_10_4_3_3_1
					MIX_11_4
					}
					else
					{
					MIX_10_4_7_3_5_2_1
					MIX_11_4_5_7_2_3_3
					}
					break;
				case 236:
				case 232:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
					MIX_10_4
					MIX_11_4_5_3_1
					}
					else
					{
					MIX_10_4_7_3_2_3_3
					MIX_11_4_7_5_5_2_1
					}
					break;
				case 109:
				case 105:
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
					MIX_00_4_1_3_1
					MIX_10_4
					}
					else
					{
					MIX_00_4_3_1_5_2_1
					MIX_10_4_7_3_2_3_3
					}
					MIX_01_4_1_5_2_1_1
					MIX_11_4_8_5_2_1_1
					break;
				case 171:
				case 43:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
					MIX_00_4
					MIX_10_4_7_3_1
					}
					else
					{
					MIX_00_4_3_1_2_3_3
					MIX_10_4_3_7_5_2_1
					}
					MIX_01_4_2_5_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 143:
				case 15:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
					MIX_00_4
					MIX_01_4_5_3_1
					}
					else
					{
					MIX_00_4_3_1_2_3_3
					MIX_01_4_1_5_5_2_1
					}
					MIX_10_4_6_7_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 124:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_3_1
					break;
				case 203:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_5_2_1_1
					MIX_10_4_6_3_1
					MIX_11_4_5_3_1
					break;
				case 62:
					MIX_00_4_0_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_7_3_1
					MIX_11_4_8_7_2_1_1
					break;
				case 211:
					MIX_00_4_3_3_1
					MIX_01_4_2_3_1
					MIX_10_4_6_3_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 118:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_3_3_1
					MIX_11_4_8_3_1
					break;
				case 217:
					MIX_00_4_1_3_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_6_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 110:
					MIX_00_4_0_3_1
					MIX_01_4_5_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_5_2_1_1
					break;
				case 155:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_3_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_7_3_1
					break;
				case 188:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_3_1
					MIX_10_4_7_3_1
					MIX_11_4_7_3_1
					break;
				case 185:
					MIX_00_4_1_3_1
					MIX_01_4_2_1_2_1_1
					MIX_10_4_7_3_1
					MIX_11_4_7_3_1
					break;
				case 61:
					MIX_00_4_1_3_1
					MIX_01_4_1_3_1
					MIX_10_4_7_3_1
					MIX_11_4_8_7_2_1_1
					break;
				case 157:
					MIX_00_4_1_3_1
					MIX_01_4_1_3_1
					MIX_10_4_6_7_2_1_1
					MIX_11_4_7_3_1
					break;
				case 103:
					MIX_00_4_3_3_1
					MIX_01_4_5_3_1
					MIX_10_4_3_3_1
					MIX_11_4_8_5_2_1_1
					break;
				case 227:
					MIX_00_4_3_3_1
					MIX_01_4_2_5_2_1_1
					MIX_10_4_3_3_1
					MIX_11_4_5_3_1
					break;
				case 230:
					MIX_00_4_0_3_2_1_1
					MIX_01_4_5_3_1
					MIX_10_4_3_3_1
					MIX_11_4_5_3_1
					break;
				case 199:
					MIX_00_4_3_3_1
					MIX_01_4_5_3_1
					MIX_10_4_6_3_2_1_1
					MIX_11_4_5_3_1
					break;
				case 220:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 158:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_6_7_2_1_1
					MIX_11_4_7_3_1
					break;
				case 234:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					MIX_01_4_2_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_5_3_1
					break;
				case 242:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					MIX_10_4_3_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 59:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					MIX_10_4_7_3_1
					MIX_11_4_8_7_2_1_1
					break;
				case 121:
					MIX_00_4_1_3_1
					MIX_01_4_2_1_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 87:
					MIX_00_4_3_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_6_3_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 79:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_5_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					MIX_11_4_8_5_2_1_1
					break;
				case 122:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 94:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 218:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 91:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 229:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_3_3_1
					MIX_11_4_5_3_1
					break;
				case 167:
					MIX_00_4_3_3_1
					MIX_01_4_5_3_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_5_7_2_1_1
					break;
				case 173:
					MIX_00_4_1_3_1
					MIX_01_4_1_5_2_1_1
					MIX_10_4_7_3_1
					MIX_11_4_5_7_2_1_1
					break;
				case 181:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_3_1
					MIX_10_4_7_3_2_1_1
					MIX_11_4_7_3_1
					break;
				case 186:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					MIX_10_4_7_3_1
					MIX_11_4_7_3_1
					break;
				case 115:
					MIX_00_4_3_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					MIX_10_4_3_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 93:
					MIX_00_4_1_3_1
					MIX_01_4_1_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 206:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					MIX_01_4_5_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					MIX_11_4_5_3_1
					break;
				case 205:
				case 201:
					MIX_00_4_1_3_1
					MIX_01_4_1_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4_6_3_1
					}
					else
					{
						MIX_10_4_7_3_6_1_1
					}
					MIX_11_4_5_3_1
					break;
				case 174:
				case 46:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4_0_3_1
					}
					else
					{
						MIX_00_4_3_1_6_1_1
					}
					MIX_01_4_5_3_1
					MIX_10_4_7_3_1
					MIX_11_4_5_7_2_1_1
					break;
				case 179:
				case 147:
					MIX_00_4_3_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4_2_3_1
					}
					else
					{
						MIX_01_4_1_5_6_1_1
					}
					MIX_10_4_7_3_2_1_1
					MIX_11_4_7_3_1
					break;
				case 117:
				case 116:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_3_1
					MIX_10_4_3_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4_8_3_1
					}
					else
					{
						MIX_11_4_5_7_6_1_1
					}
					break;
				case 189:
					MIX_00_4_1_3_1
					MIX_01_4_1_3_1
					MIX_10_4_7_3_1
					MIX_11_4_7_3_1
					break;
				case 231:
					MIX_00_4_3_3_1
					MIX_01_4_5_3_1
					MIX_10_4_3_3_1
					MIX_11_4_5_3_1
					break;
				case 126:
					MIX_00_4_0_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_3_1
					break;
				case 219:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_3_1
					MIX_10_4_6_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 125:
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
					MIX_00_4_1_3_1
					MIX_10_4
					}
					else
					{
					MIX_00_4_3_1_5_2_1
					MIX_10_4_7_3_2_3_3
					}
					MIX_01_4_1_3_1
					MIX_11_4_8_3_1
					break;
				case 221:
					MIX_00_4_1_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
					MIX_01_4_1_3_1
					MIX_11_4
					}
					else
					{
					MIX_01_4_5_1_5_2_1
					MIX_11_4_5_7_2_3_3
					}
					MIX_10_4_6_3_1
					break;
				case 207:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
					MIX_00_4
					MIX_01_4_5_3_1
					}
					else
					{
					MIX_00_4_3_1_2_3_3
					MIX_01_4_1_5_5_2_1
					}
					MIX_10_4_6_3_1
					MIX_11_4_5_3_1
					break;
				case 238:
					MIX_00_4_0_3_1
					MIX_01_4_5_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
					MIX_10_4
					MIX_11_4_5_3_1
					}
					else
					{
					MIX_10_4_7_3_2_3_3
					MIX_11_4_7_5_5_2_1
					}
					break;
				case 190:
					MIX_00_4_0_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
					MIX_01_4
					MIX_11_4_7_3_1
					}
					else
					{
					MIX_01_4_1_5_2_3_3
					MIX_11_4_5_7_5_2_1
					}
					MIX_10_4_7_3_1
					break;
				case 187:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
					MIX_00_4
					MIX_10_4_7_3_1
					}
					else
					{
					MIX_00_4_3_1_2_3_3
					MIX_10_4_3_7_5_2_1
					}
					MIX_01_4_2_3_1
					MIX_11_4_7_3_1
					break;
				case 243:
					MIX_00_4_3_3_1
					MIX_01_4_2_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
					MIX_10_4_3_3_1
					MIX_11_4
					}
					else
					{
					MIX_10_4_7_3_5_2_1
					MIX_11_4_5_7_2_3_3
					}
					break;
				case 119:
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
					MIX_00_4_3_3_1
					MIX_01_4
					}
					else
					{
					MIX_00_4_1_3_5_2_1
					MIX_01_4_1_5_2_3_3
					}
					MIX_10_4_3_3_1
					MIX_11_4_8_3_1
					break;
				case 237:
				case 233:
					MIX_00_4_1_3_1
					MIX_01_4_1_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_e_1_1
					}
					MIX_11_4_5_3_1
					break;
				case 175:
				case 47:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_e_1_1
					}
					MIX_01_4_5_3_1
					MIX_10_4_7_3_1
					MIX_11_4_5_7_2_1_1
					break;
				case 183:
				case 151:
					MIX_00_4_3_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_e_1_1
					}
					MIX_10_4_7_3_2_1_1
					MIX_11_4_7_3_1
					break;
				case 245:
				case 244:
					MIX_00_4_3_1_2_1_1
					MIX_01_4_1_3_1
					MIX_10_4_3_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_e_1_1
					}
					break;
				case 250:
					MIX_00_4_0_3_1
					MIX_01_4_2_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 123:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_3_1
					break;
				case 95:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_6_3_1
					MIX_11_4_8_3_1
					break;
				case 222:
					MIX_00_4_0_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_6_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 252:
					MIX_00_4_0_1_2_1_1
					MIX_01_4_1_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_e_1_1
					}
					break;
				case 249:
					MIX_00_4_1_3_1
					MIX_01_4_2_1_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_e_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 235:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_5_2_1_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_e_1_1
					}
					MIX_11_4_5_3_1
					break;
				case 111:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_e_1_1
					}
					MIX_01_4_5_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_5_2_1_1
					break;
				case 63:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_e_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_7_3_1
					MIX_11_4_8_7_2_1_1
					break;
				case 159:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_e_1_1
					}
					MIX_10_4_6_7_2_1_1
					MIX_11_4_7_3_1
					break;
				case 215:
					MIX_00_4_3_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_e_1_1
					}
					MIX_10_4_6_3_2_1_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 246:
					MIX_00_4_0_3_2_1_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					MIX_10_4_3_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_e_1_1
					}
					break;
				case 254:
					MIX_00_4_0_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_e_1_1
					}
					break;
				case 253:
					MIX_00_4_1_3_1
					MIX_01_4_1_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_e_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_e_1_1
					}
					break;
				case 251:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					MIX_01_4_2_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_e_1_1
					}
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 239:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_e_1_1
					}
					MIX_01_4_5_3_1
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_e_1_1
					}
					MIX_11_4_5_3_1
					break;
				case 127:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_e_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_2_1_1
					}
					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
					{
						MIX_10_4
					}
					else
					{
						MIX_10_4_7_3_2_1_1
					}
					MIX_11_4_8_3_1
					break;
				case 191:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_e_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_e_1_1
					}
					MIX_10_4_7_3_1
					MIX_11_4_7_3_1
					break;
				case 223:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
					{
						MIX_00_4
					}
					else
					{
						MIX_00_4_3_1_2_1_1
					}
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_e_1_1
					}
					MIX_10_4_6_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_2_1_1
					}
					break;
				case 247:
					MIX_00_4_3_3_1
					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
					{
						MIX_01_4
					}
					else
					{
						MIX_01_4_1_5_e_1_1
					}
					MIX_10_4_3_3_1
					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
					{
						MIX_11_4
					}
					else
					{
						MIX_11_4_5_7_e_1_1
					}
					break;
				case 255:
					if (HQx::isDifferent(w[3], w[1], trY, trU, trV, trA))
						MIX_00_4
					else
						MIX_00_4_3_1_e_1_1

					if (HQx::isDifferent(w[1], w[5], trY, trU, trV, trA))
						MIX_01_4
					else
						MIX_01_4_1_5_e_1_1

					if (HQx::isDifferent(w[7], w[3], trY, trU, trV, trA))
						MIX_10_4
					else
						MIX_10_4_7_3_e_1_1

					if (HQx::isDifferent(w[5], w[7], trY, trU, trV, trA))
						MIX_11_4
					else
						MIX_11_4_5_7_e_1_1
					break;
			}
			image++;
			output += 2;
		}
		output += lineSize;
	}

	return output;
}
