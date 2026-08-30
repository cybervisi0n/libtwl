subdir('src')

libspi_includes = include_directories('include')

libspi = library('spi',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes, libspi_includes],
    pic: build_target == 'nx',
    sources: [libspi_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libspi
