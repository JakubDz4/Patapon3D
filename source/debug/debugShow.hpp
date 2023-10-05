#pragma once
#include <glew.h>
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../Basic_OpenGL/Shader.h"
#include <chrono>

struct trackerElement
{
    unsigned int count = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> time;

    auto getTime()
    {
        return time.time_since_epoch() / count;
    }
};

class Debug
{
    static std::map<std::string, trackerElement> tracker;


public:
    static void drawCircle(glm::vec3 center, float radius, int segments) {
        std::vector<float> vertices;

        for (int i = 0; i < segments; ++i) {
            float theta = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(segments);
            float x = radius * cosf(theta);
            float y = radius * sinf(theta);

            vertices.push_back(center.x + x);
            vertices.push_back(center.y + y);
            vertices.push_back(center.z);
        }

        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        glDrawArrays(GL_LINE_LOOP, 0, segments);

        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    class Timer
    {
       std::chrono::time_point<std::chrono::high_resolution_clock> start;
       std::string name;
    public:
        Timer(const string& _name) :name(_name)
        {
            start = std::chrono::high_resolution_clock::now();
        }

        ~Timer()
        {
            stop();
        }

        inline void stop()
        {
            auto end = std::chrono::high_resolution_clock::now();

            auto duration = end - start;

            Debug::tracker[name].time += duration;
            Debug::tracker[name].count += 1;
        }

        static void showResults()
        {
            for (auto& func : tracker)
            {
                std::cout << "Function: " << func.first << " Average_runTime: " << func.second.getTime() << std::endl;
            }
        }
    };

};

std::map<std::string, trackerElement> Debug::tracker;