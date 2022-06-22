from setuptools import setup

if __name__ == "__main__":
	with open("README.md") as file:
		readme = file.read()

	setup(
		name="jdx",
		version="0.5.0",
		author="Jeffrey Shelton",
		author_email="jeffrey.shelton.c@gmail.com",
		description="Library for reading JDX files",
		long_description=readme,
		url="https://github.com/jeffreycshelton/jdx-python",
		license="MIT",
		packages=["jdx"],
		install_requires=["numpy"]
	)
