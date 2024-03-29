//#define PY_BUILD
#include "orbit.h"
#include "config.h"
#include <queue>

inline double orbit_dis(const orbit &a, const orbit &b, const double& t)
{
	return  (a.position_at_t(t) - b.position_at_t(t)).magnitude();
}

double closest_dichotomy(const orbit &a, const orbit &b, const double& t0,const double &tf,double *closest_dis=NULL)
{
	double lt = t0;
	double lr = tf;
	double tmp1;
	double tmp2;
	double disl;
	double disr;
	tmp1 = 0.5*(lt + lr);
	tmp2 = tmp1 + 0.1;
	disl = (a.position_at_t(tmp1) - b.position_at_t(tmp1)).magnitude();
	disr = (a.position_at_t(tmp2) - b.position_at_t(tmp2)).magnitude();
	int count=0;
	while (fabs(lt-lr)>1)
	{
		if (disl > disr)
		{
			lt = tmp1;
		}
		else
		{
			lr = tmp1;
		}
		tmp1 = 0.5*(lt + lr);
		tmp2 = tmp1 + 0.1;
		disl = (a.position_at_t(tmp1) - b.position_at_t(tmp1)).magnitude();
		disr = (a.position_at_t(tmp2) - b.position_at_t(tmp2)).magnitude();
		count++;
		if(count>100)
		{
			printf("closest_dichotomy not merge\n");
			break;
		}
	}
	if(closest_dis!=NULL)
	{
		*closest_dis=0.5*(disl+disr);
	}
	return 0.5*(lt + lr);
}

double orbit::period()const 
{
	return m_period;
}

double closest_distance(const orbit &a, const orbit &b, double start_time,double *closest_t=NULL)
{
	double ret=0.0;
	double min_period;
	double max_period;
	double a_period;
	double b_period;
	if(a.eccentric()>1)
	{
		a_period=INFINITY_DOUBLE;
	}
	else
	{
		a_period=a.period();
	}

	if(b.eccentric()>1)
	{
		b_period=INFINITY_DOUBLE;
	}
	else
	{
		b_period=b.period();
	}


	if(a_period < b_period)
	{
		min_period=a_period;
		max_period=b_period;
	}
	else
	{
		min_period=b_period;
		max_period=a_period;
	}
	
	double closest_dis=INFINITY_DOUBLE;
	int count=0;
	while(true)
	{
		double dis;
		double time=closest_dichotomy(a,b,start_time,start_time+min_period,&dis);
		if(closest_dis<dis)
		{
			//printf("%f\n",closest_dis);
			break;
		}
		else
		{	
			if(closest_t!=NULL)
			{
				*closest_t=time;
			}
			closest_dis=dis;
		}
		start_time+=min_period;
		count++;
		if(count>100)
		{
			printf("closest_distance not merge\n");
			return INFINITY_DOUBLE;
		}
	}
	return closest_dis;
}

double solve_soi_bsp(const orbit &planet,const orbit &vessel,const double &min_ut,const double &max_ut,const double &soi)
{
	if(min_ut>=max_ut)
	{
		return INFINITY_DOUBLE;
	}
	double ret=0.5*(min_ut+max_ut);
	double dt=0.25*(max_ut-min_ut);
	double dis1=orbit_dis(planet,vessel,min_ut);
	double dis2=orbit_dis(planet,vessel,max_ut);
	double sig=1;
	if(dis1>dis2)
	{
		sig=-1;
	}
	double dis=INFINITY_DOUBLE;
	int count =0;
	while (fabs(dis-soi)>0.5)
	{
		count++;
		dis=orbit_dis(planet,vessel,ret);
		if(dis<soi)
		{
			ret+=sig*dt;
		}
		else
		{
			ret-=sig*dt;
		}
		dt=dt*0.5;

		if(count>100)
			break;
	}
	return ret;
}

