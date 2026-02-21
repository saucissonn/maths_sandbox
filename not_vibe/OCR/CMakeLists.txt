cmake_minimum_required(VERSION 3.27)

project(OCR)

file(GLOB SRC_FILES
    "src/main.c"
    "src/globals.c"
    "src/sdl_img.c"
    "src/data.c"
    "src/nn.c"
)

set(EXEC_NAME "n")

add_executable(${EXEC_NAME} ${SRC_FILES})

#target_include_directories(${EXEC_NAME} PUBLIC include src/header)

target_include_directories(${EXEC_NAME} PRIVATE
    src
)

if(UNIX)
    message(STATUS "linux detected")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

    target_include_directories(${EXEC_NAME} PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
    )

    target_link_libraries(${EXEC_NAME} PRIVATE
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        m
    )

endif()
