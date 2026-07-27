cmake_minimum_required(VERSION 3.16)
project(glatter LANGUAGES C CXX)

# --- Project Standards ---
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Library Target ---
add_library(glatter src/glatter/glatter.c)

target_include_directories(glatter PUBLIC include)

if(WIN32)
    target_link_libraries(glatter PUBLIC opengl32)
elseif(APPLE)
    # On Apple, we explicitly find and link X11, just like on Linux,
    # because our CI installs it via XQuartz.
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    find_library(DL_LIBRARY dl)
    
    # Explicitly find the X11-compatible GL library from XQuartz.
    find_library(X11_GL_LIBRARY GL NAMES GL.1 HINTS /opt/X11/lib)

    target_link_libraries(glatter PUBLIC
        OpenGL::GL
        X11::X11
        Threads::Threads
        ${DL_LIBRARY}
        ${X11_GL_LIBRARY}
    )
else()
    # Linux/BSD
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    find_library(DL_LIBRARY dl)
    target_link_libraries(glatter PUBLIC
        OpenGL::GL
        X11::X11
        Threads::Threads
        ${DL_LIBRARY}
    )
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
install(DIRECTORY include/KHR include/EGL include/GLES include/GLES2 include/GLES3
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# --- Simple Integration Test ---
add_executable(glatter-test tests/main.cpp)
target_link_libraries(glatter-test PRIVATE glatter)
