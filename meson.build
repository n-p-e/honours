project(
    'graph-mining',
    'cpp',
    default_options: {
        'cpp_std': 'c++20',
    }
)

src = files([
    'src/main.cpp',
    'src/graph/graph.cpp',
    'src/graph/graph.hpp',
    'src/graph/kplex.cpp',
    'src/graph/kplex.hpp',
    'src/heap.hpp',
])

cxx = meson.get_compiler('cpp')
deps = [dependency('boost', modules: ['program_options'])]

if get_option('buildtype') == 'release'
  add_project_arguments('-O3', language : 'cpp')
endif

executable('main',
    src,
    dependencies: deps,
    cpp_pch: 'src/pch.hpp',
    include_directories: include_directories('src'))
