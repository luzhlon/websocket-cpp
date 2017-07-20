
target 'test'
    set_kind 'binary'
    add_headers '../src/*.hpp'
    add_files '../src/*.cpp'
    add_files 'test.cpp'
