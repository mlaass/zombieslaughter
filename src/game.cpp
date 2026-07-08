#include "game.h"
#include "grafics.h"
#include "ppcol.h"
#include <string>
#include <cstring>

CGame::CGame(void)
: m_exit(false),
m_data(NULL)
, m_btop(-200),
m_bbottom(SCREEN_H),
m_bleft(0),
m_bright(SCREEN_W),
m_offset(0,0)
, m_am_flame(MAX_FLAME)
, m_state(0)
, m_wave(0)
, m_wavetime(20000)
, m_points(0)
, m_menindex(0)
{
	strcpy(m_menitems[0], "Play");
	strcpy(m_menitems[1], "Generate");
	strcpy(m_menitems[2], "Hiscore");
	strcpy(m_menitems[3], "Help");
	strcpy(m_menitems[4], "Exit");
}

CGame::~CGame(void)
{
}
CGame* CGame::m_instance=0;
CGame* CGame::GetInstance(void)
{
	if(!CGame::m_instance)
		CGame::m_instance= new CGame;

	return CGame::m_instance;
}




void CGame::DestroyInstance(void)
{
	if(CGame::m_instance)
		delete CGame::m_instance;
}

BITMAP* CGame::GetBitmap(int id)
{
	return (BITMAP*)m_data[id].dat;
}

int CGame::Init(void)
{
	// Route Allegro's digi output through the "pulse" ALSA PCM (PulseAudio/
	// PipeWire) instead of the raw hardware "default" device. On a modern
	// desktop "default" resolves to dmix, which fails ("unable to create IPC
	// semaphore") when the display manager already holds the card — so the
	// original DIGI_AUTODETECT produced no sound at all.
	// ponytail: hardcoded to "pulse"; a user ~/.allegrorc could override it
	// only if this went through set_config_file instead of override.
	{
		const char *cfg = "[sound]\nalsa_device=pulse\n";
		override_config_data(cfg, strlen(cfg));
	}
	if(install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL)!=0)
			install_sound(DIGI_NONE, MIDI_NONE, NULL);
	
	m_data=load_datafile("data.dat");
	m_bright=SCREEN_W*3;
	m_land.Init(m_bright,SCREEN_H);
	m_land.Create();
	m_entitys.push_back(new CEntity(vec2(0.5*m_bright,0),vec2(0.5*m_bright,0),ET_HERO));
	set_pallete((RGB*)m_data[AAAPAL].dat);

	PlaySnd(SMP_GRAVES, true);
	return 0;
}

void CGame::Loop(void)
{
	while(!m_exit)
	{
		Update(GFX->m_ticks);
		Draw();
	}
}
int zombiespwn=1000, itemspwn=6000;
int zmbspwntime=3000;
int itmspwntime=12000;
int sndspd=1000;
bool kear=false;

