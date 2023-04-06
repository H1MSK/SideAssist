project(SideAssistQtClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option( ${PROJECT_NAME}_SHARED "Build a shared library.  Turn off for static." ON )
option( ${PROJECT_NAME}_WEBSOCKETS "Enable WebSockets for MQTT" ON )
option( ${PROJECT_NAME}_SSL "Enable SSL support for MQTT" ON )

if ( ${PROJECT_NAME}_SHARED )
    set( library_build_type SHARED )
    set( library_install_component Library )
else()
    set( library_build_type STATIC )
    set( library_install_component Devel )
endif()


set( ws_component )
set( ws_libname )

if ( ${PROJECT_NAME}_WEBSOCKETS )
    set( ws_component WebSockets )
    set( ws_libname   Qt6::WebSockets )
endif()

if ( NOT ${PROJECT_NAME}_SSL)
    set( ssl_defs QT_NO_SSL )
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network ${ws_component} CONFIG REQUIRED )
set( CMAKE_AUTOMOC ON )
qt_standard_project_setup()
cmake_policy( SET CMP0020 NEW ) # Automatically link Qt executables to qtmain target on Windows.


cmake_policy(SET CMP0077 NEW) # Ignore "option" command in qmqtt with the same name
set(qmqtt_SHARED ${${PROJECT_NAME}_SHARED})
set(qmqtt_SSL ${${PROJECT_NAME}_SSL})
set(qmqtt_WEBSOCKETS ${${PROJECT_NAME}_WEBSOCKETS})
add_subdirectory(third_party/qmqtt)

file(GLOB PUBLIC_HEADERS include/*)
file(GLOB SOURCES src/*)

add_library(${PROJECT_NAME} ${library_build_type}
    ${SOURCES}
    ${PUBLIC_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC include/)
target_link_libraries(${PROJECT_NAME} qmqtt)
target_compile_definitions( ${PROJECT_NAME}
    PRIVATE
        QT_BUILD_SIDEASSIST_LIB
        ${ssl_defs}
)

set_source_files_properties( ${PUBLIC_HEADERS} PROPERTIES PUBLIC_HEADER 1 )

add_subdirectory(apps)
