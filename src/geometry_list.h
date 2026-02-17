#ifndef GEOMETRY_LIST_H
#define GEOMETRY_LIST_H

#include "geometry.h"
#include "interval.h"

#include <memory>
#include <Eigen/Core>

class Geometry_List : public Geometry {
    public:
        std::vector<std::shared_ptr<Geometry>> objects;

        Geometry_List() {}

        Geometry_List(std::shared_ptr<Geometry> object) { 
            add(object);
        }

        void clear() {
            objects.clear();
        }

        void add(std::shared_ptr<Geometry> object) {
            objects.push_back(object);
        }

        bool intersect(const Ray& ray, const Interval& ray_t, Intersection_Log& intersection_log) const override {
            Intersection_Log temp_log;
            bool hit_anything = false;
            float closest_so_far = ray_t.get_max();

            for (const auto& object : objects) {
                if (object->intersect(ray, Interval(ray_t.get_min(), closest_so_far), temp_log)) {
                    hit_anything = true;
                    closest_so_far = temp_log.t;
                    intersection_log = temp_log;
                }
            }

            return hit_anything;
        }
};

#endif