from __future__ import annotations
from io import BufferedReader
from enum import Enum

class Version(Enum):
	V0 = 0

	def __repr__(self) -> str:
		return f"Version.V{self.value}"

	def __str__(self) -> str:
		return f"v{self.value}"

	@staticmethod
	def _read_from_file(file: BufferedReader) -> Version:
		return Version(int.from_bytes(file.read(1), "little"))

	def _write_to_file(self, file: BufferedReader):
		file.write(self.value.to_bytes(1, "little"))
		file.flush()
