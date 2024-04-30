
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

add_executable(${PROJECT_NAME}
  "src/main.c"
  "src/memory.c"
  "src/value.c"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
