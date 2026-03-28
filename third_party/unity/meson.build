# third_party/unity/meson.build
# Wraps ThrowTheSwitch/Unity as a Meson subproject.
#
# To use: run `meson wrap install unity` or manually place:
#   subprojects/unity.wrap

# If using wrap, Unity provides its own meson.build.
# If vendoring manually, build from source:

unity_proj = subproject('unity',
    default_options: ['warning_level=0'],
    required: false,
)

if unity_proj.found()
    unity_dep = unity_proj.get_variable('unity_dep')
else
    # Fallback: vendor Unity source directly into third_party/unity/src/
    unity_inc = include_directories('src')
    unity_lib = static_library('unity',
        'src/unity.c',
        include_directories: unity_inc,
        override_options: ['warning_level=0'],
    )
    unity_dep = declare_dependency(
        link_with: unity_lib,
        include_directories: unity_inc,
    )
endif
