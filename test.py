import Korbit
from Kmath import *
import krpc


r0=6371000
h=400000

ob1=Korbit.orbit()

ap=400000+r0
pe=150000+r0
sem=0.5*(ap+pe)
ecc=(ap-pe)/(ap+pe)
gm=398600446148608.0
ob1.set_element(sem,ecc,0,0,0,0,0,gm)
print(ob1.state_at_t(0))
print(ob1.state_at_f(0))
ob2=Korbit.orbit()

sem=r0+h
ob2.set_element(sem,0,0,0,0,0,0,gm)
period1=ob1.period()
angle=0
for i in range(1000):
    tmp=ob1.f_at_position(ob2.state_at_t(period1*i)[0])
    #print(tmp)
    if tmp<0 and angle>0:
        print((i,i*period1/86400,math.degrees(angle)))
        dock_pos=ob1.state_at_f(math.pi)[0]
        tf=ob2.t_to_f(i*period1,ob2.f_at_position(dock_pos))
        print(tf)
    angle=tmp

