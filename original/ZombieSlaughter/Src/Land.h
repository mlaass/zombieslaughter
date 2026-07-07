#pragma once

#include "grafics.h"

class CLand
{
public:
	CLand(void);
	~CLand(void);

	BITMAP* m_map;
	int Init(int w, int h);
	void DeInit(void);
	void Create(int seed=0);
	int Interpolate(int v1, int v2, int ds, int s);
};
