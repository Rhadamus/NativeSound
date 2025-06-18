cmake_minimum_required(VERSION 3.22.1)

project("AndroidNativeSound")

set(OPENAL_DIR ./lib/openal-soft)
set(OGG_DIR ./lib/ogg-master)
set(VORBIS_DIR ./lib/vorbis-master)

include_directories(./lib)
include_directories(${OPENAL_DIR}/include)
include_directories(${OGG_DIR}/include)
include_directories(${VORBIS_DIR}/include)

link_directories(${OPENAL_DIR}/build/${CMAKE_ANDROID_ARCH_ABI})

add_library(NativeSound SHARED
        CSoundPlayer.cpp
        CSound.cpp
        CSoundChannel.cpp
        CSoundFile.cpp
        CSoundFileWAV.cpp
        CSoundFileOGG.cpp
        CSoundFileMP3.cpp
        JNIBridge.cpp)

add_subdirectory(${OGG_DIR} ./ogg)
add_subdirectory(${VORBIS_DIR} ./vorbis)

set_target_properties(NativeSound PROPERTIES
        CXX_VISIBILITY_PRESET hidden)

target_link_options(NativeSound PRIVATE
        "-Wl,-z,max-page-size=16384")

target_link_libraries(NativeSound
        log
        openal
        ogg
        vorbis
        vorbisfile)