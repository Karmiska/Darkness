from HistoryIterator import HistoryIterException
from HistoryIterator import HistoryIter
#from itertools import imap

class Token:
	def __init__(self):
		self.value = ''
		self.linenumber = 0
		self.type = ''

def valid_identifier_first_character(chr):
	return chr.isalpha() or chr == '_'

def valid_identifier_character(chr):
	return chr.isalpha() or chr.isdigit() or chr == '_'

qualifiers = [
	'static',
	'const',
	'volatile'
	]

complete_system_types = [
	'void',
	'bool',
	'int',		# 32 bit signed integer
	'uint',		# 32 bit unsigned integer
	'dword',	# 32 bit unsigned integer
	'half',		# 16 bit floating point
	'float',	# 32 bit floating point
	'double',	# 64 bit floating point

	'int1',	  'int2',   'int3',   'int4',
	'int1x1', 'int2x1', 'int3x1', 'int4x1',
	'int1x2', 'int2x2', 'int3x2', 'int4x2',
	'int1x3', 'int2x3', 'int3x3', 'int4x3',
	'int1x4', 'int2x4', 'int3x4', 'int4x4',

	'uint1',  'uint2',   'uint3',   'uint4',
	'uint1x1', 'uint2x1', 'uint3x1', 'uint4x1',
	'uint1x2', 'uint2x2', 'uint3x2', 'uint4x2',
	'uint1x3', 'uint2x3', 'uint3x3', 'uint4x3',
	'uint1x4', 'uint2x4', 'uint3x4', 'uint4x4',
	
	'float1',   'float2',   'float3',   'float4',
	'float1x1', 'float2x1',	'float3x1',	'float4x1',
	'float1x2',	'float2x2',	'float3x2',	'float4x2',
	'float1x3',	'float2x3',	'float3x3',	'float4x3',
	'float1x4',	'float2x4',	'float3x4',	'float4x4',

	'double1',   'double2',   'double3',   'double4',
	'double1x1', 'double2x1', 'double3x1', 'double4x1',
	'double1x2', 'double2x2', 'double3x2', 'double4x2',
	'double1x3', 'double2x3', 'double3x3', 'double4x3',
	'double1x4', 'double2x4', 'double3x4', 'double4x4',

	'texture',
	'sampler',
    'SamplerComparisonState',
	'RaytracingAccelerationStructure',
	'ByteAddressBuffer'
	]

templated_system_types = [
	'Buffer',	# Buffer<float4>
	'Texture1D',
	'Texture1DArray',
	'Texture2D',
	'Texture2DArray',
	'Texture3D',
	'TextureCube',
	'TextureCubeArray',

	'Texture2DMS',
	'Texture2DMSArray',

	'RWBuffer',
	'RWByteAddressBuffer',
	'RWStructuredBuffer',
    'AppendStructuredBuffer',
	'RWTexture1D',
	'RWTexture1DArray',
	'RWTexture2D',
	'RWTexture2DArray',
	'RWTexture3D',

    'StructuredBuffer',
	'ConstantBuffer'
	]

arithmetic_operators = {
	'=' : 'assignment',
	'+' : 'addition',
	'-' : 'subtraction',
	# unary+
	# unary-
	'*' : 'multiplication',
	'/' : 'division',
	'%' : 'modulo',
	'++' : 'increment',
	'--' : 'decrement'
	}

comparison_operators = {
	'==' : 'equal',
	'!=' : 'not_equal',
	'>' : 'greater',
	'<' : 'lesser',
	'>=' : 'greater_equal',
	'<=' : 'lesser_equal'
	}

logical_operators = {
	'!' : 'logical_not',
	'&&' : 'logical_and',
	'||' : 'logical_or'
	}

bitwise_operators = {
	'~' : 'bitwise_not',
	'&' : 'bitwise_and',
	'|' : 'bitwise_or',
	'^' : 'bitwise_xor',
	'<<' : 'bitwise_left_shift',
	'>>' : 'bitwise_right_shift'
	}

compound_assignment_operators = {
	'+=' : 'addition_assignment',
	'-=' : 'subtraction_assignment',
	'*=' : 'multiplication_assignment',
	'/=' : 'division_assignment',
	'%=' : 'modulo_assignment',
	'&=' : 'bitwise_and_assignment',
	'|=' : 'bitwise_or_assignment',
	'^=' : 'bitwise_xor_assignment',
	'<<=' : 'bitwise_left_shift_assignment',
	'>>=' : 'bitwise_right_shift_assignment',
	}