int pl_au=0;
std::string input="";
bool enter=false;
bool muted=false;
void CGame::Update(int ticks)
{
	/*if(key[KEY_G])
		m_points++;
	if(key[KEY_T])
		m_state=GS_GAME_OVER;
*/
	if(m_state!=GS_GAME_OVER)
	{
			if(key[KEY_M])
		{
			muted=true;	
			StopSnd(SMP_GRAVES);
		}
		if(key[KEY_P]&& muted)
		{
			PlaySnd(SMP_GRAVES, true, 255,128,sndspd);
			muted=false;
		}
	}

	if(m_state==GS_GAME_OVER)
	{
		
		if(m_hiscore.Worthy(m_points))
		{
			enter=false;
			while(keypressed() && !enter)
			{
				
				int x= readkey();
				char s = x >> 8; //scancode
				char k = x & 0xff;//ascii code
				
				if(s==KEY_ENTER)
					enter =true;
				else if(s==KEY_BACKSPACE)
				{
					if(input.length()>=1)
						input.erase(input.length()-1,input.length());
				}
				else
				{
					input.push_back(k);
					
					while(input.length() >= 32)
						input.erase(0,1);
				}
			}
			if(enter||key[KEY_ENTER])
			{
			
				m_hiscore.Add(m_points,m_wave+1,input.c_str());
				Generate();
				m_state=GS_HISCORE;
			}
		}else
		{
			m_state=GS_HISCORE;
			Generate();
		}
		
		
	}
		
	if(m_state==GS_HISCORE)
	{
		if(key[KEY_ESC])
		{
			m_state= GS_MENU;
		}
	}
	if(m_state==GS_HELP)
	{
		if(key[KEY_ESC])
		{
			m_state= GS_MENU;
		}
	}
	if(m_state==GS_MENU)
	{

		if(key[KEY_UP]&&!kear)
		{
			m_menindex--;
			kear=true;
		}
		else if(key[KEY_DOWN]&&!kear)
		{
			m_menindex++;
			kear=true;
		}else if(!key[KEY_DOWN]&&!key[KEY_UP])
			kear=false;
		if(m_menindex<0)
			m_menindex= MN_ITEMS-1;
		if(m_menindex>MN_ITEMS-1)
			m_menindex= 0;
			
		if(key[KEY_ENTER])
		{
			
			switch(m_menindex)
			{
			case 0:m_state=GS_PLAY;
			AdjustSnd(SMP_GRAVES,true, 128);
				break;
			case 1:Generate();				
				break;
			case 2:
				m_state=GS_HISCORE;
				break;
			case 3:
				m_state=GS_HELP;
				break; 
			case 4:
				m_exit=true;
				break;
			}
		}

		if(key[KEY_LEFT])
			m_offset.x+= ticks;
		if(key[KEY_RIGHT])
			m_offset.x-= ticks;
		
	}

	if(m_state==GS_PLAY)
	{
		pl_au-=ticks;

		if (m_wavetime<=0)
		{
			m_wavetime=20000;
			m_wave++;
			zmbspwntime*=0.9;
			zmbspwntime*=0.999;
			sndspd*=1.02;
			AdjustSnd(SMP_GRAVES, true, 128,128,sndspd);
		}
		else
			m_wavetime-=ticks;

		poll_mouse();
		if(key[KEY_ESC])
		{
			m_state=GS_MENU;
			AdjustSnd(SMP_GRAVES, true, 255, 128,sndspd);
		}
		if(zombiespwn<0)
		{
			zombiespwn=zmbspwntime;
			m_entitys.push_back(new CEntity(vec2(rand()%(int)m_bright,0),vec2(rand()%(int)m_bright,0), ET_ZOMBIE));
		}
		else
			zombiespwn-=ticks;
		if(itemspwn<0)
		{
			itemspwn=itmspwntime;
			m_entitys.push_back(new CEntity(vec2(rand()%(int)m_bright,0),vec2(rand()%(int)m_bright,0), ET_ITEM, (rand()%20>=10)?ST_HEALTH:ST_FLAME));
		}
		else
			itemspwn-=ticks;
		
		for(itPEnt i= m_entitys.begin();i!=m_entitys.end();++i)
		{
			if((*i))
			{
				
				(*i)->Update(ticks);
				
				for(itPEnt j= m_entitys.begin();j!=m_entitys.end();++j)
				{	
					if((*j))
					{
					
						int t1=(*i)->m_type,t2=(*j)->m_type;
						BITMAP *spr1=GetMask(t1),
							*spr2=GetMask(t2);
						vec2 p1=(*i)->GetPos(),
							p2=(*j)->GetPos();
						if(check_pp_collision(spr1,spr2,p1.x,p1.y,p2.x,p2.y))
						{
							if((t1==ET_BULLET||t1==ET_FLAME) && t2==ET_ZOMBIE)
							{
								int th=(*i)->m_health;
								
									(*i)->m_health-=(*j)->m_health;
								if(th>0)
								{
									(*j)->m_health-=th;

									if((*j)->m_health<=0)
									{
										m_points+=m_wave+1;
										GAME->PlaySnd(SMP_EXPL);
									}
								}
								if(pl_au<=0)
								{
									GAME->PlaySnd(SMP_AUO);
									pl_au=200;
								}
									

								

								ParticleSplash(p1,3);
							}
							

							if((t1==ET_HERO) && t2==ET_ZOMBIE)
							{
								
								(*i)->m_health-=5;
								(*j)->m_health=0;
								ParticleSplash(p2+vec2(15,30),50,makecol(240,180,0),0.04);
								GAME->PlaySnd(SMP_EXPL);
							}
							if((t1==ET_HERO) && t2==ET_ITEM && !(*j)->m_kill)
							{
								if((*j)->m_subtype == ST_HEALTH)
								{
									(*i)->m_health+=10;
									(*i)->m_health=MIN((*i)->m_health,100);
									(*j)->m_kill=true;
									GAME->PlaySnd(SMP_AH);
								
								}else if((*j)->m_subtype == ST_FLAME)
								{
									m_am_flame+=50;
									m_am_flame=MIN(m_am_flame,MAX_FLAME);
									GAME->PlaySnd(SMP_CLICK);
									(*j)->m_kill=true;
								}

							}
						}
					}
					
				}
				vec2 &pos=(*i)->m_pos;
				vec2 &lpos=(*i)->m_lpos;
				
				(*i)->m_accel+= vec2(0,GRAVITY);

				float fric=0.999;
				if(pos.x>m_bright)
				{
					lpos.x= pos.x*fric; 
					pos.x=m_bright;
					
				}
				if(pos.x<m_bleft)
				{
					lpos.x= pos.x*fric; 
					pos.x=m_bleft;
					
				}
				if(pos.y>m_bbottom)
				{
					lpos.y= pos.y*fric; 
					pos.y=m_bbottom;
					
				}
				if(pos.y<m_btop)
				{
					lpos.y= pos.y*fric; 
					pos.y=m_btop;
					
				}
				if((*i)->m_kill)
				{
					delete (*i);
					(*i)=NULL;
				}
			}
		}
		for(itPEnt k= m_parts.begin();k!=m_parts.end();++k)
		{
			if((*k))
			{
				(*k)->Update(ticks);
				vec2 &pos=(*k)->m_pos;
				vec2 &lpos=(*k)->m_lpos;
				
				(*k)->m_accel+= vec2(0,GRAVITY);

				float fric=0.999;
				if(pos.x>m_bright)
				{
					lpos.x= pos.x*fric; 
					pos.x=m_bright;
					
				}
				if(pos.x<m_bleft)
				{
					lpos.x= pos.x*fric; 
					pos.x=m_bleft;
					
				}
				if(pos.y>m_bbottom)
				{
					lpos.y= pos.y*fric; 
					pos.y=m_bbottom;
					
				}
				if(pos.y<m_btop)
				{
					lpos.y= pos.y*fric; 
					pos.y=m_btop;
					
				}
				if((*k)->m_kill)
				{
					delete (*k);
					(*k)=NULL;
				}
			}
		}
	}
	m_offset.x=MIN(0,m_offset.x);
	m_offset.x=MAX(m_offset.x,-(m_bright-SCREEN_W));
	m_parts.remove(NULL);
	m_entitys.remove(NULL);
	poll_keyboard();
	clear_keybuf();
	
}

