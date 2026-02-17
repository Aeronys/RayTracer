#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "Camera.h"
#include "SceneParser.h"

#include "json.hpp"

class RayTracer {
    public:
        RayTracer(const nlohmann::json& j);

        void run();

    private:
        nlohmann::json json_data;
};

#endif
