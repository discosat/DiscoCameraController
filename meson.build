project('camera_controller', 'c', 'cpp', subproject_dir: 'lib', default_options: [
        'buildtype=debug',
        'c_std=gnu11',
        'b_lto=false',
        'default_library=static',
        'csp:packet_padding_bytes=42',
        'csp:buffer_count=1000',
        'csp:buffer_size=2048',
        'csp:conn_max=20',
        'csp:conn_rxqueue_len=1000',
        'csp:qfifo_len=1000',
        'csp:rdp_max_window=1000',
        'csp:port_max_bind=16',
        'csp:use_rtable=true',
        'param:have_fopen=true',
        'param:collector=false',
        'param:list_dynamic=true'
])

add_global_arguments('-I../include', language: 'c')

sources = files([
    'src/main.cpp',
    'src/message_queue/message_queue.cpp',
    'src/camera_control/vimba_controller.cpp',
    'src/camera_control/capture_controller.cpp',
    'src/communication/csp_server.c',
    'src/communication/param_config.c',
    'src/communication/serial.c',
    'src/communication/vmem_config.c',
])

cpp = meson.get_compiler('cpp')

csp_dep = dependency('csp', fallback: ['csp', 'csp_dep'])
param_dep = dependency('param', fallback: ['param', 'param_dep'])
opencv_dep = dependency('opencv4')

deps = [csp_dep,param_dep,opencv_dep]

dirs = []
dirs += include_directories(
    'include/message_queue', 
    'include/utils', 
    'include/camera_control', 
    'include/communication',
)

if meson.is_cross_build()
    message('Compiling for aarch64')
    vmb_dep = cpp.find_library('VmbCPP', dirs: join_paths(meson.source_root(),'lib/VimbaX_2023-4-ARM64/api/lib'))
    deps += [vmb_dep]
    dirs += include_directories('lib/VimbaX_2023-4-ARM64/api/include')
else
    message('Native build')
    vmb_dep = cpp.find_library('VmbCPP', dirs: join_paths(meson.source_root(),'lib/VimbaX_2023-4-Linux/api/lib'))
    deps += [vmb_dep]
    dirs += include_directories('lib/VimbaX_2023-4-Linux/api/include')
endif

executable('Disco2CameraControl', sources,
    include_directories : dirs,
    dependencies : deps,
    install : true,
    link_args : ['-Wl,--export-dynamic', '-ldl'],
)