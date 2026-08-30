subdir('src')

libmb_includes = include_directories('include')

libmb = library('mb',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes, libmb_includes],
    pic: build_target == 'nx',
    sources: [libmb_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libmb
