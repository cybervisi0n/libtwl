subdir('src')

libext = library('ext',
    c_pch: nitro_pch,
    include_directories: libtwl_includes,
    pic: build_target == 'nx',
    sources: [libext_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libext
