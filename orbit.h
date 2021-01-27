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
//��������ϵ
using namespace std;
class orbit
{
private:
	//������������
	string body;
	//��������
	double u;
	//�볤�ᣨ�ף�
	double sem;
	//ƫ����
	double ecc;
	//��ǣ����ȣ�
	double inc;
	//�����㣨���ȣ�
	double lan;
	//���ص���ǣ����ȣ�
	double aop;
	//t0ʱ������ǣ����ȣ�
	double f0;
	//t0ʱ�̣��룩
	double t0;
	//��ƽ����ʱ�����
	double tao;
	//������һ��������µ�Ӱ�����ڣ���ʱ�̣��룩,-1��ʾ������
	double t_next;
	//���ڣ��룩
	double period;
	//�Ƕ���
	Vector3 h;
	//���ص㵥λ����
	Vector3 pe;
	//��һ��Ӱ����Ĺ��
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
