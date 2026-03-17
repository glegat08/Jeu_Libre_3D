# do not touch that part
project(Editor)
set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${HEADER_DIR}/*.h"
    "${HEADER_DIR}/*.hpp"
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${SOURCE_DIR}/*.cpp"
)

add_library(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

source_group(
    TREE ${HEADER_DIR}
    PREFIX "Header Files"
    FILES ${HEADERS}
)

source_group(
    TREE ${SOURCE_DIR}
    PREFIX "Source Files"
    FILES ${SOURCES}
)

find_package(glfw3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(VulkanSDK_DIR $ENV{VULKAN_SDK})
set(VukanHeader ${VulkanSDK_DIR}/Include)
set(VulkanLib ${VulkanSDK_DIR}/Lib)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
    ${HEADER_DIR}
    ${VukanHeader}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    Engine
    ImGui
    Vulkan
    GLFW_
    GameLib
    glfw
    nlohmann_json::nlohmann_json
    ${VulkanLib}/vulkan-1.lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Editor")
