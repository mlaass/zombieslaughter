#pragma once
#include "vec2.h"
#include<list>
#include <allegro.h>

#define ET_PART 0
#define ET_HERO 1
#define ET_BULLET 2
#define ET_FLAME 3
#define ET_ZOMBIE 4
#define ET_ITEM 5

#define ST_NONE 0
#define ST_HEALTH 1
#define ST_FLAME 2

#define WP_MACH 1
#define WP_FLAME 0

#define MAX_FLAME 1200


#define LIFE_FLAME 2000
#define LIFE_PART 1000
class CEntity
{
public:
	CEntity(void);
	CEntity(vec2 p, vec2 lp, int t=ET_PART,int st=0, int c= makecol(30,40,10));
	~CEntity(void);
	vec2 m_pos;
	vec2 m_lpos;
	void Update(int ticks);
	vec2 m_accel;
	void Draw(void);
	int m_type;
	void DrawDbg(void);
private:
	void Integrate(int ticks);
	void DrawHero(void);
	void UpdateHero(int ticks);

public:
	void DrawBullet(void);
	bool m_kill;
	static int m_weapon;
	void DrawFlame(void);
	int m_life;
	bool m_air;
	void DrawZombie(void);
	void UpdateZombie(int ticks);
	vec2 GetPos(void);
	int m_health;
	int m_color;
	int m_subtype;
	void DrawItem(void);
};
using namespace std;
typedef list<CEntity*> listPEnt;
typedef listPEnt::iterator itPEnt;
