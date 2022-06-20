from jdx import Dataset
import unittest
import os

class TestDataset(unittest.TestCase):
	def test_read(self):
		Dataset.read_from("res/example.jdx")
	
	def test_write(self):
		example_dataset = Dataset.read_from("res/example.jdx")
		example_dataset.write_to("res/temp.jdx")

		read_dataset = Dataset.read_from("res/temp.jdx")
		self.assertEqual(read_dataset, example_dataset)

		os.remove("res/temp.jdx")

if __name__ == "__main__":
	unittest.main()