double t_to_capture(const orbit&obt,const double &ut,std::string &new_body)
{
	double ret=INFINITY_DOUBLE;
	new_body="";
	celestial_body body=bodies::instance()[obt.body()];
	if(body.gm==0)
	{
		return ret;
	}

	for(int i=0;i<body.satellites.size();i++)
	{
		celestial_body satelite=bodies::instance()[body.satellites[i]];

		if(satelite.orbit.periapsis().magnitude()-satelite.soi>obt.apoapsis().magnitude()
			||satelite.orbit.apoapsis().magnitude()+satelite.soi<obt.periapsis().magnitude())
		{
			continue;
		}

		double t_closest;
		double dis_closest=closest_distance(obt,satelite.orbit,ut,&t_closest);
		if(t_closest<ut)
		{
			continue;
		}
		if(dis_closest<satelite.soi)
		{
			double t_next_soi=solve_soi_bsp(satelite.orbit,obt,ut,t_closest,satelite.soi);
			if(t_next_soi<ret)
			{
				ret=t_next_soi;
				new_body=satelite.name;
			}
		}
	}
	return ret;
}

double t_to_escape(const orbit &obt,const double &ut,std::string &new_body)
{
	auto body=bodies::instance()[obt.body()];
	if((obt.eccentric()<1.0&&obt.apoapsis().magnitude()<body.soi)||obt.body()=="Sun")
	{
		return INFINITY_DOUBLE;
	}
	new_body=body.parent;
	double f_escape=obt.f_at_r(body.soi);
	double ret=obt.t_to_f(ut,f_escape);
	if(ret>0)
	{
		return ret+ut;
	}
	else
	{
		return INFINITY_DOUBLE;
	}
}

void orbit::count_next_orbit(const int &round)
{
	if(round<2)
	{
		m_t_next=INFINITY_DOUBLE;
		return;
	}

	orbit *obt=this;
	double ut=m_T0;
	std::string escape_body;
	std::string capture_body;
	double t_escape=t_to_escape((*obt),ut,escape_body);
	double t_capture=t_to_capture((*obt),ut,capture_body);
	if(t_escape==INFINITY_DOUBLE&&t_capture==INFINITY_DOUBLE)
	{
		m_t_next=INFINITY_DOUBLE;
		return ;
	}

	if(t_escape<t_capture)
	{
		m_t_next=t_escape;
		state planet=bodies::instance()[(*obt).body()].orbit.state_at_t(t_escape);
		state vessel=(*obt).state_at_t(t_escape);
		auto tmp=(*obt).position_at_t(t_escape);
		auto dis=tmp.magnitude();
		Vector3 r=vessel.r+planet.r;
		Vector3 v=vessel.v+planet.v;
		(*obt).m_Next=new orbit();
		(*obt).m_Next->reset_orbit(r,v,t_escape,escape_body,round-1);
		return;
	}
	else
	{
		m_t_next=t_capture;
		state planet=bodies::instance()[capture_body].orbit.state_at_t(t_capture);
		state vessel=(*obt).state_at_t(t_capture);
		Vector3 r=vessel.r-planet.r;
		Vector3 v=vessel.v-planet.v;
		(*obt).m_Next=new orbit();
		(*obt).m_Next->reset_orbit(r,v,t_capture,capture_body,round-1);
		return ;
	}
}

orbit::orbit()
{
	m_Body="";
	m_Gm=0.0;
	m_Sem=0.0;	
	m_Ecc=0.0;
	m_Inc=0.0;
	m_Lan=0.0;
	m_Aop=0.0;
	m_T0=0.0;
	m_M0=0.0;
	m_t_next =INFINITY_DOUBLE;
	Vector3 m_H=Vector3(0.0,0.0,0.0);
	Vector3 m_Pe=Vector3(0.0,0.0,0.0);
	m_Next=NULL;
}

orbit::orbit(const orbit & ob)
{
	m_Next=NULL;
	m_Body="";
	(*this)=ob;
}

