
target 'test'
    set_kind 'binary'
    add_headers '../src/*.hpp'
    add_files 'test.cpp'

    add_linkdirs '$(buildir)'
    add_links 'websocket'

    add_deps 'websocket'

    if os.host() ~= 'windows' then
        add_cxxflags '-std=c++11'
    elseif is_mode 'debug' then
        add_cxxflags '/MTd'
    end
