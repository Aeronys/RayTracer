#ifndef LIGHT_H
#define LIGHT_H

#include "Eigen/Core"
#include "color.h"

struct PointLight {
    Eigen::Vector3f centre;
    Color id;
    Color is;
};

#endif