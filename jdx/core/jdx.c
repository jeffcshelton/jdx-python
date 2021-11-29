#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <structmember.h>
#include "libjdx.h"

// TODO: Consider changing 'int' type to 'uint8_t' for direct compatibility with libjdx 'JDXVersion'
typedef struct {
	PyObject_HEAD
	int major, minor, patch;
} VersionObject;

static int Version__init(VersionObject *self, PyObject *args) {
	if (!PyArg_ParseTuple(args, "iii", &self->major, &self->minor, &self->patch)) {
		return -1;
	}

	return 0;
}

static void Version__dealloc(VersionObject *self) {
	Py_TYPE(self)->tp_free(self);
}

static PyObject *Version__str(VersionObject *self) {
	return PyUnicode_FromFormat("v%d.%d.%d", self->major, self->minor, self->patch);
}

static PyObject *Version__current(PyTypeObject *type, PyObject *args) {
	VersionObject *self = (VersionObject *) type->tp_alloc(type, 0);
	self->major = JDX_VERSION.major;
	self->minor = JDX_VERSION.minor;
	self->patch = JDX_VERSION.patch;

	return (PyObject *) self;
}

static PyMemberDef Version_members[] = {
	{ "major", T_INT, offsetof(VersionObject, major), 0, "Major version number" },
	{ "minor", T_INT, offsetof(VersionObject, minor), 0, "Minor version number" },
	{ "patch", T_INT, offsetof(VersionObject, patch), 0, "Patch version number" },
	{ NULL }
};

static PyMethodDef Version_methods[] = {
	{ "current", (PyCFunction) Version__current, METH_NOARGS | METH_CLASS, "Returns the current JDX version of the library." },
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
		.tp_init = (initproc) Version__init,
		.tp_dealloc = (destructor) Version__dealloc,
		.tp_str = (reprfunc) Version__str,
		.tp_members = Version_members,
		.tp_methods = Version_methods
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

	if (!PyObject_TypeCheck(version, &VersionType)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be of type 'jdx.Version'.");
		return -1;
	}

	PyObject *tmp;
	if (version) {
		tmp = (PyObject *) self->version;
		Py_INCREF(version);
		self->version = (VersionObject *) version;
		Py_XDECREF(tmp);
	}

	return 0;
}

static void Header__dealloc(HeaderObject *self) {
	Py_XDECREF(self->version);
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

		VersionObject *version = (VersionObject *) VersionType.tp_alloc(&VersionType, 0);
		version->major = header.version.major;
		version->minor = header.version.minor;
		version->patch = header.version.patch;

		self->version = version;
		self->image_width = (int) header.image_width;
		self->image_height = (int) header.image_height;
		self->bit_depth = (int) header.bit_depth;
		self->item_count = (long long) header.item_count;
	}

	return (PyObject *) self;
}

static PyMemberDef Header_members[] = {
	{ "version", T_OBJECT_EX, offsetof(HeaderObject, version), 0, "Version object" },
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

		// TODO: Simplify this greatly
		VersionObject *version = (VersionObject *) VersionType.tp_alloc(&VersionType, 0);
		version->major = (int) dataset.header.version.major;
		version->minor = (int) dataset.header.version.minor;
		version->patch = (int) dataset.header.version.patch;

		HeaderObject *header = (HeaderObject *) HeaderType.tp_alloc(&HeaderType, 0);
		header->version = version;
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
	if (PyType_Ready(&VersionType) < 0 || PyType_Ready(&HeaderType) < 0 || PyType_Ready(&ItemType) < 0 || PyType_Ready(&DatasetType) < 0) return NULL;
	Py_INCREF(&VersionType);
	Py_INCREF(&HeaderType);
	Py_INCREF(&ItemType);
	Py_INCREF(&DatasetType);

	PyObject *module = PyModule_Create(&jdxModule);
	if (module == NULL) return NULL;

	if (
		PyModule_AddObject(module, "Version", (PyObject *) &VersionType) < 0 ||
		PyModule_AddObject(module, "Header", (PyObject *) &HeaderType) < 0 ||
		PyModule_AddObject(module, "Item", (PyObject *) &ItemType) < 0 ||
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
