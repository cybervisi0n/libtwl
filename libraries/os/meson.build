subdir('src')

libos_includes = include_directories('include')

libos = library('os',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes, libos_includes],
    pic: build_target == 'nx',
    sources: [libos_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libos
