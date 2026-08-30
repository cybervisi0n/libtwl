project('libtwl', 'c', 'cpp', 'nasm',
    default_options: {
        'default_library': 'static',
        'prefer_static':    false
    },
    meson_version: '>=1.3.0',
    version: '5.5-20100715a'
)

fs = import('fs')

buildtype = get_option('buildtype')
cc = meson.get_compiler('c')
build_target = get_option('build_target')

############################################################
###                    COMPILER FLAGS                    ###
############################################################

if not meson.is_subproject() and buildtype != 'plain' and build_target == 'arm'
shared_args = [
    '-O4,p',
    '-proc', 'arm946e',
    '-enum', 'int',
    '-lang', 'c99',
    '-Cpp_exceptions', 'off',
    '-gccext,on',
    '-msgstyle', 'gcc',
    '-ipa', 'file',
    '-interworking',
    '-inline', 'on,noauto',
    '-char', 'signed',
    '-nosyspath',
    '-stdinc'
]
else
shared_args = []
endif

mwrap_ver = get_option('mwrap_ver')
if mwrap_ver != 'default' and build_target == 'arm'
    shared_args += ['-wrap:ver', mwrap_ver]
endif

mwrap_sdk = get_option('mwrap_sdk')
if mwrap_sdk != 'default' and build_target == 'arm'
  shared_args += ['-wrap:sdk', mwrap_sdk]
endif

if get_option('sp1p3_conditionals')
    shared_args += '-DSP1P3_BUG_FOR_CONDITIONAL_ASM_INSTRUCTIONS'
endif

c_args = shared_args
cpp_args = shared_args

add_project_arguments(c_args,
    language: 'c',
    native: (build_target == 'linux' or build_target == 'win64')
)

add_project_arguments(cpp_args,
    language: 'c',
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_args = [
    '-DSDK_CW_FORCE_EXPORT_SUPPORT',
    '-DSDK_TS',
    '-DSDK_4M',
    '-DSDK_FINALROM',
    '-DNNS_FINALROM'
]

if(get_option('twlmode'))
    libtwl_args += [
        '-DSDK_TWL'
    ]
endif

if (build_target == 'arm')
    libtwl_args += [
        '-DSDK_ARM9',
        '-DSDK_CW',
        '-DSDK_CODE_ARM',
        '-DSDK_BUILD_ARM'
    ]
endif

if (build_target == 'win64')
    libtwl_args += [
        '-DSDK_PORT',
        '-DSDK_X86',
        '-DSDK_CODE_X86',
        '-DSDK_BUILD_WIN64',
        '-D_NITRO',
        '-w'
    ]
endif

if (build_target == 'linux')
    libtwl_args += [
        '-DSDK_PORT',
        '-DSDK_X86',
        '-DSDK_CODE_X86',
        '-DSDK_BUILD_LINUX',
        '-D_NITRO',
        '-w',
    ]
endif

if (build_target == 'nx')
    libtwl_args += [
        '-DSDK_PORT',
        '-DSDK_AARCH64',
        '-DSDK_BUILD_NX',
        '-D_NITRO',
        '-D__SWITCH__',
        '-w',
        '-I/opt/devkitpro/portlibs/switch/include',
        '-I/opt/devkitpro/libnx/include'
    ]
endif

add_project_arguments(libtwl_args,
    language: 'c',
    native: (build_target == 'linux' or build_target == 'win64')
)

add_project_arguments(libtwl_args,
    language: 'cpp',
    native: (build_target == 'linux' or build_target == 'win64')
)

############################################################
###                  GENERATED HEADERS                   ###
############################################################
subdir('gen')
nitro_pch = meson.current_source_dir() / 'include/pch/nitro_pch.h'

############################################################
###                    INCLUDE PATHS                     ###
############################################################
libtwl_includes = include_directories('gen', 'include')
include_nitro = include_directories('include' / 'nitro')

############################################################
###                      LIBRARIES                       ###
############################################################
subdir('libraries')

############################################################
###                      SPECFILES                       ###
############################################################
#subdir('specfiles')

############################################################
###                        TOOLS                         ###
############################################################
#subdir('tools')

############################################################
###                    FOR CONSUMERS                     ###
############################################################

libtwl_dep = declare_dependency(
    compile_args: libtwl_args,
    include_directories: libtwl_includes,
    link_with: libtwl_libs + libtwl_prebuilt,
    sources: fx_const_h
    #objects: crt0_obj
)

meson.override_dependency('NitroSDK', libtwl_dep)
meson.override_dependency('TwlSDK', libtwl_dep)

if(get_option('standalone') == true)
    subdir('standalone')
endif
