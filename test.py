import Korbit
from Kmath import *
ob=Korbit.orbit()
#print(ob.__instance)

def f_at_orbit(r,orb):
    h=Vector3.Tuple3(orb.h()).unit_vector()
    r=Vector3.Tuple3(r)
    r=r-Vector3.Dot(r,h)*h
    pe=Vector3.Tuple3(orb.pe_vector())
    print(pe)
    print(r)
    ret=Vector3.Angle(pe,r)
    if Vector3.Dot(Vector3.Cross(r,pe),h)>0:
        ret=-ret
    print(math.degrees(ret))

r0=6371000
h=400000
ob.set_r_v((0.0,r0+h,0.0),(7900.0,0.0,0.0),0.0,398600446148608.0)
print(ob.period())
f_at_orbit((1,1,0),ob)