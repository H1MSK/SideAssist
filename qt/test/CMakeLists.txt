file(GLOB SRCS "./*.cpp")

add_executable(
  Test_SideAssistQtClient
  ${SRCS}
)
target_link_libraries(
  Test_SideAssistQtClient
  SideAssistQtClient
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(Test_SideAssistQtClient)

include(FetchContent)
message("Fetching googletest...")
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG v1.13.0
    GIT_SHALLOW 1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
message("googletest fetched.")
