# meson.build — top-level
project('vox_make',
    ['c', 'cpp'],
    version: '0.1.0',
    default_options: [
        'c_std=c17',
        'cpp_std=c++17',
        'warning_level=3',
        'werror=true',
        'b_lto=false',
    ],
)

# ── Platform detection ────────────────────────────────────────────────────
# Set via: meson setup builddir --native-file meson/native.ini
#      or: meson setup builddir --cross-file meson/arm-qemu.ini
target_platform = get_option('target_platform')
message('Target platform: ' + target_platform)

cc = meson.get_compiler('c')

# ── Strict warnings (both C and C++) ──────────────────────────────────────
common_args = [
    '-Wconversion',
    '-Wshadow',
    '-Wdouble-promotion',
    '-Wformat=2',
    '-fno-common',
]

# Embedded-only flags
if target_platform != 'native'
    common_args += [
        '-fno-exceptions',
        '-fno-rtti',
        '-ffunction-sections',
        '-fdata-sections',
        '-Os',
    ]
endif

add_project_arguments(common_args, language: ['c', 'cpp'])

# ── Platform include path ─────────────────────────────────────────────────
platform_inc = include_directories('src/platform/include')
core_inc     = include_directories('src/core/include')
app_inc      = include_directories('src/app/include')

# ── Third-party: Unity ────────────────────────────────────────────────────
subdir('third_party/unity')

# ── Core DSP library (C17) ────────────────────────────────────────────────
core_sources = [
    # 'src/core/ring_buffer.c',
    # 'src/core/fft.c',
]

core_lib = static_library('core',
    core_sources,
    include_directories: [core_inc, platform_inc],
    c_args: ['-std=c17'],
)

core_dep = declare_dependency(
    link_with: core_lib,
    include_directories: [core_inc, platform_inc],
)

# ── Platform HAL ──────────────────────────────────────────────────────────
if target_platform == 'native'
    portaudio_dep = dependency('portaudio-2.0', required: false)
    hal_args = []
    hal_deps = []

    if portaudio_dep.found()
        hal_args += '-DHAS_PORTAUDIO'
        hal_deps += portaudio_dep
        message('PortAudio found — audio I/O enabled')
    else
        message('PortAudio not found — file-only mode')
    endif

    hal_lib = static_library('hal',
        'src/platform/native/hal_native.c',
        include_directories: platform_inc,
        c_args: hal_args,
        dependencies: hal_deps,
    )

elif target_platform == 'qemu'
    hal_lib = static_library('hal',
        'src/platform/qemu/hal_qemu.c',
        include_directories: platform_inc,
    )

elif target_platform == 'stm32f4'
    hal_lib = static_library('hal',
        'src/platform/stm32f4/hal_stm32f4.c',
        include_directories: platform_inc,
    )
endif

hal_dep = declare_dependency(
    link_with: hal_lib,
    include_directories: platform_inc,
)

# ── Application executable ────────────────────────────────────────────────
executable('vox_make',
    'src/app/main.cpp',
    include_directories: app_inc,
    dependencies: [core_dep, hal_dep],
)

# ── Tests (native only) ──────────────────────────────────────────────────
if target_platform == 'native'
    test_sources = [
        'tests/unit/test_main.c',
        # 'tests/unit/test_ring_buffer.c',
    ]

    test_runner = executable('test_runner',
        test_sources,
        dependencies: [core_dep, hal_dep, unity_dep],
    )

    test('unit_tests', test_runner)
endif
