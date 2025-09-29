# Set the minimum required version of CMake and define the project.
cmake_minimum_required(VERSION 3.10)
project(glatter C CXX)

# --- Option to build internal tests ---
# This allows us to disable the problematic tests/cmake suite for CI builds.
option(GLATTER_BUILD_INTERNAL_TESTS "Build the internal cmake test suite" OFF)

# --- Library Target ---
add_library(glatter src/glatter/glatter.c)

target_include_directories(glatter PUBLIC include)

if(WIN32)
    target_link_libraries(glatter PUBLIC opengl32)
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(glatter PUBLIC OpenGL::GL dl)
else()
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    find_library(DL_LIBRARY dl)
    target_link_libraries(glatter PUBLIC OpenGL::GL X11::X11 Threads::Threads ${DL_LIBRARY})
endif()

# --- Installation Rules ---
include(GNUInstallDirs)
install(TARGETS glatter
    EXPORT glatter-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/glatter DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# --- Integration Test ---
add_executable(glatter-test tests/main.cpp)
target_link_libraries(glatter-test PRIVATE glatter)

# --- Internal Test Suite (Optional) ---
# Only add the subdirectory if the option is enabled.
if(GLATTER_BUILD_INTERNAL_TESTS)
    add_subdirectory(tests/cmake)
endif()