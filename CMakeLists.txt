cmake_minimum_required(VERSION 3.20)
project(testProject C)

# --- Options ---------------------------------------------------------------
option(BUILD_WASM_HTML "Emit an HTML shell for the WASM build (custom template if present)" ON)
option(USE_SDL3 "Use SDL3 for native desktop/mobile rendering" ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

# Dev server settings (override with -DSERVE_PORT=5173, etc.)
set(SERVE_PORT "8000" CACHE STRING "Dev server port")
set(SERVE_HOST "0.0.0.0" CACHE STRING "Dev server bind address")

# --- Platform Detection ----------------------------------------------------
set(PLATFORM_WEB OFF)
set(PLATFORM_DESKTOP OFF)
set(PLATFORM_MOBILE OFF)
set(PLATFORM_ANDROID OFF)
set(PLATFORM_IOS OFF)

if(EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(PLATFORM_WEB ON)
    message(STATUS "Platform: Web (Emscripten/WASM)")
elseif(ANDROID)
    set(PLATFORM_MOBILE ON)
    set(PLATFORM_ANDROID ON)
    message(STATUS "Platform: Mobile (Android)")
elseif(IOS)
    set(PLATFORM_MOBILE ON)
    set(PLATFORM_IOS ON)
    message(STATUS "Platform: Mobile (iOS)")
else()
    set(PLATFORM_DESKTOP ON)
    message(STATUS "Platform: Desktop (${CMAKE_SYSTEM_NAME})")
endif()

# --- Sources ---------------------------------------------------------------
# Start with *all* .c files from src/
file(GLOB SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

# Platform-specific renderer selection
if(PLATFORM_WEB)
    # WebGL2 renderer for Emscripten
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/render.c")
    # Remove render_native.c if it was included
    list(FILTER SOURCES EXCLUDE REGEX ".*render_native\\.c$")
elseif(PLATFORM_DESKTOP OR PLATFORM_MOBILE)
    # SDL3 + OpenGL ES renderer for native platforms
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/render_native.c")
    # Remove render.c if it was included
    list(FILTER SOURCES EXCLUDE REGEX ".*render\\.c$")
endif()

# --- Target Configuration --------------------------------------------------
if(PLATFORM_ANDROID)
    # Android requires a shared library
    add_library(testProject SHARED ${SOURCES})
elseif(PLATFORM_IOS)
    # iOS app bundle
    add_executable(testProject MACOSX_BUNDLE ${SOURCES})
else()
    # Desktop and Web: standard executable
    add_executable(testProject ${SOURCES})
endif()

target_include_directories(testProject PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/testProject
)

# --- Web (Emscripten) Configuration ----------------------------------------
if(PLATFORM_WEB)
    # Linker flags and exported functions/runtime
    target_link_options(testProject PRIVATE
        "SHELL:-sUSE_WEBGL2=1"
        "SHELL:-sMIN_WEBGL_VERSION=2"
        "SHELL:-sMAX_WEBGL_VERSION=2"
        "SHELL:-sALLOW_MEMORY_GROWTH=1"
        "SHELL:-sEXPORTED_FUNCTIONS=['_main','_initWebGL','_startMainLoop','_myFunction']"
        "SHELL:-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
    )

    # Emit index.html / index.js / index.wasm
    set_target_properties(testProject PROPERTIES
        SUFFIX ".html"
        OUTPUT_NAME "index"
    )

    # Optional custom HTML shell / post-js
    if(BUILD_WASM_HTML)
        if(EXISTS "${CMAKE_SOURCE_DIR}/html_template/index.html")
            target_link_options(testProject PRIVATE
                "SHELL:--shell-file=${CMAKE_SOURCE_DIR}/html_template/index.html"
            )
        endif()
        if(EXISTS "${CMAKE_SOURCE_DIR}/html_template/init_runtime.js")
            target_link_options(testProject PRIVATE
                "SHELL:--post-js=${CMAKE_SOURCE_DIR}/html_template/init_runtime.js"
            )
        endif()
    endif()

    # Development server target
    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_custom_target(serve
            COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan
                    "Serving ${CMAKE_CURRENT_BINARY_DIR} at http://${SERVE_HOST}:${SERVE_PORT}"
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}
                    ${Python3_EXECUTABLE} -u -m http.server ${SERVE_PORT} --bind ${SERVE_HOST}
            DEPENDS testProject
            USES_TERMINAL
            VERBATIM
        )
    endif()

# --- Desktop Configuration -------------------------------------------------
elseif(PLATFORM_DESKTOP)
    # Link math library
    target_link_libraries(testProject PRIVATE m)
    
    if(USE_SDL3)
        # Find SDL3
        find_package(SDL3 QUIET)
        
        if(SDL3_FOUND)
            message(STATUS "Found SDL3: ${SDL3_VERSION}")
            target_link_libraries(testProject PRIVATE SDL3::SDL3)
            target_compile_definitions(testProject PRIVATE USE_SDL3)
            
            # Link OpenGL
            if(WIN32)
                # Windows: OpenGL is part of the system
                target_link_libraries(testProject PRIVATE opengl32)
            elseif(APPLE)
                # macOS: Use framework
                find_library(OPENGL_LIBRARY OpenGL REQUIRED)
                target_link_libraries(testProject PRIVATE ${OPENGL_LIBRARY})
            else()
                # Linux: Find OpenGL
                find_package(OpenGL REQUIRED)
                target_link_libraries(testProject PRIVATE OpenGL::GL)
            endif()
            
            message(STATUS "Desktop build configured with SDL3 + OpenGL")
        else()
            message(WARNING "SDL3 not found. Building without renderer.")
            message(STATUS "Install SDL3 or disable with -DUSE_SDL3=OFF")
        endif()
    else()
        message(STATUS "Desktop build configured without renderer (USE_SDL3=OFF)")
    endif()

# --- Android Configuration -------------------------------------------------
elseif(PLATFORM_ANDROID)
    # Link math library
    target_link_libraries(testProject PRIVATE m log)
    
    if(USE_SDL3)
        # Find SDL3 for Android
        find_package(SDL3 REQUIRED)
        target_link_libraries(testProject PRIVATE SDL3::SDL3)
        target_compile_definitions(testProject PRIVATE USE_SDL3 PLATFORM_ANDROID)
        
        # Link Android-specific libraries
        target_link_libraries(testProject PRIVATE
            android
            EGL
            GLESv3
        )
        
        # Set Android-specific properties
        set_target_properties(testProject PROPERTIES
            LIBRARY_OUTPUT_NAME "main"
        )
        
        message(STATUS "Android build configured with SDL3 + OpenGL ES 3.0")
    else()
        message(FATAL_ERROR "Android builds require SDL3 (USE_SDL3=ON)")
    endif()

# --- iOS Configuration -----------------------------------------------------
elseif(PLATFORM_IOS)
    # Link math library
    target_link_libraries(testProject PRIVATE m)
    
    if(USE_SDL3)
        # Find SDL3 for iOS
        find_package(SDL3 REQUIRED)
        target_link_libraries(testProject PRIVATE SDL3::SDL3)
        target_compile_definitions(testProject PRIVATE USE_SDL3 PLATFORM_IOS)
        
        # Link iOS frameworks
        find_library(OPENGLES_LIBRARY OpenGLES REQUIRED)
        find_library(FOUNDATION_LIBRARY Foundation REQUIRED)
        find_library(UIKIT_LIBRARY UIKit REQUIRED)
        find_library(COREGRAPHICS_LIBRARY CoreGraphics REQUIRED)
        find_library(QUARTZCORE_LIBRARY QuartzCore REQUIRED)
        find_library(METAL_LIBRARY Metal REQUIRED)
        
        target_link_libraries(testProject PRIVATE
            ${OPENGLES_LIBRARY}
            ${FOUNDATION_LIBRARY}
            ${UIKIT_LIBRARY}
            ${COREGRAPHICS_LIBRARY}
            ${QUARTZCORE_LIBRARY}
            ${METAL_LIBRARY}
        )
        
        # iOS app bundle properties
        set_target_properties(testProject PROPERTIES
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.testproject"
            MACOSX_BUNDLE_BUNDLE_NAME "TestProject"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.yourcompany.testproject"
            XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"  # iPhone and iPad
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0"
        )
        
        message(STATUS "iOS build configured with SDL3 + OpenGL ES 3.0")
    else()
        message(FATAL_ERROR "iOS builds require SDL3 (USE_SDL3=ON)")
    endif()
endif()

# --- Compiler Warnings -----------------------------------------------------
if(MSVC)
    target_compile_options(testProject PRIVATE /W4)
else()
    target_compile_options(testProject PRIVATE -Wall -Wextra -Wpedantic)
endif()

# --- Build Summary ---------------------------------------------------------
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
if(PLATFORM_WEB)
    message(STATUS "Renderer: WebGL2 (Emscripten)")
elseif(USE_SDL3 AND (SDL3_FOUND OR PLATFORM_MOBILE))
    if(PLATFORM_DESKTOP)
        message(STATUS "Renderer: SDL3 + OpenGL 3.3")
    elseif(PLATFORM_ANDROID OR PLATFORM_IOS)
        message(STATUS "Renderer: SDL3 + OpenGL ES 3.0")
    endif()
else()
    message(STATUS "Renderer: None")
endif()
message(STATUS "==================================")