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
set(LIB_TYPE MODULE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version")

set(PLUGIN_VERSION_MAJOR 0)
set(PLUGIN_VERSION_MINOR 9)
set(PLUGIN_VERSION_PATCH 0)

set(PLUGIN_VERSION_SHORT "${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR}.${PLUGIN_VERSION_PATCH}")
set(PLUGIN_BUILD_NUMBER "0")

if(APPLE)
    set(CFBUNDLE_NAME ${PROJECT_NAME})
    set(CFBUNDLE_EXECUTABLE ${PROJECT_NAME})
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "")
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(LIB_TYPE STATIC)
    else()
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

set(BRT_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/lib/BRTLibrary")
set(SOFA_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/lib/BRTLibrary/include/third_party_libraries/libmysofa")
set(UNITY_PACKAGE_DIR "${CMAKE_SOURCE_DIR}/../brt-unity-package")
set(PLUGIN_OUTPUT_DIR "${UNITY_PACKAGE_DIR}/Runtime/Plugins")
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH_DIR "x86_64")
    else()
        set(ARCH_DIR "x86")
    endif()
    set(PLUGIN_OUTPUT_DIR "${PLUGIN_OUTPUT_DIR}/${ARCH_DIR}")
elseif(APPLE)
    set(PLUGIN_OUTPUT_DIR "${PLUGIN_OUTPUT_DIR}/osx")
elseif(ANDROID)
    set(PLUGIN_OUTPUT_DIR "${PLUGIN_OUTPUT_DIR}/Android/${CMAKE_ANDROID_ARCH_ABI}")
elseif(UNIX)
    set(PLUGIN_OUTPUT_DIR "${PLUGIN_OUTPUT_DIR}/Linux")
endif()

# Collect project sources
file(GLOB_RECURSE PROJECT_SRC CONFIGURE_DEPENDS src/*.cpp src/*.h)
source_group(TREE "${CMAKE_SOURCE_DIR}/src" PREFIX "Source" FILES ${PROJECT_SRC})

# Collect third-party sources
file(GLOB_RECURSE BRT_LIBRARY 
    "${BRT_LIBRARY_DIR}/include/*.hpp"
    "${BRT_LIBRARY_DIR}/include/*.h" 
)
source_group(TREE "${BRT_LIBRARY_DIR}/include/" PREFIX "BRTLibrary" FILES ${BRT_LIBRARY})

# Add library
add_library(AudioPluginBRTUnity ${LIB_TYPE}
    ${PROJECT_SRC}
    ${BRT_LIBRARY}
)

target_include_directories(AudioPluginBRTUnity PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    "${BRT_LIBRARY_DIR}/include"
    "${BRT_LIBRARY_DIR}/include/third_party_libraries"
    "${BRT_LIBRARY_DIR}/include/third_party_libraries/eigen"
    "${BRT_LIBRARY_DIR}/include/third_party_libraries/boost_circular_buffer"
)

target_compile_definitions(AudioPluginBRTUnity PUBLIC
    _3DTI_AXIS_CONVENTION_UNITY
    _3DTI_ANGLE_CONVENTION_LISTEN
)

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

if(APPLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist.in")
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
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    # Enable specific features or optimizations for Android
    message(STATUS "Android API Level: ${ANDROID_NATIVE_API_LEVEL}")
    message(STATUS "Android ABI: ${ANDROID_ABI}")

    target_link_directories(AudioPluginBRTUnity PRIVATE 
        "${CMAKE_SOURCE_DIR}/lib/libmysofa/android/ARM64/${CMAKE_BUILD_TYPE}"
    )
    target_link_libraries(AudioPluginBRTUnity PRIVATE 
        mysofa
        z
    )

    # Copy the .so after building
    add_custom_command(TARGET AudioPluginBRTUnity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:AudioPluginBRTUnity> ${PLUGIN_OUTPUT_DIR}
        COMMENT "Copying AudioPluginBRTUnity.so to ${PLUGIN_OUTPUT_DIR}"
    )
endif()
