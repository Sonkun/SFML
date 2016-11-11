# ------------------------------------------------------------------------------
#  Tizen CMake toolchain file, for use with Tizen Studio.
#  Requires cmake 3.5.1 or newer.
#
#  Written by Jonathan De Wachter (dewachter.jonathan@gmail.com).
#
#  This toolchain was initially written to compile the SFML project but
#  it's generic enough to be reused in other project. It supports every possible
#  configuration
#
#  This toolchain doesn't support "release minsize" and "release with debug
#  info" build as well as the creation of module or executable. Only static and
#  shared build in either release or debug mode are allowed.
#
#  Usage Linux & Mac:
#   $ export TIZEN_STUDIO=/opt/tizen-studio
#   $ cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/tizen.toolchain.cmake
#   $ make
#
#  Usage Windows:
#   > set PATH=%PATH%;C:\TizenStudio
#   > cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="../cmake/toolchains/tizen.toolchain.cmake"
#   > make
#
#  Options (can be set as cmake parameters: -D<option_name>=<value>):
#
#   * TIZEN_DEVICE="mobile-2.4" - Specifies the device type and version
#
#       Can be set to target either a mobile or wearable of different
#       versions.
#
#   Possible devices are:
#     mobile-2.4 (default)
#     mobile-2.3.1
#     mobile-2.3
#     wearable-2.3.2
#     wearable-2.3.1
#     wearable-2.3
#
#   * TIZEN_TARGET="device" - Specifies the device architecture
#
#       Can be set to build for either an existing device (arm) or an
#       emulator (x86)
#
#   Possible architectures are:
#     x86
#     arm (default)
#
#   * TIZEN_COMPILER="llvm-3.6" - Specifies the compiler (and its version) to use
#
#      Can be set to use either Clang (llvm) or GNU GCC (gcc).
#
#   Possible compilers are:
#     llvm-3.6 (default)
#     llvm-3.4
#     gcc-4.6
#     gcc-4.9

# NOTE: this script isn't finished yet!
cmake_minimum_required(VERSION 3.5.1)

# retrieve environment info
set(TIZEN_STUDIO "$ENV{TIZEN_STUDIO}")
message(STATUS ${TIZEN_STUDIO})
# todo: fail early if tizen studio env var is not set (or assume /opt ?)
# todo: add CMAke option to set TIZEN_STUDIO ?

# set default options value
if(NOT DEFINED TIZEN_DEVICE)
    set(TIZEN_DEVICE "mobile-2.4")
endif()
if(NOT DEFINED TIZEN_TARGET)
    set(TIZEN_TARGET "device")
endif()
if(NOT DEFINED TIZEN_COMPILER)
    set(TIZEN_COMPILER "llvm-3.6")
endif()

# todo": check for unsupported values ? (this script will fail if not set
# properly

# compute DEVICE_TYPE (mobile or wearable) and DEVICE_VERSION (2.3,
# 2.3.1, 2.4) from TIZEN_DEVICE variable
string(REPLACE "-" ";" DEVICE_LIST ${TIZEN_DEVICE})
list(GET DEVICE_LIST 0 DEVICE_TYPE)
list(GET DEVICE_LIST 1 DEVICE_VERSION)

# compute another useful variable
if (TIZEN_TARGET MATCHES device)
    set(SYSTEM_PROCESSOR "armel")
    set(TIZEN_ARCHITECTURE "arm")
elseif (TIZEN_TARGET MATCHES emulator)
    set(SYSTEM_PROCESSOR "i386")
    set(TIZEN_ARCHITECTURE "i386")
endif()

# set mandatory CMAKE_SYSTEM_NAME and CMAKE_SYSTEM_PROCESSOR variables
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ${SYSTEM_PROCESSOR})

# compute TIZEN_ROOTSTRAP path from variable
set(TIZEN_ROOTSTRAP "${TIZEN_STUDIO}/platforms/tizen-${DEVICE_VERSION}/${DEVICE_TYPE}/rootstraps/${TIZEN_DEVICE}-${TIZEN_TARGET}.core")

