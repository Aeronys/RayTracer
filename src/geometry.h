#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "interval.h"
#include "ray.h"

class Intersection_Log {
    public:
        Eigen::Vector3f point;
        Eigen::Vector3f normal;
        float t;
};

class Geometry {
    public:
        virtual ~Geometry() = default;
        virtual bool intersect(const Ray& ray, const Interval& ray_t, Intersection_Log& intersection_log) const = 0;
};

#endif