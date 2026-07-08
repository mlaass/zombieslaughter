#include <allegro.h>
#include <cstring>
#include <cstdlib>
#include "grafics.h"
#include "game.h"
#include "random.h"
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
	GAME->Loop(fps>0 ? 1000/fps : 16);

	CGrafics::DestroyInstance();
	CGame::DestroyInstance();
	return 0;
}
END_OF_MAIN()