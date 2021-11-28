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

static PyObject *Version_new(PyTypeObject *type, PyObject *args) {
	VersionObject *self = type->tp_alloc(type, 0);

	if (self) {
		self->major = -1;
		self->minor = -1;
		self->patch = -1;
	}

	return (PyObject *) self;
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
		.tp_new = Version_new,
		.tp_init = (initproc) Version_init,
		.tp_dealloc = (destructor) Version_dealloc,
		.tp_members = Version_members
};

typedef struct {
	PyObject_HEAD
	VersionObject *version;

	int image_width, image_height, bit_depth;
	long long item_count, compressed_size;
} HeaderObject;

static int Header_init(HeaderObject *self, PyObject *args) {
	PyObject *version;

	if (!PyArg_ParseTuple(args, "oiiiii", &version, &self->image_width, &self->image_height, &self->bit_depth, &self->item_count, &self->compressed_size)) {
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

static PyTypeObject HeaderType = {
	PyVarObject_HEAD_INIT(NULL, 0)
		.tp_name = "jdx.Header",
		.tp_doc = "Header object",
		.tp_basicsize = sizeof(HeaderObject),
		.tp_itemsize = 0,
		.tp_flags = Py_TPFLAGS_DEFAULT,
		.tp_init = (initproc) Header_init,
		.tp_new = PyType_GenericNew
};

typedef struct {
	PyObject_HEAD
} DatasetObject;

static PyTypeObject DatasetType = {
	PyVarObject_HEAD_INIT(NULL, 0)
		.tp_name = "jdx.Dataset",
		.tp_doc = "Dataset object",
		.tp_basicsize = sizeof(DatasetObject),
		.tp_itemsize = 0,
		.tp_flags = Py_TPFLAGS_DEFAULT,
		.tp_new = PyType_GenericNew
};

static PyObject *read_header_from_path(PyObject *self, PyObject *args) {
	const char *path;

	if (!PyArg_ParseTuple(args, "s", &path)) {
		return NULL;
	}

	JDXHeader header;
	JDX_ReadHeaderFromPath(&header, path);
	printf("Item count: %llu\n", header.item_count);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *read_dataset_from_path(PyObject *self, PyObject *args) {
	const char *path;

	if (!PyArg_ParseTuple(args, "s", &path)) {
		return NULL;
	}

	JDXDataset dataset;
	JDX_ReadDatasetFromPath(&dataset, path);
	printf("Item count: %llu\n", dataset.header.item_count);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef jdxMethods[] = {
	{ "read_header_from_path", read_header_from_path, METH_VARARGS, "Reads JDX Header from given path." },
	{ "read_dataset_from_path", read_dataset_from_path, METH_VARARGS, "Reads JDX Dataset from given path." },
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
