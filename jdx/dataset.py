from __future__ import annotations

from .image import Image, _LABEL_BYTES
from io import BufferedReader
from .header import Header
import numpy as np
import zlib

class Dataset:
	def __eq__(self, other: Dataset) -> bool:
		return (
			self.header == other.header
			and np.array_equal(self._raw_data, other._raw_data)
		)

	def __init__(self, header: Header, raw_data: bytes):
		if len(raw_data) % (header.image_size() + _LABEL_BYTES) != 0:
			raise ValueError

		self.header = header
		self._raw_data = np.frombuffer(raw_data, dtype=np.uint8)

	def __iter__(self) -> DatasetIterator:
		return DatasetIterator(self)

	@staticmethod
	def read_from_path(path: str | BufferedReader) -> Dataset:
		with open(path, "rb") as file:
			return Dataset.read_from_file(file)

	@staticmethod
	def read_from_file(file: BufferedReader) -> Dataset:
		header = Header.read_from_file(file)
		body_size = int.from_bytes(file.read(8), "little")

		compressed_body = file.read(body_size)
		decompressed_body = zlib.decompress(compressed_body, wbits=-15) # wbits parameter allows it to not have a zlib header & trailer
		
		return Dataset(header, decompressed_body)

	def write_to_path(self, path: str):
		with open(path, "wb") as file:
			self.write_to_file(file)

	def write_to_file(self, file: BufferedReader):
		self.header.write_to_file(file)

		compressed_body = zlib.compress(self._raw_data, 9)[2:-4]

		file.write(len(compressed_body).to_bytes(8, "little"))
		file.write(compressed_body)
		file.flush()

class DatasetIterator:
	def __init__(self, dataset: Dataset):
		self._raw_data = dataset._raw_data
		self._labels = dataset.header.labels
		self._step_size = dataset.header.image_size() + _LABEL_BYTES
		self._offset = 0

	def __next__(self) -> Image:
		start_block = self._offset
		end_block = start_block + self._step_size

		if end_block >= len(self._raw_data):
			raise StopIteration

		self._offset = end_block

		return Image(self._raw_data[start_block:end_block], self._labels)
