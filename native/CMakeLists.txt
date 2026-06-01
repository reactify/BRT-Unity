# Android Build Command
# cmake -DCMAKE_TOOLCHAIN_FILE=/Applications/Unity/Hub/Editor/6000.0.44f1/PlaybackEngines/AndroidPlayer/NDK/build/cmake/android.toolchain.cmake \
#       -DANDROID_ABI=arm64-v8a \
#       -DANDROID_PLATFORM=android-21 \
#       -B build -DCMAKE_BUILD_TYPE=Release

cmake_minimum_required(VERSION 3.15)
project(AudioPluginBRTUnity LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to MODULE (macOS bundle / Windows DLL / Android SO)
set(LIB_TYPE MODULE)

# Versioning
set(PLUGIN_VERSION_MAJOR 0)
set(PLUGIN_VERSION_MINOR 9)
set(PLUGIN_VERSION_PATCH 0)
set(PLUGIN_VERSION_SHORT "${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR}.${PLUGIN_VERSION_PATCH}")
set(PLUGIN_BUILD_NUMBER "0")

# Platform detection
if(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        # iOS settings
        set(LIB_TYPE STATIC)
        set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "18.0" CACHE STRING "" FORCE)
    else()
        # macOS settings
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version")
        set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)

        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/platform/MacOS/Info.plist.in
            ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
        )
    endif()

elseif(ANDROID)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_UNITY_ANDROID -DUNITY_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_UNITY_ANDROID -DUNITY_64")

    if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--section-start=.text=0x4000")
    endif()
endif()

# Paths
set(BRT_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/lib/BRTLibrary")
set(SOFA_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/lib/BRTLibrary/include/third_party_libraries/libmysofa")
set(UNITY_PACKAGE_DIR "${CMAKE_SOURCE_DIR}/../brt-unity-package")
set(PLUGIN_OUTPUT_DIR "${UNITY_PACKAGE_DIR}/Runtime/Plugins")

# Platform-specific output dirs
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH_DIR "x86_64")
    else()
        set(ARCH_DIR "x86")
    endif()
    set(PLUGIN_OUTPUT_DIR "${PLUGIN_OUTPUT_DIR}/${ARCH_DIR}")

elseif(APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(PLUGIN_OUTPUT_DIR "${PLUGIN_OUTPUT_DIR}/iOS")

elseif(APPLE)
    set(PLUGIN_OUTPUT_DIR "${PLUGIN_OUTPUT_DIR}/osx")

elseif(ANDROID)
    set(PLUGIN_OUTPUT_DIR "${PLUGIN_OUTPUT_DIR}/Android/${CMAKE_ANDROID_ARCH_ABI}")

elseif(UNIX)
    set(PLUGIN_OUTPUT_DIR "${PLUGIN_OUTPUT_DIR}/Linux")
endif()

# Sources
file(GLOB_RECURSE PROJECT_SRC CONFIGURE_DEPENDS src/*.cpp src/*.h)
source_group(TREE "${CMAKE_SOURCE_DIR}/src" PREFIX "Source" FILES ${PROJECT_SRC})

file(GLOB_RECURSE BRT_LIBRARY
    "${BRT_LIBRARY_DIR}/include/*.hpp"
    "${BRT_LIBRARY_DIR}/include/*.h"
)
source_group(TREE "${BRT_LIBRARY_DIR}/include/" PREFIX "BRTLibrary" FILES ${BRT_LIBRARY})

# Library
add_library(AudioPluginBRTUnity ${LIB_TYPE}
    ${PROJECT_SRC}
    ${BRT_LIBRARY}
)

# Includes
target_include_directories(AudioPluginBRTUnity PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    "${BRT_LIBRARY_DIR}/include"
    "${BRT_LIBRARY_DIR}/include/third_party_libraries"
    "${BRT_LIBRARY_DIR}/include/third_party_libraries/eigen"
    "${BRT_LIBRARY_DIR}/include/third_party_libraries/boost_circular_buffer"
)

# Defines
target_compile_definitions(AudioPluginBRTUnity PUBLIC
    _3DTI_AXIS_CONVENTION_UNITY
    _3DTI_ANGLE_CONVENTION_LISTEN
)

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

# -------------------------
# macOS (Bundle)
# -------------------------
if(APPLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    set_target_properties(AudioPluginBRTUnity PROPERTIES
        BUNDLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_IDENTIFIER "co.uk.reactify.brt-unity"
        SUFFIX ".bundle"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
        VERSION ${PLUGIN_VERSION_SHORT}
    )

    target_link_libraries(AudioPluginBRTUnity PRIVATE
        z
        "${SOFA_LIBRARY_DIR}/lib/osx/universal/libmysofa.a"
    )

    add_custom_command(TARGET AudioPluginBRTUnity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_BUNDLE_DIR:AudioPluginBRTUnity>"
            "${PLUGIN_OUTPUT_DIR}/AudioPluginBRTUnity.bundle"
    )

# -------------------------
# iOS (Static Library)
# -------------------------
elseif(APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")

    message(STATUS "APPLE=${APPLE}")
    message(STATUS "CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
    message(STATUS "CMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}")
    message(STATUS "HEEEEEELLLLLLLOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO")

    target_link_libraries(AudioPluginBRTUnity PRIVATE
        z
        "${CMAKE_SOURCE_DIR}/lib/libmysofa/ios/ARM64/${CMAKE_BUILD_TYPE}/libmysofa.a"
    )

    add_custom_command(TARGET AudioPluginBRTUnity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:AudioPluginBRTUnity>"
            "${PLUGIN_OUTPUT_DIR}/libAudioPluginBRTUnity.a"
    )

# -------------------------
# Windows
# -------------------------
elseif(WIN32)

    target_link_directories(AudioPluginBRTUnity PRIVATE
        "${SOFA_LIBRARY_DIR}/lib/vs/x64"
    )

    target_link_libraries(AudioPluginBRTUnity PRIVATE
        mysofa
        $<$<CONFIG:Debug>:zlibstaticd>
        $<$<CONFIG:Release>:zlibstatic>
    )

    add_custom_command(TARGET AudioPluginBRTUnity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:AudioPluginBRTUnity>"
            "${PLUGIN_OUTPUT_DIR}/AudioPluginBRTUnity.dll"
    )

# -------------------------
# Android
# -------------------------
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")

    message(STATUS "Android API Level: ${ANDROID_NATIVE_API_LEVEL}")
    message(STATUS "Android ABI: ${ANDROID_ABI}")

    target_link_directories(AudioPluginBRTUnity PRIVATE
        "${CMAKE_SOURCE_DIR}/lib/libmysofa/android/ARM64/${CMAKE_BUILD_TYPE}"
    )

    target_link_libraries(AudioPluginBRTUnity PRIVATE
        mysofa
        z
    )

    add_custom_command(TARGET AudioPluginBRTUnity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:AudioPluginBRTUnity>
            ${PLUGIN_OUTPUT_DIR}
    )

endif()