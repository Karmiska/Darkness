import os
import itertools
from jinja2 import Template
from SyntaxAnalyzerHLSL import SyntaxNode
from SyntaxAnalyzerHLSL import SyntaxAnalyzer
from itertools import groupby
from SyntaxAnalyzerHLSL import complete_system_types

srvs_types = [
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

    'StructuredBuffer',
	'ByteAddressBuffer'
	]

uavs_types = [
	'RWBuffer',
	'RWByteAddressBuffer',
	'RWStructuredBuffer',
    'AppendStructuredBuffer',
	'RWTexture1D',
	'RWTexture1DArray',
	'RWTexture2D',
	'RWTexture2DArray',
	'RWTexture3D'
	]

structured_types = [
	'StructuredBuffer',
	'RWStructuredBuffer',
    'AppendStructuredBuffer'
	]

cube_types = [
	'TextureCube',
	'TextureCubeArray'
	]

constant_buffer_types = [
	'ConstantBuffer'
	]

def class_name_from_filename(filename):
	basename = os.path.basename(filename)		# SomeShader.vs.hlsl
	(root, ext) = os.path.splitext(basename)	# root = SomeShader.vs, ext = .hlsl
	(sroot, sext) = os.path.splitext(root)		# sroot = SomeShader, sext = .vs
	return sroot+sext[1:].upper()				# SomeShaderVS

def pipeline_name_from_filename(filename):
	basename = os.path.basename(filename)		# SomeShader.vs.hlsl
	(root, ext) = os.path.splitext(basename)	# root = SomeShader.vs, ext = .hlsl
	(sroot, sext) = os.path.splitext(root)		# sroot = SomeShader, sext = .vs
	return sroot								# SomeShaderVS

def parsePermutations(filename):
	permutations = {'enums': [], 'options': []}
	with open(filename, 'r') as file:
		data = file.readlines()
		for line in data:
			if str(line).startswith('#if') and str(line).find('ENUM_') != -1:
				s = str(str(line).rstrip()).split(' ')
				for part in s:
					if str(part).startswith('ENUM'):
						enum_parts = str(part).split('_')
						enum_typename = str(enum_parts[1]).lower().title()
						enum_value = str(enum_parts[2]).lower().title()
						for value_part in enum_parts[3:]:
							enum_value += str(value_part).lower().title()
						#enum_value = str(str.join(enum_parts[2:], " ").title()).split(' ').join()
						#enum_value = str(enum_value[0]).upper() + str(enum_value[1:])
						found = False
						for x in range(len(permutations['enums'])):
							if enum_typename in permutations['enums'][x]:
								enum_value_found = False
								for key, value in permutations['enums'][x].iteritems():
									for evalue in value:
										if evalue['value'] == enum_value:
											enum_value_found = True
											break
								if not enum_value_found:
									permutations['enums'][x][enum_typename].append({'value': enum_value, 'flag': str(part)})
								found = True
								break
						if not found:
							permutations['enums'].append({enum_typename : [{'value': enum_value, 'flag': str(part)}]})
			if str(line).startswith('#if') and str(line).find('OPTION_') != -1:
				s = str(str(line).rstrip()).split(' ')
				for part in s:
					if str(part).startswith('OPTION'):
						option_parts = str(part).split('_')
						option_value = str(option_parts[1]).lower()
						for value_part in option_parts[2:]:
							option_value += str(value_part).lower().title()

						found = False
						for option_value_test in permutations['options']:
							for okey, ovalue in option_value_test.iteritems():
								if okey == 'value' and ovalue == option_value:
									found = True
									break
							if found:
								break
						if not found:
							if option_value not in permutations['options']:
								permutations['options'].append({'value': option_value, 'flag': str(part)})
	return permutations

def optionPermute(options):
	result = []
	count = 1 << len(options)
	currentValue = 0
	for i in range(count):
		res = []
		for a in range(len(options)):
			if currentValue & (1 << a) == (1 << a):
				res.append({'type': 'option', 'variable_name': options[a]['value'], 'value': 'true', 'flag': options[a]['flag']})
			else:
				res.append({'type': 'option', 'variable_name': options[a]['value'], 'value': 'false', 'flag': options[a]['flag']})
		result.append(res)
		currentValue += 1
	return result

def enumPermute(enums):
	lists = []
	for dic in enums:
		temp = []
		for key, value in dic.iteritems():
			for enum_value in value:
				temp.append({'type': 'enum', 'variable_name': str(key).lower(), 'value': str(key).title()+'::'+str(enum_value['value']), 'flag': enum_value['flag']})
		lists.append(temp)
	return list(itertools.product(*lists))

