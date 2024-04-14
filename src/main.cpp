#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "headers/triangle.h"

// Create a 800 x 600 window
const int WIDTH = 800;
const int HEIGHT = 600;

int main() {
    VulkanStarterTriangle app(WIDTH, HEIGHT);

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
