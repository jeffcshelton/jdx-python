from __future__ import annotations
from io import BufferedReader
from .header import Header
import numpy as np
import zlib

_LABEL_BYTES = 2

class Dataset:
	def __init__(self, header: Header, raw_data: bytes):
		if len(raw_data) % (header.image_size() + _LABEL_BYTES) != 0:
			raise ValueError

		self.header = header
		self._raw_data = np.frombuffer(raw_data, dtype=np.uint8)

	@staticmethod
	def read_from_path(path: str) -> Dataset:
		return Dataset.read_from_file(open(path, "rb"))

	@staticmethod
	def read_from_file(file: BufferedReader) -> Dataset:
		header = Header.read_from_file(file)
		body_size = int.from_bytes(file.read(8), "little")

		compressed_body = file.read(body_size)
		decompressed_body = zlib.decompress(compressed_body, wbits=-15) # wbits parameter allows it to not have a zlib header & trailer
		
		return Dataset(header, decompressed_body)

	def write_to_path(self, path: str):
		self.write_to_file(open(path, "wb"))

	def write_to_file(self, file: BufferedReader):
		self.header.write_to_file(file)

		compressed_body = zlib.compress(self.raw_data, 9)[2:-4]

		file.write(len(compressed_body).to_bytes(8, "little"))
		file.write(compressed_body)
		file.flush()
