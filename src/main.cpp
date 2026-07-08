#include <allegro.h>
#include <cstring>
#include <cstdlib>
#include "grafics.h"
#include "game.h"
#include "random.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
extern volatile int tick_counter;
static int g_step, g_prev, g_accum;
static void web_frame()
{
	al_web_pump();                       // SDL events -> input + tick_counter
	int now = tick_counter;
	g_accum += now - g_prev; g_prev = now;
	if (g_accum > 250) g_accum = 250;
	while (g_accum >= g_step && !GAME->m_exit) { GAME->Update(g_step); g_accum -= g_step; }
	GAME->Draw();
	if (GAME->m_exit) emscripten_cancel_main_loop();
}
#endif
int c=0;
bool check(int argc, const char *argv[], char *str)
{
	for(int i=0; i< argc;i++)
	{
		if(strcmp(argv[i],str)==0)
		{
			c=i;
			return true;
		}
	}
	return false;
}
int main(int argc, const  char *argv[])
{
	allegro_init();
	install_keyboard();
	install_mouse();
	bool windowed= check(argc,argv,"-windowed");
	GFX->m_debug=check(argc,argv,"-debug");
	int color=16;
	if(check(argc,argv,"-color"))
		color=atoi(argv[c+1]);

	GFX->Init(640,480,color,windowed);

	int fps=60;                                   // sim rate; movement speed scales with it
	if(check(argc,argv,"-fps"))
		fps=atoi(argv[c+1]);

	GAME->Init();
#ifdef __EMSCRIPTEN__
	g_step = fps>0 ? 1000/fps : 16;
	GFX->m_ticks = g_step;
	g_prev = tick_counter; g_accum = 0;
	emscripten_set_main_loop(web_frame, 0, 1);   // rAF-driven; never returns
#else
	GAME->Loop(fps>0 ? 1000/fps : 16);
#endif

	CGrafics::DestroyInstance();
	CGame::DestroyInstance();
	return 0;
}
END_OF_MAIN()