#include ".\grafics.h"
#include ".\fps.h"


void Count_Frames()
{
    fps = frame_counter;
    frame_counter = 0;
}
END_OF_FUNCTION(Count_Frames);

void Count_Ticks()
{
    tick_counter++;
}
END_OF_FUNCTION(Count_Ticks);

void Fps_Init()
{
	if(!fps_inited)
	{
		LOCK_VARIABLE(fps);
		LOCK_VARIABLE(frame_counter);
		LOCK_FUNCTION(Count_Frames);
		install_int_ex(Count_Frames, BPS_TO_TIMER(1));

		LOCK_VARIABLE(tick_counter);
		LOCK_FUNCTION(Count_Ticks);
		install_int_ex(Count_Ticks, MSEC_TO_TIMER(1));

		fps_inited = true;
	}
}

CGrafics::CGrafics(void)
: m_ticks(1)
, m_debug(false)
,m_buffer(NULL)
{
}

CGrafics::~CGrafics(void)
{
}
CGrafics*CGrafics::m_instance=0;
CGrafics* CGrafics::GetInstance(void)
{
	if(!CGrafics::m_instance)
		CGrafics::m_instance= new CGrafics;

	return CGrafics::m_instance;
}

void CGrafics::DestroyInstance(void)
{	
	if(CGrafics::m_instance)
		delete CGrafics::m_instance;
}

int CGrafics::Init(int w, int h, int c, bool window)
{
	int card= (window)? GFX_AUTODETECT_WINDOWED:GFX_AUTODETECT_FULLSCREEN  ;
	set_color_depth(c);
	if(set_gfx_mode(card, w, h,0,0))
	{
		return 1;
	}
	Fps_Init();
	m_buffer= create_bitmap(w,h);
	if(!m_buffer)
		return 1;
	return 0;
}

void CGrafics::StartFrame(void)
{	
	m_ticks = tick_counter;
	tick_counter =1;
	
    acquire_bitmap(m_buffer);

	clear_to_color(m_buffer, makecol(0,100,200));
}

void CGrafics::EndFrame(void)
{	
	if(m_debug)
		textprintf(m_buffer,font,0,0,makecol(128,128,128),"w: %d, h: %d, fps: %d, ticks: %d", SCREEN_W,SCREEN_H,fps,m_ticks);
	
	release_bitmap(m_buffer);
	blit(m_buffer, screen,0,0,0,0,SCREEN_W, SCREEN_H);
	frame_counter++;
}
