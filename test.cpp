#include<stdio.h>
#include<iostream>
#include<Windows.h>
#include"orbit.h"
using namespace std;

inline orbit transfer(const orbit &ob,
		 const double &slip_t,
		 const double &v)
{
	orbit obt;
	state rv=ob.state_at_t(ob.t0()+slip_t);
	rv.v=rv.v.normalized()*v;
	obt.reset_orbit(rv.r,rv.v,ob.t0()+slip_t,ob.body());
	return obt;
}

orbit get_slip_orbit(const double &start_t,
					const double&lan)
{
	double lon=1.936535765863177;
	double lat=0.3423248200098106;
	double inc=radians(28.5);
	double alt=150000;//初始轨道高度
	double range=2500000;//发射入轨射程
	double launch_t=time_to_orbit_over(start_t,lon,lat,lan,inc,"Earth")-300;
	double orbit_t=launch_t+600;
	auto earth=bodies::instance()["Earth"];
	auto moon=bodies::instance()["Moon"];
	orbit slip_obt;
	slip_obt.reset_orbit(earth.radius+alt,0,inc,lan,0,0,0,earth.gm);
	Vector3 r=earth.msl_position(lon,lat,0,launch_t);
	r=Quaternion(slip_obt.h(),range/earth.radius).rotate(r);
	auto state_init= slip_obt.state_at_f(slip_obt.f_at_position(r));

	slip_obt.reset_orbit(state_init.r,state_init.v,orbit_t,earth.name);
	return slip_obt;
}

bool init_transfer(double lon,double lat,double start_t,double &slip_t,orbit &slip_obt,double &vpe)
{
	double lan=0.04008351366616158;//月球升交点
	double inc=radians(28.5);
    slip_obt=get_slip_orbit(start_t,lan);
	double alt=150000;//初始轨道高度	
	auto earth=bodies::instance()["Earth"];
	auto moon=bodies::instance()["Moon"];
	double pe=earth.radius+alt;
	double ap=moon.orbit.position_at_t(slip_obt.t0()+500000).magnitude()+20000000;
	double sem=0.5*(pe+ap);
	double transfer_t=0.5*orbit_period(0.5*(pe+ap),earth.gm);
	double E=-0.5*earth.gm/sem;
	vpe=pow(2*(E+earth.gm/pe),0.5);
	Vector3 pe_r=-pe*moon.orbit.position_at_t(start_t+transfer_t).normalized();
	slip_t=slip_obt.t_to_f(slip_obt.t0(),slip_obt.f_at_position(pe_r));
	orbit trans_obt=transfer(slip_obt,slip_t,vpe);
	if(trans_obt.next_orbit()==NULL)
	{
		return false;
	}
	return true;
}

double equator_node_lon(double lon, double lat, double inc, bool is_ascend = false)
{
	double b = tan(lat)*tan(PI / 2 - inc);
	b = b < -1.0 ? -1.0 : b;
	b = b > 1.0 ? 1.0 : b;
	b = asin(b);
	if (is_ascend)
		return lon - b;
	else
		return lon + b;
}

double time_to_orbit_over(double t_start, double lon, double lat, double lan, double inc, std::string planet, bool ascend_orbit = false)
{
	celestial_body body = bodies::instance()[planet];
	double tmp_lan = equator_node_lon(lon, lat, inc, ascend_orbit);
	tmp_lan = lan - tmp_lan;
	if (tmp_lan < 0.0)
	{
		tmp_lan = tmp_lan + 2 * PI;
	}
}

void filter(double &a,double &b,double _max=0.01)
{
	double maxab=max(fabs(a),fabs(b));
	if(maxab>_max)
	{
		a=a/maxab*_max;
		b=b/maxab*_max;
	}
}

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
	double ap=moon.orbit.position_at_t(start_t+432000).magnitude()+20000000;
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

bool transfer_orbit(const orbit &ob,orbit &ret,double pe=200000+1737100)
{
	double bt0,bt1,bt2,br0,br1,br2;
	double sem=ob.semimajor_axis();
	double ecc=ob.eccentric();
	double inc=radians(28.5);
	double lan=ob.longitude_of_ascend_node();
	double aop=ob.argument_of_perigee();
	double m0=ob.mean_anomaly0();
	double t0=ob.t0();
	std::string body=ob.body();

	double dif=radians(0.01);
	Matrix mdif(2,2);
	Matrix bias(2,1);
	Matrix tar_dif(2,1);

	for(double i=1;i<50;i++)
	{
		orbit a0;
		a0.reset_orbit(sem,ecc,inc,lan,aop,m0,t0,body);
		if(a0.next_orbit()==NULL)
		{
			//cout<<bias<<endl;
			//cout<<tar_dif<<endl;
			ob.next_orbit()->print();
			printf("error");
			return false;
		}
		a0.next_orbit()->b_parameter(bt0,br0);

		orbit a1;
		a1.reset_orbit(sem,ecc,inc,lan+dif,aop,m0,t0,body);
		if(a1.next_orbit()==NULL)
		{
			cout<<bias<<endl;
			cout<<tar_dif<<endl;
			printf("error");
			return false;
		}
		a1.next_orbit()->b_parameter(bt1,br1);

		orbit a2;
		a2.reset_orbit(sem,ecc,inc,lan,aop+dif,m0,t0,body);
		if(a2.next_orbit()==NULL)
		{
			cout<<bias<<endl;
			cout<<tar_dif<<endl;
			printf("error");
			return false;
		}
		a2.next_orbit()->b_parameter(bt2,br2);


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
		double cos_w_theta = cos(theta + a0.next_orbit()->argument_of_perigee());
		double tar_b=-pe*pow((next_ecc+1)/(next_ecc-1),0.5);
		//if (bt0>0)
		//	tar_b=-tar_b;
		
		tar_dif[0][0]=-bt0;
		tar_dif[1][0]=tar_b-br0;
		//cout<<"next_ecc"<<next_ecc<<endl;
		mdif=mdif.inverse();
		//cout<<mdif;

		bias=mdif*tar_dif;
		//cout<<bias<<endl;
		double lan_bias=bias[0][0];
		double aop_bias=bias[1][0];
		filter(lan_bias,aop_bias);

		lan+=lan_bias;
		aop+=aop_bias;

	}
	ret.reset_orbit(sem,ecc,inc,lan,aop,m0,t0,body);
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

