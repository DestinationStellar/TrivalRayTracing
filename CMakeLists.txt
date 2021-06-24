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
        src/scene_parser.cpp
        src/texture.cpp
        src/pdf.cpp
        src/bvh.cpp
        )

SET(FINAL_INCLUDES
        deps/stb_image/stb_image.h
        deps/stb_image/stb_image_write.h
        deps/tiny_obj/tiny_obj_loader.h
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
        include/pdf.hpp
        include/scene_parser.hpp
        include/sphere.hpp
        include/transform.hpp
        include/triangle.hpp
        include/curve.hpp
        include/revsurface.hpp
        include/ray_tracer.hpp
        include/utils.hpp
        include/moving_sphere.hpp
        include/aabb.hpp
        include/bvh.hpp
        include/texture.hpp
        include/perlin.hpp
        include/rectangle.hpp
        include/box.hpp
        include/constant_medium.hpp
        include/scene_generator.hpp
        include/onb.hpp
        )

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
FIND_PACKAGE( OpenMP )
IF(OpenMP_CXX_FOUND)
    MESSAGE("OPENMP FOUND")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
ENDIF()
ADD_EXECUTABLE(${PROJECT_NAME} ${FINAL_SOURCES} ${FINAL_INCLUDES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} vecmath)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE include)