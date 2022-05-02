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
	@staticmethod
	def current() -> Version:
		return Version(0, 4, 0, BuildType.DEV)

	def __eq__(self, other: Version) -> bool:
		return (
			self.major == other.major
			and self.minor == other.minor
			and self.patch == other.patch
			and self.build_type == other.build_type
		)

	def __init__(self, major: int, minor: int, patch: int, build_type: BuildType):
		self.major = major
		self.minor = minor
		self.patch = patch
		self.build_type = build_type

	def __repr__(self) -> str:
		return f"Version {{ major: {self.major}, minor: {self.minor}, patch: {self.patch}, build_type: {self.build_type} }}"

	def __str__(self) -> str:
		build_str = [
			" (dev build)",
			"-alpha",
			"-beta",
			"-rc",
			""
		][self.build_type.value]

		return f"v{self.major}.{self.minor}.{self.patch}{build_str}"

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
