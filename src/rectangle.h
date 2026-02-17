#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "geometry.h"
#include "Eigen/Core"

class Rectangle : public Geometry {
    public:
        Rectangle(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3, const Eigen::Vector3f& p4) : p1(p1), p2(p2), p3(p3), p4(p4) {
            edge1 = p2 - p1;
            edge2 = p4 - p1;
            plane_normal = edge1.cross(edge2).normalized();
        }

        bool intersect(const Ray& ray, const Interval& ray_t, Intersection_Log& intersection_log) const override {
            

            float denominator = plane_normal.dot(ray.get_direction());
            if (std::fabs(denominator) < 1e-8f) {
                return false;
            }

            float t = (plane_normal.dot(p1 - ray.get_origin())) / denominator;
            if (!ray_t.surrounds(t)) {
                return false;
            }

                Eigen::Vector3f intersection_point = ray.at(t);
                Eigen::Vector3f intersection_point_to_p1 = intersection_point - p1;

                float u = intersection_point_to_p1.dot(edge1) / edge1.squaredNorm();
                float v = intersection_point_to_p1.dot(edge2) / edge2.squaredNorm();

                if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f) {
                    return false;
                }

                intersection_log.t = t;
                intersection_log.point = intersection_point;
                intersection_log.normal = plane_normal;

            return true;
        }

    private:
        Eigen::Vector3f p1, p2, p3, p4;
        Eigen::Vector3f edge1;
        Eigen::Vector3f edge2;
        Eigen::Vector3f plane_normal;
};

#endif