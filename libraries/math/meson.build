subdir('src')

libmath_includes = include_directories('include')

libmath = library('math',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes, libmath_includes],
    pic: build_target == 'nx',
    sources: [libmath_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libmath
