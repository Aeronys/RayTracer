#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "geometry.h"
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

        

        void render(const Geometry& world_geometry, const std::string& output_filename) {
            init_camera();
            std::vector<double> color_buffer(image_width * image_height * 3);

            std::clog << "Rendering scene" << std::endl;
            for (int j = 0; j < image_height; j++) {

                // Show rendering progress
                std::clog << std::fixed << std::setprecision(2) << "\rCompleted: " << (j * 100.0f) / image_height << '%' << std::flush;

                for (int i = 0; i < image_width; i++) {

                    Color pixel_color;
                    Ray r = get_ray(i, j);
                    pixel_color = ray_color(r, world_geometry);

                    int buffer_index = 3 * (j * image_width + i);
                    color_buffer[buffer_index + 0] = pixel_color.x();
                    color_buffer[buffer_index + 1] = pixel_color.y();
                    color_buffer[buffer_index + 2] = pixel_color.z();
                }
            }
            save_ppm(output_filename, color_buffer, image_width, image_height);
            std::clog << "\rScene rendered successfully\n" << std::endl;
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
            Eigen::Vector3f pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            Eigen::Vector3f ray_direction = (pixel_center - origin).normalized();
            return Ray(origin, ray_direction);
        }

        Color ray_color(const Ray& ray, const Geometry& world_geometry) {
            Intersection_Log intersection_log;
            Interval ray_t(0.001, std::numeric_limits<float>::infinity());

            if (world_geometry.intersect(ray, ray_t, intersection_log)) {
                return Color(0.0f, 0.0f, 0.0f);
            } else {
                return background_color;
            }
        }

};

#endif