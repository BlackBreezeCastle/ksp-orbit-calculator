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
	printf("init\n");
	Vector3 r1=Vector3(-8145501.536528384,1817763.8217677567,3043014.9528953847);
	Vector3 v1=Vector3(-7576.882691523437,-2484.3349921185913,-4910.370147643323);

	double t1=7439.4732031426565;
	double t2=349398.3154937796;
	orbit ob0;
	//for(int  i=0;i<10000;i++)
	ob0.reset_orbit(r1,v1,t1,std::string("Earth"),10);

	orbit ob1;
	ob1.reset_orbit(ob0.semimajor_axis(),
		ob0.eccentric(),
		ob0.longitude_of_ascend_node(),
		ob0.incliantion(),
		ob0.argument_of_perigee(),
		ob0.mean_anomaly0(),
		t1,
		ob0.body());
	ob0.next_orbit()->next_orbit()->print();
	ob1.next_orbit()->next_orbit()->print();	

	// tmp=ob0.next_orbit()->next_orbit()->position_at_f(0).magnitude()-6371000;
	//Vector3 r1=Vector3(-2287279.628581712,1840195.812273841,3344967.7613846688);
	//Vector3 v1=Vector3(177.18669464183063,774.61348477921,1453.8379484944592);

	//double t1=47215.5773522283;
	//orbit ob;
	//for(int i=0;i<100000;i++)20795.3667457444;
	//double t2=48;
	//auto t=next_orbit(t1,ob0,ob1);
	//t=next_orbit(t,ob1,ob2);
	//t=next_orbit(t,ob2,ob3);
	//t=next_orbit(t,ob3,ob4);
	//t=next_orbit(t,ob4,ob5);
	system("pause");
}
	
