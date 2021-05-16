#include<stdio.h>
#include<iostream>
#include<Windows.h>
//#include"orbit.h"
#include"navigation.h"
using namespace std;

double toDay(double dt)
{
	return dt/86400.0;
}

void test1()
{
	double t=244465.99940100007;
	auto ss=bodies::instance()["Moon"].orbit.position_at_t(10000);
}

void test2()
{
	double lon=1.936535765863177;
	double lat=0.3423248200098106;
	double lan=0.04008351366616158;//ÔÂÇòÉý½»µã
	bodies::instance();
	for(double i=0;i<100;i=i+1)
	{
		orbit init;
		orbit trans;
		orbit trans1;
		double slt=0;
		double vpe=0;
		double start_t=i*86400+916751.1072915426;

		if(init_transfer(start_t,init))
		{
			if(!transfer_orbit(init,trans))
			{
				//printf("%f\n",i);
				continue;
			};
			if(init_transfer(lon,lat,trans.longitude_of_ascend_node(),start_t,slt,init,vpe))
			{
				trans=acc_transfer(init,slt,vpe);
				if(transfer_orbit(trans,trans1))
				{
					if(trans1.next_orbit()!=NULL)
					{
						//trans1.next_orbit()->print();
						init=get_slip_orbit(start_t,trans1.longitude_of_ascend_node());

						slt=trans1.t0()-init.t0();
						state s1=init.state_at_t(trans1.t0());
						state s2=trans1.state_at_t(trans1.t0());
						double dis=(s1.r-s2.r).magnitude();
						if(fabs(trans1.next_orbit()->incliantion()-PI*0.5)<radians(1))
						{
							double pe_dis=trans1.next_orbit()->periapsis().magnitude()-1737100;
							printf("\nt0:%lf slt:%lf lan:%lf pe:%lf\n",init.t0(),slt,degrees(trans1.longitude_of_ascend_node()),pe_dis);
							trans1.print();
							trans1.next_orbit()->print();
							//Vector3 dv;
							//correct_orbit(trans1,trans1.t0()+10,dv);
						}

						//ret.next_orbit()->print();
					}
				};
			}
		}
	}
}

void main(void)
{
	test2();
	system("pause");
}

