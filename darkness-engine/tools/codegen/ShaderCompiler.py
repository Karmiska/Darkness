import os
import string
import random
import sys
import pickle
from optparse import OptionParser
from PreprocessorHLSL import PreprocessorException
from PreprocessorHLSL import Preprocessor
from LexicalAnalyzerHLSL import LexicalAnalyzer
from SyntaxAnalyzerHLSL import SyntaxAnalyzer
from SyntaxAnalyzerHLSL import SyntaxNode
from HistoryIterator import HistoryIterException
from HistoryIterator import HistoryIter
from CodeGenerator import CodeGenerator

def stage_from_filename(filename):
	if filename[-7:] == 'cs.hlsl':
		return 'Compute'
	if filename[-7:] == 'vs.hlsl':
		return 'Vertex'
	if filename[-7:] == 'ps.hlsl':
		return 'Pixel'
	if filename[-7:] == 'gs.hlsl':
		return 'Geometry'
	if filename[-7:] == 'hs.hlsl':
		return 'Hull'
	if filename[-7:] == 'ds.hlsl':
		return 'Domain'
	if filename[-7:] == 'rg.hlsl':
		return 'Raygeneration'
	if filename[-7:] == 'is.hlsl':
		return 'Intersection'
	if filename[-7:] == 'ms.hlsl':
		return 'Miss'
	if filename[-7:] == 'ah.hlsl':
		return 'AnyHit'
	if filename[-7:] == 'ch.hlsl':
		return 'ClosestHit'
	if filename[-8:] == 'amp.hlsl':
		return 'Amplification'
	if filename[-9:] == 'mesh.hlsl':
		return 'Mesh'


VulkanStages = {
	'Compute' : 'comp',
	'Domain'  : 'tesc',
	'Geometry': 'geom',
	'Hull'    : 'tese',
	'Pixel'   : 'frag',
	'Vertex'  : 'vert',
	'Raygeneration'  : 'raygeneration',
	'Intersection' : 'intersection',
	'Miss'    : 'miss',
	'AnyHit'  : 'anyhit',
	'ClosestHit' : 'closesthit',
	'Amplification' : 'amplification',
	'Mesh' : 'mesh'
}

VulkanSets = {
	'comp' : '0',
	'vert' : '0',
	'frag' : '1',
	'geom' : '2',
	'tesc' : '3',
	'tese' : '4',
	'raygeneration' : '5',
	'intersection' : '6',
	'miss' : '7',
	'anyhit' : '8',
	'closesthit' : '9',
	'amplification' : '10',
	'mesh' : '11'
}

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
	'RaytracingAccelerationStructure',
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

constant_buffer_types = [
	'ConstantBuffer'
	]