# setup compilers
if (TIZEN_COMPILER MATCHES llvm-*)

    #include(CMakeForceCompiler)
    #cmake_force_c_compiler("${TIZEN_STUDIO}/tools/${TIZEN_COMPILER}/bin/clang"LLVM)
    #cmake_force_cxx_compiler("${TIZEN_STUDIO}/tools/${TIZEN_COMPILER}/bin/clang++" LLVM)

    set(CMAKE_C_COMPILER "${TIZEN_STUDIO}/tools/${TIZEN_COMPILER}/bin/clang")
    set(CMAKE_CXX_COMPILER "${TIZEN_STUDIO}/tools/${TIZEN_COMPILER}/bin/clang++")

    ## With CLang compiler, it needs this set of flags
    #-target i386-tizen-linux-gnueabi
    #-gcc-toolchain /opt/tizen-studio/tools/i386-linux-gnueabi-gcc-4.9/
    #-ccc-gcc-name i386-linux-gnueabi-g++
    #-march=i386

    if (TIZEN_COMPILER MATCHES "llvm-3.4")
        set(GCC_TOOLCHAIN "gcc-4.6")
    elseif (TIZEN_COMPILER MATCHES "llvm-3.6")
        set(GCC_TOOLCHAIN "gcc-4.9")
    endif()

    set(TIZEN_FLAGS "${TIZEN_FLAGS} -target ${TIZEN_ARCHITECTURE}-tizen-linux-gnueabi")
    set(TIZEN_FLAGS "${TIZEN_FLAGS} -gcc-toolchain ${TIZEN_STUDIO}/tools/${TIZEN_ARCHITECTURE}-linux-gnueabi-${GCC_TOOLCHAIN}")
    set(TIZEN_FLAGS "${TIZEN_FLAGS} -ccc-gcc-name ${TIZEN_ARCHITECTURE}-linux-gnueabi-g++")

    if (TIZEN_TARGET MATCHES device)
        set(TIZEN_FLAGS "${TIZEN_FLAGS} -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -mtune=cortex-a8")
    elseif (TIZEN_TARGET MATCHES emulator)
        set(TIZEN_FLAGS "${TIZEN_FLAGS} -march=i386")
    endif()

    set(TIZEN_FLAGS "${TIZEN_FLAGS} --sysroot=${TIZEN_ROOTSTRAP}")

    message(STATUS ${TIZEN_FLAGS})

elseif(TIZEN_COMPILER MATCHES gcc-*)
    # /opt/tizen-studio/tools/arm-linux-gnueabi-gcc-4.6/bin/arm-linux-gnueabi-g++
    # /opt/tizen-studio/tools/i386-linux-gnueabi-gcc-4.9/bin/i386-linux-gnueabi-gcc

    set(CMAKE_C_COMPILER "${TIZEN_STUDIO}/tools/${TIZEN_ARCHITECTURE}-linux-gnueabi-${TIZEN_COMPILER}/bin/${TIZEN_ARCHITECTURE}-linux-gnueabi-gcc")
    set(CMAKE_CXX_COMPILER "${TIZEN_STUDIO}/tools/${TIZEN_ARCHITECTURE}-linux-gnueabi-${TIZEN_COMPILER}/bin/${TIZEN_ARCHITECTURE}-linux-gnueabi-g++")
    set(CMAKE_SYSROOT ${TIZEN_ROOTSTRAP})
endif()

# global flags for cmake client scripts to change behavior
set(TIZEN 1)

set(TEST_FLAGS "-I ${TIZEN_ROOTSTRAP}/usr/include -I ${TIZEN_ROOTSTRAP}/usr/include/elementary-1 -I ${TIZEN_ROOTSTRAP}/usr/include/efls-1")

