// KablunkEngine2020.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "Window.h"
#include "math.h"
#include <iostream>
#include <Windows.h>



#define LOG(x) std::cout << x << std::endl;


int main()
{

    using namespace kablunkEngine;
    using namespace graphics;
    using namespace math;

    
    vec3 a = vec3(1.0f, 2.0f, 0.0f);
    vec3 b = vec3(5.0f, 2.0f, 6.0f);

    vec3 c = a + b;

    Window window = Window("KablunkEngine", 800, 600);
    glClearColor(.2f, .3f, .8f, 1.0f);

    while (!window.closed()) {
        window.clear();
        
        if (window.isKeyPressed(GLFW_KEY_A)) {
            LOG("PRESSED");
        }

        double x, y = 0;
        window.getMousePosition(x, y);
        std::cout << a << std::endl;
        std::cout << b << std::endl;
        std::cout << c << std::endl;
        std::cout << x << ", " << y << std::endl;
        
        
        window.update();
    }

    return 0;

}
