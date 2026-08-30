subdir('src')

libcard_includes = include_directories('include')

libcard = library('card',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes, libcard_includes],
    pic: build_target == 'nx',
    sources: [libcard_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libcard