class VulkanCompiler:
	def __init__(self, defines, includes, mode, syntax_analyzer_root_level_declarations):
		# '"C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.17758.0\\x64\\fxc.exe"'
		#self.compilerBinary = ''
		#for compiler_version in DX12CompilerVersions:
		#	compiler_path = os.path.join(os.path.join(os.path.join(DX12CompilerBasePath, compiler_version), DX12CompilerArchitecture), DX12CompilerBinary)
		#	if (os.path.exists(compiler_path)):
		#		self.compilerBinary = '"'+compiler_path+'"'
		#		break
		#self.compiler_binary = 'C:\\work\\hlsl.bin\\Debug\\bin\\dxc.exe'
		self.compiler_binary = 'C:\\work\\DirectXShaderCompiler\\hlsl.bin\\Release\\bin\\dxc.exe'

		self.inputFlag = ''
		# /Od  for disable optimization
		# /Zpr = Row major
		# -fvk-use-dx-layout 
		if mode == 'debug':
			self.outputFlag = '-nologo -spirv -fspv-target-env=vulkan1.2 -Zpr -O0 -Zi -Fo'
		else:
			self.outputFlag = '-nologo -spirv -fspv-target-env=vulkan1.2 -Zpr -O3 -Zi -Fo'
		self.include_paths = []
		self.defines = []
		self.syntax_analyzer_root_level_declarations = syntax_analyzer_root_level_declarations
		
		if includes is not None:
			self.include_paths.extend(includes)
		if defines is not None:
			self.defines.extend(defines)

	def profile(self, inputFile):
		return DX12Stages[stage_from_filename(inputFile)]

	def compile(self, input_file, output_file, bindless):

		temporary_file_path = self.createPreprocessedFile(input_file)
		root_level_declarations = self.createTemplateData(input_file, self.defines, self.include_paths)

		self.updateResourceRegisters(temporary_file_path, root_level_declarations, VulkanSets[VulkanStages[stage_from_filename(input_file)]])

		# check input_file for bindless texture
		# /enable_unbounded_descriptor_tables

		defineStr = ''
		for i in range(len(self.defines)):
			defineStr += '/D'+str(self.defines[i])
			if i < len(self.defines)-1:
				defineStr += ' '

		filename, file_extension = os.path.splitext(output_file)

		print('# Compiling shader: '+input_file)

		#set_remap = ' -auto-binding-space '+VulkanSets[VulkanStages[stage_from_filename(input_file)]]
		set_remap = ''

		# compile shader
		cmd = self.compiler_binary+' -T '+self.profile(input_file)+' '+temporary_file_path+set_remap+' '+self.outputFlag+' '+output_file
		if defineStr != '':
			cmd += ' '+defineStr
		print('# Shader binary command: '+cmd)
		os.system(cmd)

		print('##########################################################################################')
		sys.stdout.flush()
		self.removePreprocessedFile(temporary_file_path)

	def removePreprocessedFile(self, input_file):
		os.remove(input_file)

	def createPreprocessedFile(self, input_file):
		(dir, filename) = os.path.split(input_file)
		temporary_file_path = os.path.join(dir, self.createTemporaryFilename(filename))
		with open(temporary_file_path, 'w') as file:
			with open(input_file, 'r') as input_file:
				preprocessor = Preprocessor(input_file, self.defines, self.include_paths)
				for chr in preprocessor:
					file.write(chr)
		return temporary_file_path

	def createTemplateData(self, input_file, defines, includes):
		# can't use pre-generated root tokens as token line numbers change based on defines
		#return self.syntax_analyzer_root_level_declarations
		with open(input_file, 'r') as file:
			preprocessor = Preprocessor(file, defines, includes)
			lexical_analyzer = LexicalAnalyzer(preprocessor)
			syntax_analyzer = SyntaxAnalyzer(lexical_analyzer)
			return syntax_analyzer.root_level_declarations()

	def type_is_srv(self, type):
		parts = type.split('<')
		return parts[0] in srvs_types

	def type_is_uav(self, type):
		parts = type.split('<')
		return parts[0] in uavs_types

	def type_is_root_constant(self, type):
		parts = type.split('<')
		return parts[0] in constant_buffer_types

	def addBindingLines(self, contents, set_number_str, to_add):
		to_add_rev_sorted = reversed(sorted(to_add, key = lambda i: i[0]))
		for linenumber in to_add_rev_sorted:
			contents.insert(linenumber[0], '[[vk::binding('+str(linenumber[1])+','+set_number_str+')]]\n')

	def updateResourceRegisters(self, temporary_file_path, root_level_declarations, set_number_str):
		contents = []
		with open(temporary_file_path, 'r') as file:
			contents.extend(file.readlines())

		count_cbuffer = 0
		count_sampler = 0
		count_srv = 0
		count_uav = 0

		binding_num = 0

		# samplers
		to_add = []
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if token.type == 'sampler' or token.type == 'SamplerComparisonState':
				contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(s'+str(count_sampler)+', space'+set_number_str+');\n'
				count_sampler += 1
				to_add.append([linenumber, binding_num])
				binding_num += 1

		# texture SRV
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_srv(token.type):
				if ('Buffer' not in token.type) and ('Bindless' not in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t'+str(count_srv)+', space'+set_number_str+');\n'
					count_srv += 1
					to_add.append([linenumber, binding_num])
					binding_num += 1

		# texture SRV bindless
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_srv(token.type):
				if 'Buffer' not in token.type:
					if 'Bindless' in token.type:
						contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t'+str(count_srv)+', space'+set_number_str+');\n'
						count_srv += 1
						to_add.append([linenumber, binding_num])
						binding_num += 1

		# buffer SRV structured
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_srv(token.type):
				if ('Buffer' in token.type) and ('Bindless' not in token.type) and ('Structured' in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t'+str(count_srv)+', space'+set_number_str+');\n'
					count_srv += 1
					to_add.append([linenumber, binding_num])
					binding_num += 1

		# buffer SRV typed
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_srv(token.type):
				if ('Buffer' in token.type) and ('Bindless' not in token.type) and ('Structured' not in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t'+str(count_srv)+', space'+set_number_str+');\n'
					count_srv += 1
					to_add.append([linenumber, binding_num])
					binding_num += 1

		# RaytracingAccelerationStructure
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_srv(token.type):
				if 'RaytracingAccelerationStructure' in token.type:
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t'+str(count_srv)+', space'+set_number_str+');\n'
					count_srv += 1
					to_add.append([linenumber, binding_num])
					binding_num += 1
		
		# texture UAV
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_uav(token.type):
				if 'Buffer' not in token.type:
					if 'Bindless' not in token.type:
						contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(u'+str(count_uav)+', space'+set_number_str+');\n'
						count_uav += 1
						to_add.append([linenumber, binding_num])
						binding_num += 1
		
		# buffer UAV structured
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_uav(token.type):
				if 'Buffer' in token.type:
					if 'Bindless' not in token.type:
						if 'Structured' in token.type:
							contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(u'+str(count_uav)+', space'+set_number_str+');\n'
							count_uav += 1
							to_add.append([linenumber, binding_num])
							binding_num += 1
		
		# buffer UAV typed
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_uav(token.type):
				if 'Buffer' in token.type:
					if 'Bindless' not in token.type:
						if 'Structured' not in token.type:
							contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(u'+str(count_uav)+', space'+set_number_str+');\n'
							count_uav += 1
							to_add.append([linenumber, binding_num])
							binding_num += 1

        # root constants
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_root_constant(token.type):
				contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(b'+str(count_cbuffer)+', space'+set_number_str+');\n'
				count_cbuffer += 1
				to_add.append([linenumber, binding_num])
				binding_num += 1

		# constants
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if token.type == 'cbuffer':
				contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(b'+str(count_cbuffer)+', space'+set_number_str+')\n'
				count_cbuffer += 1
				to_add.append([linenumber, binding_num])
				binding_num += 1

		self.addBindingLines(contents, set_number_str, to_add)

		os.remove(temporary_file_path)

		with open(temporary_file_path, 'w') as file:
			for line in contents:
				file.write(line)


	def createTemporaryFilename(self, inputFile):
		random_part = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(8))
		return inputFile + '.' + random_part

