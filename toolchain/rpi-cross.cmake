set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_AR llvm-ar)
set(CMAKE_NM llvm-nm)
set(CMAKE_RANLIB llvm-ranlib)

set(TOOLCHAIN "${CMAKE_CURRENT_LIST_DIR}/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu")

set(CMAKE_SYSROOT "${TOOLCHAIN}/aarch64-none-linux-gnu/libc")
set(CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN "${TOOLCHAIN}")
set(CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN "${TOOLCHAIN}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_compile_options(
	--target=aarch64-unknown-linux-gnu
)

add_link_options(
	--target=aarch64-unknown-linux-gnu
	-fuse-ld=lld
)
