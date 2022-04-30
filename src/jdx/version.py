from __future__ import annotations
from io import BufferedReader
from enum import Enum

class BuildType(Enum):
	DEV = 0
	ALPHA = 1
	BETA = 2
	RC = 3
	RELEASE = 4

class Version:
	def __init__(self, major: int, minor: int, patch: int, build_type: BuildType):
		self.major = major
		self.minor = minor
		self.patch = patch
		self.build_type = build_type

	@staticmethod
	def read_from_file(file: BufferedReader) -> Version:
		return Version(
			int.from_bytes(file.read(1), "little"),
			int.from_bytes(file.read(1), "little"),
			int.from_bytes(file.read(1), "little"),
			BuildType(int.from_bytes(file.read(1), "little"))
		)
