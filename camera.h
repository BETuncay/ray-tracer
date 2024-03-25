#pragma once
#include "hittable.h"
#include "ray.h"
#include "color.h"
#include "utils.h"
#include <thread>




class camera
{
public:
    // Image
    double aspect_ratio         = 16.0 / 9.0;
    unsigned image_width        = 400;
    unsigned samples_per_pixel  = 10;
    int max_depth               = 10;   // Maximum number of ray bounces into scene
    
    double vfov                 = 90;  // Vertical view angle (field of view)
    vec3 lookfrom               = vec3(0, 0, -1);
    vec3 lookat                 = vec3(0, 0, 0);
    vec3 vup                    = vec3(0, 1, 0);

    double defocus_angle = 0;
    double focus_dist = 10;

    void render(const hittable& world)
    {
        initialize();

        std::vector<std::array<int, 3>> rendertarget(image_width * image_height);
        for (int j = 0; j < image_height; ++j)
        {
            std::clog << "Scanlines remaining: " << (image_height - j) << '\n';
            for (int i = 0; i < image_width; ++i)
            {
                color pixel_color(0, 0, 0);
                for (unsigned sample = 0; sample < samples_per_pixel; ++sample)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, world, max_depth);
                }
                rendertarget[j * image_width + i] = compute_final_color(pixel_color, samples_per_pixel);
            }
        }
        std::clog << "\rDone.                 \n";

        save_ppm(rendertarget, image_width, image_height);
    }

    void render_threads(const hittable& world, int num_threads)
    {
        initialize();

        std::vector<std::thread> threadPool;
        std::vector<std::array<int, 3>> rendertarget(image_width * image_height);
        for (int i = 0; i < num_threads; ++i)
        {
            threadPool.emplace_back(std::thread(&camera::thread_loop, this, i, num_threads, std::ref(rendertarget), std::ref(world)));
        }
        
        for (std::thread& t : threadPool)
        {
            t.join();
            std::cout << "thread joined!\n";
        }

        save_ppm(rendertarget, image_width, image_height);
    }

private:
    unsigned image_height;   // Rendered image height
    point3 center;          // Camera center
    point3 pixel00_loc;     // Location of pixel 0, 0
    vec3   pixel_delta_u;   // Offset to pixel to the right
    vec3   pixel_delta_v;   // Offset to pixel below

    vec3   u, v, w;        // Camera frame basis vectors
    vec3 defocus_disk_u;
    vec3 defocus_disk_v;
    
    void initialize()
    {
        image_height = static_cast<unsigned>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        double theta = degrees_to_radians(vfov);
        double h = tan(theta / 2.0);
        double viewport_height = 2 * h * focus_dist;
        double viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

        center = lookfrom;

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        vec3 viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + (pixel_delta_u + pixel_delta_v) * 0.5;
    
        double defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    color ray_color(const ray& r, const hittable& world, int depth)
    {
        if (depth <= 0)
        {
            return color(0, 0, 0);
        }

        hit_record rec;
        if (world.hit(r, interval(0.001, infinity), rec))
        {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
            {
                return attenuation * ray_color(scattered, world, depth - 1);
            }
        }

        vec3 unit_direction = unit_vector(r.direction());
        double alpha = 0.5 * (unit_direction.y() + 1.0);
        return lerp(color(1.0, 1.0, 1.0), color(0.5, 0.7, 1.0), alpha);
    }

    ray get_ray(unsigned i, unsigned j) const
    {
        vec3 pixel_center = pixel00_loc + i * pixel_delta_u + j * pixel_delta_v;
        vec3 pixel_sample = pixel_center + pixel_sample_square();
        
        vec3 ray_origin = (defocus_angle <= 0) ? center : defoucus_dist_sample();
        vec3 ray_direction = pixel_sample - ray_origin;
        return ray(ray_origin, ray_direction);
    }

    vec3 pixel_sample_square() const
    {
        double px = -0.5 + random_double();
        double py = -0.5 + random_double();
        return (px * pixel_delta_u) + (py * pixel_delta_v);
    }

    vec3 defoucus_dist_sample() const
    {
        vec3 p = random_in_unit_disk();
        return center + p[0] * defocus_disk_u + p[1] * defocus_disk_v;
    }

    // width = 15
    // num_t = 4
    // respo = 15 / 4 = 3
    // rem   = 15 % 4 = 3
    // t0 = [0, 1, 2]
    // t1 = [3, 4, 5]
    // t2 = [6, 7, 8]
    // t3 = [9, 10, 11, 12, 13, 14] (resp + remainder)
    void thread_loop(int idx, int num_threads, std::vector<std::array<int, 3>>& rendertarget, const hittable& world)
    {
        int num_responsible = image_width / num_threads;
        int remainder = image_width % num_threads;
        int start = idx * num_responsible;
        int end = (idx + 1) * num_responsible;
        if (idx == num_threads - 1)
        {
            num_responsible += remainder;
        }

        for (int j = 0; j < image_height; ++j)
        {
            for (int i = start; i < end; ++i)
            {
                color pixel_color(0, 0, 0);
                for (unsigned sample = 0; sample < samples_per_pixel; ++sample)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, world, max_depth);
                }
                std::array<int, 3> final_color = compute_final_color(pixel_color, samples_per_pixel);
                rendertarget[j * image_width + i] = final_color;
            }
        }
    }
};