DX12Stages = {
	'Compute' : 'cs_6_3',
	'Domain'  : 'ds_6_3',
	'Geometry': 'gs_6_3',
	'Hull'    : 'hs_6_3',
	'Pixel'   : 'ps_6_3',
	'Vertex'  : 'vs_6_3',
	'Raygeneration'  : 'lib_6_3',
	'Intersection' : 'lib_6_3',
	'Miss'    : 'lib_6_3',
	'AnyHit'  : 'lib_6_3',
	'ClosestHit' : 'lib_6_3',
	'Amplification' : 'as_6_5',
	'Mesh' : 'ms_6_5'
}

DX12CompilerBasePath = 'C:\\Program Files (x86)\\Windows Kits\\10\\bin\\'
DX12CompilerArchitecture = 'x64'
DX12CompilerBinary = 'dxc.exe'
DX12CompilerVersions = [
    '10.0.19587.0',
	'10.0.19551.0',
	'10.0.19546.0',
	'10.0.19041.0',
	'10.0.18362.0',
	'10.0.17758.0',
	'10.0.17134.0'
]

class DX12CompilerCodegen:
	def __init__(self, defines, includes, mode):
		# '"C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.17758.0\\x64\\fxc.exe"'
		#self.compiler_binary = ''
		#for compiler_version in DX12CompilerVersions:
		#	compiler_path = os.path.join(os.path.join(os.path.join(DX12CompilerBasePath, compiler_version), DX12CompilerArchitecture), DX12CompilerBinary)
		#	if (os.path.exists(compiler_path)):
		#		self.compiler_binary = '"'+compiler_path+'"'
		#		break
		self.compiler_binary = 'C:\\work\\DirectXShaderCompiler\\hlsl.bin\\Release\\bin\\dxc.exe'
		#self.compiler_binary = '"C:\\Program Files (x86)\\Microsoft Durango XDK\\180712\\xdk\\FXC\\amd64\\dxc.exe"'

		self.inputFlag = ''
		# /Od  for disable optimization
		# /Zpr = Row major
		# -fvk-use-dx-layout 

		# self.outputFlag = '-nologo -spirv -fspv-target-env=vulkan1.1 -Zpr -O0 -Zi -Fo'
		if mode == 'debug':
			self.outputFlag = '-nologo -Zpr -Od -Zi -Fo'
		else:
			self.outputFlag = '-nologo -Zpr -O3 -Zi -Fo'
		self.include_paths = []
		self.defines = []
		
		if includes is not None:
			self.include_paths.extend(includes)
		if defines is not None:
			self.defines.extend(defines)

	def profile(self, inputFile):
		return DX12Stages[stage_from_filename(inputFile)]

	def compile(self, input_file, output_file, bindless):

		temporary_file_path = self.createPreprocessedFile(input_file)
		root_level_declarations = self.createTemplateData(input_file, self.defines, self.include_paths)
		self.updateResourceRegisters(temporary_file_path, root_level_declarations, VulkanSets[VulkanStages[stage_from_filename(input_file)]])

		# check input_file for bindless texture
		# /enable_unbounded_descriptor_tables

		defineStr = ''
		for i in range(len(self.defines)):
			defineStr += '/D'+str(self.defines[i])
			if i < len(self.defines)-1:
				defineStr += ' '

		base_filename_ext = os.path.basename(os.path.normpath(input_file))
		base_directory = os.path.dirname(os.path.normpath(output_file))
		input_filename, input_file_extension = os.path.splitext(os.path.join(base_directory, base_filename_ext))
		filename, file_extension = os.path.splitext(output_file)

		shader_profile = self.profile(input_file)
		dxr_shader = shader_profile == 'lib_6_3'

		print('# Compiling shader: '+input_filename)
		sys.stdout.flush()

		# compile root signature
		# /Vd flag is for DXIL.dll bug. https://github.com/microsoft/DirectXShaderCompiler/issues/2551
		#if not dxr_shader:
			#rscmd = self.compiler_binary+' -force-rootsig-ver rootsig_1_1 -E main -T '+shader_profile+' '+input_filename+'.rs -Fo '+input_filename+'.rso'
			# rscmd = self.compiler_binary+' /Vd /T rootsig_1_1 '+input_filename+'.rs /E main /Fo '+input_filename+'.rso'
		rscmd = self.compiler_binary+' /Vd /T rootsig_1_1 '+input_filename+'.rs /Fo '+input_filename+'.rso'
		print('# RootSignature command: '+rscmd)
		sys.stdout.flush()
		os.system(rscmd)

		# compile shader
		if not dxr_shader:
			if not bindless:
				cmd = self.compiler_binary+' /T '+shader_profile+' '+temporary_file_path+' '+self.outputFlag+' '+output_file+' /Fd '+filename+'.pdb'+' /setrootsignature '+input_filename+'.rso'
				if defineStr != '':
					cmd += ' '+defineStr
				print('# Shader binary command: '+cmd)
				sys.stdout.flush()
				os.system(cmd)
			else:
				cmd = self.compiler_binary+' /enable_unbounded_descriptor_tables /T '+shader_profile+' '+temporary_file_path+' '+self.outputFlag+' '+output_file+' /Fd '+filename+'.pdb'+' /setrootsignature '+input_filename+'.rso'
				if defineStr != '':
					cmd += ' '+defineStr
				print('# Shader binary command: '+cmd)
				sys.stdout.flush()
				os.system(cmd)
		else:
			if not bindless:
				cmd = self.compiler_binary+' /T '+shader_profile+' '+temporary_file_path+' '+self.outputFlag+' '+output_file+' /Fd '+filename+'.pdb'
				if defineStr != '':
					cmd += ' '+defineStr
				print('# Shader binary command: '+cmd)
				sys.stdout.flush()
				os.system(cmd)
			else:
				cmd = self.compiler_binary+' /enable_unbounded_descriptor_tables /T '+shader_profile+' '+temporary_file_path+' '+self.outputFlag+' '+output_file+' /Fd '+filename+'.pdb'
				if defineStr != '':
					cmd += ' '+defineStr
				print('# Shader binary command: '+cmd)
				sys.stdout.flush()
				os.system(cmd)

		self.removePreprocessedFile(temporary_file_path)

		print('##########################################################################################')
		sys.stdout.flush()

	def removePreprocessedFile(self, input_file):
		os.remove(input_file)

	def createPreprocessedFile(self, input_file):
		(dir, filename) = os.path.split(input_file)
		temporary_file_path = os.path.join(dir, self.createTemporaryFilename(filename))
		with open(temporary_file_path, 'w') as file:
			with open(input_file, 'r') as input_file:
				preprocessor = Preprocessor(input_file, self.defines, self.include_paths)
				for chr in preprocessor:
					file.write(chr)
		return temporary_file_path

	def createTemplateData(self, input_file, defines, includes):
		with open(input_file, 'r') as file:
			preprocessor = Preprocessor(file, defines, includes)
			lexical_analyzer = LexicalAnalyzer(preprocessor)
			syntax_analyzer = SyntaxAnalyzer(lexical_analyzer)
			return syntax_analyzer.root_level_declarations()

	def type_is_srv(self, type):
		parts = type.split('<')
		return parts[0] in srvs_types

	def type_is_uav(self, type):
		parts = type.split('<')
		return parts[0] in uavs_types

	def type_is_root_constant(self, type):
		parts = type.split('<')
		return parts[0] in constant_buffer_types

	def updateResourceRegisters(self, temporary_file_path, root_level_declarations, set_number_str):
		contents = []
		with open(temporary_file_path, 'r') as file:
			contents.extend(file.readlines())

		count_cbuffer = 0
		count_sampler = 0
		count_srv = 0
		count_uav = 0
		count_space = 0

		# samplers
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if token.type == 'sampler' or token.type == 'SamplerComparisonState':
				contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(s'+str(count_sampler)+');\n'
				count_sampler += 1

		# texture SRV
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			# Texture SRV
			if self.type_is_srv(token.type):
				if ('Buffer' not in token.type) and ('Bindless' not in token.type) and ('RaytracingAccelerationStructure' not in token.type):
						contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t'+str(count_srv)+');\n'
						count_srv += 1
			# Structured Buffer SRV
			if self.type_is_srv(token.type):
				if ('Buffer' in token.type) and ('Bindless' not in token.type) and ('Structured' in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t'+str(count_srv)+');\n'
					count_srv += 1
			# Typed Buffer SRV
			if self.type_is_srv(token.type):
				if ('Buffer' in token.type) and ('Bindless' not in token.type) and ('Structured' not in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t'+str(count_srv)+');\n'
					count_srv += 1
			# RaytracingAccelerationStructure
			if self.type_is_srv(token.type):
				if 'RaytracingAccelerationStructure' in token.type:
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t'+str(count_srv)+');\n'
					count_srv += 1
			# Texture UAV
			if self.type_is_uav(token.type):
				if ('Buffer' not in token.type) and ('Bindless' not in token.type):
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(u'+str(count_uav)+');\n'
					count_uav += 1
			# Structured Buffer UAV
			if self.type_is_uav(token.type):
				if ('Buffer' in token.type) and ('Bindless' not in token.type) and ('Structured' in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(u'+str(count_uav)+');\n'
					count_uav += 1
			# Typed Buffer UAV
			if self.type_is_uav(token.type):
				if ('Buffer' in token.type) and ('Bindless' not in token.type) and ('Structured' not in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(u'+str(count_uav)+');\n'
					count_uav += 1
			# Bindless Texture SRV
			if self.type_is_srv(token.type):
				if 'Buffer' not in token.type:
					if 'Bindless' in token.type:
						contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t0, space'+str(1 + count_space)+');\n'
						count_space += 1
			# Bindless Buffer SRV
			if self.type_is_srv(token.type):
				if 'Buffer' in token.type:
					if 'Bindless' in token.type:
						contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t0, space'+str(1 + count_space)+');\n'
						count_space += 1
			# Bindless Texture UAV
			if self.type_is_uav(token.type):
				if 'Buffer' not in token.type:
					if 'Bindless' in token.type:
						contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(u0, space'+str(1 + count_space)+');\n'
						count_space += 1
			# Bindless Buffer UAV
			if self.type_is_uav(token.type):
				if 'Buffer' in token.type:
					if 'Bindless' in token.type:
						contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(t0, space'+str(1 + count_space)+');\n'
						count_space += 1

		# root constants
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if self.type_is_root_constant(token.type):
				contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(b'+str(count_cbuffer)+');\n'
				count_cbuffer += 1

		# constants
		for token in root_level_declarations:
			linenumber = token.linenumber-1
			if token.type == 'cbuffer':
				contents[linenumber] = contents[linenumber][:str(contents[linenumber]).rfind(';')] + ' : register(b'+str(count_cbuffer)+')\n'
				count_cbuffer += 1

		os.remove(temporary_file_path)

		with open(temporary_file_path, 'w') as file:
			for line in contents:
				file.write(line)

	def createTemporaryFilename(self, inputFile):
		random_part = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(8))
		return inputFile + '.' + random_part


