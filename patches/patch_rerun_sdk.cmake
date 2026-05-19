# Patches rerun_cpp_sdk's download_and_build_arrow.cmake to replace
#   git apply --check ... && git apply ... || true
# with
#   cmake -P patch_arrow_cmake4.cmake
#
# Arrow is downloaded as a tar.gz (no git history), so "git apply" always
# fails silently (the "|| true" hides it) and the patches are never applied.
# cmake -P works regardless of VCS history.
#
# PATCHES_DIR  — absolute path to this project's patches/ directory.
#                Passed as -DPATCHES_DIR=... by the caller.

cmake_minimum_required(VERSION 3.20)

if(NOT PATCHES_DIR)
  message(FATAL_ERROR "[rerun-patch] PATCHES_DIR must be set (-DPATCHES_DIR=<abs-path>)")
endif()

set(_f "download_and_build_arrow.cmake")
if(NOT EXISTS "${_f}")
  message(FATAL_ERROR "[rerun-patch] Cannot find ${_f}; CWD must be the rerun_cpp_sdk source root")
endif()

file(READ "${_f}" _c)

# Idempotency guard
if(_c MATCHES "patch_arrow_cmake4")
  message(STATUS "[rerun-patch] ${_f} already patched, skipping")
  return()
endif()

# Replace the silent-failing git apply with a cmake -P invocation.
# \${CMAKE_COMMAND} writes the literal cmake variable reference so that
# rerun_cpp_sdk's own cmake expands it to the cmake executable path at
# Arrow ExternalProject configuration time.
string(REPLACE
  [[git apply --check ${MIMALLOC_PATCH} && git apply ${MIMALLOC_PATCH} || true]]
  "\${CMAKE_COMMAND} -P \"${PATCHES_DIR}/patch_arrow_cmake4.cmake\""
  _c "${_c}")

file(WRITE "${_f}" "${_c}")
message(STATUS "[rerun-patch] Replaced git apply with cmake -P in ${_f}")
