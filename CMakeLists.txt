cmake_minimum_required(VERSION 3.17)
project(ClientChatApp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(QT_VERSION 5)
find_package(ZLIB REQUIRED)
find_library(ZSTD_LIBRARY NAMES zstdd zstd_staticd zstd zstd_static NAMES_PER_DIR)
set(REQUIRED_LIBS Core Gui Widgets Network)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)
find_package (bson-1.0 1.7 REQUIRED)

add_executable(${PROJECT_NAME}
        mainwindow.h
        mainwindow.cpp
        main.cpp
        login.cpp
        login.h
        database_handler.h
        mainwindow.ui
        StringList.h
        chat_message.hpp
        ListViewDelegate.h)

if (NOT CMAKE_PREFIX_PATH)
    message(WARNING "CMAKE_PREFIX_PATH is not defined, you may need to set it "
            "(-DCMAKE_PREFIX_PATH=\"path/to/Qt/lib/cmake\" or -DCMAKE_PREFIX_PATH=/usr/include/{host}/qt{version}/ on Ubuntu)")
endif ()

find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED} ${ZSTD_LIBRARY} sqlite3 mongo::bson_shared)
