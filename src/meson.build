src_files = files('mpris.cpp', 'dbus_interface.cpp', 'nancho.cpp', 'player.cpp', 'pulse.cpp', 'state.cpp', 'trigger.cpp')

cxxopts = subproject('cxxopts')
cxxopts_dep = cxxopts.get_variable('cxxopts_dep')

spdlog = subproject('spdlog')
spdlog_dep = spdlog.get_variable('spdlog_dep')

dbus = dependency('dbus-1', required : true)
pulse = dependency('libpulse', required : true)

clangtidy = find_program('clang-tidy', required : false)
if clangtidy.found()
    run_target(
        'tidy',
        command: [
            clangtidy,
            '-p', meson.build_root()
        ] + src_files)
endif

executable('nancho', src_files, dependencies : [cxxopts_dep, spdlog_dep, dbus, pulse], install : true, install_dir : 'bin')