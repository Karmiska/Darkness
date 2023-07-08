from LexicalAnalyzerHLSL import Token
from LexicalAnalyzerHLSL import HistoryIterException
from LexicalAnalyzerHLSL import HistoryIter
from LexicalAnalyzerHLSL import complete_system_types
from LexicalAnalyzerHLSL import templated_system_types

parsed_system_types = [
	'vector',	# vector<float, 4>
	'matrix',	# matrix<float, 3, 3>
	'InputPatch',
	'OutputPatch',
	'struct',
	'cbuffer',
	'tbuffer'
	]

system_semantics = [
	'SV_Vertex',
	'SV_VertexID',
	'SV_InstanceID',
	'SV_IsFrontFace',
	'SV_PrimitiveID',
	'SV_Position',
	'SV_ViewportArrayIndex',

	'SV_ClipDistance',
	'SV_CullDistance',
	'SV_Coverage',
	'SV_Depth',
	'SV_DepthGreaterEqual',
	'SV_DepthLessEqual',
	'SV_RenderTargetArrayIndex',
	'SV_SampleIndex',
	'SV_Target',

	# shader model 5.0
	'SV_DispatchThreadID',
	'SV_DomainLocation',
	'SV_GroupID',
	'SV_GroupIndex',
	'SV_GroupThreadID',
	'SV_GSInstanceID',
	'SV_InsideTessFactor',
	'SV_OutputControlPointID',
	'SV_TessFactor',

	# shader model 5.1
	'SV_InnerCoverage',
	'SV_StencilRef'
	]

# user defined types follow this rule:
# typedef vector <int, #> int#;
# typedef matrix <int, #, #> int#x#;

# shader constants follor this rule: (cbuffer, tbuffer)
# BufferType [Name] [: register(b#)]
# {
#     VariableDeclaration [: packoffset(c#.xyzw)];
#      ...
# };

class SyntaxAnalyzerException(Exception):
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return repr(self.value)

class SyntaxNode:
	def __init__(self):
		self.type = ''
		self.name = ''
		self.syntax_type = ''
		self.linenumber = 0
		self.register = ''
		self.childs = []
		self.parameters = []
		self.semantic = ''
		self.qualifiers = []
		self.element_count = 1
		self.value = ''
		self.dimension = ''
		self.format = ''

