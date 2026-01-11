/**
 * @file menu_system.cpp
 * @brief Implementation of the ImGui-based debug and control menu system.
 * 
 * Provides an in-game overlay menu for debugging, lighting control, animation
 * testing, and scene manipulation. Uses Dear ImGui for immediate mode GUI
 * rendering with OpenGL backend integration.
 */

#include "menu_system.hpp"
#include "scene.hpp"
#include "config.hpp"
#include <iostream>

// ================================================================================
// Lifecycle Management
// ================================================================================

/**
 * @brief Default constructor - initializes basic state.
 */
MenuSystem::MenuSystem() {}

/**
 * @brief Destructor - ensures proper ImGui cleanup.
 */
MenuSystem::~MenuSystem() { Shutdown(); }

/**
 * @brief Initialize ImGui context and OpenGL backend.
 * 
 * Sets up Dear ImGui with OpenGL3 and GLFW backends, configures default
 * styling, and initializes all menu control variables to their default
 * values from the configuration system.
 * 
 * @param window GLFW window handle for ImGui integration
 */
void MenuSystem::Initialize(GLFWwindow *window) {
  this->window = window;

  // Setup ImGui context and backends
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // Configure ImGui appearance and backends
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(Config::Menu::IMGUI_GL_VERSION);

  // Initialize menu state (start collapsed and inactive)
  collapsed = true;
  SetActive(false);

  // Initialize point light controls with config defaults
  defaultPointLightPosition = Config::Lighting::HOUSE_LIGHT_POS;
  defaultPointLightAmbient = glm::vec3(Config::Lighting::HOUSE_AMBIENT);
  defaultPointLightDiffuse = glm::vec3(Config::Lighting::HOUSE_DIFFUSE);
  defaultPointLightSpecular = glm::vec3(Config::Lighting::HOUSE_SPECULAR);

  pointLightPosition = defaultPointLightPosition;
  pointLightAmbient = defaultPointLightAmbient;
  pointLightDiffuse = defaultPointLightDiffuse;
  pointLightSpecular = defaultPointLightSpecular;

  // Initialize directional light controls with config defaults
  defaultDirLightDirection =
      glm::normalize(Config::Lighting::DEFAULT_DIR_LIGHT);
  defaultDirLightAmbient = glm::vec3(Config::Lighting::DIR_AMBIENT);
  defaultDirLightDiffuse = glm::vec3(Config::Lighting::DIR_DIFFUSE);
  defaultDirLightSpecular = glm::vec3(Config::Lighting::DIR_SPECULAR);
}

/**
 * @brief Clean up ImGui resources and contexts.
 * 
 * Safely shuts down ImGui backends and destroys the context.
 * Safe to call multiple times.
 */
void MenuSystem::Shutdown() {
  if (ImGui::GetCurrentContext()) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }
}

// ================================================================================
// Menu State Management
// ================================================================================

void MenuSystem::SetActive(bool active) {
  menuActive = active;
  if (window) {
    // Switch mouse cursor visibility depending on menu state
    glfwSetInputMode(window, GLFW_CURSOR,
                     menuActive ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
  }
}

void MenuSystem::Toggle() {
  if (menuActive) {
    collapsed = true;
    SetActive(false);
  } else {
    collapsed = false;
    SetActive(true);
  }
}

// -------------------- Rendering --------------------

void MenuSystem::Render(Scene *scene) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  RenderMainMenu(scene);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MenuSystem::RenderMainMenu(Scene *scene) {
  bool open = true;
  ImGuiWindowFlags flags = Config::Menu::WINDOW_FLAGS;

  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 0.0f));

  if (collapsed) {
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
  }

  // Root menu window
  if (ImGui::Begin(Config::Menu::WINDOW_TITLE, &open, flags)) {
    if (!collapsed) {
      switch (currentPage) {
      case MenuPage::MAIN:
        // Main page with Quit/Help/Lighting buttons
        if (ImGui::Button(Config::Menu::BUTTON_QUIT)) {
          glfwSetWindowShouldClose(window, true);
        }
        if (ImGui::Button(Config::Menu::BUTTON_HELP)) {
          currentPage = MenuPage::HELP;
        }
        if (ImGui::Button(Config::Menu::BUTTON_LIGHTING)) {
          currentPage = MenuPage::LIGHTING;
        }
        break;

      case MenuPage::HELP:
        RenderHelpPage();
        break;

      case MenuPage::LIGHTING:
        RenderLightingPage(scene);
        break;
      }
    }
  }
  ImGui::End();
}

void MenuSystem::RenderHelpPage() {
  // Draw help text sections from config arrays
  ImGui::Text(Config::Menu::HELP_TITLE);

  ImGui::Separator();
  ImGui::Text(Config::Menu::HELP_CAMERA_TITLE);
  for (auto &line : Config::Menu::HELP_CAMERA_LINES) {
    ImGui::BulletText("%s", line);
  }

  ImGui::Separator();
  ImGui::Text(Config::Menu::HELP_COW_TITLE);
  for (auto &line : Config::Menu::HELP_COW_LINES) {
    ImGui::BulletText("%s", line);
  }

  ImGui::Separator();
  ImGui::Text(Config::Menu::HELP_GENERAL_TITLE);
  for (auto &line : Config::Menu::HELP_GENERAL_LINES) {
    ImGui::BulletText("%s", line);
  }

  ImGui::Separator();
  if (ImGui::Button(Config::Menu::BUTTON_BACK)) {
    currentPage = MenuPage::MAIN;
  }
}

