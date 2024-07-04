# CMakeList.txt: CppPool 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("CppPool")

# 设置源码目录
set(PATH_SRC "${PROJECT_SOURCE_DIR}/src")
# 设置头文件目录
set(PATH_INCLUDE "${PROJECT_SOURCE_DIR}/include")
# 设置库文件目录
set(PATH_LIB "${PROJECT_SOURCE_DIR}/lib")
# 扫描源文件
aux_source_directory(${PATH_SRC} SOURCES_FILES)

add_library("CppPool" ${SOURCES_FILES})

# 设置生成的静态库文件路径.
set_target_properties("CppPool" PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PATH_LIB})

# 包含头文件
target_include_directories("CppPool" PUBLIC ${PATH_INCLUDE})

add_subdirectory("tests")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CppPool PROPERTY CXX_STANDARD 11)
endif()

# TODO: 如有需要，请添加测试并安装目标。