void CGame::Draw(void)
{
	GFX->StartFrame();

	draw_sprite(GFXBUF,m_land.m_map,m_offset.x,m_offset.y);

	for(itPEnt i= m_entitys.begin();i!=m_entitys.end();++i)
	{
		(*i)->Draw();
	}
	
	for(itPEnt k= m_parts.begin();k!=m_parts.end();++k)
	{
		(*k)->Draw();
	}
	if(m_state==GS_GAME_OVER)
	{
		textprintf_centre(GFXBUF,(FONT*)m_data[FNT_BIG].dat,SCREEN_W/2,50,-1,"GAME OVER!");
		textprintf_centre(GFXBUF,(FONT*)m_data[FNT_BIG].dat,SCREEN_W/2,120,-1,"enter your name:");
		textprintf_centre(GFXBUF,(FONT*)m_data[FNT_SMALL].dat,SCREEN_W/2,SCREEN_H/2-20,-1,"%s",input.c_str());

		textprintf_centre(GFXBUF,font,SCREEN_W/2,SCREEN_H -16,makecol(255,255,255),"All Sound,Code,Grafics (c) Moritz Laass 2004 visit http://chillart.cjb.net");


	}
	if(m_state==GS_HELP)
	{
		draw_sprite(GFXBUF,GetBitmap(BMP_HELP),0,0);
		textprintf_centre(GFXBUF,font,SCREEN_W/2,SCREEN_H -16,makecol(255,255,255),"All Sound,Code,Grafics (c) Moritz Laass 2004 visit http://chillart.cjb.net");

	}
	if(m_state==GS_HISCORE)
	{
		textprintf(GFXBUF,(FONT*)m_data[FNT_BIG].dat,50,50,-1,"Points");
		textprintf_centre(GFXBUF,(FONT*)m_data[FNT_BIG].dat,SCREEN_W/2,50,-1,"Name");
		textprintf_right(GFXBUF,(FONT*)m_data[FNT_BIG].dat,SCREEN_W-50,50,-1,"Stage");

		textprintf_centre(GFXBUF,font,SCREEN_W/2,SCREEN_H -16,makecol(255,255,255),"All Sound,Code,Grafics (c) Moritz Laass 2004 visit http://chillart.cjb.net");


		for(int i=0; i<HS_MAX_ENTRYS;i++)
		{
			int p=m_hiscore.m_entrys[i].m_points;
			char *n=m_hiscore.m_entrys[i].m_name;
			int st=m_hiscore.m_entrys[i].m_stage;
			textprintf(GFXBUF,(FONT*)m_data[FNT_SMALL].dat,100,150+ i*30,-1,"%.5d",p);
			textprintf_centre(GFXBUF,(FONT*)m_data[FNT_SMALL].dat,SCREEN_W/2,150+ i*30,-1,"%s",n);
			textprintf_right(GFXBUF,(FONT*)m_data[FNT_SMALL].dat,SCREEN_W-100,150+ i*30,-1,"%.2d",st);

		}

	}
	if(m_state==GS_PLAY)
	{
		draw_sprite(GFXBUF, this->GetBitmap(BMP_CROSSHAIR),mouse_x-7,mouse_y-7);
		DrawHud();
		if(m_wavetime>20000-2000)
			textprintf_centre(GFXBUF,(FONT*)m_data[FNT_BIG].dat,SCREEN_W/2,60,-1,"STAGE %d",m_wave+1);

	}
	else if(m_state==GS_MENU)
	{
		text_mode(-1);
		//draw_sprite(GFXBUF,GetBitmap(BMP_TITLE),0,0);
		textprintf_centre(GFXBUF,font,SCREEN_W/2,SCREEN_H/2 -160,makecol(190,210,245),"Arrow Buttons for Preview Scrolling");
		textprintf_centre(GFXBUF,font,SCREEN_W/2,SCREEN_H/2 -140,makecol(180,200,235),"Use M and P to mute/play Music");
		textprintf_centre(GFXBUF,(FONT*)m_data[FNT_BIG].dat,SCREEN_W/2,10,-1,"ZOMBIE SLAUGHTER");
		
		textprintf_centre(GFXBUF,font,SCREEN_W/2,SCREEN_H -16,makecol(255,255,255),"All Sound,Code,Grafics (c) Moritz Laass 2004 visit http://chillart.cjb.net");
		for(int i=0; i<MN_ITEMS;i++)
		{
			if(i==m_menindex)
				textprintf_centre(GFXBUF,(FONT*)m_data[FNT_BIG].dat,SCREEN_W/2,150+ 50*i,-1,"-%s-",m_menitems[i]);
			else
				textprintf_centre(GFXBUF,(FONT*)m_data[FNT_BIG].dat,SCREEN_W/2,150+ 50*i,-1,"%s",m_menitems[i]);
		}


	}
	if(GFX->m_debug)
		textprintf(GFXBUF,font,0,12,makecol(255,128,128),"ox: %d, oy: %d, br: %d",(int)m_offset.x,(int)m_offset.y,(int) m_bright);
	
	GFX->EndFrame();
}

