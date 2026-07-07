#pragma once
#include <string.h>
class HSEntry
{
public:
	HSEntry():m_points(0),m_stage(0){strcpy(m_name,"ZOMBIE MASTER");};
	int m_points;
	int m_stage;
	char m_name[32];
};

inline bool operator<(HSEntry h1,HSEntry h2)
{
	int t1(h1.m_points), t2(h2.m_points);
	if(t1==t2)
	{
		t1=h1.m_stage;
		t2=h2.m_stage;
	}
	return t1>t2;

}

#define HS_FILE "hiscore.dat"
#define HS_MAX_ENTRYS 8

class CHiscore
{
public:
	CHiscore(void);
	~CHiscore(void);
	bool Worthy(int p);
	void Add(int p, int st, const char* name);
	HSEntry m_entrys[HS_MAX_ENTRYS];
private:

	void Load(void);
	void Save(void);
};
