#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "geometry.h"
#include "light.h"
#include "ray.h"
#include "simpleppm.h"

#include <cmath>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#include <iomanip>

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
              twosiderender(twosiderender) {}

        

        void render(const Geometry& world_geometry, const std::vector<PointLight>& world_lights, const std::string& output_filename) {
            init_camera();
            std::vector<double> color_buffer(image_width * image_height * 3);

            //std::clog << "Rendering scene" << std::endl;
            for (int j = 0; j < image_height; j++) {

                // Show rendering progress
                //std::clog << std::fixed << std::setprecision(2) << "\rCompleted: " << (j * 100.0f) / image_height << '%' << std::flush;

                for (int i = 0; i < image_width; i++) {

                    Color pixel_color;
                    Ray r = get_ray(i, j);
                    pixel_color = ray_color(r, world_geometry, world_lights);

                    int buffer_index = 3 * (j * image_width + i);
                    color_buffer[buffer_index + 0] = pixel_color.x();
                    color_buffer[buffer_index + 1] = pixel_color.y();
                    color_buffer[buffer_index + 2] = pixel_color.z();
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
        bool twosiderender;

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

        Ray get_ray(int i, int j) const {
            // Calculate center of pixel and direction of ray
            Eigen::Vector3f pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            Eigen::Vector3f ray_direction = (pixel_center - origin).normalized();
            return Ray(origin, ray_direction);
        }

        Color ray_color(const Ray& ray, const Geometry& world_geometry, const std::vector<PointLight>& world_lights) {
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

                // Calculate light contribution from each point light
                for (const auto& light : world_lights) {
                    Eigen::Vector3f shadow_point = intersection_log.point + shadow_surface_bias * normal;
                    Eigen::Vector3f light_direction = (light.centre - shadow_point).normalized();

                    // Check if light is in shadow
                    Ray shadow_ray(shadow_point, light_direction);
                    float shadow_t_max = (light.centre - shadow_point).norm();
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