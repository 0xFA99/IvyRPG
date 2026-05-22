cmake_minimum_required(VERSION 3.20)
project(IvyRPG
        VERSION 1.0.0
        DESCRIPTION "A top-down RPG game built with Raylib"
        LANGUAGES C
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ASSETS_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(RAYLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)

if(ipo_supported AND (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "LTO/IPO is supported and enabled for Release.")
else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
    message(STATUS "LTO disabled for Debug/Sanitizer compatibility.")
endif()

if(WIN32)
    set(RAYLIB_LIB_NAME "libraylib.a")
    if(MSVC)
        set(RAYLIB_LIB_NAME "raylib.lib")
    endif()
    set(RAYLIB_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows" CACHE PATH "Path to raylib library")
elseif(UNIX)
    set(RAYLIB_LIB_NAME "libraylib.a")
    set(RAYLIB_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux" CACHE PATH "Path to raylib library")
    set(PLATFORM_LIBS m pthread dl rt X11)
endif()

add_library(raylib STATIC IMPORTED GLOBAL)
set_target_properties(raylib PROPERTIES
        IMPORTED_LOCATION             "${RAYLIB_LIB_DIR}/${RAYLIB_LIB_NAME}"
        INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
)

if(WIN32)
    target_link_libraries(raylib INTERFACE winmm gdi32 opengl32 shell32 user32 setupapi)
elseif(UNIX)
    target_link_libraries(raylib INTERFACE ${PLATFORM_LIBS})
endif()

function(ivy_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W3)
    else()
        target_compile_options(${target} PRIVATE
                -Wall -Wextra -Wpedantic
                -Wno-unused-parameter
        )
    endif()
endfunction()

function(ivy_set_debug_options target)
    target_compile_definitions(${target} PRIVATE
            $<$<CONFIG:Debug>:IVY_DEBUG>
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(${target} PRIVATE
                $<$<CONFIG:Debug>:-g -O0>
                $<$<CONFIG:RelWithDebInfo>:-g -O2>
        )
        if(NOT MSVC)
            target_compile_options(${target} PRIVATE -fsanitize=address)
            target_link_options(${target} PRIVATE -fsanitize=address)
        endif()
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_options(${target} PRIVATE
                $<$<CONFIG:Release>:-O3>
                $<$<CONFIG:MinSizeRel>:-Os>
        )
    endif()
endfunction()

macro(ivy_module NAME)
    set(_SOURCES ${ARGN})
    add_library(ivy_${NAME} STATIC ${_SOURCES})
    target_include_directories(ivy_${NAME} PUBLIC ${INCLUDE_DIR})
    target_link_libraries(ivy_${NAME} PUBLIC raylib)

    if(NOT "${NAME}" STREQUAL "arena")
        target_link_libraries(ivy_${NAME} PUBLIC ivy_arena)
    endif()

    ivy_set_warnings(ivy_${NAME})
    ivy_set_debug_options(ivy_${NAME})
endmacro()

ivy_module(arena
        src/arena/linear.c
)

ivy_module(audio
        src/audio/buffer.c
        src/audio/stream.c
        src/audio/wav.c
        src/audio/ogg.c
)

ivy_module(core
        src/core/game.c
        src/core/virtual.c
        src/core/keybind.c
)

ivy_module(graphics
        src/graphics/tilemap.c
        src/graphics/camera.c
        src/graphics/gfx.c
        src/graphics/collusion.c
)

ivy_module(systems
        src/systems/scene_manager.c
        src/systems/asset_manager.c
        src/systems/locale_manager.c
        src/systems/item_manager.c
)

ivy_module(scenes
        src/scenes/title.c
        src/scenes/gameplay.c
        src/scenes/options/options.c
        src/scenes/options/options_data.c
        src/scenes/options/options_draw.c
        src/scenes/options/options_helper.c
        src/scenes/options/options_keybind.c
        src/scenes/options/options_update.c
)

ivy_module(entities
        src/entities/player.c
)

add_executable(${PROJECT_NAME} src/main.c)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
        -Wl,--start-group
        ivy_arena
        ivy_audio
        ivy_core
        ivy_graphics
        ivy_systems
        ivy_scenes
        ivy_entities
        -Wl,--end-group
        raylib
)

ivy_set_warnings(${PROJECT_NAME})
ivy_set_debug_options(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_DESKTOP)

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE $<CONFIG:Release>
    )
    # target_link_options(${PROJECT_NAME} PRIVATE -Wl,--defsym,stat64i32=stat)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ASSETS_DIR}/header.bin"
        "${ASSETS_DIR}/data.bin"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"

        COMMENT "Copying header.bin and data.bin..."
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES
        "${ASSETS_DIR}/header.bin"
        "${ASSETS_DIR}/data.bin"
        DESTINATION share/${PROJECT_NAME}/assets
        OPTIONAL
)

message(STATUS "IvyRPG v${PROJECT_VERSION}")
message(STATUS "  C Standard : C${CMAKE_C_STANDARD}")
message(STATUS "  Build Type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Raylib     : ${RAYLIB_LIB_DIR}")
message(STATUS "  Output     : ${CMAKE_BINARY_DIR}/bin")