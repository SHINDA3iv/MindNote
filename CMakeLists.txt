cmake_minimum_required(VERSION 3.16)

project(studyProject VERSION 0.1 LANGUAGES CXX)

# Настройки для C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включение автоматической генерации файлов для Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Создаем отдельную папку для runtime
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/root)

# Поиск и подключение Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Widgets Network)
endif()

# Установка переменных пути к исходникам
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Сбор всех исходных файлов
file(GLOB_RECURSE PROJECT_SOURCES
    ${SRC_DIR}/*.cpp
    ${SRC_DIR}/*.h
)

# Рекурсивный поиск поддиректорий
file(GLOB_RECURSE SRC_SUBDIRS LIST_DIRECTORIES TRUE "${SRC_DIR}/*")
list(REMOVE_ITEM SRC_SUBDIRS ${PROJECT_SOURCES})

# qt_add_resources(PROJECT_SOURCES
#     "application_resources"
#     PREFIX "/"
#     FILES
#         resources/icons/add.png
#         resources/icons/delete.png
#         resources/icons/edit.png
#         resources/icons/sync.png
#         resources/icons/logout.png
# )

# Создание исполняемого файла
add_executable(studyProject ${PROJECT_SOURCES})

# Подключение необходимых компонентов Qt
target_link_libraries(studyProject PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

# Добавление директорий включения заголовочных файлов
foreach(DIR ${SRC_SUBDIRS})
    if(IS_DIRECTORY ${DIR})
        target_include_directories(studyProject PRIVATE ${DIR})
    endif()
endforeach()

# Установка свойств для разных платформ
set_target_properties(studyProject PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Инструкции для установки
include(GNUInstallDirs)
install(TARGETS studyProject
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Финализация для Qt6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(studyProject)
endif()