def permute(permutations):
	result = []

	op = optionPermute(permutations['options'])
	en = enumPermute(permutations['enums'])
	
	permutation_id = 0
	for o in op:
		for e in en:
			tempdict = {}
			temp = []
			temp.extend(o)
			temp.extend(e)
			if len(temp) > 0:
				tempdict['list'] = temp
				tempdict['id'] = '%03d' % permutation_id

				flags = []
				for perm in tempdict['list']:
					if perm['type'] == 'option' and perm['value'] == 'true':
						flags.append(perm['flag'])
					if perm['type'] == 'enum':
						flags.append(perm['flag'])

				tempdict['defines'] = flags
				result.append(tempdict)
				permutation_id += 1
	return result

class CodeGenerator:
	def __init__(self, root_level_definition_tokens, template, output, stage, binary_path, support_path, src_path, set_start_index, set_count, perform_write):
		self.stage = stage
		self.binary_path = binary_path
		self.support_path = support_path
		filename, file_extension = os.path.splitext(os.path.basename(output))
		self.interface_path = filename + '.h'
		permutations = parsePermutations(src_path)
		p = permute(permutations)

		# permutations['enums'] structure is a list of dictionaries
		# permutations['enums'] = [{'enum_typename', ['enum_value1', 'enum_value2']}, 
		#							{'enum_typename2', ['enum_value1', 'enum_value2']}]
		# permutations['enums'][enum number]['Mode'][enum value number] == 'enum_value1'
		#for perm in permutations['enums']:
		#	for enum_typename, enum_values in perm.iteritems():
		#		for enum_value in enum_values:
		#			print str(enum_typename) + str(enum_value)
		# permutations['options'] is just a list of boolean type variable names

		self.template_data = self.produce_template_data(root_level_definition_tokens, output)
		self.template_data['permutations'] = p
		self.template_data['enums'] = permutations['enums']
		self.template_data['options'] = permutations['options']
		self.template_data['set_start_index'] = set_start_index
		self.template_data['set_count'] = set_count

		if perform_write:
			with open(template, 'r') as file:
				template_file = Template(file.read())
				with open(output, 'w') as output_file:
					output_file.write(template_file.render(self.template_data))

			if self.support_path != '':
				self.write_root_signature_file(self.template_data)
		
	def isDXRShader(self):
		return self.stage == 'Raygeneration' or \
			   self.stage == 'Intersection' or \
			   self.stage == 'Miss' or \
			   self.stage == 'AnyHit' or \
			   self.stage == 'ClosestHit'

	def write_root_signature_file(self, binding_data):
		binpath = self.binary_path.replace('\\', '/')
		base_directory = os.path.dirname(os.path.normpath(binpath))
		base_filename_ext = os.path.basename(os.path.normpath(binpath))
		base_filename, base_file_extension = os.path.splitext(base_filename_ext)
		base_directory_and_file = os.path.join(base_directory, base_filename)
		root_signature_file_name = base_directory_and_file + '.rs'

		print(root_signature_file_name)

		if os.path.exists(root_signature_file_name):
			os.remove(root_signature_file_name)

		visibility = ''
		if self.stage == 'Compute':
			visibility = ', visibility=SHADER_VISIBILITY_ALL'
		if self.stage == 'Domain':
			visibility = ', visibility=SHADER_VISIBILITY_DOMAIN'
		if self.stage == 'Geometry':
			visibility = ', visibility=SHADER_VISIBILITY_GEOMETRY'
		if self.stage == 'Hull':
			visibility = ', visibility=SHADER_VISIBILITY_HULL'
		if self.stage == 'Pixel':
			visibility = ', visibility=SHADER_VISIBILITY_PIXEL'
		if self.stage == 'Vertex':
			visibility = ', visibility=SHADER_VISIBILITY_VERTEX'
		if self.stage == 'Amplification':
			visibility = ', visibility=SHADER_VISIBILITY_AMPLIFICATION'
		if self.stage == 'Mesh':
			visibility = ', visibility=SHADER_VISIBILITY_MESH'

		with open(root_signature_file_name, 'a') as output_file:
			if self.isDXRShader():
				output_file.write('#define main "RootFlags( LOCAL_ROOT_SIGNATURE )')
			else:
				output_file.write('#define main "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT )')

			line_continue = ', " \\\n'
			line_end = '"'

			s_count = 0
			t_count = 0
			u_count = 0
			b_count = 0
			space_count = 0

			#if len(binding_data['root_constants']) > 0:
			#	for root_constant in binding_data['root_constants']:
			#		output_file.write(line_continue)
			#		output_file.write('              "CBV(b'+str(b_count)+')')
			#		b_count += len(binding_data['constant_structures'])

			if len(binding_data['root_constants']) > 0:
				for root_constant in binding_data['root_constants']:
					output_file.write(line_continue)
					output_file.write('              "RootConstants(num32BitConstants=1, b'+str(b_count)+')')
					b_count += len(binding_data['constant_structures'])

			if len(binding_data['samplers']) > 0:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(Sampler(s'+str(s_count)+', numDescriptors = '+str(len(binding_data['samplers']))+'))')
				s_count += len(binding_data['samplers'])

			if len(binding_data['texture_srvs']) > 0:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(SRV(t'+str(t_count)+', numDescriptors = '+str(len(binding_data['texture_srvs']))+', flags = DESCRIPTORS_VOLATILE)'+visibility+')')
				t_count += len(binding_data['texture_srvs'])
			if len(binding_data['texture_uavs']) > 0:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(UAV(u'+str(u_count)+', numDescriptors = '+str(len(binding_data['texture_uavs']))+', flags = DESCRIPTORS_VOLATILE)'+visibility+')')
				u_count += len(binding_data['texture_uavs'])

			if len(binding_data['buffer_srvs']) > 0:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(SRV(t'+str(t_count)+', numDescriptors = '+str(len(binding_data['buffer_srvs']))+', flags = DESCRIPTORS_VOLATILE)'+visibility+')')
				t_count += len(binding_data['buffer_srvs'])
			if len(binding_data['buffer_uavs']) > 0:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(UAV(u'+str(u_count)+', numDescriptors = '+str(len(binding_data['buffer_uavs']))+', flags = DESCRIPTORS_VOLATILE)'+visibility+')')
				u_count += len(binding_data['buffer_uavs'])

			if len(binding_data['acceleration_structures']) > 0:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(SRV(t'+str(t_count)+', numDescriptors = '+str(len(binding_data['acceleration_structures']))+', flags = DESCRIPTORS_VOLATILE)'+visibility+')')
				t_count += len(binding_data['acceleration_structures'])

			for srv in binding_data['bindless_texture_srvs']:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(SRV(t0, space = '+str(1 + space_count)+', numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE)'+visibility+')')
				space_count += 1

			for srv in binding_data['bindless_buffer_srvs']:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(SRV(t0, space = '+str(1 + space_count)+', numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE)'+visibility+')')
				space_count += 1

			for uav in binding_data['bindless_texture_uavs']:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(UAV(u0, space = '+str(1 + space_count)+', numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE)'+visibility+')')
				space_count += 1

			for uav in binding_data['bindless_buffer_uavs']:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(UAV(u0, space = '+str(1 + space_count)+', numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE)'+visibility+')')
				space_count += 1

			if len(binding_data['constant_structures']) > 0:
				output_file.write(line_continue)
				output_file.write('              "DescriptorTable(CBV(b'+str(b_count)+', numDescriptors = '+str(len(binding_data['constant_structures']))+')'+visibility+')')
				b_count += len(binding_data['constant_structures'])

			
				

			output_file.write(line_end)

	def type_is_srv(self, type):
		parts = type.split('<')
		return parts[0] in srvs_types

	def type_is_uav(self, type):
		parts = type.split('<')
		return parts[0] in uavs_types

	def type_is_root_constant(self, type):
		parts = type.split('<')
		return parts[0] in constant_buffer_types

	def type_is_structured_str(self, type):
		parts = type.split('<')
		if parts[0] in structured_types:
			return '1'
		else:
			return '0'

	def type_is_cube_str(self, type):
		parts = type.split('<')
		if parts[0] in cube_types:
			return '1'
		else:
			return '0'
		
	def engine_format_from_hlsl(self, format):
		if format == 'float':
			return 'Format::R32_FLOAT'
		if format == 'float2':
			return 'Format::R32G32_FLOAT'
		if format == 'float3':
			return 'Format::R32G32B32_FLOAT'
		if format == 'float4':
			return 'Format::R32G32B32A32_FLOAT'
		if format == 'uint':
			return 'Format::R32_UINT'
		if format == 'uint2':
			return 'Format::R32G32_UINT'
		if format == 'uint3':
			return 'Format::R32G32B32_UINT'
		if format == 'uint4':
			return 'Format::R32G32B32A32_UINT'
		return 'Format::UNKNOWN'

	def get_cpp_type(self, type):
		if self.type_is_srv(type):
			if 'Buffer' in type:
				if 'Bindless' in type:
					return 'BindlessBufferSRV'
				else:
					return 'BufferSRV'
			elif 'Texture' in type:
				if 'Bindless' in type:
					return 'BindlessTextureSRV'
				else:
					return 'TextureSRV'
		if self.type_is_uav(type):
			if 'Buffer' in type:
				if 'Bindless' in type:
					return 'BindlessBufferUAV'
				else:
					return 'BufferUAV'
			elif 'Texture' in type:
				if 'Bindless' in type:
					return 'BindlessTextureUAV'
				else:
					return 'TextureUAV'
		if self.type_is_root_constant(type):
			return 'RootConstant'
		if type in complete_system_types:
			return type[:1].upper()+type[1:]

	def produce_template_data(self, root_level_tokens, output):
		class_name = class_name_from_filename(output)
		pipeline_configuration_class = pipeline_name_from_filename(output)

		binpath = self.binary_path.replace('\\', '/')
		base_directory = os.path.dirname(os.path.normpath(binpath))
		base_filename_ext = os.path.basename(os.path.normpath(binpath))
		base_filename, base_file_extension = os.path.splitext(base_filename_ext)
		base_directory_and_file = os.path.join(base_directory, base_filename)

		base_core_path = 'C:/work/darkness/darkness-engine/data/shaders/dx12/core/'

		result = {
			'has_constants' : False,
			'has_texture_srvs' : False,
			'has_texture_uavs' : False,
			'has_bindless_texture_srvs' : False,
			'has_bindless_texture_uavs' : False,
			'has_buffer_srvs' : False,
			'has_buffer_uavs' : False,
			'has_acceleration_structures' : False,
			'has_bindless_buffer_srvs' : False,
			'has_bindless_buffer_uavs' : False,
			'has_samplers' : False,
			'has_root_constants' : False,
			'ShaderClass' : class_name,
			'class_type' : self.stage + 'Shader',
			'ShaderLoadInterfaceHeader' : self.interface_path,
			'shader_pipeline_configuration_class' : pipeline_configuration_class,
			'ShaderBinaryPath': self.binary_path.replace('\\', '/')[len(base_core_path):],

			'BaseExt': base_file_extension,
			'BasePathAndFile': base_directory_and_file.replace('\\', '/')[len(base_core_path):],

			'ShaderSupportPath': self.support_path.replace('\\', '/')[len(base_core_path):],
			'constant_structures' : [],
			'texture_srvs' : [],
			'texture_uavs' : [],
			'bindless_texture_srvs' : [],
			'bindless_texture_uavs' : [],
			'buffer_srvs' : [],
			'buffer_uavs' : [],
			'acceleration_structures' : [],
			'bindless_buffer_srvs' : [],
			'bindless_buffer_uavs' : [],
			'samplers' : [],
			'root_constants' : [],
			'descriptor_count' : 0,

			'srvs' : [],
			'uavs' : [],
			'dimensions' : [],

			'input_parameters' : [],

			'srvs_bindings' : [],
			'uavs_bindings' : [],
			'acceleration_bindings' : []
			}
		for token in root_level_tokens:
			if token.syntax_type == 'function' and token.name == 'main':
				for child in token.childs:
					result['input_parameters'].append({'name' : child.name, 'semantic' : child.semantic, 'type' : child.type })
			if token.type == 'cbuffer':
				result['has_constants'] = True
				identifiers = []
				for child in token.childs:
					identifiers.append({'type': self.get_cpp_type(child.type), 'name': child.name })
				result['constant_structures'].append({
					'name': token.name, 
					'identifier': token.name[:1].lower()+token.name[1:], 
					'identifiers': identifiers })
				result['descriptor_count'] += 1
			if token.type == 'sampler' or token.type == 'SamplerComparisonState':
				result['has_samplers'] = True
				result['samplers'].append({ 'name' : token.name, 'identifier': token.name })
			else:
				if self.type_is_srv(token.type):
					if 'Buffer' in token.type:
						if 'Bindless' in token.type:
							result['srvs_bindings'].append({'type' : 'BindlessSRVBuffer', 'index' : len(result['bindless_buffer_srvs']), 'dimension' : 'Unknown', 'format': self.engine_format_from_hlsl(token.format) })
							result['bindless_buffer_srvs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'structured': self.type_is_structured_str(token.type), 'format': self.engine_format_from_hlsl(token.format) })
							result['srvs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
							result['has_bindless_buffer_srvs'] = True
						else:
							result['srvs_bindings'].append({'type' : 'SRVBuffer', 'index' : len(result['buffer_srvs']), 'dimension' : 'Unknown', 'format': self.engine_format_from_hlsl(token.format) })
							result['buffer_srvs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'structured': self.type_is_structured_str(token.type), 'format': self.engine_format_from_hlsl(token.format) })
							result['srvs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
							result['has_buffer_srvs'] = True
						result['descriptor_count'] += 1
					else:
						if 'Bindless' in token.type:
							result['srvs_bindings'].append({'type' : 'BindlessSRVTexture', 'index' : len(result['bindless_texture_srvs']), 'dimension' : token.dimension, 'format': self.engine_format_from_hlsl(token.format) })
							result['bindless_texture_srvs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'cube': self.type_is_cube_str(token.type), 'format': self.engine_format_from_hlsl(token.format) })
							result['srvs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
							result['dimensions'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'dimension': token.dimension })
							result['has_bindless_texture_srvs'] = True
						else:
							result['srvs_bindings'].append({'type' : 'SRVTexture', 'index' : len(result['texture_srvs']), 'dimension' : token.dimension, 'format': self.engine_format_from_hlsl(token.format) })
							result['texture_srvs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'cube': self.type_is_cube_str(token.type), 'format': self.engine_format_from_hlsl(token.format) })
							result['srvs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
							result['dimensions'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'dimension': token.dimension })
							result['has_texture_srvs'] = True
						result['descriptor_count'] += 1
						
				if self.type_is_uav(token.type):
					if 'Buffer' in token.type:
						if 'Bindless' in token.type:
							result['uavs_bindings'].append({'type' : 'BindlessUAVBuffer', 'index' : len(result['bindless_buffer_uavs']), 'dimension' : 'Unknown', 'format': self.engine_format_from_hlsl(token.format) })
							result['bindless_buffer_uavs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'structured': self.type_is_structured_str(token.type), 'format': self.engine_format_from_hlsl(token.format) })
							result['uavs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
							result['has_bindless_buffer_uavs'] = True
						else:
							result['uavs_bindings'].append({'type' : 'UAVBuffer', 'index' : len(result['buffer_uavs']), 'dimension' : 'Unknown', 'format': self.engine_format_from_hlsl(token.format) })
							result['buffer_uavs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'structured': self.type_is_structured_str(token.type), 'format': self.engine_format_from_hlsl(token.format) })
							result['uavs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
							result['has_buffer_uavs'] = True
						result['descriptor_count'] += 1
						
					else:
						if 'Bindless' in token.type:
							result['uavs_bindings'].append({'type' : 'BindlessUAVTexture', 'index' : len(result['bindless_texture_uavs']), 'dimension' : token.dimension, 'format': self.engine_format_from_hlsl(token.format) })
							result['bindless_texture_uavs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'cube': self.type_is_cube_str(token.type), 'format': self.engine_format_from_hlsl(token.format) })
							result['uavs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
							result['dimensions'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'dimension': token.dimension })
							result['has_bindless_texture_uavs'] = True
						else:
							result['uavs_bindings'].append({'type' : 'UAVTexture', 'index' : len(result['texture_uavs']), 'dimension' : token.dimension, 'format': self.engine_format_from_hlsl(token.format) })
							result['texture_uavs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'cube': self.type_is_cube_str(token.type), 'format': self.engine_format_from_hlsl(token.format) })
							result['uavs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
							result['dimensions'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'dimension': token.dimension })
							result['has_texture_uavs'] = True
						result['descriptor_count'] += 1

				if 'RaytracingAccelerationStructure' in token.type:
					result['acceleration_bindings'].append({'type' : 'RaytracingAccelerationStructure', 'index' : len(result['acceleration_bindings']), 'dimension' : 'Unknown', 'format': 'Format::UNKNOWN' })
					result['acceleration_structures'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name, 'structured': self.type_is_structured_str(token.type), 'format': 'Format::UNKNOWN' })
					#result['srvs'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
					result['has_acceleration_structures'] = True
					result['descriptor_count'] += 1

				if self.type_is_root_constant(token.type):
					result['root_constants'].append({'type' : self.get_cpp_type(token.type), 'identifier': token.name })
					result['has_root_constants'] = True
					result['descriptor_count'] += 1
						
		return result
