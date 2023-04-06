project(screenshot_copier)

file(GLOB PUBLIC_HEADERS include/*)
file(GLOB SOURCES src/*)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${PUBLIC_HEADERS})

find_package(Qt6 REQUIRED COMPONENTS Gui)
target_link_libraries(${PROJECT_NAME} SideAssistQtClient Qt6::Gui)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        find_program(TOOL_WINDEPLOYQT NAMES windeployqt.debug.bat)
    else()
        find_program(TOOL_WINDEPLOYQT NAMES windeployqt)
    endif()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${TOOL_WINDEPLOYQT}
                $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Running ${TOOL_WINDEPLOYQT}..."
    )
endif()
