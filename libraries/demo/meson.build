subdir('src')

if (build_target == 'arm')
libdemo = library('demo',
    c_pch: nitro_pch,
    include_directories: libtwl_includes,
    pic: false,
    sources: [libdemo_srcs, fx_const_h]
)
else
libdemo = library('demo',
    c_pch: nitro_pch,
    include_directories: libtwl_includes,
    pic: build_target == 'nx',
    sources: [libdemo_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)
endif

libtwl_libs += libdemo