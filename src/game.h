#pragma once

#include "grafics.h"
#include "entity.h"
#include "data.h"
#include "vec2.h"
#include "land.h"
#include "ppcol.h"
#include "hiscore.h"

#define GRAVITY 300

#define GS_MENU 0
#define GS_PLAY 1
#define GS_HISCORE 2
#define GS_GAME_OVER 3
#define GS_HELP 4

#define MN_ITEMS 5

class CGame
{
private:
	CGame(void);
	~CGame(void);
	static CGame *m_instance;
public:

	char m_menitems[MN_ITEMS][64];
	DATAFILE* m_data;
	CLand m_land;

	static CGame* GetInstance(void);

	static void DestroyInstance(void);
	BITMAP* GetBitmap(int id);
	int Init(void);
	void Loop(int step=16);
	void Update(int ticks);
	void Draw(void);
	bool m_exit;
	listPEnt m_entitys;
	listPEnt m_parts;
	void DeInit(void);
	//worldboundaries
	float m_btop, m_bbottom,m_bleft,m_bright;
	vec2 m_offset;
	BITMAP* GetMask(int et);
	void DrawHud(void);
	int m_am_flame;
	void ParticleSplash(vec2 pos,int nr=100, int col=makecol(240,15,15), float sp=0.02);
	int m_state;
	int m_wave;
	int m_wavetime;
	int m_points;
	CHiscore m_hiscore;
	void PlaySnd(int id, bool loop=false, int vol=255, int pan=128, int freq=1000);
	void StopSnd(int id);
	void AdjustSnd(int id, bool loop=false, int vol=255, int pan=128, int freq=1000);
	int m_menindex;
	void Generate(void);
};
#define GAME CGame::GetInstance()