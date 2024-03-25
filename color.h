#pragma once
#include "vec3.h"
#include "interval.h"
#include <iostream>
#include <array>
using color = vec3;

static const interval intensity(0.000, 0.999);

vec3 linear_to_gamma(const color& linear_component)
{
    return sqrt3(linear_component);
}

std::array<int, 3> compute_final_color(color& pixel_color, unsigned samples)
{
    double scale = 1.0 / samples;
    pixel_color *= scale;
    pixel_color = linear_to_gamma(pixel_color);

    std::array<int, 3> col;
    col[0] = static_cast<int>(256 * intensity.clamp(pixel_color[0]));
    col[1] = static_cast<int>(256 * intensity.clamp(pixel_color[1]));
    col[2] = static_cast<int>(256 * intensity.clamp(pixel_color[2]));

    return col;
}

void write_color(std::ostream& out, color pixel_color, unsigned samples)
{
    double scale = 1.0 / samples;
    pixel_color *= scale;
    pixel_color = linear_to_gamma(pixel_color);

    static const interval intensity(0.000, 0.999);
    out << static_cast<int>(256 * intensity.clamp(pixel_color[0])) << ' '
        << static_cast<int>(256 * intensity.clamp(pixel_color[1])) << ' '
        << static_cast<int>(256 * intensity.clamp(pixel_color[2])) << '\n';
}

void save_ppm(const std::vector<std::array<int, 3>>& rendertarget, unsigned image_width, unsigned image_height)
{
    std::ofstream image("test.ppm");
    image << "P3\n";
    image << image_width << " " << image_height << "\n";
    image << "255\n";

    for (std::array<int, 3> col : rendertarget)
    {
        image << col[0] << ' ' << col[1] << ' ' << col[2] << '\n';
    }

    image.close();
}
