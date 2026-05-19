# Patch Arrow cmake modules for CMake 4.x / Apple toolchain compatibility.
# Invoked as PATCH_COMMAND in the arrow_cpp ExternalProject; CWD is the
# Arrow source root (e.g. build/_deps/rerun_cpp_sdk-build/arrow/src/arrow_cpp).

cmake_minimum_required(VERSION 3.20)

# Fix 1: BuildUtils.cmake — Apple's libtool version string changed from
# "cctools-XXXX" to "cctools_ld-XXXX" in recent Xcode / CLT releases,
# causing Arrow to mistake Apple libtool for the incompatible GNU libtool.
set(_f "cpp/cmake_modules/BuildUtils.cmake")
if(EXISTS "${_f}")
  file(READ "${_f}" _c)
  if(_c MATCHES [[cctools-\(\[0-9]])
    string(REPLACE
      [[if(NOT "${LIBTOOL_V_OUTPUT}" MATCHES ".*cctools-([0-9.]+).*")]]
      [[if(NOT "${LIBTOOL_V_OUTPUT}" MATCHES ".*cctools")]]
      _c "${_c}")
    file(WRITE "${_f}" "${_c}")
    message(STATUS "[arrow-patch] Fixed libtool detection in ${_f}")
  else()
    message(STATUS "[arrow-patch] ${_f} already patched or different version, skipping")
  endif()
endif()

# Fix 2: ThirdpartyToolchain.cmake — propagate CMAKE_POLICY_VERSION_MINIMUM
# into the nested mimalloc ExternalProject so it can configure under CMake 4.x
# (cmake_minimum_required VERSION < 3.5 became a hard error in CMake 4.0).
# This replicates the content of rerun_cpp_sdk's mimalloc_cmake4.patch without
# requiring git history.
set(_f "cpp/cmake_modules/ThirdpartyToolchain.cmake")
if(EXISTS "${_f}")
  file(READ "${_f}" _c)
  if(NOT _c MATCHES "MIMALLOC_POLICY_ARGS")
    string(REPLACE
      [[  set(MIMALLOC_CMAKE_ARGS
      ${EP_COMMON_CMAKE_ARGS}
      "-DCMAKE_INSTALL_PREFIX=${MIMALLOC_PREFIX}"
      -DMI_OVERRIDE=OFF
      -DMI_LOCAL_DYNAMIC_TLS=ON
      -DMI_BUILD_OBJECT=OFF
      -DMI_BUILD_SHARED=OFF
      -DMI_BUILD_TESTS=OFF)]]
      [[  if(CMAKE_POLICY_VERSION_MINIMUM)
    set(MIMALLOC_POLICY_ARGS
        -DCMAKE_POLICY_VERSION_MINIMUM=${CMAKE_POLICY_VERSION_MINIMUM})
  else()
    set(MIMALLOC_POLICY_ARGS "")
  endif()

  set(MIMALLOC_CMAKE_ARGS
      ${EP_COMMON_CMAKE_ARGS}
      "-DCMAKE_INSTALL_PREFIX=${MIMALLOC_PREFIX}"
      -DMI_OVERRIDE=OFF
      -DMI_LOCAL_DYNAMIC_TLS=ON
      -DMI_BUILD_OBJECT=OFF
      -DMI_BUILD_SHARED=OFF
      -DMI_BUILD_TESTS=OFF
      ${MIMALLOC_POLICY_ARGS}
  )]]
      _c "${_c}")
    file(WRITE "${_f}" "${_c}")
    message(STATUS "[arrow-patch] Fixed mimalloc policy propagation in ${_f}")
  else()
    message(STATUS "[arrow-patch] ${_f} already patched, skipping")
  endif()
endif()