class SyntaxAnalyzer:
	def __init__(self, tokens):
		token_iterator = HistoryIter(tokens)
		token_iterator.set_history_length(4000)

		self.syntax_nodes = []
		self.known_structures = []
		self.known_templated_types = []
		self.root_node = SyntaxNode()
		self.syntax_nodes.append(self.root_node)
		self.parse_syntax_tokens(token_iterator, self.root_node)

	def root_level_declarations(self):
		result = []
		for node in self.root_node.childs:
			if node.syntax_type == 'declaration':
				result.append(node)
			if node.syntax_type == 'function' and node.name == 'main':
				# we want to reprocess the child list to include only native types
				newChildList = []
				for ch in node.childs:
					if ch.type in complete_system_types:
						newChildList.append(ch)
					elif ch.type in self.known_structures:
						for rn in self.root_node.childs:
							if rn.name == ch.type:
								for rnchild in rn.childs:
									newChildList.append(rnchild)
				node.childs = newChildList
				result.append(node)
		return result

	def readaway(self, token_iterator, expectation, error):
		next_token = token_iterator.next()
		if next_token.value != expectation:
			raise SyntaxAnalyzerException(str(next_token.linenumber)+' ['+next_token.value+']'+': '+error)
		return next_token

	def parse_syntax_tokens(self, token_iterator, node):
		try:
			token = token_iterator.next()
			currentQualifiers = []
			while token.value != 'eof':
				#print token.type + " : " + token.value
				if token.type == 'qualifier':
					currentQualifiers.append(token.value)
				if token.value == 'sampler':
					new_node = SyntaxNode()
					new_node.type = 'sampler'
					new_node.syntax_type = 'declaration'
					next_token = token_iterator.next()
					new_node.name = next_token.value
					new_node.linenumber = token.linenumber
					node.childs.append(new_node)

					next_token = token_iterator.next()
					if next_token.value == 'colon':
						self.readaway(token_iterator, 'register', 'Invalid register description')
						self.readaway(token_iterator, 'left_parentheses', 'Invalid register description')
						
						next_token = token_iterator.next()
						new_node.register = next_token.value

						self.readaway(token_iterator, 'right_parentheses', 'Invalid register description')
						next_token = token_iterator.next()
					elif next_token.value != 'semicolon':
						token_iterator.prev()
					token.value = ''


				if token.value == 'cbuffer' or token.value == 'tbuffer':
					new_node = SyntaxNode()
					new_node.type = token.value
					new_node.syntax_type = 'declaration'
					next_token = token_iterator.next()
					new_node.name = next_token.value
					new_node.linenumber = token.linenumber
					node.childs.append(new_node)

					next_token = token_iterator.next()
					if next_token.value == 'colon':
						self.readaway(token_iterator, 'register', 'Invalid register description')
						self.readaway(token_iterator, 'left_parentheses', 'Invalid register description')
						
						next_token = token_iterator.next()
						new_node.register = next_token.value

						self.readaway(token_iterator, 'right_parentheses', 'Invalid register description')
						next_token = token_iterator.next()

					if next_token.value != 'left_brace':
						raise SyntaxAnalyzerException('Tried to declare cbuffer but did not find body. got value: '+next_token.value)

					self.parse_syntax_tokens(token_iterator, new_node)
					token.value = ''

				if token.value == 'struct':
					new_node = SyntaxNode()
					new_node.type = token.value
					new_node.syntax_type = 'definition'
					next_token = token_iterator.next()
					new_node.name = next_token.value
					new_node.linenumber = token.linenumber
					node.childs.append(new_node)

					if new_node.name not in self.known_structures:
						self.known_structures.append(new_node.name)

					next_token = token_iterator.next()
					if next_token.value != 'left_brace':
						raise SyntaxAnalyzerException('Tried to declare cbuffer but did not find body. got value: '+next_token.value)
					self.parse_syntax_tokens(token_iterator, new_node)
					token.value = ''

				# check for function
				token_iterator.count_reset()
				if token.value in self.known_structures or token.value in complete_system_types:
					name = token_iterator.next()
					if name.type != 'identifier':
						token_iterator.prev(token_iterator.count_get())
					else:
						left_parenth = token_iterator.next()
						if left_parenth.value != 'left_parentheses':
							token_iterator.prev(token_iterator.count_get())
						else:
							new_node = SyntaxNode()
							new_node.type = token.value
							new_node.syntax_type = 'function'
							new_node.name = name.value
							new_node.linenumber = token.linenumber
							node.childs.append(new_node)

							if new_node.name == 'main':
								interesting = True

							self.parse_syntax_tokens(token_iterator, new_node)

							max_lookout = 100000
							cur_lookout = 0
							failure = False
							left_brace = token_iterator.next()
							if left_brace.type == 'colon':
								while left_brace.value != 'left_brace':
									left_brace = token_iterator.next()
							brace_count = 1

							while brace_count > 0:
								ctoken = token_iterator.next()
								if ctoken.value == 'left_brace':
									brace_count += 1
								if ctoken.value == 'right_brace':
									brace_count -= 1
								cur_lookout += 1
								if cur_lookout > max_lookout:
									failure = True
									break
							if failure:
								print('Failed to parse away function body')
								token_iterator.prev(token_iterator.count_get())
							token.value = ''
									
				if token.value in self.known_structures:
					new_node = SyntaxNode()
					new_node.type = token.value
					new_node.syntax_type = 'declaration'
					new_node.linenumber = token.linenumber
					next_token = token_iterator.next()
					new_node.name = next_token.value

					token = token_iterator.next()
					if token.value == 'assignment':
						while token.value != 'semicolon':
							token = token_iterator.next()
					node.childs.append(new_node);

				if token.value in complete_system_types:
					new_node = SyntaxNode()
					new_node.type = token.value
					new_node.syntax_type = 'declaration'
					new_node.linenumber = token.linenumber
					new_node.qualifiers.extend(currentQualifiers)
					currentQualifiers = []
					next_token = token_iterator.next()
					if next_token.type == 'identifier':
						new_node.name = next_token.value
					else:
						token_iterator.prev()
					
					next_token = token_iterator.next()
					if next_token.value == 'comma_operator':
						# this is most likely function parameter list
						# jump right out to handle the next one
						nop = True
					else:
						if next_token.value == 'colon':
							next_token = token_iterator.next()
							new_node.semantic = next_token.value
						elif next_token.value == 'left_bracket':
							# it's an array declaration
							next_token = token_iterator.next()
							try:
								new_node.element_count = int(next_token.value)
							except ValueError as ex:
								new_node.element_count = 0
							next_token = token_iterator.next()
							if next_token.value != 'right_bracket':
								raise SyntaxAnalyzerException('Parsing array right bracket. got: '+next_token.value)
						else:
							token_iterator.prev()

						# might or might not have ; (this is also used with parameters)
						temp_token = token_iterator.next()
						readaway = False
						if temp_token.type == 'operator' and temp_token.value == 'assignment':
							# we have initialization of the value(s)
							next_token = token_iterator.next()
							if next_token.type == 'number':
								new_node.value = next_token.value
							elif next_token.value == 'left_brace':
								# initializer list
								cur_lookout = 0
								max_lookout = 100000
								brace_count = 1
								readaway = True
								failure = False
								while brace_count > 0:
								
									ctoken = token_iterator.next()
									#print ctoken.value
									if ctoken.value == 'left_brace':
										brace_count += 1
									if ctoken.value == 'right_brace':
										brace_count -= 1
									cur_lookout += 1
									if cur_lookout > max_lookout:
										failure = True
										break
								if failure:
									print('Failed to parse away function body')
									token_iterator.prev(token_iterator.count_get())
						if not readaway:
							if temp_token.value != 'semicolon':
								token_iterator.prev()

					node.childs.append(new_node)

				if token.value in templated_system_types:
					new_node = SyntaxNode()
					new_node.type = token.value
					new_node.syntax_type = 'declaration'
					new_node.linenumber = token.linenumber

					if token.value == 'Texture1D':
						new_node.dimension = 'Texture1D'
					if token.value == 'Texture2D':
						new_node.dimension = 'Texture2D'
					if token.value == 'Texture3D':
						new_node.dimension = 'Texture3D'
					if token.value == 'Texture1DArray':
						new_node.dimension = 'Texture1DArray'
					if token.value == 'Texture2DArray':
						new_node.dimension = 'Texture2DArray'
					if token.value == 'TextureCube':
						new_node.dimension = 'TextureCube'
					if token.value == 'TextureCubeArray':
						new_node.dimension = 'TextureCubeArray'

					if token.value == 'Texture2DMS':
						new_node.dimension = 'Texture2D'
					if token.value == 'Texture2DMSArray':
						new_node.dimension = 'Texture2DArray'

					if token.value == 'RWTexture1D':
						new_node.dimension = 'Texture1D'
					if token.value == 'RWTexture2D':
						new_node.dimension = 'Texture2D'
					if token.value == 'RWTexture1DArray':
						new_node.dimension = 'Texture1DArray'
					if token.value == 'RWTexture2DArray':
						new_node.dimension = 'Texture2DArray'
					if token.value == 'RWTexture3D':
						new_node.dimension = 'Texture3D'

					if token.value == 'ConstantBuffer':
						new_node.dimension = 'ConstantBuffer'

					next_token = token_iterator.next()
					if next_token.value == 'lesser':
						next_token = token_iterator.next()
						new_node.format = next_token.value
						new_node.type += '<' + new_node.format + '>'
						self.readaway(token_iterator, 'greater', 'Broken template type')
						next_token = token_iterator.next()
					new_node.name = next_token.value
					self.known_templated_types.append(new_node.type)

					next_token = token_iterator.next()
					if next_token.type == 'bracket' and next_token.value == 'left_bracket':
						while next_token.value is not 'right_bracket':
							next_token = token_iterator.next()
						self.readaway(token_iterator, 'semicolon', 'Missing semicolon')
						new_node.type += 'Bindless'
						node.childs.append(new_node)
					else:
						node.childs.append(new_node)

				if token.value == 'left_parentheses':
					# propably variable initialization.
					# we'll enter the scope
					new_node = SyntaxNode()
					new_node.type = token.value
					new_node.syntax_type = 'parentheses_scope_enter'
					new_node.name = 'parentheses_scope_enter'
					new_node.linenumber = token.linenumber
					node.childs.append(new_node)
					self.parse_syntax_tokens(token_iterator, new_node)
					token.value = ''

				if token.value == 'left_brace':
					# propably variable initialization.
					# we'll enter the scope
					new_node = SyntaxNode()
					new_node.type = token.value
					new_node.syntax_type = 'brace_scope_enter'
					new_node.name = 'brace_scope_enter'
					new_node.linenumber = token.linenumber
					node.childs.append(new_node)
					self.parse_syntax_tokens(token_iterator, node)
					token.value = ''

				if token.value == 'right_brace':
					token = token_iterator.next()
					if token.value != 'semicolon':
						token_iterator.prev()
					return

				if token.value == 'right_parentheses':
					token = token_iterator.next()
					if token.value != 'semicolon':
						token_iterator.prev()
					return

				token = token_iterator.next()
		except StopIteration:
			print('Unexpected end of tokens')
			exit()
