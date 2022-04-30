from __future__ import annotations
from io import BufferedReader
from enum import IntEnum

class BuildType(IntEnum):
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

	def write_to_file(self, file: BufferedReader):
		file.write(self.major.to_bytes(1, "little"))
		file.write(self.minor.to_bytes(1, "little"))
		file.write(self.patch.to_bytes(1, "little"))
		file.write(self.build_type.value.to_bytes(1, "little"))
		file.flush()
