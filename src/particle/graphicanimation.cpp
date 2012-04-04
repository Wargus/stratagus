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
/**@name graphicanimation.cpp - The Graphic Animation class. */
//
//      (c) Copyright 2007-2008 by Francois Beerten
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

#include "stratagus.h"
#include "particle.h"
#include "video.h"

GraphicAnimation::GraphicAnimation(CGraphic *g, int ticksPerFrame) :
	g(g), ticksPerFrame(ticksPerFrame), currentFrame(0), currTicks(0)
{
	Assert(g);
}


void GraphicAnimation::draw(int x, int y)
{
	if (!isFinished()) {
		g->DrawFrameClip(currentFrame, x - g->Width / 2, y - g->Height / 2);
	}
}

void GraphicAnimation::update(int ticks)
{
	currTicks += ticks;
	while (currTicks > ticksPerFrame) {
		currTicks -= ticksPerFrame;
		++currentFrame;
	}
}

bool GraphicAnimation::isFinished()
{
	return currentFrame >= g->NumFrames;
}

Animation *GraphicAnimation::clone()
{
	return new GraphicAnimation(g, ticksPerFrame);
}

//@}