class LexicalAnalyzer:
	def __init__(self, contents):
		self.line_number = 1
		self.tokens = []
		
		self.operators = arithmetic_operators
		self.operators.update(comparison_operators)
		self.operators.update(logical_operators)
		self.operators.update(bitwise_operators)
		self.operators.update(compound_assignment_operators)

		self.max_operator_length = max(list(map(len, self.operators)))
		max_iterator_history_length = max(self.max_operator_length, 5)

		self.history_iterator = HistoryIter(contents)
		self.history_iterator.set_history_length(max_iterator_history_length)
		self.last_token = Token()
	
	def __iter__(self):
		return self

	def __next__(self):
		return self.next()

	def next(self):
		return self.getToken(self.history_iterator)

	def getToken(self, data_iterator):
		token = Token()
		while data_iterator:
			try:
				chr = data_iterator.next()
				#print chr
				#if chr == 'r':
				#	print 'interesting'
				if chr == '\n':
					self.line_number += 1

				token.linenumber = self.line_number

				# parse identifiers (words)
				if valid_identifier_first_character(chr):
					word = chr
					chr = data_iterator.next()
					while valid_identifier_character(chr):
						word += chr
						chr = data_iterator.next()
					token.value = word
					if token.value in qualifiers:
						token.type = 'qualifier'
					elif token.value in complete_system_types or token.value in templated_system_types:
						token.type = 'system_type'
					else:
						token.type = 'identifier'

					#if token.type == 'identifier' and token.value == 'planePos':
					#	print 'here'

					data_iterator.prev()
					return token

				# parse pre negation operator
				negative_number = False
				if chr == '-' and self.last_token.type != 'number':
					negative_number = True
					chr = data_iterator.next()
					while chr == ' ':
						chr = data_iterator.next()
				if negative_number:
					if not chr.isdigit():
						data_iterator.prev()

				# parse numbers
				if chr.isdigit():
					number = chr
					chr = data_iterator.next()
					while chr.isdigit() or chr == '.' or chr == 'f':
						number += chr
						chr = data_iterator.next()
					if negative_number:
						token.value = '-'+number
					else:
						token.value = number
					token.type = 'number'
					data_iterator.prev()
					return token
				if chr == '.':
					chr_next = data_iterator.next()
					if chr_next.isdigit():
						number = '0.'+chr_next
						chr_next = data_iterator.next()
						while chr_next.isdigit() or chr_next == 'f':
							number += chr_next
							chr_next = data_iterator.next()
						if negative_number:
							token.value = '-'+number
						else:
							token.value = number
						token.type = 'number'
						data_iterator.prev()
						return token
					else:
						data_iterator.prev()

				# parse operators
				characters = chr
				received_characters = 0
				for x in range(self.max_operator_length-1):
					try:
						recchr = data_iterator.next()
						characters += recchr
						received_characters += 1
					except StopIteration:
						break
				if received_characters > 0:
					data_iterator.prev(received_characters)
				
				for operator in sorted(self.operators, key=len, reverse=True):
					if operator == characters[:len(operator)].replace(" ", ""):
						token.value = self.operators[operator]
						token.type = 'operator'
						for x in range(len(operator)-1):
							data_iterator.next()
						return token
				
				# parse other operators
				if chr == '.':
					token.value = 'dot_operator'
					token.type = 'operator'
					return token
				if chr == ',':
					token.value = 'comma_operator'
					token.type = 'operator'
					return token
				if chr == '(':
					token.value = 'left_parentheses'
					token.type = 'parentheses'
					return token
				if chr == ')':
					token.value = 'right_parentheses'
					token.type = 'parentheses'
					return token
				if chr == '[':
					token.value = 'left_bracket'
					token.type = 'bracket'
					return token
				if chr == ']':
					token.value = 'right_bracket'
					token.type = 'bracket'
					return token
				if chr == '{':
					token.value = 'left_brace'
					token.type = 'brace'
					return token
				if chr == '}':
					token.value = 'right_brace'
					token.type = 'brace'
					return token
				if chr == ';':
					token.value = 'semicolon'
					token.type = 'semicolon'
					return token
				if chr == ':':
					token.value = 'colon'
					token.type = 'colon'
					return token

			except StopIteration:
				token.value = 'eof'
				token.type = 'eof'
				return token
