#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <structmember.h>
#include "libjdx.h"

typedef struct {
	PyObject_HEAD
	int image_width, image_height, bit_depth;
	long long item_count;
} HeaderObject;

static int Header__init(HeaderObject *self, PyObject *args) {
	if (!PyArg_ParseTuple(args, "Oiiii", &self->image_width, &self->image_height, &self->bit_depth, &self->item_count)) {
		return -1;
	}

	return 0;
}

static void Header__dealloc(HeaderObject *self) {
	Py_TYPE(self)->tp_free(self);
}

static PyObject *Header__read_from_path(PyTypeObject *type, PyObject *args) {
	const char *path;

	if (!PyArg_ParseTuple(args, "s", &path)) {
		return NULL;
	}

	HeaderObject *self = (HeaderObject *) type->tp_alloc(type, 0);
	if (self) {
		JDXHeader header;
		JDXError error = JDX_ReadHeaderFromPath(&header, path);

		if (error) {
			PyErr_SetString(PyExc_Exception, "Failed to read header from file.");
			Py_DECREF(self);
			return NULL;
		}

		self->image_width = (int) header.image_width;
		self->image_height = (int) header.image_height;
		self->bit_depth = (int) header.bit_depth;
		self->item_count = (long long) header.item_count;
	}

	return (PyObject *) self;
}

static PyMemberDef Header_members[] = {
	{ "image_width", T_INT, offsetof(HeaderObject, image_width), 0, "Image width" },
	{ "image_height", T_INT, offsetof(HeaderObject, image_height), 0, "Image height" },
	{ "bit_depth", T_INT, offsetof(HeaderObject, bit_depth), 0, "Bit depth" },
	{ "item_count", T_INT, offsetof(HeaderObject, item_count), 0, "Item count" },
	{ NULL }
};

static PyMethodDef Header_methods[] = {
	{ "read_from_path", (PyCFunction) Header__read_from_path, METH_VARARGS | METH_CLASS, "Reads only the header from a JDX file." },
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
		.tp_dealloc = (destructor) Header__dealloc,
		.tp_members = Header_members,
		.tp_methods = Header_methods
};

typedef struct {
	PyObject_HEAD;
	PyObject *data;
	int label;
} ItemObject;

static int Item__init(ItemObject *self, PyObject *args) {
	PyObject *data;

	if (!PyArg_ParseTuple(args, "Oi", &data, &self->label)) {
		return -1;
	}

	if (!PyBytes_CheckExact(data)) {
		PyErr_SetString(PyExc_TypeError, "Second argument (label) must be of type 'bytes'.");
		return -1;
	}

	PyObject *tmp;
	if (data) {
		tmp = self->data;
		Py_INCREF(data);
		self->data = data;
		Py_XDECREF(tmp);
	}

	return 0;
}

static void Item__dealloc(ItemObject *self) {
	Py_XDECREF(self->data);
	Py_TYPE(self)->tp_free(self);
}

static PyMemberDef Item_members[] = {
	{ "data", T_OBJECT_EX, offsetof(ItemObject, data), 0, "Item byte data" },
	{ "label", T_INT, offsetof(ItemObject, label), 0, "Integer label" },
	{ NULL }
};

static PyTypeObject ItemType = {
	PyVarObject_HEAD_INIT(NULL, 0)
		.tp_name = "jdx.Item",
		.tp_doc = "JDX Item object",
		.tp_basicsize = sizeof(ItemObject),
		.tp_itemsize = 0,
		.tp_flags = Py_TPFLAGS_DEFAULT,
		.tp_new = PyType_GenericNew,
		.tp_init = (initproc) Item__init,
		.tp_dealloc = (destructor) Item__dealloc,
		.tp_members = Item_members
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
		self->header = (HeaderObject *) header;
	}

	PyObject *tmp;
	if (items) {
		tmp = (PyObject *) self->items;
		Py_INCREF(items);
		self->items = items;
		Py_XDECREF(tmp);
	}

	return 0;
}

static void Dataset__dealloc(DatasetObject *self) {
	Py_XDECREF(self->header);
	Py_TYPE(self)->tp_free(self);
}

