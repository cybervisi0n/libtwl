subdir('src')

libctrdg_includes = include_directories('include')

libctrdg = library('ctrdg',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes, libctrdg_includes],
    pic: build_target == 'nx',
    sources: [libctrdg_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libctrdg
