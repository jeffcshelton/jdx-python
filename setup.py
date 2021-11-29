from setuptools import Extension, setup
import os

def main():
	PROJ_DIR = os.path.dirname(os.path.abspath(__file__))
	LIBJDX_DIR = os.path.join(PROJ_DIR, "libjdx")

	os.system("git submodule update --init --recursive")
	os.system(f"cd {LIBJDX_DIR} && make")

	jdx = Extension(
		"jdx",
		sources=[
			os.path.join(PROJ_DIR, "wrapper.c")
		],
		include_dirs=[os.path.join(LIBJDX_DIR, "include")],
		extra_objects=[os.path.join(LIBJDX_DIR, "lib/libjdx.a")]
	)

	setup(
		name="jdx",
		version="0.2.0",
		description="Python wrapper library for libjdx.",
		author="Jeffrey Shelton",
		author_email="jeffrey.shelton.dev@gmail.com",
		url="https://github.com/jeffreycshelton/jdx-python",
		license="MIT",
		ext_modules=[jdx]
	)

if __name__ == "__main__":
	main()