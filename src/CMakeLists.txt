set(UI_FILES)
set(SRC_FILES main.cpp)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_add_resources(RESOURCE_FILES ../resources/resources.qrc)

add_executable(${CMAKE_PROJECT_NAME} ${UI_HEADERS} ${SRC_FILES} ${RESOURCE_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Widgets)
