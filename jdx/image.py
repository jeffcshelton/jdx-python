from typing import Sequence
import numpy as np

_LABEL_BYTES = 2

class Image:
	def __init__(self, raw: np.ndarray, labels: Sequence[str]):
		self.data = raw[:-_LABEL_BYTES]
		self.label_num = int.from_bytes(raw[-2:], "little")
		self.label = labels[self.label_num]
