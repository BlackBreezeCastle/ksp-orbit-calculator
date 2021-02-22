import krpc
import json
from Kmath import *
conn = krpc.connect(name='planets')
sun=conn.space_center.bodies['Sun']
#earth=conn.space_center.bodies['Earth']
#moon=conn.space_center.bodies['Moon']
vessel=conn.space_center.active_vessel

''
fp=open('solar_config.txt','w')
fp.close()
''

data2={}

def planet_rotation(child):
    parent=child.orbit.body
    q=Quaternion.Tuple4(child.rotation(parent.non_rotating_reference_frame))
    q=q*Quaternion.Tuple4(child.rotation(child.non_rotating_reference_frame)).inverse()
    return q.tuple4()
    

def write_json(data):
    fp=open('solar_config.txt','a+')
    fp.write(data)
    fp.write('\n')
    fp.close()

def load_json(root):
    children=[]
    parent=''
    t0=conn.space_center.ut
    for i in root.satellites:
        children.append(i.name)
    if root.orbit==None:
        sem=0.0
        ecc=0.0
        inc=0.0
        lan=0.0
        aop=0.0
        m0=0.0
        quaternion=(1.0,0.0,0.0,0.0)
    else:
        orbit=root.orbit
        parent=root.orbit.body.name
        sem=orbit.semi_major_axis
        ecc=orbit.eccentricity
        inc=orbit.inclination
        lan=orbit.longitude_of_ascending_node
        aop=orbit.argument_of_periapsis
        m0=orbit.mean_anomaly_at_ut(0.0)
        quaternion=planet_rotation(root)
    data={root.name:{'gm':root.gravitational_parameter,'radius':root.equatorial_radius,'atmosphere_depth':root.atmosphere_depth,'quaternion':quaternion,\
    'soi':root.sphere_of_influence,'satellites':children,'parent':parent,\
    'sem':sem,'ecc':ecc,'inc':inc,'lan':lan,'aop':aop,'m0':m0}}
    global data2
    data2={**data2,**data}

def traverse(root):
    load_json(root)
    for i in root.satellites:
        traverse(i)
''
traverse(sun)
data2=json.dumps(data2,sort_keys=True, indent=4, separators=(',', ': '))
write_json(data2)
''