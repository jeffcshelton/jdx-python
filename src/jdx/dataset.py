from __future__ import annotations
from io import BufferedReader
from .header import Header
import zlib

class Dataset:
	# TODO: Change to using Numpy
	def __init__(self, header: Header, raw_data: bytes):
		self.header = header
		self.raw_data = raw_data

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