orbit::orbit(const orbit && ob)
{
	m_Next=NULL;
	m_Body="";
	(*this)=ob;
}


orbit::orbit(Vector3 r, Vector3 v, double t, double gm)
{
	m_Next=NULL;
	m_Body="";
	reset_orbit(r, v, t, gm);
}



orbit::~orbit()
{
	if(m_Next!=NULL)
	{
		delete m_Next;
		m_Next=NULL;
	}
}

void orbit::reset_orbit(Vector3 r, Vector3 v, double t, std::string _body,const int &round)
{
	m_Body = _body;
	reset_orbit(r,v,t,bodies::instance()[m_Body].gm);
	count_next_orbit(round);
}

orbit&orbit::operator=(const orbit &ob)
{
	if(this!=&ob)
	{
		if(this->m_Next!=NULL)
		{
			delete m_Next;
		}
		m_Body=ob.body();
		m_Gm=ob.m_Gm;
		m_Sem=ob.m_Sem;
		m_Ecc=ob.m_Ecc;
		m_Inc=ob.m_Inc;
		m_Lan=ob.m_Lan;
		m_Aop=ob.m_Aop;
		m_T0=ob.m_T0;
		m_M0=ob.m_M0;
		m_period=ob.m_period;
		m_t_next =ob.m_t_next;
		m_H= ob.m_H;
		m_Pe=ob.m_Pe;
		m_Next=NULL;
		if(ob.m_Next!=NULL)
		{
			this->m_Next=new orbit();
			*(this->m_Next)=*(ob.m_Next);
		}
	}
	return *this;
}

/*
orbit&orbit::operator=(const orbit &&ob)
{
	m_Next=NULL;
	m_Body="";
	(*this)=ob;
	return *this;
}
*/



void orbit::set_body_name(std::string name)
{
	m_Body=name;
}

void orbit::reset_orbit(double sem,double ecc,double inc,double lan,double aop,double m0,double t0,double gm)
{
	m_Sem=sem;
	m_Ecc=ecc;
	m_Lan=lan;
	m_Inc=inc;
	m_Aop=aop;
	m_Gm=gm;
	m_T0=t0;
	m_M0=m0;
	//count the period
	m_period = 2 * PI*pow(fabs((m_Sem*m_Sem*m_Sem/m_Gm)), 0.5);

	//
	Vector3 y = Vector3(0.0, 1.0, 0.0);
	Vector3 x = Vector3(1.0, 0.0, 0.0);
	//计算角动量
	m_H=Quaternion(x,-m_Inc).rotate(y);
	m_H=Quaternion(y,-m_Lan).rotate(m_H);
	double E=0.0;
	if(m_Ecc!=1.0)
	{
		E=-0.5*m_Gm/m_Sem;
	}

	double pe=m_Sem*(1-m_Ecc);
	m_H=-sqrt(2.0*(E+gm/pe))*pe*m_H;
	//计算远地点，近地点
	m_Pe=Quaternion(y,-m_Lan).rotate(x);
	m_Pe=Quaternion(m_H,m_Aop).rotate(m_Pe);
	m_Ap=-1.0*m_Sem*(1+m_Ecc)*m_Pe;
	m_Pe=m_Pe*pe;
}

void orbit::reset_rss_orbit(double sem,double ecc,double inc,double lan,double aop,double m0,double t0,double gm,double period)
{
	reset_orbit(sem,ecc,inc,lan,aop,m0,t0,gm);
	m_period=period;
}

void orbit::reset_orbit(double sem,double ecc,double inc,double lan,double aop,double m0,double t0,std::string body,const int &round)
{
	m_Body=body;
	reset_orbit(sem,ecc,inc,lan,aop,m0,t0,bodies::instance()[body].gm);
	count_next_orbit(round);
}