#list(APPEND TIZEN_LIBRARIES "include")
list(APPEND TIZEN_LIBRARIES "appcore-agent")
list(APPEND TIZEN_LIBRARIES "appfw")
list(APPEND TIZEN_LIBRARIES "attach-panel")
list(APPEND TIZEN_LIBRARIES "badge ")
list(APPEND TIZEN_LIBRARIES "base")
list(APPEND TIZEN_LIBRARIES "cairo")
list(APPEND TIZEN_LIBRARIES "calendar-service2")
list(APPEND TIZEN_LIBRARIES "ckm")
list(APPEND TIZEN_LIBRARIES "contacts-svc")
list(APPEND TIZEN_LIBRARIES "content")
list(APPEND TIZEN_LIBRARIES "context-service")
list(APPEND TIZEN_LIBRARIES "dali")
list(APPEND TIZEN_LIBRARIES "dali-toolkit")
list(APPEND TIZEN_LIBRARIES "dbus-1.0")
list(APPEND TIZEN_LIBRARIES "dbus-1.0/include")
list(APPEND TIZEN_LIBRARIES "device")
list(APPEND TIZEN_LIBRARIES "dlog")
list(APPEND TIZEN_LIBRARIES "ecore-1")
list(APPEND TIZEN_LIBRARIES "ecore-buffer-1")
list(APPEND TIZEN_LIBRARIES "ecore-con-1")
list(APPEND TIZEN_LIBRARIES "ecore-evas-1")
list(APPEND TIZEN_LIBRARIES "ecore-file-1")
list(APPEND TIZEN_LIBRARIES "ecore-imf-1")
list(APPEND TIZEN_LIBRARIES "ecore-imf-evas-1")
list(APPEND TIZEN_LIBRARIES "ecore-input-1")
list(APPEND TIZEN_LIBRARIES "ecore-input-evas-1")
list(APPEND TIZEN_LIBRARIES "ecore-ipc-1")
list(APPEND TIZEN_LIBRARIES "ecore-x-1")
list(APPEND TIZEN_LIBRARIES "e_dbus-1")
list(APPEND TIZEN_LIBRARIES "edje-1")
list(APPEND TIZEN_LIBRARIES "eet-1")
list(APPEND TIZEN_LIBRARIES "efl-1")
list(APPEND TIZEN_LIBRARIES "efl-extension")
list(APPEND TIZEN_LIBRARIES "efreet-1")
list(APPEND TIZEN_LIBRARIES "eina-1")
list(APPEND TIZEN_LIBRARIES "eina-1/eina")
list(APPEND TIZEN_LIBRARIES "eio-1")
list(APPEND TIZEN_LIBRARIES "eldbus-1")
list(APPEND TIZEN_LIBRARIES "elementary-1")
list(APPEND TIZEN_LIBRARIES "embryo-1")
list(APPEND TIZEN_LIBRARIES "eo-1")
list(APPEND TIZEN_LIBRARIES "eom")
list(APPEND TIZEN_LIBRARIES "ethumb-1")
list(APPEND TIZEN_LIBRARIES "ethumb-client-1")
list(APPEND TIZEN_LIBRARIES "evas-1")
list(APPEND TIZEN_LIBRARIES "ewebkit2-0")
list(APPEND TIZEN_LIBRARIES "feedback")
list(APPEND TIZEN_LIBRARIES "fontconfig")
list(APPEND TIZEN_LIBRARIES "freetype2")
list(APPEND TIZEN_LIBRARIES "geofence")
list(APPEND TIZEN_LIBRARIES "gio-unix-2.0")
list(APPEND TIZEN_LIBRARIES "glib-2.0")
list(APPEND TIZEN_LIBRARIES "harfbuzz")
list(APPEND TIZEN_LIBRARIES "json-glib-1.0")
list(APPEND TIZEN_LIBRARIES "libxml2")
list(APPEND TIZEN_LIBRARIES "location")
list(APPEND TIZEN_LIBRARIES "maps")
list(APPEND TIZEN_LIBRARIES "media")
list(APPEND TIZEN_LIBRARIES "media-content")
list(APPEND TIZEN_LIBRARIES "messaging")
list(APPEND TIZEN_LIBRARIES "metadata-editor")
list(APPEND TIZEN_LIBRARIES "minicontrol")
list(APPEND TIZEN_LIBRARIES "minizip")
list(APPEND TIZEN_LIBRARIES "network")
list(APPEND TIZEN_LIBRARIES "notification")
list(APPEND TIZEN_LIBRARIES "phonenumber-utils")
list(APPEND TIZEN_LIBRARIES "sensor")
list(APPEND TIZEN_LIBRARIES "shortcut")
list(APPEND TIZEN_LIBRARIES "storage")
list(APPEND TIZEN_LIBRARIES "system")
list(APPEND TIZEN_LIBRARIES "telephony")
list(APPEND TIZEN_LIBRARIES "ui")
list(APPEND TIZEN_LIBRARIES "web")
list(APPEND TIZEN_LIBRARIES "widget_service")
list(APPEND TIZEN_LIBRARIES "widget_viewer_evas")
list(APPEND TIZEN_LIBRARIES "wifi-direct")
list(APPEND TIZEN_LIBRARIES "library")

foreach(LIBRARY ${TIZEN_LIBRARIES})
    set(TEST_FLAGS "${TEST_FLAGS} -I ${TIZEN_ROOTSTRAP}/usr/include/${LIBRARY}")
endforeach()

set(TEST_FLAGS "${TEST_FLAGS} -I ${TIZEN_ROOTSTRAP}/usr/lib/dbus-1.0/include")

# rpath makes low sense on Tizen
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "")
set(CMAKE_SKIP_RPATH TRUE CACHE BOOL "If set, runtime paths are not added when using shared libraries.")

# finalize flags
set(CMAKE_C_FLAGS      "${TIZEN_FLAGS} ${TEST_FLAGS}" CACHE STRING "c flags")
set(CMAKE_CXX_FLAGS    "${TIZEN_FLAGS} ${TEST_FLAGS} -Wall" CACHE STRING "c++ flags")
set(CMAKE_LINKER_FLAGS "${TIZEN_FLAGS} -lpthread" CACHE STRING "linker flags")

# search for headers and libraries in Tizen Studio environment only
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# append
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${TIZEN_ROOTSTRAP}/usr/lib")
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${TIZEN_ROOTSTRAP}/usr/include")

# macro to find packages on the host operating systsem
macro(find_host_package)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

    find_package(${ARGN})

    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endmacro()
