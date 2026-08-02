cmake_minimum_required(VERSION 3.16)
project(glatter VERSION 1.0.1 LANGUAGES C CXX)

include(GNUInstallDirs)

if(NOT DEFINED PROJECT_IS_TOP_LEVEL)
    if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        set(PROJECT_IS_TOP_LEVEL ON)
    else()
        set(PROJECT_IS_TOP_LEVEL OFF)
    endif()
endif()

option(GLATTER_BUILD_TESTING "Build the glatter tests" ${PROJECT_IS_TOP_LEVEL})

# --- Project Standards ---
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Library Target ---
add_library(glatter STATIC src/glatter/glatter.c)
add_library(glatter::glatter ALIAS glatter)

target_include_directories(glatter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

find_package(OpenGL REQUIRED)
target_link_libraries(glatter PUBLIC OpenGL::GL)

if(APPLE)
    # On Apple, we explicitly find and link X11, just like on Linux,
    # because our CI installs it via XQuartz.
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)

    # Explicitly find the X11-compatible GL library from XQuartz.
    find_library(GLATTER_X11_GL_LIBRARY GL NAMES GL.1 HINTS /opt/X11/lib)
    if(NOT GLATTER_X11_GL_LIBRARY)
        message(FATAL_ERROR "The X11-compatible GL library was not found")
    endif()

    add_library(glatter::x11_gl INTERFACE IMPORTED GLOBAL)
    set_property(TARGET glatter::x11_gl PROPERTY
        INTERFACE_LINK_LIBRARIES "${GLATTER_X11_GL_LIBRARY}")

    target_link_libraries(glatter PUBLIC
        X11::X11
        Threads::Threads
        ${CMAKE_DL_LIBS}
        glatter::x11_gl
    )
elseif(NOT WIN32)
    # Linux/BSD
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(glatter PUBLIC
        X11::X11
        Threads::Threads
        ${CMAKE_DL_LIBS}
    )
endif()

# --- Installation Rules ---
install(TARGETS glatter
    EXPORT glatter-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)

set(GLATTER_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/glatter")

configure_package_config_file(
    cmake/glatter-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/glatter-config.cmake"
    INSTALL_DESTINATION "${GLATTER_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/glatter-config-version.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(EXPORT glatter-targets
    FILE glatter-targets.cmake
    NAMESPACE glatter::
    DESTINATION "${GLATTER_INSTALL_CMAKEDIR}"
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/glatter-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/glatter-config-version.cmake"
    DESTINATION "${GLATTER_INSTALL_CMAKEDIR}"
)

# --- Simple Integration Test ---
if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif()

if(GLATTER_BUILD_TESTING AND BUILD_TESTING)
    add_executable(glatter-test tests/main.cpp)
    target_link_libraries(glatter-test PRIVATE glatter)
endif()
