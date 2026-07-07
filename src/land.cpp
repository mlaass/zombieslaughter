#include "land.h"
#include "game.h"
#include <cstdlib>
#include <ctime>

CLand::CLand(void):
m_map(NULL)
{
}

CLand::~CLand(void)
{
}

int CLand::Init(int w, int h)
{
	m_map=create_bitmap(w,h);
	return 0;
}

void CLand::DeInit(void)
{	
	if(m_map)
		destroy_bitmap(m_map);
}

void CLand::Create(int seed)
{
	if(seed==0)
		srand(time(NULL));
	else
		srand(seed);

	int f1[9], f2[17], f3[33];
	int s1= m_map->w/8;
	int s2= m_map->w/16;
	int s3= m_map->w/32;

	int amp=(m_map->h/3)*2;

	for(int i=0; i< 9;i++)
		f1[i]=amp/3*2+rand()%(amp/3);
	for(int i=0; i< 17;i++)
		f2[i]=rand()%(amp/9);
	for(int i=0; i< 33;i++)
		f3[i]=rand()%(amp/27);

	BITMAP *tile=GAME->GetBitmap(BMP_TILE1),
		*top=GAME->GetBitmap(BMP_LND_TOP);
	for(int ix=0;ix<=(m_map->w/tile->w);ix++)
	{
		for(int iy=0;iy<=(m_map->h/tile->h);iy++)
		{
			draw_sprite(m_map, tile,ix*tile->w,iy*tile->h);
		}
	
	}
	for(int ix=0;ix<=15;ix++)
	{
		rotate_sprite(m_map,GAME->GetBitmap(BMP_BONE1),rand()%m_map->w,rand()%m_map->h, itofix(rand()%255));
		rotate_sprite(m_map,GAME->GetBitmap(BMP_BONE2),rand()%m_map->w,rand()%m_map->h, itofix(rand()%255));
		rotate_sprite(m_map,GAME->GetBitmap(BMP_BONE3),rand()%m_map->w,rand()%m_map->h, itofix(rand()%255));


	}

	//clear_to_color(m_map,makecol(150,90,20));
	BITMAP*nmap=create_bitmap(m_map->w,m_map->h);
	clear_to_color(nmap,makecol(0,255,0));
	int c=makecol(255,0,255);
	for(int x=0; x<m_map->w;x++)
	{	
		int val= Interpolate(f1[x/s1],f1[(x/s1)+1],x%s1,s1);
		val+= Interpolate(f2[x/s2],f2[(x/s2)+1],x%s2,s2);
		val+= Interpolate(f3[x/s3],f3[(x/s3)+1],x%s3,s3);

		vline(nmap,x, m_map->h-val,m_map->h,c);
		draw_sprite(nmap, top,x,m_map->h-val);
		
		
	}
	
	draw_sprite(m_map,nmap, 0,0);
	floodfill(m_map,0,0,makecol(255,0,255));
	for(int i=0; i< 9;i++)
	{
		//putpixel(m_map,i*s1,m_map->h-f1[i], makecol(255,0,0));
	}
	destroy_bitmap(nmap);
}

int CLand::Interpolate(int v1, int v2, int ds, int s)
{
	int dv= v2-v1;
	float m= ((float)dv)/((float)s);

	return v1+ (m*((float)ds));
}