void CGame::DeInit(void)
{
	for(itPEnt i= m_entitys.begin();i!=m_entitys.end();++i)
	{
		delete(*i);
		(*i)=NULL;
	}
	for(itPEnt k= m_parts.begin();k!=m_parts.end();++k)
	{
		delete(*k);
		(*k)=NULL;
	}
	m_entitys.clear();
	m_parts.clear();

	unload_datafile(m_data);
	m_land.DeInit();
}

BITMAP* CGame::GetMask(int et)
{
	switch(et)
	{
	case ET_PART:
		return GetBitmap(BMP_BULLET);
		break;
	case ET_ITEM:
		return GetBitmap(BMP_ITEM);
		break;
	case ET_BULLET:
		return GetBitmap(BMP_BULLET);
		break;
	case ET_FLAME:
		return GetBitmap(BMP_FLAME_MASK);
		break;
	case ET_HERO:
		return GetBitmap(BMP_ZOMBIE_MASK);
		break;
	case ET_ZOMBIE:
		return GetBitmap(BMP_ZOMBIE_MASK);
		break;
	}
	return NULL;
}

void CGame::DrawHud(void)
{
	draw_sprite(GFXBUF,GetBitmap(BMP_HUD),4,3);
	int a= 4+CEntity::m_weapon*32;
	draw_sprite(GFXBUF,GetBitmap(BMP_HUD_CUR),a,3);
	int b=25;
	if(CEntity::m_weapon==WP_FLAME && m_am_flame>0)
		b=((float)m_am_flame*(1.0f/(MAX_FLAME/25)));
	else if(m_am_flame<=0)
		b=0;
	
	rectfill(GFXBUF,4+a,41,4+a+b, 45, makecol(100,180,250));
	rectfill(GFXBUF,SCREEN_W-5,5,SCREEN_W-15-100, 45, makecol(200,50,50));
	rectfill(GFXBUF,SCREEN_W-10,10,SCREEN_W-10-m_entitys.front()->m_health, 40, makecol(255,100,100));

	rectfill(GFXBUF,345,5,450, 45, makecol(45,125,200));
	rectfill(GFXBUF,350,10,350+(95*(float)m_wavetime/20000.0f), 40, makecol(95,175,250));

	textprintf(GFXBUF,(FONT*)m_data[FNT_BIG].dat,135,5,-1,"%d",m_wave+1);
	textprintf(GFXBUF,(FONT*)m_data[FNT_SMALL].dat,205,18,-1,"%.5d",m_points);


	if(GFX->m_debug)
		textprintf(GFXBUF,font,8,46,makecol(255,128,128),"%d",m_am_flame);
	

}

