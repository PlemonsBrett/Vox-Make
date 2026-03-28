# third_party/unity/CMakeLists.txt
# Fetches ThrowTheSwitch/Unity (v2.6+ ships CMake; FetchContent adds target "unity").

include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(unity)
