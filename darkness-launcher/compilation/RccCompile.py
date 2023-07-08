import os
from optparse import OptionParser

qrc_location = '../data/DarknessLauncher.qrc'

rcc_binary_win_x86 = 'C:/Qt/5.7/msvc2015/bin/rcc.exe'
rcc_binary_win_x64 = 'C:/Qt/5.7/msvc2015_64/bin/rcc.exe'

data_binary_path_x64_debug = '../bin/x64/Debug'
data_binary_path_x64_release = '../bin/x64/Release'
data_binary_path_x86_debug = '../bin/Win32/Debug'
data_binary_path_x86_release = '../bin/Win32/Release'

backend_path_dx12 = ' DX12'
backend_path_vulkan = ' Vulkan'

def main():
    script_location = os.path.dirname(os.path.realpath(__file__))
    qrc_absolute_location = os.path.normpath(os.path.join(script_location, qrc_location))

    parser = OptionParser()
    parser.add_option("-a", "--arch", dest="arch",
                      help="rcc binary architecture. Supported: x64, x86. example: -a x64", metavar="ARCH")
    parser.add_option("-c", "--config", dest="config",
                      help="rcc binary config. Supported: debug, release. example: -c debug", metavar="CONFIG")
    parser.add_option("-b", "--backend", dest="backend",
                      help="rcc binary backend. Supported: vulkan, dx12. example: -c dx12", metavar="CONFIG")
    (options, args) = parser.parse_args()

    rcc_binary = ''
    if options.arch == "x64":
        rcc_binary = rcc_binary_win_x64
    if options.arch == "x86":
        rcc_binary = rcc_binary_win_x86
    if rcc_binary == '':
        rcc_binary = rcc_binary_win_x64

    data_binary_location = ''
    if options.config == "debug" and options.arch == "x64":
        data_binary_location = data_binary_path_x64_debug
    if options.config == "release" and options.arch == "x64":
        data_binary_location = data_binary_path_x64_release
    if options.config == "debug" and options.arch == "x86":
        data_binary_location = data_binary_path_x86_debug
    if options.config == "release" and options.arch == "x86":
        data_binary_location = data_binary_path_x86_release
    if data_binary_location == '':
        data_binary_location = data_binary_path_x64_debug

    if options.backend == "dx12":
        data_binary_location += backend_path_dx12
    if options.backend == "vulkan":
        data_binary_location += backend_path_vulkan

    data_binary_location = os.path.join(data_binary_location, 'DarknessLauncher.rcc')
    qrc_binary_absolute_location = os.path.normpath(os.path.join(script_location, data_binary_location))
    qrc_binary_absolute_location = '"'+qrc_binary_absolute_location+'"'

    command = rcc_binary + ' -binary '+qrc_absolute_location+' -o '+qrc_binary_absolute_location
    os.system(command)

if __name__ == "__main__":
    main()

