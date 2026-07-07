#pragma once

#include <allegro.h>
class CGrafics
{
private:	
	CGrafics(void);
	virtual ~CGrafics(void);

	static CGrafics *m_instance;
public:
	BITMAP * m_buffer;

	static CGrafics* GetInstance(void);
	static void DestroyInstance(void);
	int Init(int w, int h, int c, bool window);
	void StartFrame(void);
	void EndFrame(void);
	int m_ticks;
	bool m_debug;
};

#define GFX CGrafics::GetInstance()
#define GFXBUF CGrafics::GetInstance()->m_buffer