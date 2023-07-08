import os
import time
import sys
from optparse import OptionParser

header_location = '../src'
moc_file_location = '../moc'

moc_keywords = ['Q_OBJECT']
moc_binary_win_x86 = 'C:/Qt/5.7/msvc2015/bin/moc.exe'
moc_binary_win_x64 = 'C:/Qt/5.7/msvc2015_64/bin/moc.exe'

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

def header_absolute_location():
    script_location = os.path.dirname(os.path.realpath(__file__))
    header_absolute_location = os.path.normpath(os.path.join(script_location, header_location))
    return header_absolute_location

def moc_file_absolute_location():
    script_location = os.path.dirname(os.path.realpath(__file__))
    moc_absolute_location = os.path.normpath(os.path.join(script_location, moc_file_location))
    return moc_absolute_location

def moc_is_older_than_source(sourceFilePath, mocFilePath):
    if not os.path.exists(mocFilePath):
        return True
    return time.ctime(os.path.getmtime(sourceFilePath)) > time.ctime(os.path.getmtime(mocFilePath))

def destination(source_file_path):
    header_abs = header_absolute_location()
    moc_abs = moc_file_absolute_location()
    file_path_abs, file = os.path.split(source_file_path)

    file_split = file_path_abs.split(os.sep)
    header_split = header_abs.split(os.sep)
    file_relative_path = os.sep.join(file_split[len(header_split):])

    filename, file_extension = os.path.splitext(file)
    destination_path = os.path.join(moc_abs, file_relative_path)
    destination_file_path = os.path.join(destination_path, filename + '_moc.cpp')
    return destination_path, destination_file_path

def needs_moc(filepath):
    with open(filepath, 'r') as file:
        for line in file.readlines():
            for keyword in moc_keywords:
                if keyword in line:
                    destination_path, destination_file_path = destination(filepath)
                    return moc_is_older_than_source(filepath, destination_file_path)
    return False

def path_list_difference(path_a, path_b):
    print str(path_a) + ' ### ' + str(path_b)
    return [item for item in path_a if item not in path_b]

def moc_file(moc_binary, filepath):
    destination_path, destination_file_path = destination(filepath)

    if not os.path.exists(destination_path):
        os.makedirs(destination_path)

    os.system(moc_binary + ' ' + filepath + ' -o ' + destination_file_path)

def main():
    parser = OptionParser()
    parser.add_option("-a", "--arch", dest="arch",
                      help="moc binary architecture. Supported: x64, x86. example: -a x64", metavar="ARCH")
    (options, args) = parser.parse_args()

    moc_binary = ''
    if options.arch == "x64":
        moc_binary = moc_binary_win_x64
    if options.arch == "x86":
        moc_binary = moc_binary_win_x86
    
    if moc_binary == '':
        moc_binary = moc_binary_win_x64

    files = recursive_filelist(header_absolute_location())
    for file in files:
        filename, file_extension = os.path.splitext(file)
        if file_extension == '.h':
            if needs_moc(file):
                moc_file(moc_binary, file)

if __name__ == "__main__":
    main()

