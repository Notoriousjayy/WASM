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
  # 4.a) Emit an .html wrapper
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

  # 4.b) Your post-JS hook
  set(POST_JS_FILE "${CMAKE_SOURCE_DIR}/html_template/init_runtime.js")

  # 4.c) Linker flags: WebGL2 + minimal runtime + raw + ccall/cwrap + custom shell
  target_link_options(${PROJECT_NAME} PRIVATE
    "SHELL:-sUSE_WEBGL2=1"
    "SHELL:-sFULL_ES3=1"
    "SHELL:-sNO_EXIT_RUNTIME=1"

    # EXPORTED_RUNTIME_METHODS must come with NO space after -s
    "SHELL:-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"

    # Export your C entry points (with leading underscores)
    "SHELL:-sEXPORTED_FUNCTIONS=['_main','_initWebGL','_startMainLoop']"

    # Custom HTML shell + post-JS
    "SHELL:--shell-file=${CMAKE_SOURCE_DIR}/html_template/index.html"
    "SHELL:--post-js=${POST_JS_FILE}"
  )
endif()

# 5) (optional) install rules, testing, etc.
# install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
# enable_testing() && add_subdirectory(tests)
