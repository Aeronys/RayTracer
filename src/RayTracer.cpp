#include "RayTracer.h"
#include "geometry_list.h"
#include "light.h"

#include <iostream>

using namespace std;

RayTracer::RayTracer(const nlohmann::json& j) : json_data(j) {}

void RayTracer::run() {
    Geometry_List world_geometry;
    std::vector<PointLight> world_lights;
    if (!parse_geometry(json_data, world_geometry) || !parse_lights(json_data, world_lights)) {
        return;
    }

    for (auto itr = json_data["output"].begin(); itr != json_data["output"].end(); itr++) {
        OutputParams params;
        if (!parse_output(*itr, params)) {
            return;
        }

        Camera camera(params.centre, params.lookat, params.up, params.fov,
                      params.ai, params.bkc, params.width, params.height,
                      params.globalillum, params.probterminate,
                      params.antialiasing, params.twosiderender);
        camera.render(world_geometry, world_lights, params.filename);
    }
}
