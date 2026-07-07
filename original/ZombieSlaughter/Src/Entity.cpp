#include ".\entity.h"
#include "game.h"
#include "fblend.h"
#include <math.h>

int CEntity::m_weapon=0;
CEntity::CEntity(void):
  m_pos(0,0)
, m_lpos(0,0)
, m_accel(0,0)
, m_type(0)
, m_kill(false)
, m_life(0)
, m_air(false)
, m_health(1)
, m_color(0)
, m_subtype(0)
  {
	switch(m_type)
	{
	case ET_PART:
		m_health=1;
		break;
	case ET_BULLET:
		m_health=2;
		break;
	case ET_FLAME:
		m_health=4;
		break;
	case ET_HERO:
		m_health=100;
		break;
	case ET_ZOMBIE:
		m_health=8;
		break;
	}
}

CEntity::CEntity(vec2 p, vec2 lp, int t,int st, int c):
m_pos(p)
, m_lpos(lp)
, m_accel(0,0)
, m_type(t)
, m_kill(false)
, m_life(0)
, m_color(c)
, m_subtype(st)
{
	switch(t)
	{
	case ET_PART:
		m_health=1;
		break;
	case ET_ITEM:
		m_health=1;
		break;
	case ET_BULLET:
		m_health=3;
		break;
	case ET_FLAME:
		m_health=4;
		break;
	case ET_HERO:
		m_health=100;
		break;
	case ET_ZOMBIE:
		m_health=16;
		break;
	}
}

CEntity::~CEntity(void)
{
}

void CEntity::Update(int ticks)
{
	Integrate(ticks);
	BITMAP*spr;
	BITMAP*lnd=GAME->m_land.m_map;

	vec2 d= m_pos+GAME->m_offset;
	vec2 of= GAME->m_offset;

	if (m_health<=0)
	{
		m_kill=true;
		switch(m_type)
		{
		case ET_ZOMBIE:
			GAME->ParticleSplash(GetPos()+vec2(10,20),60, makecol(240,10,1), 0.04);
			circlefill(GAME->m_land.m_map,GetPos().x+15,GetPos().y+60, 30,makecol(255,0,255));
			break;
		case ET_HERO:
			m_kill=false;
			break;
		}
	}
	switch(m_type)
	{
	case ET_PART:
		spr=GAME->GetMask(m_type);
		if(check_pp_collision(spr,lnd,d.x,d.y-1,of.x,of.y))
		{
			circlefill(lnd,d.x-of.x,d.y-of.y,1,makecol(230+ rand()%15,rand()%15,rand()%15));
			m_kill=true;

		}
		if(m_life>LIFE_PART)
			m_kill=true;
		break;
	case ET_ITEM:
		spr=GAME->GetMask(m_type);
		m_lpos.x=m_pos.x;
		if(check_pp_collision(spr,lnd,d.x,d.y-1,of.x,of.y))
		{
			m_lpos.y=m_pos.y*0.99;
			m_pos.y-=check_pp_collision_amount(spr,lnd,d.x,d.y-1,of.x,of.y,PPCAMOUNT_UP);
		}
		break;
	case ET_ZOMBIE: 
		UpdateZombie(ticks);
		break;
	case ET_HERO :
		UpdateHero(ticks);
		break;
	case ET_FLAME:
		spr=GAME->GetBitmap(BMP_FLAME_MASK);
		if(check_pp_collision(spr,lnd,d.x,d.y,of.x,of.y))
		{
			m_kill=true;
			circlefill(lnd,d.x+ spr->w/2-of.x,d.y+spr->h/2-of.y,12,makecol(255,0,255));
		}
		m_accel+=vec2(0,-(GRAVITY+30+rand()%30));
		if(m_life>LIFE_FLAME)
			m_kill=true;
		break;
	case ET_BULLET:
		spr=GAME->GetBitmap(BMP_BULLET);		
		if(check_pp_collision(spr,lnd,d.x,d.y,of.x,of.y))
		{
			m_kill=true;
			circlefill(lnd,d.x-of.x,d.y-of.y,7,makecol(255,0,255));
		}
		break;
	}
	m_life+=ticks;
}

void CEntity::Draw(void)
{
	vec2 d= m_pos+GAME->m_offset;
	vec2 ld= m_lpos+GAME->m_offset;
	if(GFX->m_debug)
		DrawDbg();

	switch(m_type)
	{
	case ET_BULLET: 
		DrawBullet();
		break;
	case ET_ITEM: 
		DrawItem();
		break;
	case ET_ZOMBIE: 
		DrawZombie();
		break;
	case ET_PART:
		line(GFXBUF, d.x,d.y, ld.x, ld.y, m_color);
		circlefill(GFXBUF, d.x,d.y,1,m_color);
		break;
	case ET_HERO :
		DrawHero();
        break;
	case ET_FLAME :
		DrawFlame();
	}
}

