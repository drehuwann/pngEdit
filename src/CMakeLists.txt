include_directories(../include)
IF(WIN32)
    add_compile_options(-W3)
ELSEIF(UNIX)
    add_compile_options(-Wall -Wextra)
ELSE()
    # TODO build elsewhere...
ENDIF()
add_executable(pngEditor crc32_c.c chunk.cpp pngfile.cpp controller.cpp model.cpp view.cpp main.cpp engine.cpp utf4win.cpp)
