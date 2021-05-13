project('boost.ut', 'cpp')
boostut_dep = declare_dependency(include_directories : include_directories('include'))
if meson.version().version_compare('>=0.54.0')
  meson.override_dependency('boost.ut', boostut_dep)
endif