void CEntity::Integrate(int ticks)
{	
	//verlet integration
	vec2 tpos= m_pos*2.0 -m_lpos*1.0 +m_accel* ticks*ticks* 0.000001;
	m_lpos= m_pos;
	m_pos=tpos;
	m_accel=vec2(0,0);
}

void CEntity::DrawDbg(void)
{
	vec2 d= m_pos+GAME->m_offset;
	circlefill(GFXBUF,d.x,d.y, 3,makecol(0,255,0));

	textprintf(GFXBUF,font,d.x,d.y,makecol(230,12,0),"%d",m_health);

}

void CEntity::DrawHero(void)
{
	BITMAP*hro=GAME->GetBitmap(BMP_HERO_GUN),
		*gun=GAME->GetBitmap(BMP_GUN);	
	vec2 dd=vec2(hro->w/2,hro->h);
	vec2 d= m_pos+GAME->m_offset -dd;

	vec2 md=vec2(mouse_x,mouse_y)-d-dd*0.5;
	if(mouse_x-GAME->m_offset.x>= m_pos.x)
	{
		draw_sprite(GFXBUF,hro,d.x ,d.y );
		rotate_sprite(GFXBUF,gun, d.x, d.y+dd.y/2,ftofix( atan(md.y/md.x)*256/(2*3.141593)));

	}
	else
	{
		gun=GAME->GetBitmap(BMP_GUNL);
		draw_sprite_h_flip(GFXBUF,hro,d.x ,d.y );
		rotate_sprite(GFXBUF,gun, d.x-dd.x/3, d.y+dd.y/2,ftofix( atan(md.y/md.x)*256/(2*3.141593)));
	}
}

int up=0, bul=0, fl=0;
void CEntity::UpdateHero(int ticks)
{
	if(m_health<=0)
		GAME->m_state=GS_GAME_OVER;
	
	if((key[KEY_LCONTROL]||key[KEY_LCONTROL]||mouse_b&1)&& bul<=0)
	{
		BITMAP*hro=GAME->GetBitmap(BMP_HERO_GUN);
		vec2 v1=m_pos-vec2(hro->w/3,hro->h/2);
		vec2 v2= -GAME->m_offset+vec2(mouse_x,mouse_y)-v1;
		v2= v2*(1/v2.Length());
		v1+=v2*20;
		if(m_weapon==WP_MACH)
		{
			v2=v1-v2*0.8*ticks;		 
			GAME->m_entitys.push_back(new CEntity(v1,v2,ET_BULLET));
			bul=100;
			GAME->PlaySnd(SMP_SHOOT);
		}
		
		if(m_weapon==WP_FLAME&&GAME->m_am_flame>0 )
		{
			v2=v1-v2*0.15*ticks+vec2(0.01*(rand()%100)-0.5,0.01*(rand()%100)-0.3);		 
			GAME->m_entitys.push_back(new CEntity(v1,v2,ET_FLAME));
			bul=20;
			GAME->m_am_flame--;
			
			if(fl%2)
				GAME->PlaySnd(SMP_FLAME1);
			fl++;

		}
	}
	else
		bul-=ticks;	
	if(key[KEY_2])
		m_weapon=WP_MACH;
	if(key[KEY_1])
		m_weapon=WP_FLAME;
	

	BITMAP*hro=GAME->GetBitmap(BMP_HERO_GUN),
		*lnd=GAME->m_land.m_map;	
	vec2 d= m_pos+GAME->m_offset -vec2(hro->w/2,hro->h);
	vec2 of=GAME->m_offset;
	
	if(check_pp_collision(hro,lnd,d.x,d.y,of.x,of.y))
	{
			
		int am_up=check_pp_collision_amount(hro,lnd,d.x,d.y,of.x,of.y,PPCAMOUNT_UP);
		int am_down=check_pp_collision_amount(hro,lnd,d.x,d.y,of.x,of.y,PPCAMOUNT_DOWN);
		
		if(am_up<= 20)
			m_pos.y-= am_up;
		else if(am_down<=20)
			m_pos.y+=am_down;
		else
		{
			int am_r=check_pp_collision_amount(hro,lnd,d.x,d.y,of.x,of.y,PPCAMOUNT_RIGHT);
			int am_l=check_pp_collision_amount(hro,lnd,d.x,d.y,of.x,of.y,PPCAMOUNT_LEFT);

			if(am_r<am_l)	
			{
				m_pos.x+=am_r*1,0;
				m_lpos.x=m_pos.x;
			}
			else
			{
				m_pos.x-=am_l*1.0;
				m_lpos.x=m_pos.x;
			}
			
		}

		
		m_air=false;

		m_lpos.y=m_pos.y;
	
	}
	if(!key[KEY_UP]&&!key[KEY_W]&&!key[KEY_SPACE])
				up-=ticks;
	if(!m_air)
	{
		m_lpos.x+=(m_pos.x-m_lpos.x)*0.004*ticks;
		if(key[KEY_LEFT]||key[KEY_A])
		{
			m_lpos.x=m_pos.x;
			m_pos.x-= 0.08*ticks;
			
		}
		if(key[KEY_RIGHT]||key[KEY_D])
		{
			m_lpos.x=m_pos.x;
			m_pos.x+= 0.08*ticks;
			
		}
		if((key[KEY_UP]||key[KEY_W]||key[KEY_SPACE]))
		{
			m_pos.y-= 0.25*ticks;
			up=300;
			m_air=true;

			GAME->PlaySnd(SMP_JUMP);

			if(key[KEY_LEFT]||key[KEY_A])
			{
				
				m_pos.x-= 0.08*ticks;
				
			}
			if(key[KEY_RIGHT]||key[KEY_D])
			{
				
				m_pos.x+= 0.08*ticks;
				
			}
		}
		
	}
	else//air
	{
		if((key[KEY_UP]||key[KEY_W]||key[KEY_SPACE])&&up<=0 &&up>= -400)
		{
			m_pos.y-= 0.2*ticks;
			up=-600;
			GAME->PlaySnd(SMP_JUMP);
			if(key[KEY_LEFT]||key[KEY_A])
			{
				
				m_pos.x-= 0.08*ticks;
				
			}
			if(key[KEY_RIGHT]||key[KEY_D])
			{
				
				m_pos.x+= 0.08*ticks;
				
			}
		}
	}

	GAME->m_offset.x=-(m_pos.x-SCREEN_W/2);
	
}

