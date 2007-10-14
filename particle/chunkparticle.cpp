//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name chunkparticle.cpp - The chunk particle. */
//
//      (c) Copyright 2007 by Jimmy Salmon
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
//      $Id: commands.cpp 9147 2007-09-28 17:18:24Z jim4 $

//@{

#include "stratagus.h"
#include "particle.h"


CChunkParticle::CChunkParticle(CPosition position) :
	CParticle(position)
{
	CSmokeParticle *smoke = new CSmokeParticle(position);
	ParticleManager.add(smoke);
}

CChunkParticle::~CChunkParticle()
{
}

void CChunkParticle::init()
{
	// FIXME: load graphic
}

void CChunkParticle::exit()
{
	// FIXME: free graphic
}

void CChunkParticle::draw()
{
	// FIXME: draw particle
}

void CChunkParticle::update(int ticks)
{
	// FIXME: move in an arc, periodically add smoke
}

//@}
