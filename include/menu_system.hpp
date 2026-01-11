/**
 * @file menu_system.hpp
 * @brief Declaration of the ImGui-based menu system.
 *
 * Provides an in-game menu overlay for toggling settings, quitting,
 * showing help, and adjusting lighting parameters.
 */

#ifndef MENU_SYSTEM_HPP
#define MENU_SYSTEM_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/glm.hpp>

class Scene; // forward declaration

/**
 * @brief Different pages in the menu.
 */
enum class MenuPage { MAIN, HELP, LIGHTING };

/**
 * @brief Main ImGui-based menu system.
 */
class MenuSystem {
public:
  MenuSystem();
  ~MenuSystem();

  /**
   * @brief Initialize ImGui context and setup state.
   * @param window GLFW window handle
   */
  void Initialize(GLFWwindow *window);

  /**
   * @brief Shutdown ImGui and free resources.
   */
  void Shutdown();

  /**
   * @brief Set menu active/inactive.
   * @param active Whether the menu is active
   */
  void SetActive(bool active);

  bool IsActive() const { return menuActive; }

  /**
   * @brief Toggle menu visibility.
   */
  void Toggle();

  /**
   * @brief Render menu UI.
   * @param scene Pointer to scene (for light controls)
   */
  void Render(Scene *scene);

private:
  GLFWwindow *window = nullptr;          ///< Associated GLFW window
  bool menuActive = false;               ///< Whether menu input is enabled
  bool collapsed = true;                 ///< Whether menu is collapsed
  MenuPage currentPage = MenuPage::MAIN; ///< Current page displayed

  // Lighting control state
  bool wireframeEnabled = false;
  bool houseLightEnabled = true;

  glm::vec3 pointLightPosition;
  glm::vec3 pointLightAmbient;
  glm::vec3 pointLightDiffuse;
  glm::vec3 pointLightSpecular;

  glm::vec3 defaultPointLightPosition;
  glm::vec3 defaultPointLightAmbient;
  glm::vec3 defaultPointLightDiffuse;
  glm::vec3 defaultPointLightSpecular;

  glm::vec3 defaultDirLightDirection;
  glm::vec3 defaultDirLightAmbient;
  glm::vec3 defaultDirLightDiffuse;
  glm::vec3 defaultDirLightSpecular;

  // Internal rendering helpers
  void RenderMainMenu(Scene *scene);
  void RenderHelpPage();
  void RenderLightingPage(Scene *scene);
  void RenderRenderSettings();
  void RenderDirectionalLight(Scene *scene);
  void RenderPointLights(Scene *scene);
};

#endif
