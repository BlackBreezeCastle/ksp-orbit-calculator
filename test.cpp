#include<stdio.h>
#include<iostream>
#include<Windows.h>
#include"orbit.h"
using namespace std;

inline double orbit_period(double sem,double gm)
{
	return 2 * PI*pow((pow(sem, 3) / gm), 0.5);;
}

double radians(double x)
{
	return x*PI/180;
}

double toDay(double dt)
{
	return dt/86400.0;
}
bool init_transfer(double start_t,orbit&ret)
{
	auto earth=bodies::instance()["Earth"];
	auto moon=bodies::instance()["Moon"];
	double pe=earth.radius+200000;
	double ap=moon.orbit.position_at_t(start_t+432000).magnitude();
	double sem=0.5*(pe+ap);
	double transfer_t=0.5*orbit_period(0.5*(pe+ap),earth.gm);
	double E=-0.5*earth.gm/sem;
	double vpe=pow(2*(E+earth.gm/pe),0.5);
	Vector3 pe_r=-pe*moon.orbit.position_at_t(start_t+transfer_t).normalized();
	Vector3 pe_v=Vector3::Cross(moon.orbit.angular_momentum(),pe_r).normalized()*vpe;
	//double d=toDay(transfer_t);
	ret.reset_orbit(pe_r,pe_v,start_t,"Earth");
	if(ret.next_orbit()!=NULL)
		return true;
	return false;
}

bool transfer_orbit(const orbit &a,orbit &ret,double pe=200000+1737100)
{
	double bt0,bt1,bt2,br0,br1,br2;
	double sem=a.semimajor_axis();
	double ecc=a.eccentric();
	double inc=radians(28.5);
	double lan=a.longitude_of_ascend_node();
	double aop=a.argument_of_perigee();
	double m0=a.mean_anomaly0();
	std::string body=a.body();

	double dif=radians(0.01);

	for(double i=1;i<50;i++)
	{
		orbit a0;
		a0.reset_orbit(sem,ecc,inc,lan,aop,m0,0,body);
		if(a0.next_orbit()==NULL)
		{
			printf("error");
			return false;
		}
		a0.next_orbit()->b_parameter(bt0,br0);

		orbit a1;
		a1.reset_orbit(sem,ecc,inc,lan+dif,aop,m0,0,body);
		if(a1.next_orbit()==NULL)return false;
		a1.next_orbit()->b_parameter(bt1,br1);

		orbit a2;
		a2.reset_orbit(sem,ecc,inc,lan,aop+dif,m0,0,body);
		if(a2.next_orbit()==NULL)return false;
		a2.next_orbit()->b_parameter(bt2,br2);

		Matrix mdif(2,2);
		mdif[0][0]=(bt1-bt0)/dif;
		mdif[1][0]=(br1-br0)/dif;
		mdif[0][1]=(bt2-bt0)/dif;
		mdif[1][1]=(br2-br0)/dif;
		//cout<<mdif;

		//double next_sem=a.next_orbit()->semimajor_axis();
		double next_ecc=a0.next_orbit()->eccentric();
		if(next_ecc<1.0)
		{
			printf("\n ecc<1.0\n");
			return false;
		}
		double a = a0.conic_a();
		double b = a0.conic_b();
		double theta = atan(b / a);
		double cos_w_theta = cos(theta + a0.argument_of_perigee());
		double tar_b=-pe*pow((next_ecc+1)/(next_ecc-1),0.5);
		if (cos_w_theta<0)
			tar_b=-tar_b;
		Matrix tar_dif(2,1);
		tar_dif[0][0]=-bt0;
		tar_dif[1][0]=tar_b-br0;
		//cout<<tar_dif;
		mdif=mdif.inverse();
		//cout<<mdif;
		Matrix bias(2,1);
		bias=mdif*tar_dif;
		//cout<<bias<<endl;
		double lan_bias=bias[0][0];
		double aop_bias=bias[1][0];
		lan+=lan_bias*0.02*i;
		aop+=aop_bias*0.02*i;
	}
	ret.reset_orbit(sem,ecc,inc,lan,aop,m0,0,body);
	//ret.print();
	if(ret.next_orbit()==NULL)
		return false;
	ret.next_orbit()->print();
	sem=ret.semimajor_axis();
	auto dis=ret.next_orbit()->periapsis().magnitude()-1737100;
	return true;
}


void PrintVector(Vector3 vec)
{
	printf("x:%f y:%f z:%f",vec.x(),vec.y(),vec.z());
}
void main(void)
{
	bodies::instance();
	for(double i=0;i<16000;i=i+1)
	{
		orbit init;
		orbit ret;
		if(init_transfer(4800.0*i,init))
		{
			if(!transfer_orbit(init,ret))
			{
				printf("%f\n",i);
			};
		}
	}
	system("pause");
}

