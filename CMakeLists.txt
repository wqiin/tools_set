cmake_minimum_required(VERSION 3.16)

project(Helper LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(Helper
  main.cpp
  filesystem_operation.hpp
  time_operation.hpp
  filesystem_operation.hpp json_operation.hpp string_operation.hpp time_operation.hpp xml_operation.hpp
  general_operation.hpp
  test_Demo.cpp


  std_map_ex.hpp
  general_define.hpp
  qthread.hpp
  qt_delayed_timer.hpp
  custom_error_code.hpp
)

#新增第三方库的头文件搜索路径
target_include_directories(Helper PRIVATE ${PROJECT_SOURCE_DIR}/3rd_party/iguana)
target_include_directories(Helper PRIVATE ${PROJECT_SOURCE_DIR}/3rd_party/nlohmann/include)
target_include_directories(Helper PRIVATE ${PROJECT_SOURCE_DIR}/3rd_party/nlohmann/single_include)
target_include_directories(Helper PRIVATE ${PROJECT_SOURCE_DIR}/3rd_party/xpack)

target_link_libraries(Helper PRIVATE Qt${QT_VERSION_MAJOR}::Core)

include(GNUInstallDirs)
install(TARGETS Helper
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
