
#include <Python.h>
#include "orbit.h"
#include "orbit.cpp"
static PyObject *method_position_at(PyObject *self, PyObject *args) {
	//str是要写入ss文件流的字符串。
	//filename是要写入的文件的名称。
	double x0, y0, z0;
	double vx0, vy0, vz0;
	double t;
	char *body=NULL;


	// Parse arguments
	if (!PyArg_ParseTuple(args, "(ddd)(ddd)ds", &x0,&y0,&z0,&vx0,&vy0,&vz0,&t,&body)) {
		return NULL;
	}
	orbit ob;
	ob.reset_orbit(Vector3(x0, y0, z0), Vector3(vx0, vy0, vz0), t, string(body));
	Vector3 ret = ob.position_at_t(t);
	PyObject* pyRet= Py_BuildValue("(ddd)", ret.x(), ret.y(), ret.z());
	return pyRet;
}
static PyMethodDef keplerMethods[] = {
	{ "position_at", method_position_at, METH_VARARGS, "Python interface for kepler C library function to extrapolate the kepler orbit" },
	{ NULL, NULL, 0, NULL }
};


static struct PyModuleDef keplermodule = {
	PyModuleDef_HEAD_INIT,
	"kepler",
	"Python interface for the kepler C library function",
	-1,
	keplerMethods
};
PyMODINIT_FUNC PyInit_kepler(void) {
	return PyModule_Create(&keplermodule);
}
