cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UECS VERSION 0.8.4)
message(STATUS "[Project] ${PROJECT_NAME}")

include(FetchContent)
message(STATUS "find package: UCMake 0.5.3")
find_package(UCMake 0.5.3 QUIET)
if(NOT UCMake_FOUND)
  message(STATUS "UCMake 0.5.3 not found")
  set(_address "https://github.com/Ubpa/UCMake")
  message(STATUS "fetch: ${_address} with tag 0.5.3")
  FetchContent_Declare(
    UCMake
    GIT_REPOSITORY ${_address}
    GIT_TAG "0.5.3"
  )
  FetchContent_MakeAvailable(UCMake)
  message(STATUS "UCMake 0.5.3 build done")
endif()

Ubpa_InitProject()

Ubpa_AddDep(UContainer 0.0.4)
Ubpa_AddDep(UTemplate 0.4.2)

Ubpa_AddSubDirsRec(include)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(DIRECTORIES "include")
