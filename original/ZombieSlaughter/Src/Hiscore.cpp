#include ".\hiscore.h"
#include <algorithm>
#include <fstream>
using namespace std;

CHiscore::CHiscore(void)
{
	Load();
}

CHiscore::~CHiscore(void)
{
}

bool CHiscore::Worthy(int p)
{
	Load();
	return(p> m_entrys[HS_MAX_ENTRYS-1].m_points );
}

void CHiscore::Add(int p, int st, const char* name)
{
	Load();
	strcpy(m_entrys[HS_MAX_ENTRYS-1].m_name, name);
	m_entrys[HS_MAX_ENTRYS-1].m_points=p;
	m_entrys[HS_MAX_ENTRYS-1].m_stage=st;
	sort(m_entrys,m_entrys + HS_MAX_ENTRYS);
	Save();
}

void CHiscore::Load(void)
{
	ifstream  file(HS_FILE, ios::binary );
		
	if(!file.good())
	{
		file.close();
		for(int i =0; i< HS_MAX_ENTRYS;i++)
		{
			strcpy(m_entrys[i].m_name, "!!MOE!!");
			m_entrys[i].m_points=(i+1)*128;
			m_entrys[i].m_stage=(i+1)*2;
		}
		sort(m_entrys,m_entrys + HS_MAX_ENTRYS);
        Save();
	}
	else
	{
		file.read((char*)(m_entrys),sizeof(HSEntry)*HS_MAX_ENTRYS);
		sort(m_entrys,m_entrys + HS_MAX_ENTRYS);
		file.close();
	}
	
}
void CHiscore::Save(void)
{
	ofstream  file(HS_FILE, ios::binary | ios::trunc);
		
	if(!file.good())
	{
		file.close();
	}
	else
	{
		file.write((char*)(m_entrys),sizeof(HSEntry)*HS_MAX_ENTRYS);
	}
}
