#ifndef SCENEPARSER_H
#define SCENEPARSER_H

#include "geometry_list.h"
#include "json.hpp"
#include "light.h"
#include "rectangle.h"
#include "sphere.h"

#include <Eigen/Core>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

struct OutputParams {
    std::string filename;
    int width, height;
    Eigen::Vector3f centre, lookat, up;
    float fov;
    Eigen::Vector3f ai, bkc;
    bool globalillum = false;
    float probterminate = 0.0f;
    bool antialiasing = false;
    bool twosiderender = true;
};

inline Eigen::Vector3f parse_vector(const nlohmann::json& arr) {
    if (arr.size() < 3) {
        throw std::runtime_error("Vector must contain 3 elements");
    } else if (arr.size() > 3) {
        std::cout << "Warning: Too many entries in vector" << std::endl;
    }
    return Eigen::Vector3f(arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>());
}

inline Eigen::Affine3f parse_transform(const nlohmann::json& arr) {
    if (arr.size() != 16) {
        throw std::runtime_error("Transform must contain 16 elements");
    }
    Eigen::Matrix4f m;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m(i, j) = arr[i * 4 + j].get<float>();
        }
    }
    return Eigen::Affine3f(m);
}

// parse all geometry objects from the json data
inline bool parse_geometry(const nlohmann::json& json_data, Geometry_List& world_geometry) {
    for (auto itr = json_data["geometry"].begin(); itr != json_data["geometry"].end(); itr++) {
        if (!itr->contains("type")) {
            std::cout << "Fatal error: geometry must always contain a type" << std::endl;
            return false;
        }

        if (itr->contains("visible") && !(*itr)["visible"].get<bool>()) {
            continue;
        }

        // parse geometry type
        std::string type = (*itr)["type"].get<std::string>();
        std::shared_ptr<Geometry> geometry;
        if (type == "sphere") {
            Eigen::Vector3f centre = parse_vector((*itr)["centre"]);
            float radius = (*itr)["radius"].get<float>();
            geometry = std::make_shared<Sphere>(centre, radius);

        } else if (type == "rectangle") {
            Eigen::Vector3f p1 = parse_vector((*itr)["p1"]);
            Eigen::Vector3f p2 = parse_vector((*itr)["p2"]);
            Eigen::Vector3f p3 = parse_vector((*itr)["p3"]);
            Eigen::Vector3f p4 = parse_vector((*itr)["p4"]);
            geometry = std::make_shared<Rectangle>(p1, p2, p3, p4);

        } else {
            std::cout << "Fatal error: invalid geometry type: " << type << std::endl;
            return false;
        }

        // parse material properties
        if (!itr->contains("ac")) {
            std::cout << "Fatal error: geometry must always contain 'ac' (ambient color)" << std::endl;
            return false;
        }
        geometry->material.ac = parse_vector((*itr)["ac"]);

        if (!itr->contains("dc")) {
            std::cout << "Fatal error: geometry must always contain 'dc' (diffuse color)" << std::endl;
            return false;
        }
        geometry->material.dc = parse_vector((*itr)["dc"]);

        if (!itr->contains("sc")) {
            std::cout << "Fatal error: geometry must always contain 'sc' (specular color)" << std::endl;
            return false;
        }
        geometry->material.sc = parse_vector((*itr)["sc"]);

        if (!itr->contains("ka")) {
            std::cout << "Fatal error: geometry must always contain 'ka' (ambient coefficient)" << std::endl;
            return false;
        }
        geometry->material.ka = (*itr)["ka"].get<float>();

        if (!itr->contains("kd")) {
            std::cout << "Fatal error: geometry must always contain 'kd' (diffuse coefficient)" << std::endl;
            return false;
        }
        geometry->material.kd = (*itr)["kd"].get<float>();

        if (!itr->contains("ks")) {
            std::cout << "Fatal error: geometry must always contain 'ks' (specular coefficient)" << std::endl;
            return false;
        }
        geometry->material.ks = (*itr)["ks"].get<float>();

        if (!itr->contains("pc")) {
            std::cout << "Fatal error: geometry must always contain 'pc' (phong exponent)" << std::endl;
            return false;
        }
        geometry->material.pc = (*itr)["pc"].get<float>();

        // parse transform (if it exists)
        if (itr->contains("transform")) {
            geometry->set_transform(parse_transform((*itr)["transform"]));
        }

        // add the geometry to the world
        world_geometry.add(geometry);

    }
    return true;
}

