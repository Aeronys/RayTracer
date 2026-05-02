#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "geometry.h"
#include "light.h"
#include "ray.h"
#include "simpleppm.h"

#include <algorithm>
#include <cmath>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#include <iomanip>
#include <limits>
#include <random>
#include <vector>

class Camera {
    public:
        Camera(const Eigen::Vector3f& origin,
               const Eigen::Vector3f& lookat,
               const Eigen::Vector3f& up,
               float fov,
               const Color& ambient_intensity,
               const Color& background_color,
               int image_width,
               int image_height,
               bool globalillum,
               float probterminate,
               bool antialiasing,
               std::vector<unsigned int> raysperpixel,
               bool twosiderender)
            : origin(origin),
              lookat(lookat),
              up(up),
              fov(fov),
              ambient_intensity(ambient_intensity),
              background_color(background_color),
              image_width(image_width),
              image_height(image_height),
              globalillum(globalillum),
              probterminate(probterminate),
              antialiasing(antialiasing),
              raysperpixel(raysperpixel),
              twosiderender(twosiderender) {}

        

        void render(const Geometry& world_geometry,
                    const std::vector<PointLight>& point_lights,
                    const std::vector<AreaLight>& area_lights,
                    const std::string& output_filename) {
            init_camera();
            std::vector<double> color_buffer(image_width * image_height * 3);

            stratified = false;
            aa_samples = DEFAULT_NUM_SAMPLES;
            random_generator.seed(std::random_device{}());

            // Determine antialiasing settings
            if (antialiasing) {
                if (raysperpixel.size() == 1) {
                    aa_samples = static_cast<int>(raysperpixel[0]);
                } else if (raysperpixel.size() == 2) {
                    stratified = true;
                    grid_width = static_cast<int>(raysperpixel[0]);
                    grid_height = static_cast<int>(raysperpixel[0]);
                    aa_samples = static_cast<int>(raysperpixel[1]);
                } else if (raysperpixel.size() == 3) {
                    stratified = true;
                    grid_width = static_cast<int>(raysperpixel[0]);
                    grid_height = static_cast<int>(raysperpixel[1]);
                    aa_samples = static_cast<int>(raysperpixel[2]);
                } else if (raysperpixel.size() > 3) {
                    std::cout << "Invalid number of rays per pixel" << std::endl;
                    std::cout << "Using default of " << DEFAULT_NUM_SAMPLES << " samples per pixel" << std::endl;
                }
                // empty raysperpixel: keep aa_samples == DEFAULT_NUM_SAMPLES
            }

            // if less than 1 sample, set to 1
            if (aa_samples < 1) {
                aa_samples = 1;
                std::cout << "Number of samples can not be less than 1, 1 sample per pixel will be used" << std::endl;
            }

            // if stratified and grid width or height is less than 1, set to false
            if (stratified && (grid_width < 1 || grid_height < 1)) {
                stratified = false;
                std::cout << "Invalid grid width or height, uniform sampling will be used" << std::endl;
            }

            //std::clog << "Rendering scene" << std::endl;

            if (!antialiasing) {
                for (int j = 0; j < image_height; j++) {
                    // Show rendering progress
                    // std::clog << std::fixed << std::setprecision(2) << "\rCompleted: " << (j * 100.0f) / image_height << '%' << std::flush;

                    for (int i = 0; i < image_width; i++) {
                        Ray r = get_ray_center(i, j);
                        Color pixel_color = ray_color(r, world_geometry, point_lights, area_lights);
                        int buffer_index = 3 * (j * image_width + i);
                        color_buffer[buffer_index + 0] = pixel_color.x();
                        color_buffer[buffer_index + 1] = pixel_color.y();
                        color_buffer[buffer_index + 2] = pixel_color.z();
                    }
                }
            } else if (!stratified) {
                for (int j = 0; j < image_height; j++) {
                    // Show rendering progress
                    // std::clog << std::fixed << std::setprecision(2) << "\rCompleted: " << (j * 100.0f) / image_height << '%' << std::flush;

                    for (int i = 0; i < image_width; i++) {
                        Color pixel_color = Color::Zero();
                        for (int k = 0; k < aa_samples; k++) {
                            Ray r = get_ray_uniform(i, j);
                            pixel_color += ray_color(r, world_geometry, point_lights, area_lights);
                        }
                        pixel_color /= static_cast<float>(aa_samples);
                        int buffer_index = 3 * (j * image_width + i);
                        color_buffer[buffer_index + 0] = pixel_color.x();
                        color_buffer[buffer_index + 1] = pixel_color.y();
                        color_buffer[buffer_index + 2] = pixel_color.z();
                    }
                }
            } else {
                const int total_samples = grid_width * grid_height * aa_samples;
                for (int j = 0; j < image_height; j++) {
                    // Show rendering progress
                    // std::clog << std::fixed << std::setprecision(2) << "\rCompleted: " << (j * 100.0f) / image_height << '%' << std::flush;

                    for (int i = 0; i < image_width; i++) {
                        Color pixel_color = Color::Zero();
                        for (int sy = 0; sy < grid_height; sy++) {
                            for (int sx = 0; sx < grid_width; sx++) {
                                for (int k = 0; k < aa_samples; k++) {
                                    Ray r = get_ray_stratified(i, j, sx, sy);
                                    pixel_color += ray_color(r, world_geometry, point_lights, area_lights);
                                }
                            }
                        }
                        pixel_color /= static_cast<float>(total_samples);
                        int buffer_index = 3 * (j * image_width + i);
                        color_buffer[buffer_index + 0] = pixel_color.x();
                        color_buffer[buffer_index + 1] = pixel_color.y();
                        color_buffer[buffer_index + 2] = pixel_color.z();
                    }
                }
            }

            save_ppm(output_filename, color_buffer, image_width, image_height);
            //std::clog << "\rScene rendered successfully\n" << std::endl;
        }

