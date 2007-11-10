//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name particle.h - The base particle headerfile. */
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

#ifndef __PARTICLE_H__
#define __PARTICLE_H__

//@{

#include <vector>

class CGraphic;
class CViewport;


struct CPosition
{
	CPosition(float x, float y) : x(x), y(y) {}
	float x;
	float y;
};


// Base particle class
class CParticle
{
public:
	CParticle(CPosition position) :
		pos(position), destroyed(false)
	{}
	virtual ~CParticle() {}

	virtual void draw() {}
	virtual void update(int ticks) {}

	inline void destroy() { destroyed = true; }
	inline bool isDestroyed() { return destroyed; }

protected:
	CPosition pos;
	bool destroyed;
};


// Flame particle
class CFlameParticle : public CParticle
{
public:
	CFlameParticle(CPosition position);
	virtual ~CFlameParticle();

	static void init();
	static void exit();

	virtual void draw();
	virtual void update(int ticks);

protected:
	CGraphic *g;
	int frame;
	int numFrames;
	int currTicks;

	static CGraphic *large[];
	static CGraphic *medium[];
	static CGraphic *small[];
};


// Flash particle
class CFlashParticle : public CParticle
{
public:
	CFlashParticle(CPosition position);
	virtual ~CFlashParticle();

	static void init();
	static void exit();

	virtual void draw();
	virtual void update(int ticks);

protected:
	int frame;
	int currTicks;

	static CGraphic *flash;
	static int numFrames;
};	


// Chunk particle
class CChunkParticle : public CParticle
{
public:
	CChunkParticle(CPosition position);
	virtual ~CChunkParticle();

	static void init();
	static void exit();

	virtual void draw();
	virtual void update(int ticks);

protected:
	CPosition initialPos;
	int initialVelocity;
	float trajectoryAngle;
	int nextSmokeTicks;
	int lifetime;
	int age;
	float height;

	struct {
		float x;
		float y;
	} direction;
};


// Smoke particle
class CSmokeParticle : public CParticle
{
public:
	CSmokeParticle(CPosition position);
	virtual ~CSmokeParticle();

	static void init();
	static void exit();

	virtual void draw();
	virtual void update(int ticks);

protected:
	CGraphic *g;
	int frame;
	int currTicks;

	static int numFrames;
	static CGraphic *lightSmoke[];
	static CGraphic *darkSmoke[];
};


// Explosion system
class CExplosion : public CParticle
{
public:
	CExplosion(CPosition position);
	virtual ~CExplosion();

	static void init();
	static void exit();

	virtual void draw();
	virtual void update(int ticks);
};

enum {
	PARTICLE_NONE,
	PARTICLE_EXPLOSION,
};

class CParticleManager
{
public:
	CParticleManager();
	~CParticleManager();

	static void init();
	static void exit();

	void draw(const CViewport *vp);
	void update();

	void add(CParticle *particle);
	void add(int type, CPosition &pos);
	void clear();
	
	CPosition getScreenPos(const CPosition &pos);
	int getType(const std::string &name);

	inline void setLowDetail(bool detail) { lowDetail = detail; }
	inline bool getLowDetail() const { return lowDetail; }

private:
	std::vector<CParticle *> particles;
	std::vector<CParticle *> new_particles;
	const CViewport *vp;
	unsigned long lastTicks;
	bool lowDetail;
};

extern CParticleManager ParticleManager;

//@}

#endif // !__PARTICLE_H__
