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
	printf("init");
	for(int i=0;i<1000000;i++)
		bodies::instance()["Mun"].orbit.position_at_t(1000000.0);
	system("pause");
}
	
