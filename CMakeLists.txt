cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UECS VERSION 0.14.1)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake()

Ubpa_InitProject()

Ubpa_AddDep(UContainer 0.0.7)
Ubpa_AddDep(UTemplate  0.4.9)
Ubpa_AddDep(UGraphviz  0.1.5)

Ubpa_AddSubDirsRec(include)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(
  TARGET
  DIRECTORIES
    "include"
)
