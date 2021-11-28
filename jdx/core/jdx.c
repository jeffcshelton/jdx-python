#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <structmember.h>
#include "libjdx.h"

// TODO: Consider changing 'int' type to 'uint8_t' for direct compatibility with libjdx 'JDXVersion'
typedef struct {
	PyObject_HEAD
	int major, minor, patch;
} VersionObject;

static void Version_dealloc(VersionObject *self) {
	Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Version_init(VersionObject *self, PyObject *args) {
	if (!PyArg_ParseTuple(args, "iii", &self->major, &self->minor, &self->patch)) {
		return -1;
	}

	return 0;
}

static PyMemberDef Version_members[] = {
	{ "major", T_INT, offsetof(VersionObject, major), 0, "Major version number" },
	{ "minor", T_INT, offsetof(VersionObject, minor), 0, "Minor version number" },
	{ "patch", T_INT, offsetof(VersionObject, patch), 0, "Patch version number" },
	{ NULL }
};

static PyTypeObject VersionType = {
	PyVarObject_HEAD_INIT(NULL, 0)
		.tp_name = "jdx.Version",
		.tp_doc = "Version object",
		.tp_basicsize = sizeof(VersionObject),
		.tp_itemsize = 0,
		.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
		.tp_new = PyType_GenericNew,
		.tp_init = (initproc) Version_init,
		.tp_dealloc = (destructor) Version_dealloc,
		.tp_members = Version_members
};

typedef struct {
	PyObject_HEAD
	VersionObject *version;

	int image_width, image_height, bit_depth;
	long long item_count;
} HeaderObject;

static int Header__init(HeaderObject *self, PyObject *args) {
	PyObject *version;

	if (!PyArg_ParseTuple(args, "Oiiii", &version, &self->image_width, &self->image_height, &self->bit_depth, &self->item_count)) {
		return -1;
	}

	PyObject *tmp;
	if (version) {
		tmp = self->version;
		Py_INCREF(version);
		self->version = version;
		Py_XDECREF(tmp);
	}

	return 0;
}

static PyMemberDef Header_members[] = {
	{ "version", T_OBJECT_EX, offsetof(HeaderObject, version), 0, "Version object" },
	{ "image_width", T_INT, offsetof(HeaderObject, image_width), 0, "Image width" },
	{ "image_height", T_INT, offsetof(HeaderObject, image_height), 0, "Image height" },
	{ "bit_depth", T_INT, offsetof(HeaderObject, bit_depth), 0, "Bit depth" },
	{ "item_count", T_INT, offsetof(HeaderObject, item_count), 0, "Item count" },
	{ NULL }
};

static PyTypeObject HeaderType = {
	PyVarObject_HEAD_INIT(NULL, 0)
		.tp_name = "jdx.Header",
		.tp_doc = "Header object",
		.tp_basicsize = sizeof(HeaderObject),
		.tp_itemsize = 0,
		.tp_flags = Py_TPFLAGS_DEFAULT,
		.tp_new = PyType_GenericNew,
		.tp_init = (initproc) Header__init,
		.tp_members = Header_members
};

typedef struct {
	PyObject_HEAD
	HeaderObject *header;
	PyObject *items;
} DatasetObject;

static int Dataset__init(DatasetObject *self, PyObject *args) {
	PyObject *header, *items;
	if (!PyArg_ParseTuple(args, "OO", &header, &items)) {
		return -1;
	}

	if (!PyObject_TypeCheck(header, &HeaderType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be of type Header.");
		return -1;
	}

	if (!PyList_Check(items)) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be a list of Items.");
		return -1;
	}

	if (header) {
		Py_XDECREF(self->header);
		Py_INCREF(header);
		self->header = header;
	}

	PyObject *tmp;
	if (items) {
		tmp = self->items;
		Py_INCREF(items);
		self->items = items;
		Py_XDECREF(tmp);
	}

	return 0;
}

static PyMemberDef Dataset_members[] = {
	{ "header", T_OBJECT_EX, offsetof(DatasetObject, header), 0, "Header object" },
	{ "items", T_OBJECT_EX, offsetof(DatasetObject, items), 0, "List of items" },
	{ NULL }
};

static PyTypeObject DatasetType = {
	PyVarObject_HEAD_INIT(NULL, 0)
		.tp_name = "jdx.Dataset",
		.tp_doc = "Dataset object",
		.tp_basicsize = sizeof(DatasetObject),
		.tp_itemsize = 0,
		.tp_flags = Py_TPFLAGS_DEFAULT,
		.tp_new = PyType_GenericNew,
		.tp_init = Dataset__init,
		.tp_members = Dataset_members
};

static PyMethodDef jdxMethods[] = {
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef jdxModule = {
	PyModuleDef_HEAD_INIT,
	"jdx",
	"Python wrapper for libjdx.",
	-1,
	jdxMethods
};

PyMODINIT_FUNC PyInit_jdx(void) {
	if (PyType_Ready(&VersionType) < 0 || PyType_Ready(&HeaderType) < 0 || PyType_Ready(&DatasetType) < 0) return NULL;
	Py_INCREF(&VersionType);
	Py_INCREF(&HeaderType);
	Py_INCREF(&DatasetType);

	PyObject *module = PyModule_Create(&jdxModule);
	if (module == NULL) return NULL;

	if (
		PyModule_AddObject(module, "Version", (PyObject *) &VersionType) < 0 ||
		PyModule_AddObject(module, "Header", (PyObject *) &HeaderType) < 0 ||
		PyModule_AddObject(module, "Dataset", (PyObject *) &DatasetType) < 0
	) {
		Py_DECREF(&VersionType);
		Py_DECREF(&HeaderType);
		Py_DECREF(&DatasetType);
		Py_DECREF(module);

		return NULL;
	}

	return module;
}
