cmake_minimum_required(VERSION 3.20)
project(IvyRPG C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(LIB_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ASSETS_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(ASSETS_DEST $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR})

if(WIN32)
    set(RAYLIB_LIB   ${LIB_DIR}/raylib.dll)
    set(PLATFORM_LIBS winmm gdi32)
else()
    set(RAYLIB_LIB   ${LIB_DIR}/libraylib.so)
    set(PLATFORM_LIBS m pthread dl)
    set(CMAKE_BUILD_RPATH   "$ORIGIN/lib")
    set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")
endif()

macro(ivy_add_library TARGET_NAME)
    add_library(${TARGET_NAME} STATIC ${ARGN})
    target_include_directories(${TARGET_NAME} PUBLIC ${INCLUDE_DIR})
    target_link_libraries(${TARGET_NAME} PUBLIC ${RAYLIB_LIB})
endmacro()

ivy_add_library(ivy_tilemap
        src/tilemap/tilemap.c
        src/tilemap/tilemap_internal.c
        src/tilemap/autotile/border.c
        src/tilemap/autotile/carpet.c
        src/tilemap/autotile/table.c
        src/tilemap/autotile/wall.c
)

ivy_add_library(ivy_player
        src/player/player.c
        src/player/player_internal.c
        src/player/player_portrait.c
)
target_link_libraries(ivy_player PUBLIC ivy_tilemap)

ivy_add_library(ivy_scene
        src/scenes/scenes.c
        src/scenes/scene_title.c
        src/scenes/scene_gameplay.c
        src/scenes/scene_options.c
)
target_link_libraries(ivy_scene PUBLIC ivy_player)

add_executable(${PROJECT_NAME}
        src/main.c
        src/game.c
        src/virtual.c
        src/utils.c
        src/camera.c
        src/collision.c
        src/item.c
        src/inventory/inventory.c
        src/inventory/inventory_ui.c

        src/arena/linear.c
        src/arena/pool.c

        src/npc/npc.c
        src/npc/npc_internal.c

        src/locale.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE ivy_scene ${PLATFORM_LIBS})

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIB_DIR}/raylib.dll
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIB_DIR}/libraylib.so
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_SRC}
        ${ASSETS_DEST}
        COMMENT "Copying assets..."
)