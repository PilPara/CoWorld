/**
 * @file main.cpp
 * @brief Entry point for the CowSandBox 3D graphics application.
 * 
 * Initializes and runs the main Application instance, which manages
 * the entire graphics engine including OpenGL context, scene rendering,
 * input handling, and animation systems.
 */

#include "application.hpp"

/**
 * @brief Main entry point for the application.
 * 
 * Creates and runs the main Application instance. The application
 * handles all initialization, main loop, and cleanup automatically.
 * 
 * @return Exit code (0 for successful execution)
 */
int main() {

  Application *app = new Application();
  app->Run();
  return 0;
}
