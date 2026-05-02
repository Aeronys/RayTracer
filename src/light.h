#ifndef LIGHT_H
#define LIGHT_H

#include "Eigen/Core"
#include "color.h"

struct PointLight {
    Eigen::Vector3f centre;
    Color id;
    Color is;
};

struct AreaLight {
    Eigen::Vector3f p1;
    Eigen::Vector3f p2;
    Eigen::Vector3f p3;
    Eigen::Vector3f p4;
    Color id;
    Color is;
    int n = 1;
    bool usecenter = false;
};

#endif