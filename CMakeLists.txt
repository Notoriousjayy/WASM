cmake_minimum_required(VERSION 3.15)
project(testProject VERSION 1.0 LANGUAGES C CXX)

# ——————————————————————————————————————————————————————————————
# 1) C standards & build type defaults
# ——————————————————————————————————————————————————————————————
set(CMAKE_C_STANDARD    99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD  17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build (Debug or Release)" FORCE)
endif()

# ——————————————————————————————————————————————————————————————
# 2) Source files
# ——————————————————————————————————————————————————————————————
set(SOURCES
    src/main.c
    src/render.c
    # src/module.c
)

# ——————————————————————————————————————————————————————————————
# 3) Executable target
# ——————————————————————————————————————————————————————————————
add_executable(${PROJECT_NAME} ${SOURCES})

# — Make both include/ and include/testProject/ available —
# so that:
#   #include "render.h"      → include/testProject/render.h
#   #include <testProject/module.h> → include/testProject/module.h
target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include/testProject
    ${CMAKE_SOURCE_DIR}/include
)

# ——————————————————————————————————————————————————————————————
# 4) Emscripten / WebAssembly support
# ——————————————————————————————————————————————————————————————
if(CMAKE_C_COMPILER_ID STREQUAL "Emscripten")
    # HTML wrapper, WebGL2, ES3, no-exit, JS interop:
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
      -s USE_WEBGL2=1
      -s FULL_ES3=1
      -s NO_EXIT_RUNTIME=1
      -s EXPORTED_RUNTIME_METHODS=["ccall"]
      --shell-file=${CMAKE_SOURCE_DIR}/html_template/index.html
    )
    # target_sources(${PROJECT_NAME} PRIVATE src/module.c)
endif()

# ——————————————————————————————————————————————————————————————
# 5) Installation (optional)
# ——————————————————————————————————————————————————————————————
# install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
# install(DIRECTORY include/ DESTINATION include)

# ——————————————————————————————————————————————————————————————
# 6) Testing support (optional)
# ——————————————————————————————————————————————————————————————
# enable_testing()
# add_subdirectory(tests)