void CGame::ParticleSplash(vec2 pos,int nr, int col, float sp)
{
	for(int i=0; i< nr;i++)
	{
		vec2 rp= vec2(sp*(50-rand()%100),sp*(40-rand()%100));
		m_parts.push_back(new CEntity(pos+rp,pos,0, ET_PART, col));
	}
}

void CGame::PlaySnd(int id, bool loop, int vol, int pan, int freq)
{
	play_sample((SAMPLE*)m_data[id].dat,vol,pan,freq,(loop)?TRUE:FALSE);
}

void CGame::StopSnd(int id)
{
	stop_sample((SAMPLE*)m_data[id].dat);
}

void CGame::AdjustSnd(int id, bool loop, int vol, int pan, int freq)
{
	adjust_sample((SAMPLE*)m_data[id].dat,vol,pan,freq,(loop)?TRUE:FALSE);
}

void CGame::Generate(void)
{
		m_land.Create(0);
		itPEnt i= m_entitys.begin();
		++i;
		for(;i!=m_entitys.end();++i)
		{
			if((*i))
				delete (*i);
			(*i)=NULL;
		}
		if(!m_parts.empty())
			for(i=m_parts.begin();i!=m_parts.end();++i)
			{
				if(*i)
					delete (*i);
				(*i)=NULL;
			}
		m_parts.remove(NULL);
		m_entitys.remove(NULL);
		m_wave=0;
		m_wavetime=20000;
		zmbspwntime=3000;
		sndspd=1000;
		AdjustSnd(SMP_GRAVES, true, 255,128,sndspd);
		itmspwntime=12000;
		m_points=0;
		m_entitys.front()->m_pos=vec2(0.5*m_bright,0);
		m_entitys.front()->m_lpos=vec2(0.5*m_bright,0);
		m_entitys.front()->m_health=100;
		m_am_flame=MAX_FLAME;
}
