//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name sprite.cpp - The general sprite functions. */
//
//      (c) Copyright 2000-2011 by Lutz Sammer, Stephan Rasenberg,
//                                 Nehal Mistry, Jimmy Salmon and Pali Rohár
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

#if defined(USE_OPENGL) || defined(USE_GLES)

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/** Draw a rectangular part of a CGraphic to the screen.
**
**  This function does not attempt to clip the CGraphic based on the
**  screen coordinates.  If the caller wants clipping, it can set the
**  parameters accordingly, or perhaps configure OpenGL to clip the
**  output.
**
**  @param g
**    The graphic to be drawn.  It may consist of multiple
**    OpenGL textures if it is too large to fit in one texture.
**  @param textures
**    The OpenGL textures to be drawn.  There must be g->NumTextures
**    elements in the array.  These textures may be the same as
**    g->Textures, or perhaps variants of them with different colors
**    for a specific player.
**  @param gx_beg
**    X coordinate of the left side of the rectangle to be drawn from @a *g.
**  @param gy_beg
**    Y coordinate of the top of the rectangle to be drawn from @a *g.
**  @param gx_end
**    X coordinate of the right side of the rectangle to be drawn from @a *g.
**  @param gy_end
**    Y coordinate of the bottom of the rectangle to be drawn from @a *g.
**  @param sx_beg
**    X coordinate of the left side of the graphic on the screen.
**  @param sy_beg
**    Y coordinate of the top of the graphic on the screen.
**  @param flip
**    Whether to flip the graphic in the X direction.
**    In any case, the graphic will extend from @a sx_beg
**    to (@a gx_end - @a gx_beg + @a sx_beg) on the screen.
**    Flipping controls which of those values corresponds
**    to @a gx_beg and which one to @a gx_end.
*/
void DrawTexture(const CGraphic *g, GLuint *textures,
				 int gx_beg, int gy_beg, int gx_end, int gy_end,
				 int sx_beg, int sy_beg, int flip)
{
	// gx and gy coordinates count pixels from the top left corner
	//           of the CGraphic, which can span multiple textures.
	// tx and ty coordinates are in an individual texture,
	//           as GLfloats between 0.0 and 1.0, like OpenGL requires.
	// sx and sy coordinates are on the screen.  This function does not
	//           know what the origin is there.

	Assert(0 <= gx_beg);
	Assert(0 <= gy_beg);
	Assert(gx_beg <= gx_end); // draws nothing if equal
	Assert(gy_beg <= gy_end); // draws nothing if equal
	Assert(gx_end <= g->GraphicWidth);
	Assert(gy_end <= g->GraphicHeight);

	for (int tex_gy_beg = gy_beg / GLMaxTextureSize * GLMaxTextureSize;;
		 tex_gy_beg += GLMaxTextureSize) {
		int tex_gy_end = tex_gy_beg + GLMaxTextureSize;
		int clip_gy_beg = std::max<int>(gy_beg, tex_gy_beg);
		int clip_gy_end = std::min<int>(gy_end, tex_gy_end);
		if (clip_gy_beg >= clip_gy_end) {
			break;
		}

		int clip_sy_beg = clip_gy_beg - gy_beg + sy_beg;
		int clip_sy_end = clip_gy_end - gy_beg + sy_beg;
		Assert(clip_sy_end != clip_sy_beg);
		Assert(abs(clip_sy_end - clip_sy_beg) <= g->GraphicHeight);

		GLfloat clip_ty_beg, clip_ty_end;
		if (tex_gy_end >= g->GraphicHeight) {
			// This is the last row of textures in
			// the Y direction.  These textures may
			// be smaller than the ones at the top.
			clip_ty_beg = (clip_gy_beg - tex_gy_beg)
						  * g->TextureHeight
						  / (g->GraphicHeight - tex_gy_beg);
			clip_ty_end = (clip_gy_end - tex_gy_beg)
						  * g->TextureHeight
						  / (g->GraphicHeight - tex_gy_beg);
		} else {
			clip_ty_beg = (clip_gy_beg - tex_gy_beg)
						  / GLfloat(GLMaxTextureSize);
			clip_ty_end = (clip_gy_end - tex_gy_beg)
						  / GLfloat(GLMaxTextureSize);
		}
		Assert(0.0f <= clip_ty_beg);
		Assert(clip_ty_beg < clip_ty_end);
		Assert(clip_ty_end <= 1.0f);

		for (int tex_gx_beg = gx_beg / GLMaxTextureSize * GLMaxTextureSize;;
			 tex_gx_beg += GLMaxTextureSize) {
			int tex_gx_end = tex_gx_beg + GLMaxTextureSize;
			int clip_gx_beg = std::max<int>(gx_beg, tex_gx_beg);
			int clip_gx_end = std::min<int>(gx_end, tex_gx_end);
			if (clip_gx_beg >= clip_gx_end) {
				break;
			}

			// Flipping does not change which parts of the
			// CGraphic get drawn.  It only changes where
			// they get drawn.
			int clip_sx_beg, clip_sx_end;
			if (flip) {
				clip_sx_beg = sx_beg + (gx_end - clip_gx_beg);
				clip_sx_end = sx_beg + (gx_end - clip_gx_end);
			} else {
				clip_sx_beg = sx_beg + (clip_gx_beg - gx_beg);
				clip_sx_end = sx_beg + (clip_gx_end - gx_beg);
			}
			Assert(clip_sx_end != clip_sx_beg);
			Assert(abs(clip_sx_end - clip_sx_beg) <= g->GraphicWidth);

			GLfloat clip_tx_beg, clip_tx_end;
			if (tex_gx_end >= g->GraphicWidth) {
				// This is the last column of textures in
				// the X direction.  These textures may
				// be smaller than the ones at the left.
				clip_tx_beg = (clip_gx_beg - tex_gx_beg)
							  * g->TextureWidth
							  / (g->GraphicWidth - tex_gx_beg);
				clip_tx_end = (clip_gx_end - tex_gx_beg)
							  * g->TextureWidth
							  / (g->GraphicWidth - tex_gx_beg);
			} else {
				clip_tx_beg = (clip_gx_beg - tex_gx_beg)
							  / GLfloat(GLMaxTextureSize);
				clip_tx_end = (clip_gx_end - tex_gx_beg)
							  / GLfloat(GLMaxTextureSize);
			}
			Assert(0.0f <= clip_tx_beg);
			Assert(clip_tx_beg < clip_tx_end);
			Assert(clip_tx_end <= 1.0f);

			int texture = tex_gy_beg / GLMaxTextureSize
						  * ((g->GraphicWidth - 1) / GLMaxTextureSize + 1)
						  + tex_gx_beg / GLMaxTextureSize;
			Assert(texture >= 0 && texture < g->NumTextures);

			glBindTexture(GL_TEXTURE_2D, textures[texture]);

#ifdef USE_GLES
			float texCoord[] = {
				clip_tx_beg, clip_ty_beg,
				clip_tx_end, clip_ty_beg,
				clip_tx_beg, clip_ty_end,
				clip_tx_end, clip_ty_end
			};

			float vertex[] = {
				2.0f / (GLfloat)Video.Width *clip_sx_beg - 1.0f, -2.0f / (GLfloat)Video.Height *clip_sy_beg + 1.0f,
				2.0f / (GLfloat)Video.Width *clip_sx_end - 1.0f, -2.0f / (GLfloat)Video.Height *clip_sy_beg + 1.0f,
				2.0f / (GLfloat)Video.Width *clip_sx_beg - 1.0f, -2.0f / (GLfloat)Video.Height *clip_sy_end + 1.0f,
				2.0f / (GLfloat)Video.Width *clip_sx_end - 1.0f, -2.0f / (GLfloat)Video.Height *clip_sy_end + 1.0f
			};

			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnableClientState(GL_VERTEX_ARRAY);

			glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
			glVertexPointer(2, GL_FLOAT, 0, vertex);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
#endif

#ifdef USE_OPENGL
			glBegin(GL_QUADS);
			glTexCoord2f(clip_tx_beg, clip_ty_beg);
			glVertex2i(clip_sx_beg, clip_sy_beg);
			glTexCoord2f(clip_tx_beg, clip_ty_end);
			glVertex2i(clip_sx_beg, clip_sy_end);
			glTexCoord2f(clip_tx_end, clip_ty_end);
			glVertex2i(clip_sx_end, clip_sy_end);
			glTexCoord2f(clip_tx_end, clip_ty_beg);
			glVertex2i(clip_sx_end, clip_sy_beg);
			glEnd();
#endif

		}
	}
}

//@}

#endif