    private:
        Eigen::Vector3f origin;
        Eigen::Vector3f lookat;
        Eigen::Vector3f up;
        Color ambient_intensity;
        Color background_color;
        int image_width;
        int image_height;
        float fov;
        Eigen::Vector3f u, v, w;
        float viewport_height, viewport_width;
        Eigen::Vector3f viewport_u, viewport_v;
        Eigen::Vector3f pixel00_loc;
        Eigen::Vector3f pixel_delta_u, pixel_delta_v;
        Eigen::Vector3f viewport_upper_left;
        bool globalillum;
        float probterminate;
        bool antialiasing;
        std::vector<unsigned int> raysperpixel;
        bool stratified = false;
        static constexpr int DEFAULT_NUM_SAMPLES = 10;
        int aa_samples = DEFAULT_NUM_SAMPLES;
        int grid_width = 1;
        int grid_height = 1;
        bool twosiderender;
        std::mt19937 random_generator;
        std::uniform_real_distribution<float> random_range{0.0f, 1.0f};

        void init_camera() {
            // Calculate viewport dimensions
            float focal_length = 1.0f;
            float theta = fov * M_PI / 180.0f;
            float h = tan(theta / 2.0f);
            viewport_height = 2.0f * h * focal_length;
            viewport_width = viewport_height * (float(image_width) / image_height);

            // Calculate u, v, and w vectors
            w = -lookat.normalized();
            u = up.cross(w).normalized();
            v = w.cross(u);

            // Calculate vectors across vertical and horizontal viewport edges
            viewport_u = viewport_width * u;
            viewport_v = viewport_height * -v;

            //Calculate horizontal and vertical deltas
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            // Calculate location of upper left most pixel
            viewport_upper_left = origin - (focal_length * w) - viewport_u / 2 - viewport_v / 2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
        }

        Ray get_ray_center(int i, int j) const {
            Eigen::Vector3f pixel_center = pixel00_loc + (static_cast<float>(i) * pixel_delta_u) + (static_cast<float>(j) * pixel_delta_v);
            Eigen::Vector3f ray_direction = (pixel_center - origin).normalized();
            return Ray(origin, ray_direction);
        }

        Ray get_ray_uniform(int i, int j) {
            const float ru = random_range(random_generator);
            const float rv = random_range(random_generator);
            Eigen::Vector3f pixel_sample = get_pixel_sample(i, j, ru, rv);
            Eigen::Vector3f ray_direction = (pixel_sample - origin).normalized();
            return Ray(origin, ray_direction);
        }

        Ray get_ray_stratified(int i, int j, int sx, int sy) {
            const float xi = random_range(random_generator);
            const float eta = random_range(random_generator);
            const float ru = (static_cast<float>(sx) + xi) / static_cast<float>(grid_width);
            const float rv = (static_cast<float>(sy) + eta) / static_cast<float>(grid_height);
            Eigen::Vector3f pixel_sample = get_pixel_sample(i, j, ru, rv);
            Eigen::Vector3f ray_direction = (pixel_sample - origin).normalized();
            return Ray(origin, ray_direction);
        }

        Eigen::Vector3f get_pixel_sample(int i, int j, float ru, float rv) const {
            Eigen::Vector3f pixel_center = pixel00_loc + (static_cast<float>(i) * pixel_delta_u) + (static_cast<float>(j) * pixel_delta_v);
            Eigen::Vector3f pixel_corner = pixel_center - 0.5f * pixel_delta_u - 0.5f * pixel_delta_v;
            return pixel_corner + ru * pixel_delta_u + rv * pixel_delta_v;
        }

        static Eigen::Vector3f area_light_position(const AreaLight& L, float u, float v) {
            const Eigen::Vector3f edge1 = L.p2 - L.p1;
            const Eigen::Vector3f edge2 = L.p4 - L.p1;
            return L.p1 + u * edge1 + v * edge2;
        }

