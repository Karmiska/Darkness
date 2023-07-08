import re
import os.path
from HistoryIterator import HistoryIterException
from HistoryIterator import HistoryIter
from LexicalAnalyzerHLSL import LexicalAnalyzer
from LexicalAnalyzerHLSL import arithmetic_operators
from LexicalAnalyzerHLSL import comparison_operators
from LexicalAnalyzerHLSL import logical_operators
from LexicalAnalyzerHLSL import bitwise_operators
from LexicalAnalyzerHLSL import compound_assignment_operators

class PreprocessorException(Exception):
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return repr(self.value)

keywords = {
	'definition'	: '#define',
	'else'			: '#else',
	'if'			: '#if',
	'if_def'		: '#ifdef',
	'if_not_def'	: '#ifndef',
	'include'		: '#include',
	'undefine'		: '#undef',
	'endif'			: '#endif'
	}

class Preprocessor:
	def __init__(self, file, defines, includes):
		self.open_filename = file.name
		self.include_paths = []
		self.defines = []
		self.in_defined = []
		self.files_included = []
		if includes is not None:
			self.include_paths.extend(includes)
		if defines is not None:
			self.defines.extend(defines)

		self.content_stack = []

		hi = HistoryIter(file.read())
		hi.set_history_length(self.max_keyword_length(keywords))
		self.content_stack.append(hi)

	def __iter__(self):
		return self

	def next(self):
		result = ''
		while len(self.content_stack) > 0:
			try:
				return self.parse()
			except StopIteration:
				if len(self.content_stack) > 0:
					self.content_stack = self.content_stack[:-1]
					if len(self.content_stack) > 0:
						return self.parse()
		raise StopIteration

	def parse(self):
		contents = self.content_stack[-1:][0]
		for key, value in keywords.iteritems():
			if self.match(contents, value):
				if self.active_code() and value == '#define':
					self.match(contents, ' ') # read away one space if it exists
					rest_of_line = self.read_until(contents, '\n').strip()
					parts = rest_of_line.split(' ')
					if parts[0] not in self.defines:
						self.defines.append(parts[0])
					return '#define'+' '+rest_of_line
				elif self.active_code() and value == '#undef':
					self.match(contents, ' ') # read away one space if it exists
					rest_of_line = self.read_until(contents, '\n').strip()
					parts = rest_of_line.split(' ')
					if parts[0] in self.defines:
						self.defines.remove(parts[0])
					return ''
				elif self.active_code() and value == '#include':
					self.match(contents, ' ') # read away one space if it exists
					rest_of_line = self.read_until(contents, '\n').strip().replace('"', '')
					include_file = self.locate_include(rest_of_line)

					if include_file not in self.files_included:
						self.files_included.append(include_file)
						with open(include_file, 'r') as file:
							include_contents = HistoryIter(file.read())
							include_contents.set_history_length(self.max_keyword_length(keywords))
							self.content_stack.append(include_contents)
							return self.parse()
					return ''
				elif value == '#ifdef':
					self.match(contents, ' ') # read away one space if it exists
					rest_of_line = self.read_until(contents, '\n').strip()
					parts = rest_of_line.split(' ')
					self.in_defined.append({ 'define' : parts[0], 'true': parts[0] in self.defines })
					return ''
				elif value == '#ifndef':
					self.match(contents, ' ') # read away one space if it exists
					rest_of_line = self.read_until(contents, '\n').strip()
					parts = rest_of_line.split(' ')
					self.in_defined.append({ 'define' : parts[0], 'true': False })
					return ''
				elif value == '#else':
					self.in_defined[-1:][0]['true'] = not self.in_defined[-1:][0]['true']
					return ''
				elif value == '#endif':
					self.in_defined.pop()
					return ''
				elif value == '#if':
					rest_of_line = self.read_until(contents, '\n')
					lexer = LexicalAnalyzer(rest_of_line)
					lexer_tokens = []
					for ltoken in lexer:
						if ltoken.value == 'eof':
							break
						lexer_tokens.append(ltoken)

					for x in range(len(lexer_tokens)):
						if lexer_tokens[x].value == 'defined':
							lexer_tokens[x].value = 'self.is_defined'
						elif lexer_tokens[x].type == 'operator':
							if lexer_tokens[x].value == 'logical_not':
								lexer_tokens[x].value = ' not '
							if lexer_tokens[x].value == 'logical_and':
								lexer_tokens[x].value = ' and '
							if lexer_tokens[x].value == 'logical_or':
								lexer_tokens[x].value = ' or '
						elif lexer_tokens[x].value == 'left_parentheses':
							lexer_tokens[x].value = '('
						elif lexer_tokens[x].value == 'right_parentheses':
							lexer_tokens[x].value = ')'
						elif lexer_tokens[x].type == 'identifier':
							lexer_tokens[x].value = '"' + lexer_tokens[x].value + '"'
						elif lexer_tokens[x].value == 'eof':
							break

					evaluation_str = "".join([token.value for token in lexer_tokens])
					self.in_defined.append({'define': 'eval', 'true' : eval(evaluation_str)})
					return ''

		if self.match(contents, '/*'):
			self.in_defined.append({'define': 'eval', 'true' : False})
		if self.match(contents, '*/'):
			self.in_defined.pop()
		if self.match(contents, '//'):
			rest_of_line = self.read_until(contents, '\n')

		if self.active_code():
			return contents.next()
		else:
			contents.next()
			return ''

	def is_defined(self, define):
		return define in self.defines
								
	def active_code(self):
		result = True
		for defin in self.in_defined:
			if not defin['true']:
				return False
		return result

	def locate_include(self, filename):
		# first try from includes
		for include in self.include_paths:
			filepath = os.path.normpath(os.path.join(include, filename.strip()))
			if os.path.exists(filepath):
				return filepath

		# try relative to the file itself
		filepath = os.path.normpath(os.path.join(os.path.dirname(self.open_filename), filename.strip()))
		if os.path.exists(filepath):
			return filepath

		# then try absolute
		filepath = os.path.abspath(filename)
		if os.path.exists(filepath):
			return filepath

		# try going through existing includes and their paths
		for include in self.files_included:
			filepath = os.path.normpath(os.path.join(os.path.dirname(include), filename.strip()))
			if os.path.exists(filepath):
				return filepath

		print('Preprocessor coult not locate include: ' + filename)
		raise PreprocessorException(1)				

	def max_keyword_length(self, keywords):
		max_len = 0
		for key, value in iter(keywords.items()):
			if len(value) > max_len:
				max_len = len(value)
		return max_len

	def read_until(self, iterator, char):
		if isinstance(char, list):
			result = ''
			chr = ''
			while chr not in char:
				chr = iterator.next()
				if chr not in char:
					result += chr
			iterator.prev()
			return result
		else:
			result = ''
			chr = ''
			while chr != char:
				chr = iterator.next()
				if chr != char:
					result += chr
			iterator.prev()
			return result

	def match(self, iterator, word):
		reverse = 0
		result = True
		for chr in word:
			next_chr = iterator.next()
			reverse += 1
			if chr != next_chr:
				result = False
				break
		if not result and reverse > 0:
			iterator.prev(reverse)
		return result

