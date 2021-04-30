import Korbit
from Kmath import *

def f_at_orbit(r,orb):
    h=Vector3.Tuple3(orb.h()).unit_vector()
    r=Vector3.Tuple3(r)
    r=r-Vector3.Dot(r,h)*h
    pe=Vector3.Tuple3(orb.pe_vector())
    ret=Vector3.Angle(pe,r)
    if Vector3.Dot(Vector3.Cross(r,pe),h)<0:
        ret=-ret
    return ret;

r0=6371000
h=400000

ob1=Korbit.orbit()
ob1.set_r_v((0.0,r0+h,0.0),(7800.0,0.0,0.0),0.0,398600446148608.0)

ob2=Korbit.orbit()
ob2.set_r_v((0.0,r0+h,0.0),(7850.0,0.0,0.0),1000.0,398600446148608.0)

period1=ob1.period()
angle=0
for i in range(100000):
    tmp=f_at_orbit(ob2.state_at_t(period1*i)[0],ob1)
    if tmp>0 and angle<0 and tmp>angle:
        print((i,math.degrees(angle)))
    angle=tmp

	