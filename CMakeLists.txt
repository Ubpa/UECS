cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UECS VERSION 0.7.0)
message(STATUS "[Project] ${PROJECT_NAME}")

include(FetchContent)
message(STATUS "find package: UCMake 0.5.1")
find_package(UCMake 0.5.1 QUIET)
if(NOT UCMake_FOUND)
  message(STATUS "UCMake 0.5.1 not found")
  set(_address "https://github.com/Ubpa/UCMake")
  message(STATUS "fetch: ${_address} with tag 0.5.1")
  FetchContent_Declare(
    UCMake
    GIT_REPOSITORY ${_address}
    GIT_TAG "0.5.1"
  )
  FetchContent_MakeAvailable(UCMake)
  message(STATUS "UCMake 0.5.1 build done")
endif()

Ubpa_InitProject()

Ubpa_AddDep(UContainer 0.0.2)
Ubpa_AddDep(UTemplate 0.4.1)

Ubpa_AddSubDirsRec(include)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(DIRECTORIES "include")
