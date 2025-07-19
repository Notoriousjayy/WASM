cmake_minimum_required(VERSION 3.15)
project(testProject VERSION 1.0 LANGUAGES C CXX)

# 1) C standards & default build type
set(CMAKE_C_STANDARD    99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD  17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build (Debug or Release)" FORCE)
endif()

# 2) Your sources
set(SOURCES
  src/main.c
  src/render.c
)

# 3) Executable target
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/testProject
)

# 4) Emscripten / WebAssembly tweaks
if(CMAKE_C_COMPILER_ID STREQUAL "Emscripten")
  # 4.a) emit an .html wrapper
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

  # 4.b) point to our post-JS file (must exist at html_template/init_runtime.js)
  set(POST_JS_FILE "${CMAKE_SOURCE_DIR}/html_template/init_runtime.js")

  # 4.c) linker flags for WebGL2 + minimal runtime + JS interop + auto-init
  target_link_options(${PROJECT_NAME} PRIVATE
    "-s USE_WEBGL2=1"
    "-s FULL_ES3=1"
    "-s NO_EXIT_RUNTIME=1"

    # bring in ccall & cwrap helpers
    "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"

    # export your C functions so both raw and ccall work
    "-s EXPORTED_FUNCTIONS=['_initWebGL','_startMainLoop','_main']"

    # use your custom HTML shell
    "--shell-file=${CMAKE_SOURCE_DIR}/html_template/index.html"

    # append our runtime-init JS after the generated glue
    "--post-js=${POST_JS_FILE}"
  )
endif()

# 5) (optional) install rules, testing, etc.
# install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
# enable_testing() && add_subdirectory(tests)
