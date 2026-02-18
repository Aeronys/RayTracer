#ifndef SPHERE_H
#define SPHERE_H

#include "geometry.h"
#include "Eigen/Core"
#include "Eigen/Geometry"

class Sphere : public Geometry {
    public:
        Sphere(const Eigen::Vector3f& centre, float radius) : centre(centre), radius(std::fmax(radius, 0.0f)) {}

        bool intersect(const Ray& r, const Interval& ray_t, Intersection_Log& intersection_log) const override {
            Ray ray = r;

            if (has_transform()) {
                ray = transform_ray(ray);
            }

            Eigen::Vector3f oc = centre - ray.get_origin();
            float a = ray.get_direction().squaredNorm();
            float h = ray.get_direction().dot(oc);
            float c = oc.squaredNorm() - (radius * radius);

            float discriminant = h * h - a * c;
            if (discriminant < 0.0f) {
                return false;
            }

            float sqrt_discriminant = std::sqrt(discriminant);
            float root = (h - sqrt_discriminant) / a;

            if (!ray_t.surrounds(root)) {
                root = (h + sqrt_discriminant) / a;
                if (!ray_t.surrounds(root)) {
                    return false;
                }
            }

            intersection_log.t = root;
            intersection_log.point = ray.at(intersection_log.t);
            intersection_log.normal = (intersection_log.point - centre) / radius;

            if (has_transform()) {
                transform_hit(intersection_log);
            }

            return true;
        }

    private:
        Eigen::Vector3f centre;
        float radius;
};

#endif