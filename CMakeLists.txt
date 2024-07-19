cmake_minimum_required(VERSION 3.10)
project(cbrutekrag VERSION 0.6.0 LANGUAGES C)

add_subdirectory(src)

# Habilita testing
enable_testing()

# Añadir subdirectorio de tests
add_subdirectory(tests)
