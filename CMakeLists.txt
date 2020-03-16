# 设置 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# 设定项目
project(UECS VERSION 1.0)
message(STATUS "[Project] ${PROJECT_NAME}")

# 修改默认的安装路径前缀
if(CMAKE_INSTALL_PREFIX_InitIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "install prefix" FORCE)
endif()

# 在 Debug 模式下自动对 dll 和 lib 文件名后边加 d，但 exe 仍然需要手动设置
set(CMAKE_DEBUG_POSTFIX d)

# 确定 C++ 版本
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
 
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
endif()

# 将生成的 exe, dll, lib 等放置在 bin, lib 目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/lib")

# 开启 VS 的文件夹功能
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)
FetchContent_Declare(
  UGit
  GIT_REPOSITORY https://github.com/Ubpa/UGit
  GIT_TAG        59787a5
)
FetchContent_MakeAvailable(UGit)

Ubpa_InitGit()
Ubpa_UpdateSubModule()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/external/UCMake/cmake")
include(UbpaTool)

include_directories (
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/external/UTemplate/include
	# $ENV{XXX_INCLUDE_PATH}
	# ...
)
link_directories (
	${PROJECT_SOURCE_DIR}/lib
	# $ENV{XXX_LIB_PATH}
	# ...
)

Ubpa_AddSubDirsRec(src)

Ubpa_Export()
