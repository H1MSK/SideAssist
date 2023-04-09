cmake_minimum_required(VERSION 3.14)

project(SideAssist)

option(BUILD_QT_CLIENT "Build qt client" ON)
option(BUILD_FLUTTER_DASHBOARD "Build flutter dashboard" ON)

if (BUILD_QT_CLIENT)
add_subdirectory(SideAssist.Client.Qt)
endif(BUILD_QT_CLIENT)

if (BUILD_FLUTTER_DASHBOARD)
add_subdirectory(SideAssist.Dashboard.Flutter)
endif(BUILD_FLUTTER_DASHBOARD)
