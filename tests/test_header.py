from jdx import Version
from jdx import Header
import unittest

class TestHeader(unittest.TestCase):
	def test_read(self):
		header = Header.read_from("res/example.jdx")

		self.assertTrue(header.version == Version.V0)
		self.assertEqual(header.image_width, 52)
		self.assertEqual(header.image_height, 52)
		self.assertEqual(header.bit_depth, 24)
		self.assertEqual(header.image_count, 8)

if __name__ == "__main__":
	unittest.main()
