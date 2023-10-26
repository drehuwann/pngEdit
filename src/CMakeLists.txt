include_directories(../include)
IF(WIN32)
    add_compile_options(-W3)
ELSEIF(UNIX)
    add_compile_options(-Wall -Wextra)
ELSE()
    # TODO build elsewhere...
ENDIF()
add_executable(pngEditor chunk.cpp pngfile.cpp controller.cpp model.cpp main.cpp engine.cpp utf4win.cpp ztools.cpp)
