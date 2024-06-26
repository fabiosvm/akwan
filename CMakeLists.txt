
cmake_minimum_required(VERSION 3.15)

project(akwan
  VERSION 0.1.0
  DESCRIPTION "The Akwan Programming Language"
  LANGUAGES C)

set(CMAKE_C_STANDARD 11)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

if(NOT MSVC)
  add_compile_options("$<$<CONFIG:Debug>:-fsanitize=address>")
  add_link_options("$<$<CONFIG:Debug>:-fsanitize=address>")
endif()

add_executable(${PROJECT_NAME}
  "src/array.c"
  "src/buffer.c"
  "src/chunk.c"
  "src/compiler.c"
  "src/dump.c"
  "src/error.c"
  "src/lexer.c"
  "src/main.c"
  "src/memory.c"
  "src/range.c"
  "src/string.c"
  "src/value.c"
  "src/vm.c"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NOT MSVC)
  target_link_libraries(${PROJECT_NAME} m)
endif()
