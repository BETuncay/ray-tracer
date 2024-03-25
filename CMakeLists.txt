cmake_minimum_required (VERSION 3.8)

project ("ray-tracer")

add_executable (ray-tracer main.cpp vec3.h color.h ray.h hittable.h hittable_list.h sphere.h utils.h interval.h camera.h material.h aabb.h bvh.h)