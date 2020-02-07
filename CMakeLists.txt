cmake_minimum_required(VERSION 3.10)

project(Nancho VERSION 0.0.1)

set (CMAKE_CXX_STANDARD 17)

find_package(PkgConfig)

pkg_check_modules(DBUS dbus-1)
pkg_check_modules(PULSE libpulse)

include_directories(${DBUS_INCLUDE_DIRS})
link_directories(${DBUS_LIBRARY_DIRS})

include_directories(${PULSE_INCLUDE_DIRS})
link_directories(${PULSE_LIBRARY_DIRS})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


file(GLOB nancho_SRC
    "src/*.h"
    "src/*.cpp"
)

add_executable(nancho ${nancho_SRC})

target_link_libraries(nancho ${DBUS_LIBRARIES})
target_link_libraries(nancho ${PULSE_LIBRARIES})

install(TARGETS nancho
        RUNTIME DESTINATION /usr/bin/)