#include<stdio.h>
#include<iostream>
#include<Windows.h>
#include"orbit.h"
using namespace std;



void PrintVector(Vector3 vec)
{
	printf("x:%f y:%f z:%f",vec.x(),vec.y(),vec.z());
}
void main(void)
{
	bodies::instance();
	Vector3 r0 = Vector3(-132224.61342726392,
		2.830656299250064,
		1618467.1840451267);
	Vector3 v0 = Vector3(-1444.6640559256796,
		0.0023276433870329572,
		1314.7761868364778);
	double t0 = 2705.579718627118;
	orbit ob;
	ob.reset_orbit(r0, v0, t0, "Kerbin");
	auto s1=ob.next_orbit()->state_at_f(0.0);

	orbit ob1;
	Vector3 r1 = Vector3(0,700000, 0);
	Vector3 v1 = Vector3(3500, 0,0);
	ob1.reset_orbit(r1, v1, 0, "Kerbin");
	double bt, br;
	ob1.b_parameter(bt, br);
	//auto pe=ob.next_orbit()->periapsis().magnitude()-bodies::instance()["Mun"].radius;
	system("pause");
}
	