        Color ray_color(const Ray& ray,
                        const Geometry& world_geometry,
                        const std::vector<PointLight>& point_lights,
                        const std::vector<AreaLight>& area_lights) {
            Intersection_Log intersection_log;
            Interval ray_t(0.001f, std::numeric_limits<float>::infinity());

            if (world_geometry.intersect(ray, ray_t, intersection_log)) {
                Eigen::Vector3f normal = intersection_log.normal;

                // If two-sided rendering is enabled, flip normal if it's facing wrong way
                if (twosiderender && normal.dot(ray.get_direction()) > 0) {
                    normal = -normal;
                }
                
                // Calculate ambient lighting
                Color color = intersection_log.material.ka * intersection_log.material.ac.cwiseProduct(ambient_intensity);

                // To prevent shadow acne
                const float shadow_surface_bias = 0.001f;
                Eigen::Vector3f shadow_point = intersection_log.point + shadow_surface_bias * normal;

                // Calculate light contribution from each point light
                for (const auto& light : point_lights) {
                    Eigen::Vector3f light_direction = (light.centre - shadow_point).normalized();

                                       // Check if light is in shadow (t_max stops short of the light, same as area lights)
                    Ray shadow_ray(shadow_point, light_direction);
                    const float shadow_dist = (light.centre - shadow_point).norm();
                    const float shadow_t_max = std::max(shadow_dist - shadow_surface_bias, 1e-5f);
                    Interval shadow_t(0.0f, shadow_t_max);
                    Intersection_Log shadow_log;

                    // Skip if light is in shadow
                    if (world_geometry.intersect(shadow_ray, shadow_t, shadow_log)) {
                        continue;
                    }

                    // Calculate diffuse lighting
                    float diffuse_factor = std::max(0.0f, normal.dot(light_direction));
                    color += intersection_log.material.kd * intersection_log.material.dc.cwiseProduct(light.id) * diffuse_factor;

                    // Calculate specular lighting (Blinn-Phong)
                    Eigen::Vector3f view_direction = (-ray.get_direction()).normalized();
                    Eigen::Vector3f half_vector = (light_direction + view_direction).normalized();
                    float specular_factor = std::pow(std::max(0.0f, normal.dot(half_vector)), intersection_log.material.pc);
                    color += intersection_log.material.ks * intersection_log.material.sc.cwiseProduct(light.is) * specular_factor;
                }

                // Calculate light contribution from each area light
                for (const auto& light : area_lights) {
                    const int grid_n = std::max(1, light.n);
                    
                    // If AA is enabled, treat as point light
                    const bool use_center_sample = light.usecenter || antialiasing;
                    const int num_samples = use_center_sample ? 1 : grid_n * grid_n;
                    Color area_color = Color::Zero();

                    auto add_area_sample = [&](float u, float v) {
                        Eigen::Vector3f lpos = area_light_position(light, u, v);
                        Eigen::Vector3f to_light = lpos - shadow_point;
                        float dist = to_light.norm();
                        if (dist < 1e-6f) {
                            return;
                        }
                        Eigen::Vector3f light_direction = to_light / dist;
                        Ray shadow_ray(shadow_point, light_direction);
                        const float shadow_t_max = std::max(dist - shadow_surface_bias, 1e-5f);
                        Interval shadow_t(0.0f, shadow_t_max);
                        Intersection_Log shadow_log;
                        if (world_geometry.intersect(shadow_ray, shadow_t, shadow_log)) {
                            return;
                        }
                        float diffuse_factor = std::max(0.0f, normal.dot(light_direction));
                        area_color += intersection_log.material.kd * intersection_log.material.dc.cwiseProduct(light.id) * diffuse_factor;
                        Eigen::Vector3f view_direction = (-ray.get_direction()).normalized();
                        Eigen::Vector3f half_vector = (light_direction + view_direction).normalized();
                        float specular_factor = std::pow(std::max(0.0f, normal.dot(half_vector)), intersection_log.material.pc);
                        area_color += intersection_log.material.ks * intersection_log.material.sc.cwiseProduct(light.is) * specular_factor;
                    };

                    if (use_center_sample) {
                        add_area_sample(0.5f, 0.5f);
                    } else {
                        const float inv = 1.0f / static_cast<float>(grid_n);
                        for (int sj = 0; sj < grid_n; sj++) {
                            for (int si = 0; si < grid_n; si++) {
                                float u = (static_cast<float>(si) + random_range(random_generator)) * inv;
                                float v = (static_cast<float>(sj) + random_range(random_generator)) * inv;
                                add_area_sample(u, v);
                            }
                        }
                    }

                    color += area_color / static_cast<float>(num_samples);
                }

                // Clamp color values to be between 0 and 1
                color = Color(
                    std::min(1.0f, std::max(0.0f, color.x())),
                    std::min(1.0f, std::max(0.0f, color.y())),
                    std::min(1.0f, std::max(0.0f, color.z()))
                );

                return color;
            } else {
                // Return background color if no intersection is found
                return background_color;
            }
        }

};

#endif