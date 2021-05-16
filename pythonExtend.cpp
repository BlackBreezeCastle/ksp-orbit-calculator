#include <Python.h>
#include "structmember.h"
#define PY_BUILD
#include "navigation.cpp"

typedef struct _OrbitObject
{
    PyObject_HEAD
    orbit __instance;
} OrbitObject;

static PyMemberDef OrbitObject_DataMembers[] = {  //类/结构的数据成员类说明 表.   根据官方文档说明此类表必须要要以一个元素全为NULL的数据结构结尾，后面还有一个Method 表也是如此
     { "__instance", T_OBJECT, offsetof(OrbitObject, __instance), 0, "The instance of orbit" },
     { NULL, NULL, NULL, 0, NULL }   
 };

static PyObject *
OrbitObejct_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    OrbitObject *self;
    self = (OrbitObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
		self->__instance=orbit();
    }
    return (PyObject *) self;
}
 
 static void OrbitObejct_init(OrbitObject* Self, PyObject* pArgs)        //构造方法.
 {
	 Self->__instance=orbit();
 }
 
 
 static void  OrbitObejct_Destruct(OrbitObject* Self)                   //析构方法.
 {
     Py_TYPE(Self)->tp_free((PyObject*)Self);                //释放对象/实例.
 }

static PyObject *set_r_v(OrbitObject *self, PyObject *args) {
	double x0, y0, z0;
	double vx0, vy0, vz0;
	double t,gm;

	// Parse arguments
	if (!PyArg_ParseTuple(args, "(ddd)(ddd)dd", &x0,&y0,&z0,&vx0,&vy0,&vz0,&t,&gm)) {
		return NULL;
	}

	self->__instance.reset_orbit(Vector3(x0, y0, z0), Vector3(vx0, vy0, vz0), t, gm);
	//self->__instance.print();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *set_r_v_t_body(OrbitObject *self, PyObject *args) {
	double x0, y0, z0;
	double vx0, vy0, vz0; 
	double t;
    char *body;

	// Parse arguments
	if (!PyArg_ParseTuple(args, "(ddd)(ddd)ds", &x0,&y0,&z0,&vx0,&vy0,&vz0,&t,&body)) {
		return NULL;
	}
    //printf("%s",body);
	self->__instance.reset_orbit(Vector3(x0, y0, z0), Vector3(vx0, vy0, vz0), t, std::string(body));
	//self->__instance.print();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *set_element(OrbitObject *self, PyObject *args) {
	double sem,ecc,inc,lan,aop,m0,t0,gm;

	// Parse arguments
	if (!PyArg_ParseTuple(args, "dddddddd", &sem,&ecc,&inc,&lan,&aop,&m0,&t0,&gm)) {
		return NULL;
	}

	self->__instance.reset_orbit(sem,ecc,inc,lan,aop,m0,t0,gm);
	//self->__instance.print();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *state_at_t(OrbitObject *self, PyObject *args) {
	double t;
	// Parse arguments
	if (!PyArg_ParseTuple(args, "d", &t)) {
		return NULL;
	}

	state ret = self->__instance.state_at_t(t);
	PyObject* pyRet= Py_BuildValue("((ddd)(ddd))", ret.r.x(), ret.r.y(), ret.r.z(),ret.v.x(), ret.v.y(), ret.v.z());
	return pyRet;
}

static PyObject *pe_vector(OrbitObject *self, PyObject *args) {

	Vector3 ret = self->__instance.periapsis();
	PyObject* pyRet= Py_BuildValue("(ddd)", ret.x(), ret.y(), ret.z());
	return pyRet;
}

static PyObject *h(OrbitObject *self, PyObject *args) {

	Vector3 ret = self->__instance.h();
	PyObject* pyRet= Py_BuildValue("(ddd)", ret.x(), ret.y(), ret.z());
	return pyRet;
}

