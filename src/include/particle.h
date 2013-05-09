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
/**@name particle.h - The base particle headerfile. */
//
//      (c) Copyright 2007-2008 by Jimmy Salmon and Francois Beerten
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


struct CPosition {
	CPosition(float x, float y) : x(x), y(y) {}
	float x;
	float y;
};

class GraphicAnimation
{
	CGraphic *g;
	int ticksPerFrame;
	int currentFrame;
	int currTicks;
public:
	GraphicAnimation(CGraphic *g, int ticksPerFrame);
	~GraphicAnimation() {}

	/**
	**  Draw the current frame of the animation.
	**  @param x x screen coordinate where to draw the animation.
	**  @param y y screen coordinate where to draw the animation.
	*/
	void draw(int x, int y);

	/**
	**  Update the animation.
	**  @param ticks the number of ticks elapsed since the last call.
	*/
	void update(int ticks);

	bool isFinished();
	bool isVisible(const CViewport &vp, const CPosition &pos);
	GraphicAnimation *clone();
};



// Base particle class
class CParticle
{
public:
	CParticle(CPosition position, int drawlevel = 0) :
		pos(position), destroyed(false), drawLevel(drawlevel)
	{}
	virtual ~CParticle() {}

	virtual bool isVisible(const CViewport &vp) const = 0;
	virtual void draw() = 0;
	virtual void update(int) = 0;

	inline void destroy() { destroyed = true; }
	inline bool isDestroyed() { return destroyed; }

	virtual CParticle *clone() = 0;

	int getDrawLevel() const { return drawLevel; }
	void setDrawLevel(int value) { drawLevel = value; }

protected:
	CPosition pos;
	bool destroyed;
	int drawLevel;
};


class StaticParticle : public CParticle
{
public:
	StaticParticle(CPosition position, GraphicAnimation *flame, int drawlevel = 0);
	virtual ~StaticParticle();

	virtual bool isVisible(const CViewport &vp) const;
	virtual void draw();
	virtual void update(int ticks);
	virtual CParticle *clone();

protected:
	GraphicAnimation *animation;
};


// Chunk particle
class CChunkParticle : public CParticle
{
public:
	CChunkParticle(CPosition position, GraphicAnimation *smokeAnimation, GraphicAnimation *debrisAnimation,
				   GraphicAnimation *destroyAnimation,
				   int minVelocity = 0, int maxVelocity = 400,
				   int minTrajectoryAngle = 77, int maxTTL = 0, int drawlevel = 0);
	virtual ~CChunkParticle();

	virtual bool isVisible(const CViewport &vp) const;
	virtual void draw();
	virtual void update(int ticks);
	virtual CParticle *clone();
	int getSmokeDrawLevel() const { return smokeDrawLevel; }
	int getDestroyDrawLevel() const { return destroyDrawLevel; }
	void setSmokeDrawLevel(int value) { smokeDrawLevel = value; }
	void setDestroyDrawLevel(int value) { destroyDrawLevel = value; }

protected:
	CPosition initialPos;
	int initialVelocity;
	float trajectoryAngle;
	int maxTTL;
	int nextSmokeTicks;
	int lifetime;
	int age;
	int minVelocity;
	int maxVelocity;
	int minTrajectoryAngle;
	float height;
	int smokeDrawLevel;
	int destroyDrawLevel;
	GraphicAnimation *debrisAnimation;
	GraphicAnimation *smokeAnimation;
	GraphicAnimation *destroyAnimation;

	struct {
		float x;
		float y;
	} direction;
};


// Smoke particle
class CSmokeParticle : public CParticle
{
public:
	CSmokeParticle(CPosition position, GraphicAnimation *animation, float speedx = 0, float speedy = -22.0f, int drawlevel = 0);
	virtual ~CSmokeParticle();

	virtual bool isVisible(const CViewport &vp) const;
	virtual void draw();
	virtual void update(int ticks);
	virtual CParticle *clone();

protected:
	GraphicAnimation *puff;
	struct {
		float x;
		float y;
	} speedVector;
};

class CRadialParticle : public CParticle
{
public:
	CRadialParticle(CPosition position, GraphicAnimation *animation, int maxSpeed, int drawlevel = 0);
	virtual ~CRadialParticle();

	virtual bool isVisible(const CViewport &vp) const;
	virtual void draw();
	virtual void update(int ticks);
	virtual CParticle *clone();

protected:
	GraphicAnimation *animation;
	float direction;
	int speed;
	int maxSpeed;
};


class CParticleManager
{
public:
	CParticleManager();
	~CParticleManager();

	static void init();
	static void exit();

	void prepareToDraw(const CViewport &vp, std::vector<CParticle *> &table);
	void endDraw();

	void update();

	void add(CParticle *particle);
	void clear();

	CPosition getScreenPos(const CPosition &pos) const;

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