void CEntity::DrawBullet(void)
{
	BITMAP*spr=GAME->GetBitmap(BMP_BULLET);	
	vec2 d= m_pos+GAME->m_offset;
	
	draw_sprite(GFXBUF,spr,d.x ,d.y );
}

void CEntity::DrawFlame(void)
{
	BITMAP*spr=GAME->GetBitmap(BMP_FLAME);	
	vec2 d= m_pos+GAME->m_offset;
	
	//draw_sprite(GFXBUF,spr,d.x ,d.y );
	fblend_add(spr, GFXBUF,d.x ,d.y ,255- 255*m_life/LIFE_FLAME);
}

void CEntity::DrawZombie(void)
{
	BITMAP*hro=GAME->GetBitmap(BMP_ZOMBIE);	
	vec2 dd=vec2(hro->w/2,hro->h);
	vec2 d= m_pos+GAME->m_offset -dd;
	if(GAME->m_entitys.front()->m_pos.x>= m_pos.x)
		draw_sprite(GFXBUF,hro,d.x ,d.y );
	else
		draw_sprite_h_flip(GFXBUF,hro,d.x ,d.y );


}

void CEntity::UpdateZombie(int ticks)
{
	BITMAP*hro=GAME->GetBitmap(BMP_ZOMBIE_MASK),
		*lnd=GAME->m_land.m_map;	
	
	vec2 of=GAME->m_offset;

	m_lpos.x=m_pos.x;
	float f= -0.035;
	if(m_pos.x<GAME->m_entitys.front()->m_pos.x )
		f=0.035;
	m_pos.x+= f*ticks;
	vec2 d= m_pos+GAME->m_offset -vec2(hro->w/2,hro->h);
	if(check_pp_collision(hro,lnd,d.x,d.y,of.x,of.y))
	{
			
		int am_up=check_pp_collision_amount(hro,lnd,d.x,d.y,of.x,of.y,PPCAMOUNT_UP);
		int am_down=check_pp_collision_amount(hro,lnd,d.x,d.y,of.x,of.y,PPCAMOUNT_DOWN);
		
		if(am_up<= 15)
			m_pos.y-= am_up;
		else if(am_down<15)
			m_pos.y+=am_down;
		else
		{
			int am_r=check_pp_collision_amount(hro,lnd,d.x,d.y,of.x,of.y,PPCAMOUNT_RIGHT);
			int am_l=check_pp_collision_amount(hro,lnd,d.x,d.y,of.x,of.y,PPCAMOUNT_LEFT);

			
			if(am_r<am_l)	
			{
				m_pos.x+=am_r*1,0;
				m_lpos.x=m_pos.x;
			}
			else
			{
				m_pos.x-=am_l*1.0;
				m_lpos.x=m_pos.x;
			}
			
			
		}

		m_air=false;

		m_lpos.y=m_pos.y;
	
	}
	
	
}

vec2 CEntity::GetPos(void)
{
	vec2 d= m_pos;
	BITMAP*s=GAME->GetMask(m_type);
	switch(m_type)
	{
	case ET_PART:
		return d;
		break;
	case ET_BULLET:
		return d;
		break;
	case ET_FLAME:
		return d-vec2(s->w/2,s->h/2);
		break;
	case ET_HERO:
		return d-vec2(s->w/2,s->h);
		break;
	case ET_ZOMBIE:
		return d-vec2(s->w/2,s->h);
		break;
	default:
		return d;
		break;
	}
}

void CEntity::DrawItem(void)
{
	BITMAP*spr=GAME->GetBitmap(BMP_ITEM);	
	vec2 d= m_pos+GAME->m_offset;
	draw_sprite(GFXBUF,spr,d.x ,d.y );
}
