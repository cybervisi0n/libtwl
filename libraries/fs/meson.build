subdir('src')

libfs_includes = include_directories('include')

libfs = library('fs',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes, libfs_includes],
    pic: build_target == 'nx',
    sources: [libfs_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libfs
