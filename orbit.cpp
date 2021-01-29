#include "orbit.h"
#include "config.h"
#include <queue>

orbit::orbit()
{
	body="";
	u=0.0;
	sem=0.0;	
	ecc=0.0;
	inc=0.0;
	lan=0.0;
	aop=0.0;
	f0=0.0;
	t0=0.0;
	tao=0.0;
	t_next = -1.0;
	Vector3 h=Vector3(0.0,0.0,0.0);
	Vector3 pe=Vector3(0.0,0.0,0.0);
	orbit * next=NULL;
}
orbit::orbit(Vector3 r, Vector3 v, double t, double gm)
{
	reset_orbit(r, v, t, gm);
}

void orbit::reset_orbit(Vector3 r, Vector3 v, double t, string _body)
{
	body = _body;
	//reset_orbit(r,v,t,bodies::instance()[body].gm);
}

void orbit::set_body_name(std::string name)
{
	body=name;
}

void orbit::reset_orbit(Vector3 r, Vector3 v, double t, double gm)
{
	next = NULL;
	t_next = -1;
	t0 = t;
	u = gm;
	//energy
	double E = 0.5*pow(v.magnitude(), 2) - u / r.magnitude();

	Vector3 y = Vector3(0.0, 1.0, 0.0);
	Vector3 x = Vector3(1.0, 0.0, 0.0);
	h = Vector3::Cross(r, v);
	double h2 = Vector3::Dot(h, h);
	if (E != 0.0)
	{
		sem = -0.5*u / E;
		ecc = pow(max(1 -h2/ (u*sem),0.0), 0.5);
	}

	period = 2 * PI*pow(sem, 1.5) / pow(u, 0.5);

	if(ecc!=0.0)
	{
		f0 = R_to_F(ecc,sem,r.magnitude());
	}
	else
	{
		f0=0.0;
	}

	if (Vector3::Dot(r, v) < 0)
	{
		f0 = -f0;
	}

	pe = Quaternion(h, -f0).rotate(r).normalized();

	inc = PI - Vector3::Angle(h, y);

	Vector3 vlan = Vector3::Cross(y, h);
	if(vlan.magnitude()<1e-16)
	{
		vlan=pe;
	}

	lan = Vector3::Angle(vlan, x);
	if (Vector3::Dot(h, y) > 0.0)
	{
		lan = -lan;
	}


	aop = Vector3::Angle(pe, vlan);
	if (Vector3::Dot(Vector3::Cross(pe, vlan), h) > 0)
	{
		aop = 2*PI-aop;
	}

	E = F_to_E(ecc, f0);
	double M = E_to_M(ecc, F_to_E(ecc, f0));
	double n = pow(fabs(u / (sem*sem*sem)), 0.5);
	tao = t0 - M / n;
	//printf("\nC++ E M F\n%.17lf %.17lf %.17lf\n", E, M, f0);
	//printf("%.17lf %lf %.17lf %.17lf %.17lf %.17lf\n",ecc,sem,inc,lan,aop,f0);
}

Vector3 orbit::position_at_t(double t)
{
	double n = pow(fabs(u / (sem*sem*sem)), 0.5);
	double M = n*(t - tao);
	M = fmod(M, PI * 2);
	if (M > PI)
	{
		M = M - 2*PI;
	}
	double E = M_to_E(ecc, M);
	double F = E_to_F(ecc,E);
	//printf("\nC++ E M F\n%.17lf %.17lf %.17lf\n", E, M, F);
	return Quaternion(h, F).rotate(pe).normalized() * F_to_R(ecc, sem, F);
}

state orbit::state_at_t(double t)
{
	state ret;
	ret.t = t;
	double n = pow(fabs(u / (sem*sem*sem)), 0.5);
	double M = n*(t - tao);
	M = fmod(M, PI * 2);
	if (M > PI)
	{
		M = M - 2 * PI;
	}
	double E = M_to_E(ecc, M);
	double F = E_to_F(ecc, E);
	//printf("\nC++ E M F\n%.17lf %.17lf %.17lf\n", E, M, F);
	ret.r= Quaternion(h, F).rotate(pe).normalized() * F_to_R(ecc, sem, F);
	Vector3 wr = Vector3::Cross(h, ret.r).normalized()*h.magnitude()/ret.r.magnitude();

	double tmp = 1 + ecc*cos(F);
	tmp = tmp*tmp;
	double dr_dt = sem*(ecc*ecc-1)*ecc*sin(F) / tmp;
	ret.v = dr_dt*wr.magnitude()/ret.r.magnitude() *ret.r.normalized() + wr;
	return ret;
}

bodies::bodies()
{
	Config cfg("kerbal_config.txt");
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

		if (name != "Sun")
		{
			double parent_gm = m_bodies.find(b.parent)->second.gm;
			double t0 = tmp["t0"].asDouble();
			Vector3 position(tmp["position"][0].asDouble(), tmp["position"][1].asDouble(), tmp["position"][2].asDouble());
			Vector3 velocity(tmp["velocity"][0].asDouble(), tmp["velocity"][1].asDouble(), tmp["velocity"][2].asDouble());
			b.orbit.reset_orbit(position, velocity, t0, parent_gm);
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

inline double orbit_dis(orbit &a, orbit &b, const double& t)
{
	return  (a.position_at_t(t) - b.position_at_t(t)).magnitude();
}

double closest_dichotomy(orbit &a, orbit &b, const double& t0,const double &tf,int count=20)
{
	double lt = t0;
	double lr = tf;
	while (count > 0)
	{
		double tmp1 = 0.5*(lt + lr);
		double tmp2 = tmp1 + 0.1;
		double disl = (a.position_at_t(tmp1) - b.position_at_t(tmp1)).magnitude();
		double disr = (a.position_at_t(tmp2) - b.position_at_t(tmp2)).magnitude();
		if (disl > disr)
		{
			lt = tmp1;
		}
		else
		{
			lr = tmp1;
		}
		count--;
	}
	return 0.5*(lt + lr);
}

//double find_closest_t(const orbit &a, const orbit &b, int round = 1)
//{
//double period = a.period < b.period ? a.period : b.period;

//}
