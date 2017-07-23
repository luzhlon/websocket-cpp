
target 'websocket'
    set_kind 'static'
    add_headers '*.hpp'
    add_files '*.cpp'

    if os.host() ~= 'windows' then
        add_cxxflags '-std=c++11'
    elseif is_mode 'debug' then
        add_cxxflags '/MTd'
    end
