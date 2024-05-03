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
#include <memory>

class CGraphic;
class CViewport;


struct CPosition {
	CPosition(float x, float y) : x(x), y(y) {}
	float x;
	float y;
};

class GraphicAnimation
{
	const CGraphic *g;
	int ticksPerFrame;
	int currentFrame = 0;
	int currTicks = 0;
public:
	GraphicAnimation(const CGraphic &g, int ticksPerFrame);
	~GraphicAnimation() = default;

	/**
	**  Draw the current frame of the animation.
	**  @param x x screen coordinate where to draw the animation.
	**  @param y y screen coordinate where to draw the animation.
	*/
	void draw(int x, int y) const;

	/**
	**  Update the animation.
	**  @param ticks the number of ticks elapsed since the last call.
	*/
	void update(int ticks);

	bool isFinished() const;
	bool isVisible(const CViewport &vp, const CPosition &pos) const;
	GraphicAnimation *clone() const;
};



// Base particle class
class CParticle
{
public:
	CParticle(CPosition position, int drawlevel = 0) : pos(position), drawLevel(drawlevel) {}
	virtual ~CParticle() {}

	virtual CParticle *clone() const = 0;
	virtual void draw() = 0;
	virtual bool isVisible(const CViewport &vp) const = 0;
	virtual void update(int) = 0;

	void destroy() { destroyed = true; }
	bool isDestroyed() const { return destroyed; }

	int getDrawLevel() const { return drawLevel; }
	void setDrawLevel(int value) { drawLevel = value; }

protected:
	CPosition pos;
	bool destroyed = false;
	int drawLevel = 0;
};


class StaticParticle : public CParticle
{
public:
	StaticParticle(CPosition position, const GraphicAnimation &flame, int drawlevel = 0);
	~StaticParticle() override = default;

	CParticle *clone() const override;
	void draw() override;
	bool isVisible(const CViewport &vp) const override;
	void update(int ticks) override;

protected:
	std::unique_ptr<GraphicAnimation> animation;
};


// Chunk particle
class CChunkParticle : public CParticle
{
public:
	CChunkParticle(CPosition position,
	               const GraphicAnimation &smokeAnimation,
	               const GraphicAnimation &debrisAnimation,
	               const GraphicAnimation &destroyAnimation,
	               int minVelocity = 0,
	               int maxVelocity = 400,
	               int minTrajectoryAngle = 77,
	               int maxTTL = 0,
	               int drawlevel = 0);
	~CChunkParticle() override = default;

	CParticle *clone() const override;
	void draw() override;
	bool isVisible(const CViewport &vp) const override;
	void update(int ticks) override;

	int getSmokeDrawLevel() const { return smokeDrawLevel; }
	int getDestroyDrawLevel() const { return destroyDrawLevel; }
	void setSmokeDrawLevel(int value) { smokeDrawLevel = value; }
	void setDestroyDrawLevel(int value) { destroyDrawLevel = value; }

protected:
	CPosition initialPos;
	int initialVelocity = 0;
	float trajectoryAngle = 0;
	int maxTTL = 0;
	int nextSmokeTicks = 0;
	int lifetime = 0;
	int age = 0;
	int minVelocity = 0;
	int maxVelocity = 0;
	int minTrajectoryAngle = 0;
	float height = 0;
	int smokeDrawLevel = 0;
	int destroyDrawLevel = 0;
	std::unique_ptr<GraphicAnimation> debrisAnimation;
	std::unique_ptr<GraphicAnimation> smokeAnimation;
	std::unique_ptr<GraphicAnimation> destroyAnimation;

	struct {
		float x;
		float y;
	} direction;
};


// Smoke particle
class CSmokeParticle : public CParticle
{
public:
	CSmokeParticle(CPosition position, const GraphicAnimation &animation, float speedx = 0, float speedy = -22.0f, int drawlevel = 0);
	~CSmokeParticle() override = default;

	CParticle *clone() const override;
	void draw() override;
	bool isVisible(const CViewport &vp) const override;
	void update(int ticks) override;

protected:
	std::unique_ptr<GraphicAnimation> puff;
	struct {
		float x;
		float y;
	} speedVector;
};

class CRadialParticle : public CParticle
{
public:
	CRadialParticle(CPosition position, const GraphicAnimation &animation, int maxSpeed, int drawlevel = 0);
	~CRadialParticle() override = default;

	CParticle *clone() const override;
	void draw() override;
	bool isVisible(const CViewport &vp) const override;
	void update(int ticks) override;

protected:
	std::unique_ptr<GraphicAnimation> animation;
	float direction;
	int speed;
	int maxSpeed;
};


class CParticleManager
{
public:
	CParticleManager() = default;
	~CParticleManager() = default;

	CParticleManager(const CParticleManager &) = delete;

	static void init();
	static void exit();

	std::vector<CParticle *> prepareToDraw(const CViewport &);
	void endDraw();

	void update();

	void add(CParticle* particle); // For tolua++

	void add(std::unique_ptr<CParticle> particle);
	void clear();

	CPosition getScreenPos(const CPosition &pos) const;

	void setLowDetail(bool detail) { lowDetail = detail; }
	bool getLowDetail() const { return lowDetail; }

private:
	std::vector<std::unique_ptr<CParticle>> particles;
	std::vector<std::unique_ptr<CParticle>> new_particles;
	const CViewport *vp = nullptr;
	unsigned long lastTicks = 0;
	bool lowDetail = false;
};

extern CParticleManager ParticleManager;

//@}

#endif // !__PARTICLE_H__
