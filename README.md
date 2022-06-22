# JDX Python

jdx-python is a Python wrapper around libjdx, the low-level C library that manages JDX files directly. It can be used in Python projects (particularly those involving machine learning) to simplify your storage of classified images alongside using the [JDX Command Line Tool](https://github.com/jeffreycshelton/jdx-clt) to create and manage JDX files from the terminal.

## Usage

jdx-python is not yet listed on [PyPi](pypi.org), so it can't be directly installed using `pip install`. However, it will be listed there as soon as it's ready for release. You can install jdx-python directly onto your machine by cloning this repository, navigating to its directory, and typing in your terminal:

`$ python3 setup.py install`

## Examples

```python
from jdx import Dataset

dset = Dataset.read_from("path/to/file.jdx")

# Get width, height, and bit depth of the images
# NOTE: These values are guaranteed to be the same across all images in the dataset
width = dset.header.image_width
height = dset.header.image_height
bit_depth = dset.header.bit_depth

# Each possible value of 'header.bit_depth' corresponds to the number of bits in each pixel:
# - 8 bits => 1 byte per pixel (luma)
# - 24 bits => 3 bytes per pixel (red, green, blue)
# - 32 bits => 4 bytes per pixel (red, green, blue, alpha)

for (image, label) in dset:
	# Each image is NumPy array consisting of its raw pixel data in the shape (width, height, bytes-per-pixel)
	# NOTE: For color images, data is stored in either RGB or RGBA format. OpenCV uses BGR format, so a 'cv.cvtColor' call is required to convert it for use with OpenCV.

	# Individual pixels can be accessed using normal indexing of a 3D NumPy array:
	px = image[y, x]

	# Labels are int values which correspond to a class in the 'classes' field of the header
	# The label can be used to get the class name as a string:
	class_name = dset.get_class(label)

# Additionally, a dataset can be indexed using the Dataset.get method
(third_image, third_label) = dset.get(2)
```

## Development

jdx-python and the rest of the [JDX Project](https://github.com/jeffreycshelton/jdx) are in early development and under constant change. New features, bug fixes, documentation, and releases will be added frequently, so check back here often! Also, if you enjoy using jdx-python and would like to contribute to its development, please do! Contributions and issue submissions are welcome and help to make JDX a more capable format and tool.

## License

The JDX Python wrapper is licensed under the [MIT License](LICENSE).