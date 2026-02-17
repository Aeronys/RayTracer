#ifndef RAY_H
#define RAY_H

#include "Eigen/Core"

class Ray {
    public:
        Ray(const Eigen::Vector3f& origin, const Eigen::Vector3f& direction) : origin(origin), direction(direction) {}
        
        const Eigen::Vector3f& get_origin() const {
            return origin;
        }
        
        const Eigen::Vector3f& get_direction() const {
            return direction;
        }
        
        Eigen::Vector3f at(float t) const {
            return origin + t * direction;
        }
        
    private:
        Eigen::Vector3f origin;
        Eigen::Vector3f direction;
};

#endif