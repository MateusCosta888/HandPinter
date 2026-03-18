cmake_minimum_required(VERSION 3.16)

project(HandPaint LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar OpenCV instalado pelo MSYS2
find_package(OpenCV REQUIRED)

# Ficheiros fonte
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Menu.cpp
    src/HandTracker.cpp
    src/Canvas.cpp
    src/Stroke.cpp
    src/UIOverlay.cpp
)

# Ficheiros header
set(HEADERS
    src/Application.h
    src/Menu.h
    src/HandTracker.h
    src/Canvas.h
    src/Stroke.h
    src/UIOverlay.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