void orbit::reset_orbit(Vector3 r, Vector3 v, double t, double gm)
{
	m_Next = NULL;
	m_t_next =INFINITY_DOUBLE;
	m_Gm = gm;
	m_T0=t;
	double F0=0.0;
	//energy
	double E = 0.5*pow(v.magnitude(), 2) - m_Gm / r.magnitude();

	Vector3 y = Vector3(0.0, 1.0, 0.0);
	Vector3 x = Vector3(1.0, 0.0, 0.0);
	m_H = Vector3::Cross(r, v);
	double h2 = Vector3::Dot(m_H, m_H);
	if (E != 0.0)
	{
		m_Sem = -0.5*m_Gm / E;
		m_Ecc = pow(max(1 -h2/ (m_Gm*m_Sem),0.0), 0.5);
	}

	//count the period
	m_period = 2 * PI*pow(fabs((m_Sem*m_Sem*m_Sem/m_Gm)), 0.5);


	if(m_Ecc!=0.0)
	{
		F0 = R_to_F(m_Ecc,m_Sem,r.magnitude());
	}
	else
	{
		F0=0.0;
	}

	if (Vector3::Dot(r, v) < 0)
	{
		F0 = -F0;
	}

	m_Pe = Quaternion(m_H, -F0).rotate(r).normalized();
	m_Ap=-1.0*m_Pe*(1+m_Ecc)*m_Sem;
	m_Pe=m_Pe*(1-m_Ecc)*m_Sem;

	m_Inc = PI - Vector3::Angle(m_H, y);

	Vector3 vlan = Vector3::Cross(y, m_H);
	if(vlan.magnitude()<1e-16)
	{
		vlan=m_Pe;
	}

	m_Lan = Vector3::Angle(vlan, x);
	if (Vector3::Dot(y, Vector3::Cross(vlan , x)) < 0.0)
	{
		m_Lan = -m_Lan;
	}


	m_Aop = Vector3::Angle(m_Pe, vlan);
	if (Vector3::Dot(Vector3::Cross(m_Pe, vlan), m_H) > 0)
	{
		m_Aop = 2*PI-m_Aop;
	}

	double _E(0);
	double M(0);
	if(m_Ecc<1.0)
	{
		_E=F2Estd(m_Ecc,F0);
		M=E2Mstd(m_Ecc,_E);
	}
	else
	{
		_E=F2Ehyp(m_Ecc,F0);
		M=E2Mhyp(m_Ecc,_E);
	}

	double n = 2*PI/m_period;
	m_M0=M-n*t;
	//printf("\nC++ E M F\n%.17lf %.17lf %.17lf\n", E, M, m_F0);
	//printf("%.17lf %lf %.17lf %.17lf %.17lf %.17lf\n",m_Ecc,m_Sem,m_Inc,m_Lan,m_Aop,m_F0);
}

Vector3 orbit::position_at_t(double t)const
{
	if(t>m_t_next)
	{
		return m_Next->position_at_t(t);
	}
	double F=f_at_t(t);
	return position_at_f(F);
}

Vector3 orbit::position_at_f(double f)const
{
	return Quaternion(m_H, f).rotate(m_Pe).normalized() * F_to_R(m_Ecc, m_Sem, f);
}

double orbit::f_at_position(Vector3 pos)const
{
	Vector3 normal=m_H.normalized();
	pos=pos-normal*Vector3::Dot(normal,pos);
	double ret=Vector3::Angle(pos,m_Pe);
	if(Vector3::Dot(Vector3::Cross(m_Pe,pos),normal)<0)
	{
		ret=-ret;
	}
	return ret;
}

double orbit::f_at_t(double t)const
{
	double n = 2*PI/m_period;
	double M = n*t+m_M0;
	M = fmod(M, PI * 2);
	if (M > PI&&m_Ecc<1.0)
	{
		M = M - 2*PI;
	}
	double E(0);
	double F(0);
	if(m_Ecc<1.0)
	{
		E=M2Estd(m_Ecc,M);
		F=E2Fstd(m_Ecc,E);
	}
	else
	{
		E=M2Ehyp(m_Ecc,M);
		F=E2Fhyp(m_Ecc,E);
	}
	//printf("\nC++ E M F\n%.17lf %.17lf %.17lf\n", E, M, F);
	return F;
}

