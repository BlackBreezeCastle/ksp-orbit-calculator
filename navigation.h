#ifndef PY_BUILD
#include"orbit.h"
#else
#include"orbit.cpp"
#endif

orbit get_slip_orbit(const double &start_t,
					const double&lan);

bool init_transfer(double lon,
				   double lat,
				   double lan,
				   double start_t,
				   double &slip_t,
				   orbit &slip_obt,
				   double &vpe);

bool init_transfer(double start_t,orbit&ret);

bool transfer_orbit(const orbit &ob,orbit &ret,double pe=200000+1737100);

inline orbit acc_transfer(const orbit &ob,
		 const double &slip_t,
		 const double &v);

bool polar_orbit_transfer_correct(Vector3 r,
								  Vector3 v,
								  double t,
								  std::string target_body,
								  double target_pe,
								  double maneuver_t,
								  Vector3 &dv);
/*
bool polar_orbit_tranfser_init(std::string launch_body,
							   std::string target_body,
							   double lon,
							   double lat,
							   double t0,
							   double t_orbit,//”√ ±
							   double range,//…‰≥Ã
							   double t_advance,
							   double target_pe,
							   double &slip_t,
							   orbit &init_orbit,
							   orbit &tranfer_orbit
							   );
*/