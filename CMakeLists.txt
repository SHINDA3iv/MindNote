cmake_minimum_required(VERSION 3.16)

project(studyProject VERSION 0.1 LANGUAGES CXX)

# Настройки для C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включение автоматической генерации файлов для Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Подключение Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Поиск исходных файлов
file(GLOB_RECURSE PROJECT_SOURCES
    ${SRC_DIR}/*.cpp
    ${SRC_DIR}/*.h
)

# Рекурсивный поиск всех поддиректорий внутри src
file(GLOB_RECURSE SRC_SUBDIRS LIST_DIRECTORIES TRUE "${SRC_DIR}/*")
list(REMOVE_ITEM SRC_SUBDIRS ${PROJECT_SOURCES}) # Исключаем файлы, оставляем только директории

# Создание исполняемого файла
add_executable(studyProject ${PROJECT_SOURCES})

# Подключение библиотек Qt
target_link_libraries(studyProject PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Добавление всех директорий в пути для заголовочных файлов
foreach(DIR ${SRC_SUBDIRS})
    if(IS_DIRECTORY ${DIR})
        target_include_directories(studyProject PRIVATE ${DIR})
    endif()
endforeach()

# Установка свойств для macOS и Windows
set_target_properties(studyProject PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Добавление инструкций для установки
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
