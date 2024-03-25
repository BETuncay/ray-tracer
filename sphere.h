#pragma once
#include "hittable.h"
#include "vec3.h"
#include "material.h"

class sphere : public hittable
{
public:
    sphere(vec3 center, double radius, std::shared_ptr<material> material) 
        : m_center(center), m_radius(radius), m_material(material) 
    {
        vec3 rvec = vec3(radius, radius, radius);
        m_bbox = aabb(center - rvec, center + rvec);
    }

    bool hit(const ray& r, const interval& ray_t, hit_record& rec) const override
    {
        vec3 oc = r.origin() - m_center;
        double a = r.direction().length_squared();
        double half_b = dot(r.direction(), oc);
        double c = oc.length_squared() - m_radius * m_radius;
        
        double discriminant = half_b * half_b - a * c;
        if (discriminant < 0)
        {
            return false;
        }

        double discriminant_sqrt = sqrt(discriminant);
        double root = (-half_b - discriminant_sqrt) / a;
        if (!ray_t.surrounds(root))
        {
            root = (-half_b + discriminant_sqrt) / a;
            if (!ray_t.surrounds(root))
            {
                return false;
            }
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - m_center) / m_radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = m_material;
        return true;
    }

    aabb bounding_box() const override { return m_bbox; }

private:
    vec3 m_center;
    double m_radius;
    std::shared_ptr<material> m_material;
    aabb m_bbox;
};