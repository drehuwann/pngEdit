cmake_minimum_required(VERSION 3.22)
project (pngEditor VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

IF (WIN32)
    add_link_options(/SUBSYSTEM:WINDOWS)
    add_compile_definitions(_UNICODE UNICODE WINDOWS)
ELSEIF(UNIX)
    find_package(wxWidgets REQUIRED COMPONENTS core base)
    include(${wxWidgets_USE_FILE})
    add_compile_definitions(_UNICODE UNICODE POSIX)
ELSE()
    # TODO build elsewhere...
ENDIF()

add_subdirectory(src)

IF(WIN32)
    target_link_libraries(pngEditor ws2_32)
ELSEIF(UNIX)
    target_link_libraries(pngEditor ${wxWidgets_LIBRARIES})
ELSE()
    # TODO build elsewhere...
ENDIF()

configure_file(pngEditorConfig.h.in pngEditor.h)