static PyObject *period(OrbitObject *self, PyObject *args) {

	double ret = self->__instance.period();
	PyObject* pyRet= Py_BuildValue("d", ret);
	return pyRet;
}

static PyObject *print(OrbitObject *self, PyObject *args) {

	self->__instance.print();
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *state_at_f(OrbitObject *self, PyObject *args) {
	double f;
	// Parse arguments
	if (!PyArg_ParseTuple(args, "d", &f)) {
		return NULL;
	}

	state ret = self->__instance.state_at_f(f);
	PyObject* pyRet= Py_BuildValue("((ddd)(ddd))", ret.r.x(), ret.r.y(), ret.r.z(),ret.v.x(), ret.v.y(), ret.v.z());
	return pyRet;
}

static PyObject *t_to_f(OrbitObject *self, PyObject *args) {
	double t0,f;
	// Parse arguments
	if (!PyArg_ParseTuple(args, "dd", &t0,&f)) {
		return NULL;
	}

	double ret = self->__instance.t_to_f(t0,f);
	PyObject* pyRet= Py_BuildValue("d", ret);
	return pyRet;
}  
 
static PyObject *f_at_position(OrbitObject *self, PyObject *args) {
	double x0, y0, z0;

	// Parse arguments
	if (!PyArg_ParseTuple(args, "(ddd)", &x0,&y0,&z0)) {
		return NULL;
	}
    
	double f=self->__instance.f_at_position(Vector3(x0, y0, z0));
	PyObject* pyRet= Py_BuildValue("d", f);
	return pyRet;
}
  
static PyMethodDef orbit_methods[] = {
    {"set_r_v", (PyCFunction) set_r_v,METH_VARARGS,
     "set orbit by position and velocity,parameter:r,v,t,gm"
    },
    
    {"set_r_v_t_body", (PyCFunction) set_r_v_t_body,METH_VARARGS,
     "set orbit by position and velocity at t when in body,parameter:r,v,t,body(string)"
    },
    
    {"set_element", (PyCFunction) set_element,METH_VARARGS,
     "set orbit by element parameter:sem,ecc,inc,lan,aop,m0,t0,gm"
    },
    
	{"state_at_t", (PyCFunction) state_at_t,METH_VARARGS,
     "return the position and velocity at time t,parameter:t,return ((r)(v))"
    },
	
	{"f_at_position", (PyCFunction) f_at_position,METH_VARARGS,
     "return the true anomaly f at the input position,parameter:(x,y,z)"
    },
	
	{"state_at_f", (PyCFunction) state_at_f,METH_VARARGS,
     "return the position and velocity at true anomaly,parameter:f,return ((r)(v))"
    },
     
	{"t_to_f", (PyCFunction) t_to_f,METH_VARARGS,
     "return the time from t0 to desired true anomaly,parameter:t0,f,return time"
    },
    
	{"pe_vector", (PyCFunction) pe_vector,METH_NOARGS,
     "return vector of periapsis"
    },
	{"h", (PyCFunction) h,METH_NOARGS,
     "return vector of angular momentum when mass is 1kg"
    },

	{"period", (PyCFunction) period,METH_NOARGS,
     "return vector of angular momentum when mass is 1kg"
    },

	{"print", (PyCFunction) print,METH_NOARGS,
     "print orbit info"
    },
    { NULL} /* Sentinel */
};

static PyTypeObject OrbitObject_ClassInfo =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    "Korbit.orbit",            //可以通过__class__获得这个字符串. CPP可以用类.__name__获取.   const char *
    sizeof(OrbitObject),                 // tp_basicsize 类/结构的长度.调用PyObject_New时需要知道其大小.  Py_ssize_t
    0,                              //tp_itemsize  Py_ssize_t
    (destructor)OrbitObejct_Destruct,    //类的析构函数.      destructor
    0,                            //类的print 函数      printfunc
    0,                               //类的getattr 函数  getattrfunc
    0,                              //类的setattr 函数   setattrfunc
    0,                              //formerly known as tp_compare(Python 2) or tp_reserved (Python 3)  PyAsyncMethods *
    0,          //tp_repr 内置函数调用。    reprfunc
    0,                              //tp_as_number   指针   PyNumberMethods *
    0,                              //tp_as_sequence 指针   PySequenceMethods *
    0,                              // tp_as_mapping 指针  PyMappingMethods *
    0,                              // tp_hash   hashfunc
    0,                              //tp_call     ternaryfunc
    0,          //tp_str/print内置函数调用.   reprfunc
    0,                          //tp_getattro    getattrofunc
    0,                          //tp_setattro     setattrofunc
    0,                          //tp_as_buffer 指针 Functions to access object as input/output buffer   PyBufferProcs
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                 //tp_flags    如果没有提供方法的话，为Py_TPFLAGS_DEFAULE     unsigned long
    "Korbit Module write by C++!",                   // tp_doc  __doc__,类/结构的DocString.  const char *
    0,                                                   //tp_traverse    call function for all accessible objects    traverseproc
    0,                                                      // tp_clear   delete references to contained objects    inquiry
    0,                                                      //  tp_richcompare   richcmpfunc
    0,                                                      //tp_weaklistoffset  Py_ssize_t
    0,                                                      // tp_iter  getiterfunc
    0,                                          //tp_iternext    iternextfunc


    /* Attribute descriptor and subclassing stuff */

    orbit_methods,        //类的所有方法集合.   PyMethodDef *
    OrbitObject_DataMembers,          //类的所有数据成员集合.  PyMemberDef *
    0,                                              // tp_getset   PyGetSetDef *
    0,                                              //  tp_base   _typeobject *
    0,                                              //  tp_dict     PyObject *
    0,                                              // tp_descr_get   descrgetfunc
    0,                                                          //tp_descr_set   descrsetfunc
    0,                                              //tp_dictoffset   Py_ssize_t
    (initproc)OrbitObejct_init,      //类的构造函数.tp_init       initproc
    0,                      //tp_alloc  allocfunc
    OrbitObejct_new,                          //tp_new   newfunc
    0,                        // tp_free    freefunc
    0,                      //  tp_is_gc     inquiry
};

