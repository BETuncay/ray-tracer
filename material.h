#pragma once
#include "ray.h"
#include "color.h"
#include "hittable.h"

class hit_record;

class material
{
public:
	virtual ~material() = default;
	virtual bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
};


class lambertian : public material
{
public:
	lambertian(const color& albedo) : m_albedo(albedo) {}

	bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		vec3 scatter_direction = rec.normal + random_unit_vector();
		if (scatter_direction.near_zero())
		{
			scatter_direction = rec.normal;
		}

		scattered = ray(rec.p, scatter_direction);
		attenuation = m_albedo;
		return true;
	}

private:
	color m_albedo;
};

class metal : public material
{
public:
	metal(const color& albedo, double f) : m_albedo(albedo) 
	{
		if (interval(0, 1).contains(f))
		{
			m_fuzz = f;
		}
		else
		{
			m_fuzz = 1;
		}
	}

	bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		vec3 reflected = reflect(unit_vector(ray_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + m_fuzz * random_unit_vector());
		attenuation = m_albedo;
		return true;
	}
private:
	color m_albedo;
	double m_fuzz;
};


class dielectric : public material
{
public:
	dielectric(double index_of_refraction) : m_ir(index_of_refraction) {}

	bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		attenuation = color(1.0, 1.0, 1.0);
		double refraction_ratio = rec.front_face ? (1.0 / m_ir) : m_ir;

		vec3 unit_direction = unit_vector(ray_in.direction());
		double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
		double sin_theta = sqrt(1. - cos_theta * cos_theta);

		vec3 direction;
		bool cannot_refract = refraction_ratio * sin_theta > 1.;
		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
		{
			direction = reflect(unit_direction, rec.normal);
		}
		else
		{
			direction = refract(unit_direction, rec.normal, refraction_ratio);
		}

		scattered = ray(rec.p, direction);
		return true;
	}
private:
	double m_ir;

	static double reflectance(double cosine, double ref_idx) {
		// Use Schlick's approximation for reflectance.
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};