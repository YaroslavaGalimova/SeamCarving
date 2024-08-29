set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
project(SeamCarver)

add_library(${PROJECT_NAME} include/${PROJECT_NAME}.hpp src/${PROJECT_NAME}.cpp
                            include/Image.hpp           src/Image.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include)

add_library(big1::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(runUnitTests tests/test.cpp)
target_link_libraries(runUnitTests PRIVATE GTest::GTest big1::${PROJECT_NAME})
gtest_discover_tests(runUnitTests)

if(COMPILE_OPTS)
    target_compile_options(${PROJECT_NAME} PUBLIC ${COMPILE_OPTS})
    target_link_options(${PROJECT_NAME} PUBLIC ${LINK_OPTS})

    target_compile_options(runUnitTests PUBLIC ${COMPILE_OPTS})
    target_link_options(runUnitTests PUBLIC ${LINK_OPTS})
endif()
