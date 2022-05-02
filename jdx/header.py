from __future__ import annotations
from io import BufferedReader
from .version import Version
from typing import List

class Header:
	def image_size(self) -> int:
		return (
			self.image_width
			* self.image_height
			* self.bit_depth // 8
		)

	def __init__(self, version: Version, image_width: int, image_height: int, bit_depth: int, image_count: int, labels: List[str]):
		self.version = version
		self.image_width = image_width
		self.image_height = image_height
		self.bit_depth = bit_depth
		self.image_count = image_count
		self.labels = labels

	@staticmethod
	def read_from_path(path: str) -> Header:
		return Header.read_from_file(open(path, "rb"))

	@staticmethod
	def read_from_file(file: BufferedReader) -> Header:
		if file.read(3) != b"JDX": # Corruption check
			raise IOError
		
		version = Version.read_from_file(file)
		image_width = int.from_bytes(file.read(2), "little")
		image_height = int.from_bytes(file.read(2), "little")
		bit_depth = int.from_bytes(file.read(1), "little")
		label_bytes = int.from_bytes(file.read(4), "little")
		image_count = int.from_bytes(file.read(8), "little")

		labels = [label.decode("utf-8") for label in file.read(label_bytes).split(b'\0') if label]

		return Header(version, image_width, image_height, bit_depth, image_count, labels)

	def write_to_file(self, file: BufferedReader):
		file.write(b"JDX")
		self.version.write_to_file(file)

		file.write(self.image_width.to_bytes(2, "little"))
		file.write(self.image_height.to_bytes(2, "little"))
		file.write(self.bit_depth.to_bytes(1, "little"))

		raw_labels = b"".join([label.encode("utf-8") + b"\0" for label in self.labels])
		
		file.write(len(raw_labels).to_bytes(4, "little"))
		file.write(self.image_count.to_bytes(8, "little"))
		file.write(raw_labels)
		file.flush()