void MenuSystem::RenderLightingPage(Scene *scene) {
  // Render lighting sub-menus
  RenderRenderSettings();
  RenderDirectionalLight(scene);
  RenderPointLights(scene);

  ImGui::Separator();
  if (ImGui::Button(Config::Menu::BUTTON_BACK)) {
    currentPage = MenuPage::MAIN;
  }
}

void MenuSystem::RenderRenderSettings() {
  // Wireframe toggle
  ImGui::Text(Config::Menu::RENDER_SETTINGS_TITLE);
  if (ImGui::Checkbox(Config::Menu::WIREFRAME_LABEL, &wireframeEnabled)) {
    glPolygonMode(GL_FRONT_AND_BACK, wireframeEnabled ? GL_LINE : GL_FILL);
  }
  ImGui::Separator();
}

void MenuSystem::RenderDirectionalLight(Scene *scene) {
  if (!scene)
    return;

  ImGui::Text(Config::Menu::DIR_LIGHT_TITLE);
  auto &dirLight = scene->directionalLight;

  // Edit directional light properties live
  ImGui::DragFloat3(Config::Menu::DIR_LIGHT_DIRECTION,
                    (float *)&dirLight.direction, Config::Menu::DRAG_SPEED);
  ImGui::ColorEdit3(Config::Menu::DIR_LIGHT_AMBIENT,
                    (float *)&dirLight.ambient);
  ImGui::ColorEdit3(Config::Menu::DIR_LIGHT_DIFFUSE,
                    (float *)&dirLight.diffuse);
  ImGui::ColorEdit3(Config::Menu::DIR_LIGHT_SPECULAR,
                    (float *)&dirLight.specular);

  // Reset button
  if (ImGui::Button(Config::Menu::BUTTON_RESET_DIR)) {
    dirLight.direction = defaultDirLightDirection;
    dirLight.ambient = glm::vec4(defaultDirLightAmbient, 1.0f);
    dirLight.diffuse = glm::vec4(defaultDirLightDiffuse, 1.0f);
    dirLight.specular = glm::vec4(defaultDirLightSpecular, 1.0f);
  }

  ImGui::Separator();
}

void MenuSystem::RenderPointLights(Scene *scene) {
  if (!scene)
    return;

  ImGui::Text(Config::Menu::POINT_LIGHT_TITLE);

  // Toggle house point light
  if (ImGui::Checkbox(Config::Menu::POINT_LIGHT_ENABLE, &houseLightEnabled)) {
    scene->ClearPointLights();
    if (houseLightEnabled) {
      PointLight houseLight;
      houseLight.position = pointLightPosition;
      houseLight.k0 = Config::Lighting::HOUSE_LIGHT_K0;
      houseLight.k1 = Config::Lighting::HOUSE_LIGHT_K1;
      houseLight.k2 = Config::Lighting::HOUSE_LIGHT_K2;
      houseLight.ambient = glm::vec4(pointLightAmbient, 1.0f);
      houseLight.diffuse = glm::vec4(pointLightDiffuse, 1.0f);
      houseLight.specular = glm::vec4(pointLightSpecular, 1.0f);
      scene->AddPointLight(houseLight);
    }
  }

  // If enabled, allow live editing of point light parameters
  if (!scene->pointLights.empty()) {
    PointLight &pl = scene->pointLights[0];

    ImGui::DragFloat3(Config::Menu::POINT_LIGHT_POSITION,
                      (float *)&pointLightPosition, Config::Menu::DRAG_SPEED);
    pl.position = pointLightPosition;

    ImGui::ColorEdit3(Config::Menu::POINT_LIGHT_AMBIENT,
                      (float *)&pointLightAmbient);
    pl.ambient = glm::vec4(pointLightAmbient, 1.0f);

    ImGui::ColorEdit3(Config::Menu::POINT_LIGHT_DIFFUSE,
                      (float *)&pointLightDiffuse);
    pl.diffuse = glm::vec4(pointLightDiffuse, 1.0f);

    ImGui::ColorEdit3(Config::Menu::POINT_LIGHT_SPECULAR,
                      (float *)&pointLightSpecular);
    pl.specular = glm::vec4(pointLightSpecular, 1.0f);

    // Reset button
    if (ImGui::Button(Config::Menu::BUTTON_RESET_POINT)) {
      pointLightPosition = defaultPointLightPosition;
      pointLightAmbient = defaultPointLightAmbient;
      pointLightDiffuse = defaultPointLightDiffuse;
      pointLightSpecular = defaultPointLightSpecular;

      pl.position = defaultPointLightPosition;
      pl.ambient = glm::vec4(defaultPointLightAmbient, 1.0f);
      pl.diffuse = glm::vec4(defaultPointLightDiffuse, 1.0f);
      pl.specular = glm::vec4(defaultPointLightSpecular, 1.0f);
    }
  }

  ImGui::Separator();
}
