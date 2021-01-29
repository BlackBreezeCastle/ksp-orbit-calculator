import krpc
import json
from Kmath import *
conn = krpc.connect(name='planets')
sun=conn.space_center.bodies['Sun']
#earth=conn.space_center.bodies['Earth']
#moon=conn.space_center.bodies['Moon']
vessel=conn.space_center.active_vessel

''
fp=open('config.txt','w')
fp.close()
''

data2={}

def planet_rotation(child):
    parent=child.orbit.body
    q=Quaternion.Tuple4(child.rotation(parent.non_rotating_reference_frame))
    q=q*Quaternion.Tuple4(child.rotation(child.non_rotating_reference_frame)).inverse()
    return q.tuple4()
    

def write_json(data):
    fp=open('config.txt','a+')
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
        position=(0.0,0.0,0.0)
        velocity=(0.0,0.0,0.0)
        quaternion=(1.0,0.0,0.0,0.0)
    else:
        parent=root.orbit.body.name
        reference_frame=root.orbit.body.non_rotating_reference_frame
        t0=conn.space_center.ut
        position=root.position(reference_frame)
        velocity=root.velocity(reference_frame)
        quaternion=planet_rotation(root)

    data={root.name:{'gm':root.gravitational_parameter,'radius':root.equatorial_radius,'atmosphere_depth':root.atmosphere_depth,\
    'soi':root.sphere_of_influence,'satellites':children,'parent':parent,\
    'position':position,'velocity':velocity,'quaternion':quaternion,'t0':t0}}
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