static PyObject* polar_orbit_transfer_correct(PyObject* self, PyObject*args)
{
    double rx,ry,rz,vx,vy,vz;
    char *body;
    double target_pe;
    double t,maneuver_t;
    if(!PyArg_ParseTuple(args, "(ddd)(ddd)dsdd", &rx,&ry,&rz,&vx,&vy,&vz,&t,&body,&target_pe,maneuver_t))
    {
        return NULL;
    }
    Vector3 r(rx,ry,rz);
    Vector3 v(vx,vy,vz);
    Vector3 dv;
    bool ok=polar_orbit_transfer_correct(r,v,t,body,target_pe,maneuver_t,dv);
    if(ok)
    {
        return Py_BuildValue("(ddd)", dv.x(),dv.y(),dv.z());
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef navigation_methods[] =
{
    {"polar_orbit_transfer_correct", (PyCFunction)polar_orbit_transfer_correct
    , METH_VARARGS, "count the correct dv in polar transfer orbit"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef ModuleInfo =
{
	PyModuleDef_HEAD_INIT,
	"Korbit",               //模块的内置名--__name__.
	NULL,                 //模块的DocString.__doc__
	-1,
	navigation_methods, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit_Korbit(void)       //模块外部名称为--PyVcam
{
	if (PyType_Ready(&OrbitObject_ClassInfo) < 0)
        return NULL;
    PyObject* pReturn = 0;
    pReturn = PyModule_Create(&ModuleInfo);
    if (pReturn == NULL)
        return NULL;
    Py_INCREF(&OrbitObject_ClassInfo);
    PyModule_AddObject(pReturn, "orbit", (PyObject*)&OrbitObject_ClassInfo); //将这个类加入到模块的Dictionary中.
    return pReturn;
}
