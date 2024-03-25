#pragma once
#include "ray.h"
#include "interval.h"
#include "aabb.h"

class material;

class hit_record 
{
public:
    vec3 p;
    vec3 normal;
    std::shared_ptr<material> mat;
    double t;
    bool front_face;

    // assumes outward_normal is unit vector
    void set_face_normal(const ray& r, const vec3& outward_normal)
    {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable
{
public:
    virtual ~hittable() = default;
    virtual bool hit(const ray& r, const interval& ray_t, hit_record& rec) const = 0;
    virtual aabb bounding_box() const = 0;
};