double orbit::f_at_r(double r)const
{
	return R_to_F(m_Ecc,m_Sem,r);
}

double orbit::t_to_f(double t0,double f)const
{
	double E(0);
	double M(0);
	if(m_Ecc<1.0)
	{
		E=F2Estd(m_Ecc,f);
		M=E2Mstd(m_Ecc,E);
	}
	else
	{
		E=F2Ehyp(m_Ecc,f);
		M=E2Mhyp(m_Ecc,E);
	}
	double n = 2*PI/m_period;
	double t = (M-m_M0)/n-t0;
	t=fmod(t,m_period);
	if(t<0)
	{
		t+=m_period;
	}
	//printf("\n t0:%lf,f:%lf,E:%lf,M:%lf,n:%lf,t%lf,M0:%lf\n",t0,f,E,M,n,t,m_M0);
	return t;
}

state orbit::Kstate_at_t(double t)const
{
	if(t>m_t_next)
	{
		return m_Next->state_at_t(t);
	}
	state ret;
	ret.body = m_Body;
	ret.t = t;
	double F = f_at_t(t);
	ret.r= position_at_f(F);

	Vector3 wr = Vector3::Cross(m_H, ret.r).normalized()*m_H.magnitude()/ret.r.magnitude();
	double tmp = 1 + m_Ecc*cos(F);
	tmp = tmp*tmp;
	double dr_dt = m_Sem*(m_Ecc*m_Ecc-1)*m_Ecc*sin(-F) / tmp;
	ret.v = dr_dt*wr.magnitude()/ret.r.magnitude() *ret.r.normalized() + wr;
	return ret;
}

state orbit::state_at_t(double t)const
{
	state ret;
	ret.body = m_Body;
	ret.t = t;
	double F = f_at_t(t);
	ret.r = position_at_f(F);

	Vector3 wr = Vector3::Cross(m_H, ret.r).normalized()*m_H.magnitude() / ret.r.magnitude();
	double tmp = 1 + m_Ecc*cos(F);
	tmp = tmp*tmp;
	double dr_dt = m_Sem*(m_Ecc*m_Ecc - 1)*m_Ecc*sin(-F) / tmp;
	ret.v = dr_dt*wr.magnitude() / ret.r.magnitude() *ret.r.normalized() + wr;
	return ret;
}

state orbit::state_at_f(double F)const
{
	state ret;
	ret.body = m_Body;
	ret.t = t_to_f(m_T0, F)+ m_T0;
	ret.r = position_at_f(F);

	Vector3 wr = Vector3::Cross(m_H, ret.r).normalized()*m_H.magnitude() / ret.r.magnitude();
	double tmp = 1 + m_Ecc*cos(F);
	tmp = tmp*tmp;
	double dr_dt = m_Sem*(m_Ecc*m_Ecc - 1)*m_Ecc*sin(-F) / tmp;
	ret.v = dr_dt*wr.magnitude() / ret.r.magnitude() *ret.r.normalized() + wr;
	return ret;
}

void orbit::print()const
{
	printf("\n Body:%s\n Gm:%lf\n m_Ecc:%.17lf \n m_Sem:%.17lf \n m_Inc:%.17lf \n m_Lan:%.17lf \n m_Aop:%.17lf \n m_M0 %.17lf \n pe: %.17lf\n ap: %.17lf\n",
		m_Body.c_str(), m_Gm, m_Ecc, m_Sem, m_Inc * 180 / PI, m_Lan * 180 / PI, m_Aop * 180 / PI, m_M0, m_Pe.magnitude()-bodies::instance()[m_Body].radius,m_Ap.magnitude()-bodies::instance()[m_Body].radius);
}

