subdir('src')

libinit_includes = include_directories('include')

libinit = library('init',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes, libinit_includes],
    pic: build_target == 'nx',
    sources: [libinit_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

crt0_obj = libinit.extract_all_objects(recursive: true)