class DX12Compiler:
	def __init__(self, defines, includes, mode):
		# '"C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.17758.0\\x64\\fxc.exe"'
		self.compilerBinary = ''
		for compiler_version in DX12CompilerVersions:
			compiler_path = os.path.join(os.path.join(os.path.join(DX12CompilerBasePath, compiler_version), DX12CompilerArchitecture), DX12CompilerBinary)
			if (os.path.exists(compiler_path)):
				self.compilerBinary = '"'+compiler_path+'"'
				break

		self.inputFlag = ''
		# /Od  for disable optimization
		# /Zpr = Row major
		if mode == 'debug':
			self.outputFlag = '/nologo /Zpr /Od /Zi /Fo'
		else:
			self.outputFlag = '/nologo /Zpr /O3 /Zi /Fo'

		self.include_paths = []
		self.defines = []
		
		if includes is not None:
			self.include_paths.extend(includes)
		if defines is not None:
			self.defines.extend(defines)

	def profile(self, inputFile):
		return DX12Stages[stage_from_filename(inputFile)]

	def compile(self, input_file, output_file, bindless):

		# check input_file for bindless texture
		# /enable_unbounded_descriptor_tables

		defineStr = ''
		for i in range(len(self.defines)):
			defineStr += '/D'+str(self.defines[i])
			if i < len(self.defines)-1:
				defineStr += ' '

		
		base_filename_ext = os.path.basename(os.path.normpath(input_file))
		base_directory = os.path.dirname(os.path.normpath(output_file))
		input_filename, input_file_extension = os.path.splitext(os.path.join(base_directory, base_filename_ext))


		filename, file_extension = os.path.splitext(output_file)

		# compile root signature
		rscmd = self.compilerBinary+' /T rootsig_1_1 '+input_filename+'.rs /E main /Fo '+input_filename+'.rso'
		os.system(rscmd)

		# compile shader
		if not bindless:
			cmd = self.compilerBinary+' /T '+self.profile(input_file)+' '+input_file+' '+self.outputFlag+' '+output_file+' /Fd '+filename+'.pdb'+' /setrootsignature '+input_filename+'.rso'
			if defineStr != '':
				cmd += ' '+defineStr
			os.system(cmd)
		else:
			cmd = self.compilerBinary+' /enable_unbounded_descriptor_tables /T '+self.profile(input_file)+' '+input_file+' '+self.outputFlag+' '+output_file+' /Fd '+filename+'.pdb'+' /setrootsignature '+input_filename+'.rso'
			if defineStr != '':
				cmd += ' '+defineStr
			os.system(cmd)

