//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name glgraphicdraw.cpp - The general sprite functions. */
//
//      (c) Copyright 2000-2008 by Lutz Sammer, Stephan Rasenberg,
//                                 Nehal Mistry, and Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

void DrawTexture(const CGraphic *g, GLuint *textures, int sx, int sy,
	int ex, int ey, int x, int y, int flip)
{
	GLfloat stx, etx;
	GLfloat sty, ety;
	int texture;
	int minw, minh;
	int maxw, maxh;
	int i, j;
	int tw, th;
	int sx2, sy2;
	int ex2, ey2;
	int w, h;
	int x2, y2;
	int nextsx2, nextsy2;

	tw = ex / GLMaxTextureSize - sx / GLMaxTextureSize + 1;
	th = ey / GLMaxTextureSize - sy / GLMaxTextureSize + 1;

	x2 = x;
	y2 = y;

	sy2 = sy;
	for (j = 0; j < th; ++j) {
		minh = sy2 / GLMaxTextureSize * GLMaxTextureSize;
		maxh = std::min((const int)(minh + GLMaxTextureSize), g->GraphicHeight);
		if (sy > minh) {
			h = ey - sy;
		} else {
			h = ey - minh;
		}
		if (h > maxh) {
			h = maxh;
		}

		sx2 = sx;
		for (i = 0; i < tw; ++i) {
			minw = sx2 / GLMaxTextureSize * GLMaxTextureSize;
			maxw = std::min((const int)(minw + GLMaxTextureSize), g->GraphicWidth);
			if (sx > minw) {
				w = ex - sx;
			} else {
				w = ex - minw;
			}
			if (w > maxw) {
				w = maxw;
			}

			stx = (GLfloat)(sx2 - minw) / (maxw - minw);
			sty = (GLfloat)(sy2 - minh) / (maxh - minh);
			if (ex > maxw) {
				ex2 = maxw;
			} else {
				ex2 = ex;
			}
			etx = (GLfloat)(ex2 - sx2);
			if (maxw == g->GraphicWidth) {
				stx *= g->TextureWidth;
				etx = stx + etx / (maxw - minw) * g->TextureWidth;
			} else {
				etx = stx + (etx / GLMaxTextureSize);
			}
			if (ey > maxh) {
				ey2 = maxh;
			} else {
				ey2 = ey;
			}
			ety = (GLfloat)(ey2 - sy2);
			if (maxh == g->GraphicHeight) {
				sty *= g->TextureHeight;
				ety = sty + ety / (maxh - minh) * g->TextureHeight;
			} else {
				ety = sty + (ety / GLMaxTextureSize);
			}

			texture = sy2 / GLMaxTextureSize * ((g->GraphicWidth - 1) / GLMaxTextureSize + 1) +
				sx2 / GLMaxTextureSize;

			glBindTexture(GL_TEXTURE_2D, textures[texture]);
			glBegin(GL_QUADS);
			if (!flip) {
				glTexCoord2f(stx, sty);
				glVertex2i(x2, y2);
				glTexCoord2f(stx, ety);
				glVertex2i(x2, y2 + h);
				glTexCoord2f(etx, ety);
				glVertex2i(x2 + w, y2 + h);
				glTexCoord2f(etx, sty);
				glVertex2i(x2 + w, y2);
			} else {
				glTexCoord2f(stx, sty);
				glVertex2i(x + (ex - sx) - (x2 - x), y2);
				glTexCoord2f(stx, ety);
				glVertex2i(x + (ex - sx) - (x2 - x), y2 + h);
				glTexCoord2f(etx, ety);
				glVertex2i(x + (ex - sx) - (x2 + w - x), y2 + h);
				glTexCoord2f(etx, sty);
				glVertex2i(x + (ex - sx) - (x2 + w - x), y2);
			}
			glEnd();

			nextsx2 = (sx2 + GLMaxTextureSize) / GLMaxTextureSize * GLMaxTextureSize;
			x2 += nextsx2 - sx2;
			sx2 = nextsx2;
		}

		nextsy2 = (sy2 + GLMaxTextureSize) / GLMaxTextureSize * GLMaxTextureSize;
		y2 += nextsy2 - sy2;
		sy2 = nextsy2;
	}
}

//@}
