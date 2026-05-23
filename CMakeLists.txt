cmake_minimum_required(VERSION 3.20)
project(IvyRPG
        VERSION 1.0.0
        DESCRIPTION "A top-down RPG game built with Raylib"
        LANGUAGES C
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ASSETS_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/assets)

if(WIN32)
    set(RAYLIB_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows")
    set(RAYLIB_LIB_NAME "libraylib.a")
    set(PLATFORM_LIBS winmm gdi32 opengl32 shell32 user32 setupapi)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(RAYLIB_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux")
    set(RAYLIB_LIB_NAME "libraylib.a")
    set(PLATFORM_LIBS m pthread dl rt X11)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

add_library(raylib STATIC IMPORTED GLOBAL)
set_target_properties(raylib PROPERTIES
        IMPORTED_LOCATION             "${RAYLIB_LIB_DIR}/${RAYLIB_LIB_NAME}"
        INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
)
target_link_libraries(raylib INTERFACE ${PLATFORM_LIBS})

function(ivy_set_warnings target)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target} PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wno-unused-parameter
                -Wshadow
        )
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
        target_compile_options(${target} PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wno-unused-parameter
        )
    endif()
endfunction()

function(ivy_set_build_options target)
    target_compile_definitions(${target} PRIVATE
            $<$<CONFIG:Debug>:IVY_DEBUG>
            $<$<CONFIG:RelWithDebInfo>:IVY_DEBUG>
    )

    target_compile_options(${target} PRIVATE
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:RelWithDebInfo>:-O2>
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:MinSizeRel>:-Os>
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        if(WIN32 AND CMAKE_C_COMPILER_ID MATCHES "Clang")
            target_compile_options(${target} PRIVATE
                    -g
                    -gdwarf-4
                    -gpubnames
            )
            target_link_options(${target} PRIVATE
                    -Wl,/debug
                    -Wl,/pdb:$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.pdb
            )
            message(STATUS "Debug info: DWARF4 + PDB for ${target}")
        else()
            target_compile_options(${target} PRIVATE
                    -g
                    -gdwarf-4
            )
        endif()
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${target} PRIVATE
                -fno-omit-frame-pointer
                -fno-optimize-sibling-calls
                -fno-inline
                -fno-inline-functions
        )
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_options(${target} PRIVATE
                -fsanitize=address
                -fsanitize=undefined
        )
        target_link_options(${target} PRIVATE
                -fsanitize=address
                -fsanitize=undefined
        )
        message(STATUS "ASan+UBSan enabled for ${target}")
    endif()
endfunction()

function(ivy_module NAME)
    add_library(ivy_${NAME} STATIC ${ARGN})
    target_include_directories(ivy_${NAME} PUBLIC ${INCLUDE_DIR})
    target_link_libraries(ivy_${NAME} PUBLIC raylib)

    if(NOT "${NAME}" STREQUAL "arena")
        target_link_libraries(ivy_${NAME} PUBLIC ivy_arena)
    endif()

    ivy_set_warnings(ivy_${NAME})
    ivy_set_build_options(ivy_${NAME})
endfunction()

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
        src/systems/profile_manager.c
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
        ivy_arena
        ivy_audio
        ivy_core
        ivy_graphics
        ivy_systems
        ivy_scenes
        ivy_entities
        raylib
)

target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_DESKTOP)

ivy_set_warnings(${PROJECT_NAME})
ivy_set_build_options(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE OFF
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ASSETS_DIR}/header.bin"
        "${ASSETS_DIR}/data.bin"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Copying assets..."
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES
        "${ASSETS_DIR}/header.bin"
        "${ASSETS_DIR}/data.bin"
        DESTINATION share/${PROJECT_NAME}/assets
        OPTIONAL
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_target(debug_info
            COMMAND ${CMAKE_COMMAND} -E echo "Debug Build Information"
            COMMAND ${CMAKE_COMMAND} -E echo "Compiler: ${CMAKE_C_COMPILER}"
            COMMAND ${CMAKE_COMMAND} -E echo "Build type: ${CMAKE_BUILD_TYPE}"
            COMMAND ${CMAKE_COMMAND} -E echo "C flags: ${CMAKE_C_FLAGS_DEBUG}"
            COMMAND ${CMAKE_COMMAND} -E echo "Linker flags: ${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
            VERBATIM
    )
endif()

message(STATUS "")
message(STATUS "IvyRPG v${PROJECT_VERSION}")
message(STATUS "  Compiler    : ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  C Standard  : C${CMAKE_C_STANDARD}")
message(STATUS "  Build Type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Raylib      : ${RAYLIB_LIB_DIR}/${RAYLIB_LIB_NAME}")
message(STATUS "  Output      : ${CMAKE_BINARY_DIR}/bin")
message(STATUS "")