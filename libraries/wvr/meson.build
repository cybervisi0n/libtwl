subdir('src')

libwvr = library('wvr',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes],
    pic: build_target == 'nx',
    sources: [libwvr_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libwvr
