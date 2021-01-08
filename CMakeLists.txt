# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14)

#-------------------------------------------------------------------------------
# PROJECT configuration
#-------------------------------------------------------------------------------
set(Library_NAME vCUDA CACHE STRING "The name of the main library.")
set(component_NAME device)

string(TOLOWER ${Library_NAME}   library_NAME)
string(TOUPPER ${Library_NAME}   LIBRARY_NAME)
string(TOUPPER ${component_NAME} COMPONENT_NAME)

project(${component_NAME}
  VERSION   0.0.1
  LANGUAGES CXX)

#-------------------------------------------------------------------------------
# COMPONENT configuration
#-------------------------------------------------------------------------------
configure_file(src/include/${library_NAME}/${component_NAME}/config.h.in
               src/include/${library_NAME}/${component_NAME}/config.h
  @ONLY)

add_library(${PROJECT_NAME})
add_library(${Library_NAME}::${component_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PRIVATE src/device/cleanup.cxx src/device/exit.cxx src/device/init.cxx
          src/device/launch.cxx src/device/memalloc.cxx src/device/memcpy.cxx
          src/device/memfree.cxx src/device/memset.cxx src/device/panic.cxx
          src/device/poweroff.cxx src/device/poweron.cxx src/device/read.cxx
          src/device/shm.cxx src/device/signal.cxx src/device/write.cxx
          # these are only included below so that they appear when using IDEs
          include/vcuda/device.h src/include/vcuda/device/config.h.in)

target_include_directories(${PROJECT_NAME}
 PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
         $<INSTALL_INTERFACE:include>
 PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>)

target_link_libraries(${PROJECT_NAME}
  PUBLIC OpenMP::OpenMP_CXX Threads::Threads $<$<BOOL:${LIBRT}>:${LIBRT}>)

#-------------------------------------------------------------------------------
# EXPORT configuration
#-------------------------------------------------------------------------------
include(GenerateExportHeader)

generate_export_header(${PROJECT_NAME}
  PREFIX_NAME      ${LIBRARY_NAME}_
  EXPORT_FILE_NAME include/${library_NAME}/${component_NAME}/export.h)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC $<IF:$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>,
          ${LIBRARY_NAME}_${COMPONENT_NAME}_STATIC_DEFINE,>)

#-------------------------------------------------------------------------------
# EXTERNAL DEPENDENCY configuration
#-------------------------------------------------------------------------------
find_package(OpenMP REQUIRED)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if(UNIX AND NOT APPLE)
  find_library(LIBRT rt REQUIRED)
endif()

#-------------------------------------------------------------------------------
# INTERNAL DEPENDENCY configuration
#-------------------------------------------------------------------------------
include(FetchContent)

set(component_DEPS core)

foreach(dep ${component_DEPS})
  # Check if results have been cached. If not, then first look for a installed
  # package.
  if(NOT (${dep}_FOUND OR ${library_NAME}-${dep}_POPULATED))
    message("-- Looking for ${Library_NAME}::${dep}")

    find_package(${dep} CONFIG QUIET
      PATHS ${CMAKE_INSTALL_PREFIX}/lib/cmake/${library_NAME})

    set(${dep}_FOUND ${${dep}_FOUND} CACHE INTERNAL
      "Indicator variable for package ${dep}")

    if(${dep}_FOUND)
      message("-- Looking for ${Library_NAME}::${dep} - found")
    else()
      message("-- Looking for ${Library_NAME}::${dep} - installing from source")
    endif()
  endif()

  # If not package was found, then install from source
  if(NOT ${dep}_FOUND)
    FetchContent_Declare(${library_NAME}-${dep}
      GIT_REPOSITORY https://github.com/jiverson002/${library_NAME}-${dep}.git
      GIT_TAG        dev)

    FetchContent_MakeAvailable(${library_NAME}-${dep})

    set(${library_NAME}-${dep}_POPULATED ${${library_NAME}-${dep}_POPULATED}
      CACHE INTERNAL
      "Indicator variable for source installation of ${dep}")
  endif()

  # Check if any installation was successful
  if(NOT (${dep}_FOUND OR ${library_NAME}-${dep}_POPULATED))
    message(FATAL_ERROR "${Library_NAME}::${dep} not available")
  else()
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Library_NAME}::${dep})
  endif()
endforeach()

#-------------------------------------------------------------------------------
# TEST configuration
#-------------------------------------------------------------------------------
#if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
#  include(CTest)
#  if(BUILD_TESTING)
#    add_subdirectory(test)
#  endif()
#endif()

#-------------------------------------------------------------------------------
# INSTALL configuration
#-------------------------------------------------------------------------------
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

configure_package_config_file(${component_NAME}-config.cmake.in
                      cmake/${component_NAME}-config.cmake
  INSTALL_DESTINATION lib/cmake/${library_NAME})

write_basic_package_version_file(cmake/${component_NAME}-config-version.cmake
  VERSION       ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(TARGETS ${PROJECT_NAME}
  EXPORT   ${PROJECT_NAME}Targets
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
           COMPONENT   ${Library_NAME}_Runtime
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
           COMPONENT          ${Library_NAME}_Runtime
           NAMELINK_COMPONENT ${Library_NAME}_Development
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
           COMPONENT   ${Library_NAME}_Development
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}Targets
  FILE        ${component_NAME}-targets.cmake
  NAMESPACE   ${Library_NAME}::
  DESTINATION lib/cmake/${library_NAME}
  COMPONENT   ${Library_NAME}_Development)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
                  ${CMAKE_CURRENT_BINARY_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT   ${Library_NAME}_Development
  PATTERN     "*.in" EXCLUDE)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cmake/
  DESTINATION lib/cmake/${library_NAME}
  COMPONENT   ${Library_NAME}_Development)
