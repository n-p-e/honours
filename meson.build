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
    'src/graph/graphv2.cpp',
    'src/graph/graphv2.hpp',
    'src/graph/kplex.cpp',
    'src/graph/kplex.hpp',
    'src/graph/kdefective.cpp',
    'src/graph/kdefective.hpp',
    'src/heap.hpp',
])

cxx = meson.get_compiler('cpp')
deps = [dependency('boost', modules: ['program_options'])]

executable('main',
    src,
    dependencies: deps,
    cpp_pch: 'src/pch.hpp',
    include_directories: include_directories('src'))
