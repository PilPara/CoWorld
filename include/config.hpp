#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <array>
#include <imgui.h>

namespace Config {

// ---------------- OpenGL Context ----------------
namespace GL {
constexpr int CONTEXT_VERSION_MAJOR = 3;
constexpr int CONTEXT_VERSION_MINOR = 3;
constexpr int PROFILE = GLFW_OPENGL_CORE_PROFILE;
#ifdef __APPLE__
constexpr int FORWARD_COMPAT = GL_TRUE;
#else
constexpr int FORWARD_COMPAT = GL_FALSE;
#endif
} // namespace GL

// ---------------- Window ----------------
namespace Window {
constexpr int DEFAULT_WIDTH = 1280;
constexpr int DEFAULT_HEIGHT = 720;
constexpr const char *TITLE = "Cow World";
} // namespace Window

// ---------------- Render ----------------
namespace Render {
constexpr float CLEAR_COLOR_R = 0.2f;
constexpr float CLEAR_COLOR_G = 0.3f;
constexpr float CLEAR_COLOR_B = 0.3f;
constexpr float CLEAR_COLOR_A = 1.0f;
} // namespace Render

// ---------------- Camera ----------------
namespace Camera {
constexpr float DEFAULT_FOV = 45.0f;
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 1000.0f;
constexpr glm::vec3 UP_DIR{0.0f, 1.0f, 0.0f};

// Movement settings
constexpr float ACCELERATION = 150.0f;
constexpr float DAMPING = 5.0f;
constexpr float MAX_SPEED = 10.0f;
constexpr float FAST_MULTIPLIER = 10.0f;
constexpr float MOUSE_SENSITIVITY = 4.0f;
constexpr float FAST_COEF = 3; ///< Speed multiplier for fast mode

// Collision constraints
constexpr float MIN_COW_DISTANCE = 2.0f;
constexpr float COLLISION_RADIUS = 0.2f;
constexpr float MIN_HEIGHT = 0.5f;

// Free cam defaults
const glm::vec3 FREE_POS{10.0f, 5.0f, 10.0f};
const glm::vec3 FREE_TARGET{0.0f, 0.0f, 0.0f};

// Follow cam offset
const glm::vec3 FOLLOW_OFFSET{5.0f, 3.0f, 5.0f};

// POV offsets
const glm::vec3 POV_EYE_OFFSET{0.0f, 0.5f, 0.0f};
const glm::vec3 POV_LOOK_OFFSET{0.0f, 0.5f, 1.0f};

// Misc
constexpr int POV_INDEX = 2;
} // namespace Camera

// ---------------- Cow ----------------
namespace Cow {
constexpr float MOVEMENT_SPEED = 0.03f;
constexpr float GROUND_LEVEL = 0.05f;
const glm::vec3 DEFAULT_POSITION{0.0f, GROUND_LEVEL, 0.0f};
const glm::vec3 DEFAULT_SIZE{1.0f};
const glm::vec3 EYE_OFFSET{0.0f, 0.5f, 0.0f};
} // namespace Cow

// ---------------- Animation ----------------
namespace Animation {
constexpr int MAX_BONE_INFLUENCE = 4;
constexpr float DEFAULT_TICKS_PER_SECOND = 25.0f;
constexpr int MAX_BONES = 100;

// Vertex layout
constexpr int VERTEX_FLOATS = 8;
constexpr int ATTRIB_POSITION = 0;
constexpr int ATTRIB_NORMAL = 1;
constexpr int ATTRIB_TEXCOORD = 2;
constexpr int ATTRIB_BONE_IDS = 3;
constexpr int ATTRIB_WEIGHTS = 4;

// Material defaults
constexpr float DEFAULT_SHININESS = 0.5f;
constexpr float DEFAULT_TEXCOORD = 0.0f;

// Animator defaults
constexpr float EPSILON_TIME_DELTA = 0.001f;

// Bone filters
inline const std::array<const char *, 3> ALLOWED_BONES = {"DEF-", "head",
                                                          "tail.001"};
inline const std::array<const char *, 8> HEAD_BONE_FILTERS = {
    "DEF-head", "DEF-skull", "DEF-jaw",      "DEF-nose",
    "DEF-ear",  "DEF-neck",  "DEF-Bone.001", "DEF-Bone.002"};
inline const std::array<const char *, 1> TAIL_BONE_FILTERS = {"DEF-tail"};
} // namespace Animation

// ---------------- Static Mesh ----------------
namespace StaticMesh {
constexpr int VERTEX_FLOATS = 8;
constexpr int ATTRIB_POSITION = 0;
constexpr int ATTRIB_NORMAL = 1;
constexpr int ATTRIB_TEXCOORD = 2;
} // namespace StaticMesh

// ---------------- Plane ----------------
namespace Plane {
constexpr int VERTEX_FLOATS = 8;
constexpr float SIZE = 1000.0f;
constexpr float TEX_SCALE = 500.0f;
constexpr const char *TEXTURE_FILE = "grass.jpg";
} // namespace Plane

// ---------------- Lighting ----------------
namespace Lighting {
const glm::vec3 DEFAULT_DIR_LIGHT{-0.3f, -1.0f, -0.2f};
const glm::vec4 DIR_AMBIENT{0.2f, 0.2f, 0.2f, 1.0f};
const glm::vec4 DIR_DIFFUSE{0.8f, 0.8f, 0.7f, 1.0f};
const glm::vec4 DIR_SPECULAR{1.0f, 1.0f, 0.9f, 1.0f};

// House light
const glm::vec3 HOUSE_LIGHT_POS{-3.0f, 5.5f, -7.0f};
constexpr float HOUSE_LIGHT_K0 = 0.7f;
constexpr float HOUSE_LIGHT_K1 = 0.08f;
constexpr float HOUSE_LIGHT_K2 = 0.02f;
const glm::vec4 HOUSE_AMBIENT{0.08f, 0.06f, 0.04f, 1.0f};
const glm::vec4 HOUSE_DIFFUSE{1.2f, 1.0f, 0.6f, 1.0f};
const glm::vec4 HOUSE_SPECULAR{0.8f, 0.6f, 0.4f, 1.0f};
} // namespace Lighting

// ---------------- Assets ----------------
namespace Assets {
constexpr const char *COW_MODEL =
    "assets/models/AnimatedCow/gltf/CowBlackAnimatedHeadTail.gltf";
constexpr const char *FARMHOUSE_MODEL =
    "assets/models/FarmHouse/farm_house/scene.gltf";
constexpr const char *TRACTOR_MODEL = "assets/models/tractor/scene.gltf";
constexpr const char *SHED_MODEL =
    "assets/models/metal_storage_building/scene.gltf";
constexpr const char *MILK_CAN_MODEL =
    "assets/models/aliminum_milk_can/scene.gltf";
constexpr const char *GRASS_TEXTURE = "assets/grass.jpg";

const std::array<const char *, 6> SKYBOX_FACES = {
    "assets/models/faces/right.png", "assets/models/faces/left.png",
    "assets/models/faces/top.png",   "assets/models/faces/bottom.png",
    "assets/models/faces/front.png", "assets/models/faces/back.png"};
} // namespace Assets

// ---------------- Model Transforms ----------------
namespace ModelTransforms {
const glm::vec3 FARMHOUSE_POS{0.0f, 4.87f, -10.0f};
const glm::vec3 FARMHOUSE_SCALE{5.0f, 5.0f, 5.0f};
const glm::vec3 FARMHOUSE_ROTATION{-90.0f, 0.0f, 0.0f};

const glm::vec3 TRACTOR_POS{-8.0f, 0.0f, -20.0f};
const glm::vec3 TRACTOR_SCALE{0.002f};
const glm::vec3 TRACTOR_ROTATION{-90.0f, 0.0f, 0.0f};

const glm::vec3 SHED_POS{-8.0f, 0.2f, -60.0f};
const glm::vec3 SHED_SCALE{0.10f};
const glm::vec3 SHED_ROTATION{-90.0f, 0.0f, 0.0f};

// Milk cans
constexpr int MILK_CAN_COUNT = 4;
const glm::vec3 MILK_CAN_BASE_POS{5.0f, 0.5f, -5.7f};
const glm::vec3 MILK_CAN_SCALE{0.05f};
const glm::vec3 MILK_CAN_OFFSET{-1.0f, 0.0f, 0.0f};
} // namespace ModelTransforms

// ---------------- Collision ----------------
namespace Collision {
constexpr float COW_BUILDING_MARGIN = 0.2f;
constexpr float COW_COLLISION_MARGIN = 0.05f;
constexpr float CAMERA_COLLISION_MARGIN = 0.05f;
} // namespace Collision

// ---------------- Scene ----------------
namespace Scene {
constexpr const char *STATIC_VERTEX_SHADER = "shaders/object.vs";
constexpr const char *STATIC_FRAGMENT_SHADER = "shaders/object.glsl";
constexpr const char *ANIMATED_VERTEX_SHADER = "shaders/animated_object.glsl";
constexpr const char *ANIMATED_FRAGMENT_SHADER = "shaders/object.glsl";

// Animation keys
constexpr const char *ANIM_IDLE_PRIMARY = "Cow idle 1 (1_140)";
constexpr const char *ANIM_WALK_PRIMARY = "Cow Walk (1_25)";
constexpr const char *ANIM_IDLE_FALLBACK = "Idle";
constexpr const char *ANIM_WALK_FALLBACK = "Cow idle 1 (1_140)";

// Head controls
constexpr const char *ANIM_HEAD_UP = "LookUp";
constexpr const char *ANIM_HEAD_DOWN = "LookDown.001";
constexpr const char *ANIM_HEAD_LEFT = "LookLeft";
constexpr const char *ANIM_HEAD_RIGHT = "LookRight";

// Tail controls
constexpr const char *ANIM_TAIL_UP = "TailUp";
constexpr const char *ANIM_TAIL_LEFT = "TailLeft";
constexpr const char *ANIM_TAIL_RIGHT = "TailRight";

// Camera indices
constexpr int FREE_CAMERA_INDEX = 0;
constexpr int FOLLOW_CAMERA_INDEX = 1;
constexpr int POV_CAMERA_INDEX = 2;

// Log & error messages
constexpr const char *ERR_COW_LOAD =
    "[ERROR::SCENE] Failed to create animated cow";
constexpr const char *LOG_CAM_SWITCH = "[SCENE] Switched to camera index ";

} // namespace Scene

// ---------------- Shader ----------------
namespace Shader {
constexpr const char *ERR_CREATE =
    "[ERROR::SHADER::CREATE] Failed to create shader";
constexpr const char *ERR_FILE = "[ERROR::SHADER::FILE] Could not open file: ";
constexpr const char *ERR_VERTEX = "[ERROR::SHADER::VERTEX]";
constexpr const char *ERR_FRAGMENT = "[ERROR::SHADER::FRAGMENT]";
constexpr const char *ERR_LINK = "[ERROR::SHADER::LINKING]";
constexpr const char *ERR_RUNTIME = "Failed to create shader program";

// Uniforms
constexpr const char *UNIFORM_VP = "vp";
constexpr const char *UNIFORM_VIEW_POS = "viewPos";
constexpr const char *UNIFORM_BONES = "finalBonesMatrices";
constexpr const char *UNIFORM_POINT_CNT = "nPointLights";
constexpr const char *UNIFORM_SPOT_CNT = "nSpotLights";

// Limits
constexpr int LOG_BUF_SIZE = 1024;
constexpr int MAX_POINT_LIGHTS = 21;
constexpr int MAX_SPOT_LIGHTS = 5;
} // namespace Shader

// ---------------- Texture ----------------
namespace Texture {
constexpr const char *ERR_LOAD =
    "[ERROR::TEXTURE::LOADING] Failed to load texture: ";
constexpr GLenum DEFAULT_WRAP_S = GL_REPEAT;
constexpr GLenum DEFAULT_WRAP_T = GL_REPEAT;
constexpr GLenum DEFAULT_MAG_FILTER = GL_LINEAR;
constexpr GLenum DEFAULT_MIN_FILTER = GL_LINEAR_MIPMAP_LINEAR;
} // namespace Texture

namespace Menu {
constexpr const char *IMGUI_GL_VERSION = "#version 330 core";

// Window
constexpr const char *WINDOW_TITLE = "CowWorld Menu - Press M to toggle menu";
constexpr ImGuiWindowFlags WINDOW_FLAGS =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;

// Buttons
constexpr const char *BUTTON_QUIT = "Quit";
constexpr const char *BUTTON_HELP = "Help";
constexpr const char *BUTTON_LIGHTING = "Change Lighting";
constexpr const char *BUTTON_BACK = "Back";
constexpr const char *BUTTON_RESET_DIR = "Reset Directional Light";
constexpr const char *BUTTON_RESET_POINT = "Reset Point Light";

// Help page
constexpr const char *HELP_TITLE = "Controls Overview";
constexpr const char *HELP_CAMERA_TITLE = "Camera Controls";
constexpr const char *HELP_COW_TITLE = "Cow Controls";
constexpr const char *HELP_GENERAL_TITLE = "General Controls";

inline const std::array<const char *, 5> HELP_CAMERA_LINES = {
    "W / A / S / D: Move the free-fly camera.", "Q / E: Move camera down / up.",
    "F: Hold for faster movement.", "TAB: Switch cameras (free, follow, POV).",
    "Mouse: Rotate free-fly and POV cameras."};

inline const std::array<const char *, 3> HELP_COW_LINES = {
    "Arrow Keys: Move the cow.", "SPACE (POV mode): Cow moves forward.",
    "1/2/3/4: Head animations. Z/X/C: Tail animations."};

inline const std::array<const char *, 3> HELP_GENERAL_LINES = {
    "M: Toggle menu.", "ESC: Quit.",
    "Wireframe Mode: Toggle in Lighting menu."};

// Lighting labels
constexpr const char *RENDER_SETTINGS_TITLE = "Render Settings";
constexpr const char *WIREFRAME_LABEL = "Wireframe Mode";

constexpr const char *DIR_LIGHT_TITLE = "Directional Light";
constexpr const char *DIR_LIGHT_DIRECTION = "Direction";
constexpr const char *DIR_LIGHT_AMBIENT = "Ambient";
constexpr const char *DIR_LIGHT_DIFFUSE = "Diffuse";
constexpr const char *DIR_LIGHT_SPECULAR = "Specular";

constexpr const char *POINT_LIGHT_TITLE = "Point Lights";
constexpr const char *POINT_LIGHT_ENABLE = "Enable Window Light";
constexpr const char *POINT_LIGHT_POSITION = "Position (PL)";
constexpr const char *POINT_LIGHT_AMBIENT = "Ambient (PL)";
constexpr const char *POINT_LIGHT_DIFFUSE = "Diffuse (PL)";
constexpr const char *POINT_LIGHT_SPECULAR = "Specular (PL)";

// Controls
constexpr float DRAG_SPEED = 0.1f;
} // namespace Menu

} // namespace Config

#endif
