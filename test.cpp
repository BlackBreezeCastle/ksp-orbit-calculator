#include<stdio.h>
#include<iostream>
#include<Windows.h>
#include"orbit.h"
using namespace std;

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
	double cur_lan = equator_node_lon(lon, lat, inc, ascend_orbit);
	cur_lan=cur_lan+body.init_rotation+t_start*body.rotate_speed;
	cur_lan=normalized_rad(cur_lan);
	double dif_lan = lan - cur_lan;
	if(!ascend_orbit)
	{
		dif_lan-=PI;
	}
	while (dif_lan < 0.0)
	{
		dif_lan = dif_lan + 2 * PI;
	}
	double t_offset=dif_lan/body.rotate_speed;
	double ret=t_start+t_offset;
	return ret;
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

double degrees(double x)
{
	return x*180/PI;
}

double toDay(double dt)
{
	return dt/86400.0;
}

inline orbit acc_transfer(const orbit &ob,
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

bool init_transfer(double lon,double lat,double lan,double start_t,double &slip_t,orbit &slip_obt,double &vpe)
{
	double inc=radians(28.5);
    slip_obt=get_slip_orbit(start_t,lan);
	double alt=150000;//初始轨道高度	
	auto earth=bodies::instance()["Earth"];
	auto moon=bodies::instance()["Moon"];
	double pe=earth.radius+alt;
	double ap=moon.orbit.position_at_t(slip_obt.t0()+460000).magnitude()+20000000;
	double sem=0.5*(pe+ap);
	double transfer_t=0.5*orbit_period(0.5*(pe+ap),earth.gm);
	double E=-0.5*earth.gm/sem;
	vpe=pow(2*(E+earth.gm/pe),0.5);
	Vector3 pe_r=-pe*moon.orbit.position_at_t(start_t+transfer_t).normalized();
	slip_t=slip_obt.t_to_f(slip_obt.t0(),slip_obt.f_at_position(pe_r));
	orbit trans_obt=acc_transfer(slip_obt,slip_t,vpe);
	if(trans_obt.next_orbit()==NULL)
	{
		return false;
	}
	return true;
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

	const double init_lan=lan;
	const double init_aop=aop;
	auto earth=bodies::instance()["Earth"];

	const double init_period=orbit_period(ob.periapsis().magnitude(),earth.gm);
	const double period=ob.period();

	auto m_offset=[=](double _lan,double _aop)
	{
		double ret_t=((_lan-init_lan)/earth.rotate_speed+(_aop-init_aop)*init_period*0.5/PI)*2*PI/period;
		return -ret_t;
	};

	for(double i=1;i<50;i++)
	{
		orbit a0;
		a0.reset_orbit(sem,ecc,inc,lan,aop,m0+m_offset(lan,aop),t0,body);
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
		a1.reset_orbit(sem,ecc,inc,lan+dif,aop,m0+m_offset(lan+dif,aop),t0,body);
		if(a1.next_orbit()==NULL)
		{
			cout<<bias<<endl;
			cout<<tar_dif<<endl;
			printf("error");
			return false;
		}
		a1.next_orbit()->b_parameter(bt1,br1);

		orbit a2;
		a2.reset_orbit(sem,ecc,inc,lan,aop+dif,m0+m_offset(lan,aop+dif),t0,body);
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
		tar_dif[1][0]=(tar_b-br0);
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
	ret.reset_orbit(sem,ecc,inc,lan,aop,m0+m_offset(lan,aop),t0-m_offset(lan,aop)*period*0.5/PI,body);
	//ret.print();
	if(ret.next_orbit()==NULL)
		return false;
	return true;
}

bool correct_orbit(const orbit &ob,
				   const double &correct_t,
				   Vector3 &dv,
				   double pe=200000+1737100)
{
	double bt0,bt1,bt2,br0,br1,br2;
	dv=Vector3(0,0,0);
	state s0=ob.state_at_t(correct_t);

	Vector3 dx=Vector3::Cross(s0.r,s0.v).normalized();
	Vector3 dy=s0.v.normalized();

	double dif=radians(0.01);
	Matrix mdif(2,2);
	Matrix bias(2,1);
	Matrix tar_dif(2,1);

	auto earth=bodies::instance()["Earth"];

	const double init_period=orbit_period(ob.periapsis().magnitude(),earth.gm);
	const double period=ob.period();


	for(double i=1;i<50;i++)
	{
		orbit a0;
		a0.reset_orbit(s0.r,s0.v+dv,s0.t,s0.body);
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
		a1.reset_orbit(s0.r,s0.v+dv+dx*dif,s0.t,s0.body);
		if(a1.next_orbit()==NULL)
		{
			cout<<bias<<endl;
			cout<<tar_dif<<endl;
			printf("error");
			return false;
		}
		a1.next_orbit()->b_parameter(bt1,br1);

		orbit a2;
		a2.reset_orbit(s0.r,s0.v+dv+dy*dif,s0.t,s0.body);
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
		tar_dif[1][0]=(tar_b-br0);
		//cout<<"next_ecc"<<next_ecc<<endl;
		mdif=mdif.inverse();
		//cout<<mdif;

		bias=mdif*tar_dif;
		//cout<<bias<<endl;
		double dx_bias=bias[0][0];
		double dy_bias=bias[1][0];
		//(dx_bias,dy_bias,0.1);

		dv=dv+dx_bias*dx;
		dv=dv+dy_bias*dy;;

	}
	orbit ret;
	ret.reset_orbit(s0.r,s0.v+dv,s0.t,s0.body);
	//ret.print();
	if(ret.next_orbit()==NULL)
		return false;
	ret.next_orbit()->print();
	auto dis=dv.magnitude();
	printf("dv:%lf\n",dis);
	return true;
}


void PrintVector(Vector3 vec)
{
	printf("x:%f y:%f z:%f",vec.x(),vec.y(),vec.z());
}

void test1()
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
}

void test2()
{
	double lon=1.936535765863177;
	double lat=0.3423248200098106;
	double lan=0.04008351366616158;//月球升交点
	bodies::instance();
	for(double i=0;i<100;i=i+1)
	{
		orbit init;
		orbit trans;
		orbit trans1;
		double slt=0;
		double vpe=0;
		double start_t=i*86400;

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
							printf("\nt0:%lf slt:%lf lan:%lf pe:%lf\n",toDay(init.t0()),slt,degrees(trans1.longitude_of_ascend_node()),pe_dis);

							Vector3 dv;
							correct_orbit(trans1,trans1.t0()+10,dv);
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