std::string orbit::body()const
{
	return m_Body;
}

const orbit* orbit::next_orbit()const
{
	return m_Next;
}

Vector3 orbit::apoapsis()const
{
	return m_Ap;
}

Vector3 orbit::periapsis()const
{
	return m_Pe;
}

Vector3 orbit::h()const
{
	return m_H;
}

Vector3 orbit::angular_momentum()const
{
	return m_H;
}

double orbit::eccentric()const
{
	return m_Ecc;
}

double orbit::semimajor_axis()const
{
	return m_Sem;
}
double orbit::longitude_of_ascend_node()const
{
	return m_Lan;
}

double orbit::incliantion()const
{
	return m_Inc;
}

double orbit::argument_of_perigee()const
{
	return m_Aop;
}

double orbit::gravity_parameter()const
{
	return m_Gm;
}

double orbit::mean_anomaly0()const
{
	return m_M0;
}

double orbit::t0()const
{
	return m_T0;
}

double orbit::conic_a()const
{
	return abs(m_Sem);
}

double orbit::conic_b()const
{
	return pow((abs(m_Ecc*m_Ecc - 1.0))*m_Sem*m_Sem, 0.5);
}

bool orbit::b_parameter(double & bt, double & br) const
{
	if (m_Ecc <= 1.0)
	{
		return false;
	}
	double a = conic_a();
	double b = conic_b();
	double theta = atan(b / a);
	double cosi = cos(m_Inc);
	double sini = sin(m_Inc);
	double cos_w_theta = cos(theta + m_Aop);
	double div= (cosi*cosi + sini*sini*cos_w_theta*cos_w_theta);
	bt = (b*cosi) / div;
	br = (b*sini*cos_w_theta / div);
	return true;
}

bodies::bodies()
{
	Config cfg("solar_config.txt");
	queue<string>q;
	q.push("Sun");
	while (!q.empty())
	{
		string name= q.front();
		auto tmp = cfg[name];
		celestial_body b;
		b.name = name;
		b.gm = tmp["gm"].asDouble();
		b.soi = tmp["soi"].asDouble();
		b.radius = tmp["radius"].asDouble();
		b.atmosphere_depth = tmp["atmosphere_depth"].asDouble();
		b.parent = tmp["parent"].asString();
		b.rotation = Quaternion(tmp["quaternion"][0].asDouble(), tmp["quaternion"][1].asDouble(), tmp["quaternion"][2].asDouble(), tmp["quaternion"][3].asDouble());
		b.rotate_speed = tmp["rotate_speed"].asDouble();
		b.init_rotation = tmp["init_rotation"].asDouble();
		if (name != "Sun")
		{
			double sem = tmp["sem"].asDouble();
			double ecc = tmp["ecc"].asDouble();
			double inc = tmp["inc"].asDouble();
			double lan = tmp["lan"].asDouble();
			double aop = tmp["aop"].asDouble();
			double m0 = tmp["m0"].asDouble();
			double orbit_period=tmp["orbit_period"].asDouble();
			double parent_gm = m_bodies.find(b.parent)->second.gm;
			b.orbit.reset_rss_orbit(sem,ecc,inc,lan,aop,m0,0.0,parent_gm,orbit_period);
			b.orbit.set_body_name(b.parent);
		}

		for (int i = 0; i < tmp["satellites"].size();i++)
		{
			q.push(tmp["satellites"][i].asString());
			b.satellites.push_back(tmp["satellites"][i].asString());
		}

		m_bodies.insert(map<string, celestial_body>::value_type(b.name, b));
		q.pop();
	}
}

bodies &bodies::instance()
{
	static bodies m_pbodies;
	return m_pbodies;
}

celestial_body bodies::operator[](string name)
{
	auto it = m_bodies.find(name);
	if (it != m_bodies.end())
	{
		return it->second;
	}
	celestial_body invalid;
	return invalid;
}