// parse all the light objects from the json data
inline bool parse_lights(const nlohmann::json& json_data, std::vector<PointLight>& world_lights) {
    for (auto itr = json_data["light"].begin(); itr != json_data["light"].end(); itr++) {
        if (!itr->contains("type")) {
            std::cout << "Fatal error: light must always contain a type" << std::endl;
            return false;
        }

        // skip light if it is not used
        if (itr->contains("use") && !(*itr)["use"].get<bool>()) {
            continue;
        }

        // parse light type
        std::string type = (*itr)["type"].get<std::string>();

        if (type == "point") {
            PointLight light;
            if (!itr->contains("centre")) {
                std::cout << "Fatal error: point light must always contain a centre" << std::endl;
                return false;
            }
            light.centre = parse_vector((*itr)["centre"]);

            if (!itr->contains("id")) {
                std::cout << "Fatal error: point light must always contain an id" << std::endl;
                return false;
            }
            light.id = parse_vector((*itr)["id"]);

            if (!itr->contains("is")) {
                std::cout << "Fatal error: point light must always contain an is" << std::endl;
                return false;
            }
            light.is = parse_vector((*itr)["is"]);

            world_lights.push_back(light);

        } else if (type == "area") {
            std::cout << "Skipping area lights for now" << std::endl;
        } else {
            std::cout << "Fatal error: invalid light type: " << type << std::endl;
            return false;
        }
    }

    return true;
}

// parse all the output parameters from the json data
inline bool parse_output(const nlohmann::json& json_data, OutputParams& params) {
    if (!json_data.contains("filename")) {
        std::cout << "Fatal error: output must always contain a filename" << std::endl;
        return false;
    }
    params.filename = json_data["filename"].get<std::string>();

    if (!json_data.contains("size")) {
        std::cout << "Fatal error: output must always contain a size" << std::endl;
        return false;
    }
    if (json_data["size"].size() < 2) {
        std::cout << "Fatal error: size must contain 2 elements" << std::endl;
        return false;
    }
    if (json_data["size"].size() > 2) {
        std::cout << "Warning: Too many entries in size" << std::endl;
    }
    params.width = json_data["size"][0].get<int>();
    params.height = json_data["size"][1].get<int>();

    if (!json_data.contains("centre")) {
        std::cout << "Fatal error: output must always contain a centre" << std::endl;
        return false;
    }
    params.centre = parse_vector(json_data["centre"]);

    if (!json_data.contains("lookat")) {
        std::cout << "Fatal error: output must always contain a lookat" << std::endl;
        return false;
    }
    params.lookat = parse_vector(json_data["lookat"]);

    if (!json_data.contains("up")) {
        std::cout << "Fatal error: output must always contain a up" << std::endl;
        return false;
    }
    params.up = parse_vector(json_data["up"]);

    if (!json_data.contains("fov")) {
        std::cout << "Fatal error: output must always contain a fov" << std::endl;
        return false;
    }
    params.fov = json_data["fov"].get<float>();

    if (!json_data.contains("ai")) {
        std::cout << "Fatal error: output must always contain a ai" << std::endl;
        return false;
    }
    params.ai = parse_vector(json_data["ai"]);

    if (!json_data.contains("bkc")) {
        std::cout << "Fatal error: output must always contain a bkc" << std::endl;
        return false;
    }
    params.bkc = parse_vector(json_data["bkc"]);

    if (json_data.contains("globalillum")) {
        params.globalillum = json_data["globalillum"].get<bool>();
    }
    if (json_data.contains("probterminate")) {
        params.probterminate = json_data["probterminate"].get<float>();
        if (params.probterminate < 0.0f || params.probterminate > 1.0f) {
            std::cout << "Warning: probterminate must be between 0.0 and 1.0" << std::endl;
            std::cout << "Setting probterminate to 0.0" << std::endl;
            params.probterminate = 0.0f;
        }
    }
    if (json_data.contains("antialiasing")) {
        params.antialiasing = json_data["antialiasing"].get<bool>();
    }
    if (json_data.contains("twosiderender")) {
        params.twosiderender = json_data["twosiderender"].get<bool>();
    }

    return true;
}

#endif
