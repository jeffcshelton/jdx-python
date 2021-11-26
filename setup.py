from setuptools import Extension, setup
import os

def main():
	PROJ_DIR = os.path.dirname(os.path.abspath(__file__))
	LIBJDX_INCLUDE_DIR = os.path.join(PROJ_DIR, "libjdx/include")

	jdx = Extension(
		"jdx",
		sources=[],
		include_dirs=[LIBJDX_INCLUDE_DIR]
	)

	setup(
		name="jdx",
		version="v0.2.0",
		description="Python wrapper library for libjdx.",
		author="Jeffrey Shelton",
		author_email="jeffrey.shelton.dev@gmail.com",
		url="https://github.com/jeffreycshelton/jdx-python",
		license="MIT",
		ext_modules=[jdx]
	)

if __name__ == "__main__":
	main()