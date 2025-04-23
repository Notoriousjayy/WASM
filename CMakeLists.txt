cmake_minimum_required(VERSION 3.0)
project(testProject C CXX)

# C standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.c
    src/render.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

# If using Emscripten, enable WebGL2 and ES3
if(CMAKE_C_COMPILER_ID STREQUAL "Emscripten")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

    target_link_options(${PROJECT_NAME} PRIVATE
        "-s" "USE_WEBGL2=1"
        "-s" "FULL_ES3=1"
        "-s" "NO_EXIT_RUNTIME=1"
        "-s" "EXPORTED_RUNTIME_METHODS=['ccall']"
        "--shell-file=${CMAKE_SOURCE_DIR}/html_template/index.html"
    )
endif()
