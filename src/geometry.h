#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "color.h"
#include "interval.h"
#include "ray.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

struct Material {
    Color ac;
    Color dc;
    Color sc;
    float ka;
    float kd;
    float ks;
    float pc;
};

class Intersection_Log {
    public:
        Eigen::Vector3f point;
        Eigen::Vector3f normal;
        Material material;
        float t;
};

class Geometry {
    public:
        virtual ~Geometry() = default;
        virtual bool intersect(const Ray& ray, const Interval& ray_t, Intersection_Log& intersection_log) const = 0;

        virtual void set_transform(const Eigen::Affine3f& t) {
            transform = t;
            // we need inverse transform because we need to transform the ray back to the original space
            inverse_transform = t.inverse();
        }
    
        Material material;

    protected:
        Eigen::Affine3f transform = Eigen::Affine3f::Identity();
        Eigen::Affine3f inverse_transform = Eigen::Affine3f::Identity();

        bool has_transform() const {
            return transform.matrix() != Eigen::Affine3f::Identity().matrix();
        }

        // transform the ray to the local space
        Ray transform_ray(const Ray& ray) const {
            Eigen::Vector3f transformed_origin = inverse_transform * ray.get_origin();
            Eigen::Vector3f transformed_direction = inverse_transform.linear() * ray.get_direction().normalized();
            return Ray(transformed_origin, transformed_direction);
        }

        // transform the hit point and normal back to the original space
        void transform_hit(Intersection_Log& log) const {
            log.point = transform * log.point;
            log.normal = (inverse_transform.linear().transpose() * log.normal).normalized();
        }

    private:
};

#endif