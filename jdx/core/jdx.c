#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "libjdx.h"

typedef struct {
	PyObject_HEAD
} HeaderObject;

static PyTypeObject HeaderType = {
	PyObject_HEAD_INIT(NULL)
		.tp_name = "jdx.Header",
		.tp_doc = "Header object",
		.tp_basicsize = sizeof(HeaderObject),
		.tp_itemsize = 0,
		.tp_flags = Py_TPFLAGS_DEFAULT,
		.tp_new = PyType_GenericNew
};

typedef struct {
	PyObject_HEAD
} DatasetObject;

static PyTypeObject DatasetType = {
	PyObject_HEAD_INIT(NULL)
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
	if (PyType_Ready(&HeaderType) < 0 || PyType_Ready(&DatasetType) < 0) return NULL;
	Py_INCREF(&HeaderType);
	Py_INCREF(&DatasetType);

	PyObject *module = PyModule_Create(&jdxModule);
	if (module == NULL) return NULL;

	if (PyModule_AddObject(module, "Header", (PyObject *) &HeaderType) < 0) {
		Py_DECREF(&HeaderType);
		Py_DECREF(&DatasetType);
		Py_DECREF(module);

		return NULL;
	}

	if (PyModule_AddObject(module, "Dataset", (PyObject *) &DatasetType) < 0) {
		Py_DECREF(&HeaderType);
		Py_DECREF(&DatasetType);
		Py_DECREF(module);

		return NULL;
	}

	return module;
}
