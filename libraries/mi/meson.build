subdir('src')

libmi_includes = include_directories('include')

libmi = library('mi',
    c_pch: nitro_pch,
    include_directories: [libtwl_includes, libmi_includes],
    pic: build_target == 'nx',
    sources: [libmi_srcs, fx_const_h],
    native: (build_target == 'linux' or build_target == 'win64')
)

libtwl_libs += libmi
