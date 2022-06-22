from __future__ import annotations

from io import BufferedReader, BufferedWriter
from typing import Tuple, Union
from .header import Header
import numpy as np
import zlib

_LABEL_BYTES = 2

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

	def get(self, index: int) -> Tuple[np.ndarray, int]:
		start_block = index * (self.header.image_size() + _LABEL_BYTES)
		end_image = start_block + self.header.image_size()
		end_label = end_image + _LABEL_BYTES

		if end_label > len(self._raw_data):
			raise IndexError
		
		image_shape = (
			self.header.image_height,
			self.header.image_width,
			self.header.bit_depth // 8
		)

		return (
			np.reshape(self._raw_data[start_block:end_image], image_shape),
			int.from_bytes(self._raw_data[end_image:end_label], "little")
		)

	def get_class(self, label) -> str:
		return self.header.classes[label]

	@staticmethod
	def read_from(input: Union[str, BufferedReader]) -> Dataset:
		if type(input) == str:
			file = open(input, "rb")
		elif isinstance(input, BufferedReader):
			file = input
		else:
			raise TypeError

		header = Header.read_from(file)
		body_size = int.from_bytes(file.read(8), "little")

		compressed_body = file.read(body_size)
		decompressed_body = zlib.decompress(compressed_body, wbits=-15) # wbits parameter allows it to not have a zlib header & trailer

		if type(input) == str:
			file.close()

		return Dataset(header, decompressed_body)
		
	def write_to(self, output: Union[str, BufferedWriter]):
		if type(output) == str:
			file = open(output, "wb")
		elif isinstance(output, BufferedWriter):
			file = output
		else:
			raise TypeError

		self.header.write_to(file)

		compressed_body = zlib.compress(self._raw_data, 9)[2:-4]
		file.write(len(compressed_body).to_bytes(8, "little"))
		file.write(compressed_body)

		if type(output) == str:
			file.close()
		else:
			file.flush()

class DatasetIterator:
	def __init__(self, dataset: Dataset):
		self._dataset = dataset
		self._index = 0

	def __next__(self) -> Tuple[np.ndarray, int]:
		self._index += 1

		return self._dataset.get(self._index - 1)