static PyObject *Dataset__read_from_path(PyTypeObject *type, PyObject *args) {
	const char *path;

	if (!PyArg_ParseTuple(args, "s", &path)) {
		return NULL;
	}

	DatasetObject *self = (DatasetObject *) type->tp_alloc(type, 0);
	if (self) {
		JDXDataset dataset;
		JDXError error = JDX_ReadDatasetFromPath(&dataset, path);

		if (error) {
			PyErr_SetString(PyExc_Exception, "Failed to read dataset from file.");
			Py_DECREF(self);
			return NULL;
		}

		HeaderObject *header = (HeaderObject *) HeaderType.tp_alloc(&HeaderType, 0);
		header->image_width = (int) dataset.header.image_width;
		header->image_height = (int) dataset.header.image_height;
		header->bit_depth = (int) dataset.header.bit_depth;
		header->item_count = (long long) dataset.header.item_count;

		size_t image_size = (
			(size_t) dataset.header.image_width *
			(size_t) dataset.header.image_height *
			(size_t) dataset.header.bit_depth
		);

		PyObject *items = PyList_New(dataset.header.item_count);
		for (uint64_t i = 0; i < dataset.header.item_count; i++) {
			ItemObject *item = (ItemObject *) ItemType.tp_alloc(&ItemType, 0);
			item->data = PyBytes_FromStringAndSize((char *) dataset.items[i].data, image_size);
			item->label = (int) dataset.items[i].label;

			PyList_SetItem(items, i, (PyObject *) item);
		}

		self->header = header;
		self->items = items;
		JDX_FreeDataset(dataset);
	}

	return (PyObject *) self;
}

static PyMemberDef Dataset_members[] = {
	{ "header", T_OBJECT_EX, offsetof(DatasetObject, header), 0, "Header object" },
	{ "items", T_OBJECT_EX, offsetof(DatasetObject, items), 0, "List of items" },
	{ NULL }
};

static PyMethodDef Dataset_methods[] = {
	{ "read_from_path", (PyCFunction) Dataset__read_from_path, METH_VARARGS | METH_CLASS, "Reads Dataset from JDX file." },
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
		.tp_init = (initproc) Dataset__init,
		.tp_dealloc = (destructor) Dataset__dealloc,
		.tp_members = Dataset_members,
		.tp_methods = Dataset_methods
};

static PyObject *jdx__version(void) {
	const char *build_type_str;

	if (JDX_VERSION.build_type == JDXBuildType_DEV) build_type_str = " (dev build)";
	else if (JDX_VERSION.build_type == JDXBuildType_ALPHA) build_type_str = "-alpha";
	else if (JDX_VERSION.build_type == JDXBuildType_BETA) build_type_str = "-beta";
	else if (JDX_VERSION.build_type == JDXBuildType_RC) build_type_str = "-rc";
	else if (JDX_VERSION.build_type == JDXBuildType_RELEASE) build_type_str = "";

	return PyUnicode_FromFormat("v%u.%u.%u%s", JDX_VERSION.major, JDX_VERSION.minor, JDX_VERSION.patch, build_type_str);
}

static PyMethodDef jdx_methods[] = {
	{ "version", (PyCFunction) jdx__version, METH_NOARGS, "Returns the current JDX version of the library." },
	{ NULL }
};

static struct PyModuleDef jdxModule = {
	PyModuleDef_HEAD_INIT,
	"jdx",
	"Python wrapper for libjdx.",
	-1,
	jdx_methods
};

PyMODINIT_FUNC PyInit_jdx(void) {
	if (PyType_Ready(&HeaderType) < 0 || PyType_Ready(&ItemType) < 0 || PyType_Ready(&DatasetType) < 0) return NULL;
	Py_INCREF(&HeaderType);
	Py_INCREF(&ItemType);
	Py_INCREF(&DatasetType);

	PyObject *module = PyModule_Create(&jdxModule);
	if (module == NULL) return NULL;

	if (
		PyModule_AddObject(module, "Header", (PyObject *) &HeaderType) < 0 ||
		PyModule_AddObject(module, "Item", (PyObject *) &ItemType) < 0 ||
		PyModule_AddObject(module, "Dataset", (PyObject *) &DatasetType) < 0
	) {
		Py_DECREF(&HeaderType);
		Py_DECREF(&ItemType);
		Py_DECREF(&DatasetType);
		Py_DECREF(module);

		return NULL;
	}

	return module;
}