class Compiler:
	def __init__(self, graphicsApi, defines, includes, mode, syntax_analyzer_root_level_declarations):
		if graphicsApi.lower() == "vulkan":
			self.compiler = VulkanCompiler(defines, includes, mode, syntax_analyzer_root_level_declarations)
		elif graphicsApi.lower() == "dx12":
			self.compiler = DX12CompilerCodegen(defines, includes, mode)

	def compile(self, inputFile, outputFile, bindless):
		self.compiler.compile(inputFile, outputFile, bindless)

# cd "$(ProjectDir)..\..\data\engine\graphics\shaders" && 
# del %(Filename).frag.spv && 
# C:\VulkanSDK\1.0.21.1\Bin\glslangValidator.exe -s -V "%(FullPath)" && 
# rename frag.spv %(Filename).frag.spv
# -i C:\work\darkness\darkness-engine\shaders\core\culling\OcclusionCulling.cs.hlsl -t C:\work\darkness\darkness-engine\tools\codegen\ShaderLoadInterfaceTemplate.cpp -o C:\work\darkness\darkness-engine\include\shaders\core\culling\OcclusionCulling.cs.cpp -b C:\work\darkness\darkness-engine\data\shaders\dx12\core\culling\OcclusionCulling.cs.cso -s Compute -x C:\work\darkness\darkness-engine\data\shaders\dx12\core\culling\OcclusionCulling.cs.support

