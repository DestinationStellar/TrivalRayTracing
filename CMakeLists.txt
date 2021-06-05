CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(FINAL CXX)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

ADD_SUBDIRECTORY(deps/vecmath)

SET(FINAL_SOURCES
        src/image.cpp
        src/main.cpp
        src/mesh.cpp
        src/scene_parser.cpp)

SET(FINAL_INCLUDES
        include/camera.hpp
        include/group.hpp
        include/hit.hpp
        include/image.hpp
        include/light.hpp
        include/material.hpp
        include/mesh.hpp
        include/object3d.hpp
        include/plane.hpp
        include/ray.hpp
        include/scene_parser.hpp
        include/sphere.hpp
        include/transform.hpp
        include/triangle.hpp
        include/curve.hpp
        include/revsurface.hpp
        include/ray_tracer.hpp
        include/utils.hpp
        )

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

ADD_EXECUTABLE(${PROJECT_NAME} ${FINAL_SOURCES} ${FINAL_INCLUDES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} vecmath)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE include)