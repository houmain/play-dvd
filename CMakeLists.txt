cmake_minimum_required(VERSION 3.5)
project(play-dvd LANGUAGES CXX)

set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

find_package(Git)
if(NOT VERSION AND GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
configure_file("src/version.h.in" "${CMAKE_SOURCE_DIR}/src/_version.h")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DVDREAD REQUIRED dvdread)

link_libraries(${DVDREAD_LIBRARIES})
include_directories(${DVDREAD_INCLUDE_DIRS})
add_compile_options(${DVDREAD_CFLAGS_OTHER})

add_executable(play-dvd
  src/main.cpp
  src/dump.cpp
  src/dump.h
  src/Rating.cpp
  src/Rating.h
  src/Settings.cpp
  src/Settings.h
  src/Title.cpp
  src/Title.h
  src/mpv.cpp
  src/mpv.h)

install(TARGETS play-dvd DESTINATION "bin")
install(DIRECTORY extra/share DESTINATION .)

# package
set(CPACK_PACKAGE_NAME "play-dvd")
set(CPACK_PACKAGE_DESCRIPTION "Playback DVD with mpv")
set(CPACK_PACKAGE_VENDOR "Albert Kalchmair")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "play-dvd")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/houmain/play-dvd")
set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_HOMEPAGE_URL})
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/share/pixmaps/play-dvd.svg")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_DEBIAN_PACKAGE_SECTION utils)
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_PACKAGE_GROUP "SApplications/Multimedia")

if(VERSION MATCHES "^[0-9]+\.[0-9]+\.[0-9]+")
  string(REGEX MATCHALL "[0-9]+" VERSION_LIST "${VERSION}")
  list(GET VERSION_LIST 0 CPACK_PACKAGE_VERSION_MAJOR)
  list(GET VERSION_LIST 1 CPACK_PACKAGE_VERSION_MINOR)
  list(GET VERSION_LIST 2 CPACK_PACKAGE_VERSION_PATCH)
endif()

include(CPack)
