import os
from optparse import OptionParser
import subprocess
from ShaderCompiler import stage_from_filename
from ShaderCompiler import srvs_types
from ShaderCompiler import uavs_types
from ShaderCompiler import constant_buffer_types
from CodeGenerator import class_name_from_filename
from CodeGenerator import parsePermutations
from CodeGenerator import optionPermute
from CodeGenerator import enumPermute
from CodeGenerator import permute
from jinja2 import Template
import sys
import json
import hashlib
import time
from PreprocessorHLSL import Preprocessor
from LexicalAnalyzerHLSL import LexicalAnalyzer
from SyntaxAnalyzerHLSL import SyntaxAnalyzer
from SyntaxAnalyzerHLSL import SyntaxNode
import pickle

relative_path_shader_compiler = 'ShaderCompiler.py'
relative_path_shader_codegen = 'ShaderCodegen.py'
relative_path_shader_cpp_interface_template = 'ShaderLoadInterfaceTemplate.h'
relative_path_shader_cpp_implementation_template = 'ShaderLoadInterfaceTemplate.cpp'
relative_path_pipeline_cpp_interface_template = 'ShaderPipelineTemplate.h'
relative_path_pipeline_cpp_implementation_template = 'ShaderPipelineTemplate.cpp'

api_specific_subpath = {
	'vulkan' : 'vulkan',
	'dx12' : 'dx12'
	}

api_specific_binary_ext = {
	'vulkan' : '.spv',
	'dx12' : '.cso'
	}

relative_path_shader_locations = [
	{ 
		'source_folder' : '../../shaders', 
		'destination_folder' : '../../data/shaders', 
		'interface_folder' : '../../include/shaders' }
	]

def recursive_filelist(location):
    result = []
    for root, dirs, files in os.walk(location):
        if dirs:
            for dir in dirs:
                result.extend(recursive_filelist(dir))
        if files:
            for file in files:
                result.append(os.path.join(root, file))
    return result

def printStdout(proc):
	while True:
		line = proc.stdout.readline()
		if line != '':
			print(line.rstrip())
		else:
			break

