#pragma once
#include<string>
#include<vector>
#include<map>
#include"kepler.h"
//#include"kepler.cpp"
//#include"config.h"
struct state
{
	Vector3 r;
	Vector3 v;
	double t;
};
//左手坐标系
using namespace std;
class orbit
{
private:
	//中心天体名称
	string body;
	//重力常数
	double u;
	//半长轴（米）
	double sem;
	//偏心率
	double ecc;
	//倾角（弧度）
	double inc;
	//升交点（弧度）
	double lan;
	//近地点辐角（弧度）
	double aop;
	//t0时刻真近角（弧度）
	double f0;
	//t0时刻（秒）
	double t0;
	//零平近角时间参数
	double tao;
	//到达下一条轨道（新的影响球内）的时刻（秒）,-1表示不存在
	double t_next;
	//周期（秒）
	double period;
	//角动量
	Vector3 h;
	//近地点单位向量
	Vector3 pe;
	//下一个影响球的轨道
	orbit * next;
public:
	orbit();
	orbit(Vector3 r, Vector3 v, double t, double gm);
	void reset_orbit(Vector3 r, Vector3 v, double t, double gm);
	void reset_orbit(Vector3 r, Vector3 v, double t, string body);

	Vector3 position_at_t(double t);

	state state_at_t(double t);

	//double friend find_closest_t(const orbit &a, const orbit &b,int round=1);
};

struct celestial_body
{
	double gm;
	double radius;
	double atmosphere_depth;
	double soi;
	string name;
	string parent;
	vector<string> satellites;
	Quaternion rotation;
	orbit orbit;
};

class bodies
{
private:
	map<string, celestial_body>m_bodies;
private:
	bodies();
public:
	static bodies &instance();
	celestial_body operator[](string name);
};
