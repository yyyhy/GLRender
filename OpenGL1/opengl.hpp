
#ifndef OPENGL_H
#define OPENGL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include"settings.hpp"

#define GL_INIT glfwInit(); \
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); \
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); \
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); \
				glfwWindowHint(GLFW_SAMPLES, 4); 

#define GL_CHECK_CREATE_WINDOW(window) if (window == NULL) \
                                    { \
                                        std::cout << "Failed to create GLFW window" << std::endl; \
                                        glfwTerminate(); \
                                        return -1; \
                                    }

#define GL_CHECK_GLAD if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) \
                        { \
                            std::cout << "Failed to initialize GLAD" << std::endl; \
                            return -1; \
                        }


#define RENDER_MAIN_LOOP(w) while (!glfwWindowShouldClose(w))

#define CLEAR_TEXTURE2D_BIND glBindTexture(GL_TEXTURE_2D, 0)

#endif // OPENGL_H
