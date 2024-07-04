cmake_minimum_required(VERSION 3.8)

project("tests")

aux_source_directory(${PROJECT_SOURCE_DIR} SOURCE_FILES)

add_executable("test" ${SOURCE_FILES})

target_include_directories("test" PRIVATE ${PATH_INCLUDE})
# 添加静态库连接
target_link_libraries("test" PRIVATE "CppPool")
# 添加静态库自定义目录.
target_link_directories("test" PRIVATE ${PATH_LIB})