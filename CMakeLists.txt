cmake_minimum_required(VERSION 2.6)

project(e)

# Extend the CMake module search path.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

# Flags are credit to Abhishek "G" Mukherjee
set(e_wanal "")
set(e_wanal "${e_wanal} -pedantic")
set(e_wanal "${e_wanal} -std=c++98")
set(e_wanal "${e_wanal} -Wabi")
set(e_wanal "${e_wanal} -Wall")
set(e_wanal "${e_wanal} -Wcast-align")
set(e_wanal "${e_wanal} -Wcast-qual")
set(e_wanal "${e_wanal} -Wctor-dtor-privacy")
set(e_wanal "${e_wanal} -Weffc++")
set(e_wanal "${e_wanal} -Wextra")
set(e_wanal "${e_wanal} -Wfloat-equal")
set(e_wanal "${e_wanal} -Wformat=2")
set(e_wanal "${e_wanal} -Winit-self")
set(e_wanal "${e_wanal} -Winline")
set(e_wanal "${e_wanal} -Woverloaded-virtual")
set(e_wanal "${e_wanal} -Wshadow")
set(e_wanal "${e_wanal} -Wsign-promo")
set(e_wanal "${e_wanal} -Wstrict-overflow=5")
set(e_wanal "${e_wanal} -Wswitch-default")
set(e_wanal "${e_wanal} -Wswitch-enum")
set(e_wanal "${e_wanal} -Wunsafe-loop-optimizations")
set(e_wanal "${e_wanal} -Wwrite-strings")

set(CMAKE_CXX_FLAGS "${e_wanal} ${CMAKE_CXX_FLAGS}")

# Set the test sources
set(e_test_srcs
    test/buffer.cc
    test/convert.cc
    test/guard.cc
    test/intrusive_ptr.cc
)

# Installed headers
set(e_headers
    include/e/buffer.h
    include/e/convert.h
    include/e/guard.h
    include/e/intrusive_ptr.h
    include/e/set.h
    include/e/timer.h
    include/e/tuple_compare.h
)

# Setup the include paths
include_directories(include)

# Tests
ENABLE_TESTING()

add_executable(testrunner test/runner.cc ${e_test_srcs})
target_link_libraries(testrunner
    pthread
    gtest
)

add_test(ConvertTest.Uint64NormalCases testrunner
    --gtest_filter=ConvertTest.Uint64NormalCases)
add_test(ConvertTest.Uint32NormalCases testrunner
    --gtest_filter=ConvertTest.Uint32NormalCases)
add_test(ConvertTest.Uint16NormalCases testrunner
    --gtest_filter=ConvertTest.Uint16NormalCases)
add_test(ConvertTest.Uint8NormalCases testrunner
    --gtest_filter=ConvertTest.Uint8NormalCases)

add_test(BufferTest.CtorAndDtor testrunner
    --gtest_filter=BufferTest.CtorAndDtor)
add_test(BufferTest.PackBuffer testrunner
    --gtest_filter=BufferTest.PackBuffer)
add_test(BufferTest.UnpackBuffer testrunner
    --gtest_filter=BufferTest.UnpackBuffer)
add_test(BufferTest.UnpackErrors testrunner
    --gtest_filter=BufferTest.UnpackErrors)
add_test(BufferTest.TrimPrefix testrunner
    --gtest_filter=BufferTest.TrimPrefix)
add_test(BufferTest.Index testrunner
    --gtest_filter=BufferTest.Index)
add_test(BufferTest.Contains testrunner
    --gtest_filter=BufferTest.Contains)

# Installation targets.
install(
    FILES ${e_headers}
    DESTINATION  include/e
    COMPONENT development
)
