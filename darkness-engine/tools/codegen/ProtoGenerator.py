import os
from optparse import OptionParser
import subprocess
import sys
import json

relative_path_proto_compiler = '../../../darkness-externals/output/win64/release_lib/protoc.exe'
relative_path_proto_compiler_debug = '../../../darkness-externals/output/win64/debug_lib/protoc.exe'

relative_path_proto_locations = [
	{ 
		'source_folder' : '../../protocols', 
		'interface_folder' : '../../include/protocols' }
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
			print line.rstrip()
		else:
			break

class ProtoBuilder:
	def __init__(self, root_path):
		self.proto_builder_path = root_path
		self.proto_compiler_path = os.path.join(os.path.dirname(root_path), relative_path_proto_compiler)
		if(not os.path.exists(self.proto_compiler_path)):
			self.proto_compiler_path = os.path.join(os.path.dirname(root_path), relative_path_proto_compiler_debug)
		self.all_protos = []
		
		for proto_location in relative_path_proto_locations:
			src_path = self.get_absolute_path(root_path, proto_location['source_folder'])
			dst_path = self.get_absolute_path(root_path, proto_location['interface_folder'])
			
			files = recursive_filelist(src_path)
			for file in files:
				if file.endswith('.proto'):
					relative_path = os.path.relpath(os.path.dirname(file), src_path)
					if not os.path.exists(os.path.join(dst_path, relative_path)):
						os.makedirs(os.path.join(dst_path, relative_path))

					dst_file_path = os.path.join(dst_path, relative_path)

					src_file = os.path.basename(file)
					src_file_modified = os.path.getmtime(file)
					(root, ext) = os.path.splitext(src_file)
					dst_file = os.path.join(dst_file_path, root + '.pb.h')
					perform_compile = False
					if os.path.exists(dst_file):
						last_modified = os.path.getmtime(dst_file)
						if last_modified < src_file_modified:
							perform_compile = True
					else:
						perform_compile = True

					if perform_compile:
						printStdout(subprocess.Popen([self.proto_compiler_path, '--proto_path='+os.path.join(src_path, relative_path), '--cpp_out='+dst_file_path, file], stdout = subprocess.PIPE))

	def get_absolute_path(self, root_path, path):
		return os.path.normpath(os.path.join(os.path.dirname(root_path), path))

def main():
	this_file_path = os.path.realpath(__file__)

	parser = OptionParser()
	options, arguments = parser.parse_args()

	proto_builder = ProtoBuilder(this_file_path)

if __name__ == "__main__":
	main()