class ShaderBuilder:
	def __init__(self, root_path, graphics_api, mode):
		self.shader_builder_path = root_path
		self.shader_compiler_path = os.path.join(os.path.dirname(root_path), relative_path_shader_compiler)
		self.shader_code_generator_path = os.path.join(os.path.dirname(root_path), relative_path_shader_codegen)
		self.shader_cpp_interface_template = os.path.join(os.path.dirname(root_path), relative_path_shader_cpp_interface_template)
		self.shader_cpp_implementation_template = os.path.join(os.path.dirname(root_path), relative_path_shader_cpp_implementation_template)
		self.all_shaders = []
		

		src_filename_set_start_vs = ''
		src_filename_set_start_ps = ''
		src_filename_set_start_gs = ''
		src_filename_set_start_hs = ''
		src_filename_set_start_ds = ''
		src_filename_set_start_cs = ''
		src_filename_set_start_rg = ''
		src_filename_set_start_ins = ''
		src_filename_set_start_ms = ''
		src_filename_set_start_ah = ''
		src_filename_set_start_ch = ''
		src_filename_set_start_amp = ''
		src_filename_set_start_mesh = ''

		set_start = {}

		# gather set numbers for all stages
		for shader_location in relative_path_shader_locations:
			src_path = self.get_absolute_path(root_path, shader_location['source_folder'])
			dst_path = self.get_absolute_path(root_path, shader_location['destination_folder'])
			interface_path = self.get_absolute_path(root_path, shader_location['interface_folder'])
			files = recursive_filelist(src_path)
			for file in files:
				if file.endswith('.hlsl'):

					relative_path = os.path.relpath(os.path.dirname(file), src_path)
				
					dst_path_rebuilt = os.path.abspath(
						os.path.join(dst_path, api_specific_subpath[str(graphics_api).lower()], relative_path, os.path.basename(file)))
					dst_file = os.path.basename(dst_path_rebuilt)
					(root, ext) = os.path.splitext(dst_file)
					binary_file = os.path.join(os.path.dirname(dst_path_rebuilt), root + api_specific_binary_ext[str(graphics_api).lower()])

					interface_path_rebuilt = os.path.abspath(
						os.path.join(interface_path, relative_path, os.path.basename(file)))
					idst_file = os.path.basename(interface_path_rebuilt)
					(iroot, iext) = os.path.splitext(idst_file)
					interface_file = os.path.join(os.path.dirname(interface_path_rebuilt), iroot + '.h')
					implementation_file = os.path.join(os.path.dirname(interface_path_rebuilt), iroot + '.cpp')
				
					if not os.path.exists(os.path.dirname(interface_file)):
						os.makedirs(os.path.dirname(interface_file))

					if not os.path.exists(os.path.dirname(binary_file)):
						os.makedirs(os.path.dirname(binary_file))

					support_file_path = os.path.join(os.path.dirname(dst_path_rebuilt), root + '.support')
					syntax_support_file = os.path.splitext(support_file_path)[0]+'.syntax_support'

					if not os.path.exists(os.path.dirname(interface_file)):
						os.makedirs(os.path.dirname(interface_file))

					if not os.path.exists(os.path.dirname(binary_file)):
						os.makedirs(os.path.dirname(binary_file))
					
					# check if the source file has changed after last generation
					src_file_exists = os.path.exists(file)
					src_file_modified = os.path.getmtime(file)

					changed = False

					# generate shader interface
					interface_file_exists = os.path.exists(interface_file)
					interface_file_modified = src_file_modified
					if interface_file_exists:
						interface_file_modified = os.path.getmtime(interface_file)
					if not interface_file_exists or interface_file_modified < src_file_modified:
						changed = True
						# run the HLSL code analysis
						# --------------------------------------------------------
						with open(file, 'r') as pfile:
							preprocessor = Preprocessor(pfile, [], [])
							lexical_analyzer = LexicalAnalyzer(preprocessor)
							syntax_analyzer = SyntaxAnalyzer(lexical_analyzer)
							
							with open(syntax_support_file, 'wb') as ssf:
								p = pickle.Pickler(ssf)
								p.dump(syntax_analyzer.root_level_declarations())
						# --------------------------------------------------------

					stage = stage_from_filename(file)
					if stage == 'Vertex':
						src_filename_set_start_vs = syntax_support_file
					if stage == 'Pixel':
						src_filename_set_start_ps = syntax_support_file
					if stage == 'Geometry':
						src_filename_set_start_gs = syntax_support_file
					if stage == 'Hull':
						src_filename_set_start_hs = syntax_support_file
					if stage == 'Domain':
						src_filename_set_start_ds = syntax_support_file
					if stage == 'Compute':
						src_filename_set_start_cs = syntax_support_file
					if stage == 'Raygeneration':
						src_filename_set_start_rg = syntax_support_file
					if stage == 'Intersection':
						src_filename_set_start_ins = syntax_support_file
					if stage == 'Miss':
						src_filename_set_start_ms = syntax_support_file
					if stage == 'AnyHit':
						src_filename_set_start_ah = syntax_support_file
					if stage == 'ClosestHit':
						src_filename_set_start_ch = syntax_support_file
					if stage == 'Amplification':
						src_filename_set_start_amp = syntax_support_file
					if stage == 'Mesh':
						src_filename_set_start_mesh = syntax_support_file

		set_current_index = 0
		if src_filename_set_start_vs != '':
			set_count = self.get_set_count(src_filename_set_start_vs)
			set_start['Vertex'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_ps != '':
			set_count = self.get_set_count(src_filename_set_start_ps)
			set_start['Pixel'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_gs != '':
			set_count = self.get_set_count(src_filename_set_start_gs)
			set_start['Geometry'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_hs != '':
			set_count = self.get_set_count(src_filename_set_start_hs)
			set_start['Hull'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_ds != '':
			set_count = self.get_set_count(src_filename_set_start_ds)
			set_start['Domain'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_cs != '':
			set_count = self.get_set_count(src_filename_set_start_cs)
			set_start['Compute'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_rg != '':
			set_count = self.get_set_count(src_filename_set_start_rg)
			set_start['Raygeneration'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_ins != '':
			set_count = self.get_set_count(src_filename_set_start_ins)
			set_start['Intersection'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_ms != '':
			set_count = self.get_set_count(src_filename_set_start_ms)
			set_start['Miss'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_ah != '':
			set_count = self.get_set_count(src_filename_set_start_ah)
			set_start['AnyHit'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_ch != '':
			set_count = self.get_set_count(src_filename_set_start_ch)
			set_start['ClosestHit'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_amp != '':
			set_count = self.get_set_count(src_filename_set_start_amp)
			set_start['Amplification'] = [set_current_index, set_count]
			set_current_index += set_count
		if src_filename_set_start_mesh != '':
			set_count = self.get_set_count(src_filename_set_start_mesh)
			set_start['Mesh'] = [set_current_index, set_count]
			set_current_index += set_count

		#print(set_start['Pixel'][1])

		# generate CPP and H files
		for shader_location in relative_path_shader_locations:
			src_path = self.get_absolute_path(root_path, shader_location['source_folder'])
			dst_path = self.get_absolute_path(root_path, shader_location['destination_folder'])
			interface_path = self.get_absolute_path(root_path, shader_location['interface_folder'])
			
			files = recursive_filelist(src_path)
			for file in files:
				if file.endswith('.hlsl'):

					if file.endswith('test.mesh.hlsl'):
						foundLocation = True

					relative_path = os.path.relpath(os.path.dirname(file), src_path)
				
					dst_path_rebuilt = os.path.abspath(
						os.path.join(dst_path, api_specific_subpath[str(graphics_api).lower()], relative_path, os.path.basename(file)))
					dst_file = os.path.basename(dst_path_rebuilt)
					(root, ext) = os.path.splitext(dst_file)
					binary_file = os.path.join(os.path.dirname(dst_path_rebuilt), root + api_specific_binary_ext[str(graphics_api).lower()])

					interface_path_rebuilt = os.path.abspath(
						os.path.join(interface_path, relative_path, os.path.basename(file)))
					idst_file = os.path.basename(interface_path_rebuilt)
					(iroot, iext) = os.path.splitext(idst_file)
					interface_file = os.path.join(os.path.dirname(interface_path_rebuilt), iroot + '.h')
					implementation_file = os.path.join(os.path.dirname(interface_path_rebuilt), iroot + '.cpp')
				
					if not os.path.exists(os.path.dirname(interface_file)):
						os.makedirs(os.path.dirname(interface_file))

					if not os.path.exists(os.path.dirname(binary_file)):
						os.makedirs(os.path.dirname(binary_file))

					# check if the source file has changed after last generation
					src_file_exists = os.path.exists(file)
					src_file_modified = os.path.getmtime(file)

					changed = False

					# generate shader interface
					interface_file_exists = os.path.exists(interface_file)
					interface_file_modified = src_file_modified
					if interface_file_exists:
						interface_file_modified = os.path.getmtime(interface_file)
					if not interface_file_exists or interface_file_modified < src_file_modified:
						changed = True
						#print 'python '+self.shader_code_generator_path+' -i '+file+' -t '+os.path.join(os.path.dirname(root_path), relative_path_shader_cpp_interface_template)+' -o '+interface_file+' -b '+binary_file+' -s '+stage_from_filename(file)

						# generate hotreload shader support file
						support_file_content = {'executable': sys.executable, 'shader_compiler_path': self.shader_compiler_path, 'graphics_api': graphics_api, 'file': file, 'binary_file': binary_file, 'root_path': root_path }
						support_file_path = os.path.join(os.path.dirname(dst_path_rebuilt), root + '.support')
						with open(support_file_path, 'w') as support_file:
							support_file.write(json.dumps(support_file_content, sort_keys=True, indent=4))

						# generate CPP interface
						temporary_interface_file = os.path.join(os.path.join(os.path.dirname(interface_path_rebuilt), 'temporary'), iroot + '.h')
						temporary_implementation_file = os.path.join(os.path.join(os.path.dirname(interface_path_rebuilt), 'temporary'), iroot + '.cpp')

						if not os.path.exists(os.path.join(os.path.dirname(interface_path_rebuilt), 'temporary')):
							os.mkdir(os.path.join(os.path.dirname(interface_path_rebuilt), 'temporary'))

						printStdout(subprocess.Popen([
							'python', 
							self.shader_code_generator_path, 
							'-i', file, 
							'-t', self.shader_cpp_interface_template,
							'-o', temporary_interface_file,
							'-b', binary_file,
							'-x', support_file_path,
							'-u', str(set_start[stage_from_filename(file)][0]),
							'-p', str(set_start[stage_from_filename(file)][1]),
							'-s', stage_from_filename(file)], stdout = subprocess.PIPE, stderr = subprocess.PIPE))

						# compare interface if it changed
						self.updateIfChanged(interface_file, interface_file_exists, temporary_interface_file)

						# generate CPP implementation
						implementation_file_exists = os.path.exists(implementation_file)

						printStdout(subprocess.Popen([
							'python', 
							self.shader_code_generator_path, 
							'-i', file, 
							'-t', self.shader_cpp_implementation_template,
							'-o', temporary_implementation_file,
							'-b', binary_file,
							'-x', support_file_path,
							'-u', str(set_start[stage_from_filename(file)][0]),
							'-p', str(set_start[stage_from_filename(file)][1]),
							'-s', stage_from_filename(file)], stdout = subprocess.PIPE, stderr = subprocess.PIPE))

						# compare interface if it changed
						self.updateIfChanged(implementation_file, implementation_file_exists, temporary_implementation_file)

						if os.path.exists(os.path.join(os.path.dirname(interface_path_rebuilt), 'temporary')):
							os.rmdir(os.path.join(os.path.dirname(interface_path_rebuilt), 'temporary'))

					# save stage for pipeline processing
					info = self.get_shader_info(file, interface_file, changed)
					common_name = self.get_common_name(info['shader_type_name'])
					found = False
					for x in range(len(self.all_shaders)):
						if self.all_shaders[x]['common_shader_name'] == common_name:
							self.all_shaders[x]['stages'].append(info)
							found = True
					if not found:
						self.all_shaders.append({
							'common_shader_name' : common_name,
							'stages' : [info]
							})

		# create PipelineConfiguration
		for common_info in self.all_shaders:
			if common_info['stages'][0]['changed']:
				dir = os.path.dirname(common_info['stages'][0]['shader_interface_filepath'])
				vs = False
				ps = False
				gs = False
				hs = False
				ds = False
				cs = False
				rg = False
				ins = False
				ms = False
				ah = False
				ch = False
				amp = False
				mesh = False

				cvs = 'false'
				cps = 'false'
				cgs = 'false'
				chs = 'false'
				cds = 'false'
				ccs = 'false'
				crg = 'false'
				cins = 'false'
				cms = 'false'
				cah = 'false'
				cch = 'false'
				camp = 'false'
				cmesh = 'false'

				vs_type = ''
				ps_type = ''
				gs_type = ''
				hs_type = ''
				ds_type = ''
				cs_type = ''
				rg_type = ''
				ins_type = ''
				ms_type = ''
				ah_type = ''
				ch_type = ''
				amp_type = ''
				mesh_type = ''

				vs_if = ''
				ps_if = ''
				gs_if = ''
				hs_if = ''
				ds_if = ''
				cs_if = ''
				rg_if = ''
				ins_if = ''
				ms_if = ''
				ah_if = ''
				ch_if = ''
				amp_if = ''
				mesh_if = ''

				for stage in common_info['stages']:
					if stage['stage'] == 'Vertex':
						vs = True
						cvs = 'true'
						vs_type = stage['shader_type_name']
						vs_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Pixel':
						ps = True
						cps = 'true'
						ps_type = stage['shader_type_name']
						ps_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Geometry':
						gs = True
						cgs = 'true'
						gs_type = stage['shader_type_name']
						gs_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Hull':
						hs = True
						chs = 'true'
						hs_type = stage['shader_type_name']
						hs_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Domain':
						ds = True
						cds = 'true'
						ds_type = stage['shader_type_name']
						ds_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Compute':
						cs = True
						ccs = 'true'
						cs_type = stage['shader_type_name']
						cs_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Raygeneration':
						rg = True
						crg = 'true'
						rg_type = stage['shader_type_name']
						rg_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Intersection':
						ins = True
						cins = 'true'
						ins_type = stage['shader_type_name']
						ins_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Miss':
						ms = True
						cms = 'true'
						ms_type = stage['shader_type_name']
						ms_if = stage['shader_interface_filepath']
					if stage['stage'] == 'AnyHit':
						ah = True
						cah = 'true'
						ah_type = stage['shader_type_name']
						ah_if = stage['shader_interface_filepath']
					if stage['stage'] == 'ClosestHit':
						ch = True
						cch = 'true'
						ch_type = stage['shader_type_name']
						ch_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Amplification':
						amp = True
						camp = 'true'
						amp_type = stage['shader_type_name']
						amp_if = stage['shader_interface_filepath']
					if stage['stage'] == 'Mesh':
						mesh = True
						cmesh = 'true'
						mesh_type = stage['shader_type_name']
						mesh_if = stage['shader_interface_filepath']

				pipeline_if_filepath = os.path.join(dir, common_info['common_shader_name']+'.h')
				pipeline_im_filepath = os.path.join(dir, common_info['common_shader_name']+'.cpp')
				pipeline_info = {
					'pipeline_interface_filepath' : os.path.basename(os.path.join(dir, common_info['common_shader_name']+'.h')),
					'pipeline_implementation_filepath' : os.path.basename(os.path.join(dir, common_info['common_shader_name']+'.cpp')),
					'pipeline_type_name' : common_info['common_shader_name'],
					'has_vertex_shader' : vs,
					'has_pixel_shader' : ps,
					'has_geometry_shader' : gs,
					'has_hull_shader' : hs,
					'has_domain_shader' : ds,
					'has_compute_shader' : cs,
					'has_raygeneration_shader' : rg,
					'has_intersection_shader' : ins,
					'has_miss_shader' : ms,
					'has_anyhit_shader' : ah,
					'has_closesthit_shader' : ch,
					'has_amplification_shader' : amp,
					'has_mesh_shader' : mesh,
					'chas_vertex_shader' : cvs,
					'chas_pixel_shader' : cps,
					'chas_geometry_shader' : cgs,
					'chas_hull_shader' : chs,
					'chas_domain_shader' : cds,
					'chas_compute_shader' : ccs,
					'chas_raygeneration_shader' : crg,
					'chas_intersection_shader' : cins,
					'chas_miss_shader' : cms,
					'chas_anyhit_shader' : cah,
					'chas_closesthit_shader' : cch,
					'chas_amplification_shader' : camp,
					'chas_mesh_shader' : cmesh,
					'vertex_shader_type' : vs_type,
					'pixel_shader_type' : ps_type,
					'geometry_shader_type' : gs_type,
					'hull_shader_type' : hs_type,
					'domain_shader_type' : ds_type,
					'compute_shader_type' : cs_type,
					'raygeneration_shader_type' : rg_type,
					'intersection_shader_type' : ins_type,
					'miss_shader_type' : ms_type,
					'anyhit_shader_type' : ah_type,
					'closesthit_shader_type' : ch_type,
					'amplification_shader_type' : amp_type,
					'mesh_shader_type' : mesh_type,
					'vertex_shader_if' : os.path.basename(vs_if),
					'pixel_shader_if' : os.path.basename(ps_if),
					'geometry_shader_if' : os.path.basename(gs_if),
					'hull_shader_if' : os.path.basename(hs_if),
					'domain_shader_if' : os.path.basename(ds_if),
					'compute_shader_if' : os.path.basename(cs_if),
					'raygeneration_shader_if' : os.path.basename(rg_if),
					'intersection_shader_if' : os.path.basename(ins_if),
					'miss_shader_if' : os.path.basename(ms_if),
					'anyhit_shader_if' : os.path.basename(ah_if),
					'closesthit_shader_if' : os.path.basename(ch_if),
					'amplification_shader_if' : os.path.basename(amp_if),
					'mesh_shader_if' : os.path.basename(mesh_if)
					}

				pipeline_interface_template = os.path.join(os.path.dirname(root_path), relative_path_pipeline_cpp_interface_template)
				pipeline_implementation_template = os.path.join(os.path.dirname(root_path), relative_path_pipeline_cpp_implementation_template)

				pipeline_if_filepath_temporary = os.path.join(os.path.join(dir, 'temporary'), common_info['common_shader_name']+'.h')
				pipeline_im_filepath_temporary = os.path.join(os.path.join(dir, 'temporary'), common_info['common_shader_name']+'.cpp')

				op_success = False
				while op_success == False:
					try:
						if not os.path.exists(os.path.join(dir, 'temporary')):
							os.mkdir(os.path.join(dir, 'temporary'))
						op_success = True
					except IOError:
						print("Failed with temporary. Trying again")
						time.sleep(1)

				with open(pipeline_interface_template, 'r') as file:
					template_file = Template(file.read())
					with open(pipeline_if_filepath_temporary, 'w') as output_file:
						output_file.write(template_file.render(pipeline_info))

				self.updateIfChanged(pipeline_if_filepath, os.path.exists(pipeline_if_filepath), pipeline_if_filepath_temporary)

				with open(pipeline_implementation_template, 'r') as file:
					template_file = Template(file.read())
					with open(pipeline_im_filepath_temporary, 'w') as output_file:
						output_file.write(template_file.render(pipeline_info))

				self.updateIfChanged(pipeline_im_filepath, os.path.exists(pipeline_im_filepath), pipeline_im_filepath_temporary)

				if os.path.exists(os.path.join(dir, 'temporary')):
					os.rmdir(os.path.join(dir, 'temporary'))

		# need to generate set allocations.
		# for example vertex shader might be using bindless resource, so it needs set 0 and set 1..n for the bindless
		# now the pixel shader that normally goes to set 1, no needs to go to set n + 1
		# and pixel shader binless resources should go to sets n + 2 .. n + 2 + x


		# compile all shaders
		for shader_location in relative_path_shader_locations:
			src_path = self.get_absolute_path(root_path, shader_location['source_folder'])
			dst_path = self.get_absolute_path(root_path, shader_location['destination_folder'])
			interface_path = self.get_absolute_path(root_path, shader_location['interface_folder'])
			
			files = recursive_filelist(src_path)
			for file in files:
				if file.endswith('.hlsl'):
					relative_path = os.path.relpath(os.path.dirname(file), src_path)
				
					dst_path_rebuilt = os.path.abspath(
						os.path.join(dst_path, api_specific_subpath[str(graphics_api).lower()], relative_path, os.path.basename(file)))
					dst_file = os.path.basename(dst_path_rebuilt)
					(root, ext) = os.path.splitext(dst_file)
					binary_file = os.path.join(os.path.dirname(dst_path_rebuilt), root + api_specific_binary_ext[str(graphics_api).lower()])

					interface_path_rebuilt = os.path.abspath(
						os.path.join(interface_path, relative_path, os.path.basename(file)))
					idst_file = os.path.basename(interface_path_rebuilt)
					(iroot, iext) = os.path.splitext(idst_file)
					interface_file = os.path.join(os.path.dirname(interface_path_rebuilt), iroot + '.h')
					implementation_file = os.path.join(os.path.dirname(interface_path_rebuilt), iroot + '.cpp')
				
					if not os.path.exists(os.path.dirname(interface_file)):
						os.makedirs(os.path.dirname(interface_file))

					if not os.path.exists(os.path.dirname(binary_file)):
						os.makedirs(os.path.dirname(binary_file))

					# check if the source file has changed after last generation
					src_file_exists = os.path.exists(file)
					src_file_modified = os.path.getmtime(file)

					changed = False

					# we need to know the permutations so we can check for changed binary files
					permutations = parsePermutations(file)
					p = permute(permutations)

					binary_file_for_check = binary_file

					binary_file_exists = False
					forceRebuild = False

					support_file_path = os.path.join(os.path.dirname(dst_path_rebuilt), root + '.support')

					if len(p) > 0:
						found_all_permutation_binaries = True
						for combination in p:

							binpath = binary_file
							base_directory = os.path.dirname(os.path.normpath(binpath))
							base_filename_ext = os.path.basename(os.path.normpath(binpath))
							base_filename, base_file_extension = os.path.splitext(base_filename_ext)
							base_directory_and_file = os.path.join(base_directory, base_filename)
							binary_file_perm = base_directory_and_file + '_' + str(combination['id']) + base_file_extension

							permutation_exists = os.path.exists(binary_file_perm)
							if permutation_exists:
								permutation_modified = os.path.getmtime(binary_file_perm)
								if permutation_modified < src_file_modified:
									forceRebuild = True
							else:
								found_all_permutation_binaries = False
						binary_file_exists = found_all_permutation_binaries


					# compile shader
					binary_file_modified = src_file_modified
					if len(p) == 0:
						binary_file_exists = os.path.exists(binary_file_for_check)
						if binary_file_exists:
							binary_file_modified = os.path.getmtime(binary_file_for_check)

					if not binary_file_exists or binary_file_modified < src_file_modified or forceRebuild:
						changed = True
						
						if len(p) > 0:
							for combination in p:
								flags = []
								# combination['id'] == 001
								for perm in combination['list']:
									if perm['type'] == 'option' and perm['value'] == 'true':
										flags.append(perm['flag'])
									if perm['type'] == 'enum':
										flags.append(perm['flag'])
								flagStr = ''
								flagLen = len(flags)
								for i in range(flagLen):
									flagStr += '-D'+str(flags[i])
									if i < flagLen-1:
										flagStr += ' '

								binpath = binary_file
								base_directory = os.path.dirname(os.path.normpath(binpath))
								base_filename_ext = os.path.basename(os.path.normpath(binpath))
								base_filename, base_file_extension = os.path.splitext(base_filename_ext)
								base_directory_and_file = os.path.join(base_directory, base_filename)
								binary_file_perm = base_directory_and_file + '_' + str(combination['id']) + base_file_extension

								print('##########################################################################################')
								print('# '+sys.executable+' '+self.shader_compiler_path+' -g '+graphics_api+' -m '+mode+' -i '+file+' -o '+binary_file_perm+' '+flagStr)
								printStdout(subprocess.Popen([sys.executable, self.shader_compiler_path, '-g', graphics_api, '-m', mode, '-i', file, '-o', binary_file_perm, '-x', support_file_path, '', flagStr], stdout = subprocess.PIPE))
						else:
							print('##########################################################################################')
							print('# '+str(sys.executable)+' '+str(self.shader_compiler_path)+' -g '+str(graphics_api)+' -m '+str(mode)+' -i '+str(file)+' -o '+str(binary_file))
							printStdout(subprocess.Popen([sys.executable, self.shader_compiler_path, '-g', graphics_api, '-m', mode, '-i', file, '-o', binary_file, '-x', support_file_path], stdout = subprocess.PIPE))



	def get_shader_info(self, file_name, interface_path, changed):
		return {
			'shader_interface_filepath' : interface_path,
			'shader_type_name' : class_name_from_filename(file_name),
			'stage' : stage_from_filename(file_name),
			'changed' : changed
			}

	def get_common_name(self, shader_type_name):
		if shader_type_name.endswith('MESH'):
			return shader_type_name[:-4]
		if shader_type_name.endswith('AMP'):
			return shader_type_name[:-3]
		return shader_type_name[:-2]

	def get_absolute_path(self, root_path, path):
		return os.path.normpath(os.path.join(os.path.dirname(root_path), path))

	def updateIfChanged(self, targetFile, targetExists, temporaryFile):
		if targetExists:
			hash = ''
			with open(temporaryFile, "rb") as f:
				bytes = f.read()
				hash = hashlib.sha256(bytes).hexdigest()

			orig_hash = ''
			with open(targetFile, "rb") as f:
				bytes = f.read()
				orig_hash = hashlib.sha256(bytes).hexdigest()

			if hash == orig_hash:
				os.remove(temporaryFile)
			else:
				os.remove(targetFile)
				os.rename(temporaryFile, targetFile)
		else:
			os.rename(temporaryFile, targetFile)

	def type_is_srv(self, type):
		parts = type.split('<')
		return parts[0] in srvs_types

	def type_is_uav(self, type):
		parts = type.split('<')
		return parts[0] in uavs_types

	def type_is_root_constant(self, type):
		parts = type.split('<')
		return parts[0] in constant_buffer_types

	def get_set_count(self, syntax_support_file):
		ssf = open(syntax_support_file, 'rb')
		syntax_analyzer_root_level_declarations = pickle.load(ssf)
		ssf.close()

		#print(syntax_support_file)

		some_resources = False
		bindless_count = 0
		for token in syntax_analyzer_root_level_declarations:

			#print(token.type)
			# sampler
			if token.type == 'sampler' or token.type == 'SamplerComparisonState':
				some_resources = True

			# texture SRV
			if self.type_is_srv(token.type):
				if ('Buffer' not in token.type) and ('Bindless' not in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					some_resources = True

			# texture SRV bindless
			if self.type_is_srv(token.type):
				if 'Buffer' not in token.type:
					if 'Bindless' in token.type:
						bindless_count += 1

			# buffer SRV structured
			if self.type_is_srv(token.type):
				if ('Buffer' in token.type) and ('Bindless' not in token.type) and ('Structured' in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					some_resources = True

			# buffer SRV typed
			if self.type_is_srv(token.type):
				if ('Buffer' in token.type) and ('Bindless' not in token.type) and ('Structured' not in token.type) and ('RaytracingAccelerationStructure' not in token.type):
					some_resources = True

			# RaytracingAccelerationStructure
			if self.type_is_srv(token.type):
				if 'RaytracingAccelerationStructure' in token.type:
					some_resources = True

			# texture UAV
			if self.type_is_uav(token.type):
				if 'Buffer' not in token.type:
					if 'Bindless' not in token.type:
						some_resources = True

			# buffer UAV structured
			if self.type_is_uav(token.type):
				if 'Buffer' in token.type:
					if 'Bindless' not in token.type:
						if 'Structured' in token.type:
							some_resources = True

			# buffer UAV typed
			if self.type_is_uav(token.type):
				if 'Buffer' in token.type:
					if 'Bindless' not in token.type:
						if 'Structured' not in token.type:
							some_resources = True

			# root constants
			if self.type_is_root_constant(token.type):
				some_resources = True

			# constants
			if token.type == 'cbuffer':
				some_resources = True

		result = bindless_count
		if some_resources:
			result += 1

		return result

def main():
	this_file_path = os.path.realpath(__file__)

	parser = OptionParser()
	parser.add_option("-g", "--graphics-api", dest="graphicsapi", help="select graphics api. example 1: -g VULKAN , example 2: -g DX12")
	parser.add_option("-i", "--input", dest="input", help="input file. example: -i C:\\work\\Test.hlsl")
	parser.add_option("-o", "--output", dest="output", help="output file. example: -o C:\\work\\Test.frag.spv")
	parser.add_option("-m", "--mode", dest="mode", help="release or debug. example: -o debug")
	parser.add_option("-D", "--define", action='append', dest="define", help="example: -D DEBUG")
	parser.add_option("-I", "--include", action='append', dest="include", help="example: -I ../inc")

	options, arguments = parser.parse_args()

	shader_builder = ShaderBuilder(this_file_path, options.graphicsapi, options.mode)

if __name__ == "__main__":
	main()
