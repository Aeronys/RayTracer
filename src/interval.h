#ifndef INTERVAL_H
#define INTERVAL_H

#include <limits>

class Interval {
    public:
        Interval() : min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::min()) {}
        Interval(float min, float max) : min(min), max(max) {}

        float get_min() const {
            return min;
        }

        float get_max() const {
            return max;
        }

        void set_min(float min) {
            this->min = min;
        }
        void set_max(float max) {
            this->max = max;
        }

        float size() const {
            return max - min;
        }

        bool contains(float x) const {
            return x >= min && x <= max;
        }

        bool surrounds(float x) const {
            return x > min && x < max;
        }

        float clamp(float x) const {
            if (x < min) return min;
            if (x > max) return max;
            return x;
        }

        static const Interval empty, universe;

    private:
        float min; // minimum value of the interval
        float max; // maximum value of the interval
};

#endif