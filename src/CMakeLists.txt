include_directories(../include)
add_compile_options(-Wall -Wextra)
add_executable(pngEditor chunk.cpp pngfile.cpp controller.cpp model.cpp main.cpp engine.cpp utf4win.cpp ztools.cpp)
