libtwl_link_args = []

libtwl_exe = executable(
    'libtwl',
    sources: files('main.cpp'),
    dependencies: [ libtwl_dep ],
    link_args: [ libtwl_link_args ],
    native: build_target != 'arm',
)
