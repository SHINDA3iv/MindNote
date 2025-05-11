cmake_minimum_required(VERSION 3.16)

project(Desktop VERSION 0.1 LANGUAGES CXX)

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
set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

# Сбор всех исходных файлов
file(GLOB_RECURSE PROJECT_SOURCES
    ${SRC_DIR}/*.cpp
    ${SRC_DIR}/*.h
)

# Добавление ресурсов
set(RESOURCE_FILES
    ${RESOURCES_DIR}/resources.qrc
)

# Создание исполняемого файла
add_executable(Desktop ${PROJECT_SOURCES} ${RESOURCE_FILES})

# Подключение необходимых компонентов Qt
target_link_libraries(Desktop PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

# Добавление директорий включения заголовочных файлов
target_include_directories(Desktop PRIVATE
    ${SRC_DIR}
    ${SRC_DIR}/view
    ${SRC_DIR}/settings
    ${SRC_DIR}/theme
    ${SRC_DIR}/data
    ${SRC_DIR}/data/elements
)

# Копирование ресурсов в директорию сборки
file(COPY ${RESOURCES_DIR}/icons DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources)

# Установка свойств для разных платформ
set_target_properties(Desktop PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Инструкции для установки
include(GNUInstallDirs)
install(TARGETS Desktop
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Финализация для Qt6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Desktop)
endif()
