#pragma once
#include <random>
#include <unordered_map>

class RandomNumbers {
private:
    std::mt19937 gen; // Generator liczb losowych
    std::unordered_map<std::string, std::uniform_real_distribution<float>> distributions;

    RandomNumbers() {
        std::random_device rd;
        gen.seed(rd());
    }

public:
    static RandomNumbers& getInstance() {
        static RandomNumbers instance;
        return instance;
    }

    float getDistribution(const std::string& name, float min = 1.0f, float max = 1.0f) {
        if (distributions.find(name) == distributions.end()) {
            distributions[name] = std::uniform_real_distribution<float>(min, max);
        }
        return distributions[name](gen);
    }
};