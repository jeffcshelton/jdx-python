from __future__ import annotations

from io import BufferedReader, BufferedWriter
from typing import Sequence, Union
from .version import Version

class Header:
	def image_size(self) -> int:
		return (
			self.image_width
			* self.image_height
			* self.bit_depth // 8
		)

	def __eq__(self, other: Header) -> bool:
		return (
			self.version == other.version
			and self.image_width == other.image_width
			and self.image_height == other.image_height
			and self.bit_depth == other.bit_depth
			and self.image_count == other.image_count
			and self.labels == other.labels
		)

	def __init__(self, version: Version, image_width: int, image_height: int, bit_depth: int, image_count: int, labels: Sequence[str]):
		self.version = version
		self.image_width = image_width
		self.image_height = image_height
		self.bit_depth = bit_depth
		self.image_count = image_count
		self.labels = list(labels)

	@staticmethod
	def read_from(input: Union[str, BufferedReader]) -> Header:
		if type(input) == str:
			file = open(input, "rb")
		elif isinstance(input, BufferedReader):
			file = input
		else:
			raise TypeError

		if file.read(3) != b"JDX": # Corruption check
			raise IOError
		
		version = Version.read_from_file(file)
		image_width = int.from_bytes(file.read(2), "little")
		image_height = int.from_bytes(file.read(2), "little")
		bit_depth = int.from_bytes(file.read(1), "little")
		label_bytes = int.from_bytes(file.read(4), "little")
		image_count = int.from_bytes(file.read(8), "little")

		labels = [label.decode("utf-8") for label in file.read(label_bytes).split(b'\0') if label]

		if type(input) == str:
			file.close()

		return Header(version, image_width, image_height, bit_depth, image_count, labels)

	def write_to(self, output: Union[str, BufferedWriter]):
		if type(output) == str:
			file = open(output, "wb")
		elif isinstance(output, BufferedWriter):
			file = output
		else:
			raise TypeError

		file.write(b"JDX")
		self.version.write_to_file(file)

		file.write(self.image_width.to_bytes(2, "little"))
		file.write(self.image_height.to_bytes(2, "little"))
		file.write(self.bit_depth.to_bytes(1, "little"))

		raw_labels = b"".join([label.encode("utf-8") + b"\0" for label in self.labels])
		
		file.write(len(raw_labels).to_bytes(4, "little"))
		file.write(self.image_count.to_bytes(8, "little"))
		file.write(raw_labels)

		if type(output) == str:
			file.close()
		else:
			file.flush()