def main():
	parser = OptionParser()
	parser.add_option("-g", "--graphics-api", dest="graphicsapi", help="select graphics api. example 1: -g VULKAN , example 2: -g DX12")
	parser.add_option("-i", "--input", dest="input", help="input file. example: -i C:\\work\\Test.frag")
	parser.add_option("-o", "--output", dest="output", help="output file. example: -o C:\\work\\Test.frag.spv")
	parser.add_option("-m", "--mode", dest="mode", help="release or debug. example: -o debug")
	parser.add_option("-D", "--define", action='append', dest="define", help="example: -DDEBUG")
	parser.add_option("-I", "--include", action='append', dest="include", help="example: -I ../inc")
	parser.add_option("-x", "--support_path", dest='support_path', help="example: -x C:\\work\\some_vertex.vs.support")

	options, arguments = parser.parse_args()

	#print('ABZCXFGNB  syntax support thing: '+options.support_path)

	syntax_support_file = os.path.splitext(options.support_path)[0]+'.syntax_support'
	ssf = open(syntax_support_file, 'rb')
	syntax_analyzer_root_level_declarations = pickle.load(ssf)
	ssf.close()

	bindless = False
	with open(options.input, 'r') as file:
		#preprocessor = Preprocessor(file, options.define, options.include)
		#lexical_analyzer = LexicalAnalyzer(preprocessor)
		#syntax_analyzer = SyntaxAnalyzer(lexical_analyzer)

		for token in syntax_analyzer_root_level_declarations:
			if token.type != 'cbuffer':
				if 'Bindless' in token.type:
					bindless = True

	compiler = Compiler(options.graphicsapi, options.define, options.include, options.mode, syntax_analyzer_root_level_declarations)
	compiler.compile(options.input, options.output, bindless)

if __name__ == "__main__":
	main()
