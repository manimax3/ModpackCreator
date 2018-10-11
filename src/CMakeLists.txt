set(UI_FILES mainwindow.ui searchModsDialog.ui)
set(SRC_FILES main.cpp mainwindow.cpp modfinder.cpp tinyxml2.cpp cursemeta.cpp searchModsDialog.cpp)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_add_resources(RESOURCE_FILES ../resources/resources.qrc)

add_executable(${CMAKE_PROJECT_NAME} ${UI_HEADERS} ${SRC_FILES} ${RESOURCE_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${TIDY_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Core ${TIDY_LIBRARIES})
