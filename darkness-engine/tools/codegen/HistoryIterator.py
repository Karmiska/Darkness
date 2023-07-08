
class HistoryIterException(Exception):
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return repr(self.value)

class HistoryIter(object):
	def __init__(self, *args):
		self._it = iter(*args)
		self.history = []
		self.history_ptr = 0
		self.history_length = 0
		self.count = 0

	def __iter__(self):
		return self

	def count_reset(self):
		self.count = 0

	def count_get(self):
		return self.count;

	def set_history_length(self, len):
		self.history_length = len

	def prev_data_length(self):
		if len(self.history) == 0:
			return 0
		return self.history_ptr

	def current(self):
		if len(self.history) == 0:
			raise HistoryIterException(0)
		if self.history_ptr >= len(self.history):
			raise HistoryIterException(0)
		return self.history[self.history_ptr]

	def next(self):
		self.count += 1

		# length of 0 will cause immediate return
		if self.history_length == 0:
			return self._it.next()

		# history not full and we're on the last cell
		if len(self.history) == 0:
			self.history.append(self._it.next())
			res = self.history[self.history_ptr]
			self.history_ptr = len(self.history)
			return res
		elif len(self.history) < self.history_length and self.history_ptr == len(self.history):
			self.history.append(self._it.next())
			res = self.history[self.history_ptr]
			self.history_ptr = len(self.history)
			return res
		elif len(self.history) == self.history_length and self.history_ptr == len(self.history):
			self.history.append(self._it.next())
			self.history = self.history[1:]
			return self.history[self.history_ptr-1]
		elif self.history_ptr < len(self.history):
			# somewhere in between
			res = self.history[self.history_ptr];
			self.history_ptr += 1
			return res

	def prev(self, count = 1):
		if self.history_ptr == 0:
			raise HistoryIterException(1)
		if self.history_ptr - count < 0:
			raise HistoryIterException(2)
		self.history_ptr = self.history_ptr - count
		return self.history[self.history_ptr]


class HistoryIterTestException(Exception):
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return repr(self.value)

def test_setup():
	test_data = [0, 1, 2, 3, 4, 5]
	hi = HistoryIter(test_data)
	return hi

def test_first_value():
	hi = test_setup()
	print('[TEST HistoryIter]: first value')
	val = hi.next()
	if val != 0:
		raise HistoryIterTestException(3)
	print('[TEST HistoryIter]: first value: PASS')

def test_prev_value_failure():
	hi = test_setup()
	val = hi.next()
	val = hi.next()
	val = hi.next()
	print('[TEST HistoryIter]: prev value fail')
	raised = False
	try:
		val = hi.prev()
	except HistoryIterException:
		raised = True
	if not raised:
		raise HistoryIterTestException(3)
	print('[TEST HistoryIter]: prev value fail: PASS')

def test_history_length_one():
	hi = test_setup()
	hi.set_history_length(1)
	print('[TEST HistoryIter]: test behaviour of history length 1')
	val = hi.next()
	if val != 0:
		raise HistoryIterTestException(3)
	val = hi.prev()
	if val != 0:
		raise HistoryIterTestException(3)
	raised = False
	try:
		val = hi.prev()
	except HistoryIterException:
		raised = True
	if not raised:
		raise HistoryIterTestException(3)
	val = hi.next()
	if val != 0:
		raise HistoryIterTestException(3)
	val = hi.next()
	if val != 1:
		raise HistoryIterTestException(3)
	val = hi.prev()
	if val != 1:
		raise HistoryIterTestException(3)
	val = hi.next()
	if val != 1:
		raise HistoryIterTestException(3)
	val = hi.next()
	if val != 2:
		raise HistoryIterTestException(3)
	val = hi.next()
	if val != 3:
		raise HistoryIterTestException(3)
	val = hi.prev()
	if val != 3:
		raise HistoryIterTestException(3)

	raised = False
	try:
		val = hi.prev()
	except HistoryIterException:
		raised = True
	if not raised:
		raise HistoryIterTestException(3)
	print('[TEST HistoryIter]: test behaviour of history length 1: PASS')

def test_history_eof():
	print('[TEST HistoryIter]: test eof')
	hi = test_setup()
	val = hi.next()
	val = hi.next()
	val = hi.next()
	val = hi.next()
	val = hi.next()

	raised = False
	try:
		val = hi.prev()
	except HistoryIterException:
		raised = True
	if not raised:
		raise HistoryIterTestException(3)
	print('[TEST HistoryIter]: test eof: PASS')

def perform_history_iter_tests():
	test_first_value();
	test_prev_value_failure();
	test_history_length_one();
	test_history_eof();
