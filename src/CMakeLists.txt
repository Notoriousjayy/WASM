# src/CMakeLists.txt

# Build a library from your sources
add_library(${PROJECT_NAME}_lib
    module.c
    render.c
)

target_include_directories(${PROJECT_NAME}_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Native executable
add_executable(${PROJECT_NAME}_native main.c)
target_link_libraries(${PROJECT_NAME}_native PRIVATE ${PROJECT_NAME}_lib)

# WASM target (when using an Emscripten toolchain)
if (CMAKE_C_COMPILER_ID STREQUAL "Emscripten")
    add_executable(${PROJECT_NAME}_wasm
        main.c
        module.c
        render.c
    )
    set_target_properties(${PROJECT_NAME}_wasm PROPERTIES SUFFIX ".html")
    target_link_options(${PROJECT_NAME}_wasm PRIVATE
        -s USE_WEBGL2=1
        -s FULL_ES3=1
        -s NO_EXIT_RUNTIME=1
        -s EXPORTED_RUNTIME_METHODS=["ccall"]
        --shell-file=${CMAKE_SOURCE_DIR}/html/index.html
    )
endif()
