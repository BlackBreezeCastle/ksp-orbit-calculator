import Korbit
from Kmath import *
import krpc
#print(help(Korbit.orbit))
def ApSpeed(new_pe,ap,gm):
    sem=0.5*(new_pe+ap)
    return math.sqrt(2.0*(gm/ap-0.5*gm/sem))

r0=6371000
h=400000

ob1=Korbit.orbit()

ap=400000+r0
pe=150000+r0
sem=0.5*(ap+pe)
ecc=(ap-pe)/(ap+pe)
gm=398600446148608.0
ob1.set_element(sem,ecc,0,0,0,0,0,gm)


print(Korbit.mul(1,1))
''
#print(ob1.state_at_f(0))
print(ob1.state_at_f(math.pi))
start_t=ob1.t_to_f(0,math.pi) 
#print(ob1.state_at_t(start_t))
#print('start_t',start_t)
#print('period1',ob1.period())
''
ob2=Korbit.orbit()
sem=r0+h
ob2.set_element(sem,0,0,0,0,0,0,gm)
#print(ob2.state_at_f(0))
print(ob2.state_at_f(math.pi))
print('leo150x400',ob1.period())
print('leo400x400',ob2.period())
period1=ob1.period()
period2=ob2.period()
sem=pow((0.5*(period2-200)/math.pi)**2*gm,1.0/3.0)
angle=0
t=start_t
for i in range(1000):
    tmp=ob1.f_at_position(ob2.state_at_t(period1*i+start_t)[0])
    #print(tmp)
    if tmp>0 and angle<0:
        print((i,t/86400,math.degrees(angle)))
        dock_pos=ob1.state_at_t(t)[0]
        #print('ap vector',dock_pos)
        #print('ap1 Vector3',ob2.state_at_t(t)[0])
        new_period=ob2.t_to_f(t,ob2.f_at_position(dock_pos))
        #print('ap2 Vector3',ob2.state_at_t(t+new_period)[0])
        sem=pow((0.5*new_period/math.pi)**2*gm,1.0/3.0)
        new_pe=2*sem-ap
        speed=ApSpeed(new_pe,ap,gm)
        print((t,speed,new_period,2*sem-ap-r0))
        print('\n')
    angle=tmp
    t=period1*i+start_t
input()
''
