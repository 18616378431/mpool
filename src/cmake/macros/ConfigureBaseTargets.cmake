# An interface library to make the target com available to other targets
add_library(mpool-compile-option-interface INTERFACE)

# Use -std=c++11 instead of -std=gnu++11
set(CXX_EXTENSIONS OFF)

# Enable C++20 support
set(CMAKE_CXX_STANDARD 20)
message(STATUS "Enabled С++20 standard")

# An interface library to make the warnings level available to other targets
# This interface taget is set-up through the platform specific script
add_library(mpool-warning-interface INTERFACE)

# An interface used for all other interfaces
add_library(mpool-default-interface INTERFACE)

target_link_libraries(mpool-default-interface
  INTERFACE
    mpool-compile-option-interface)

# An interface used for silencing all warnings
add_library(mpool-no-warning-interface INTERFACE)

if (MSVC)
  target_compile_options(mpool-no-warning-interface
    INTERFACE
      /W0)
else()
  target_compile_options(mpool-no-warning-interface
    INTERFACE
      -w)
endif()

# An interface library to change the default behaviour
# to hide symbols automatically.
add_library(mpool-hidden-symbols-interface INTERFACE)

# An interface amalgamation which provides the flags and definitions
# used by the dependency targets.
add_library(mpool-dependency-interface INTERFACE)
target_link_libraries(mpool-dependency-interface
  INTERFACE
    mpool-default-interface
    mpool-no-warning-interface
    mpool-hidden-symbols-interface)

# An interface amalgamation which provides the flags and definitions
# used by the core targets.
add_library(mpool-interface INTERFACE)
target_link_libraries(mpool-interface
  INTERFACE
    mpool-default-interface
    mpool-warning-interface)
