/*
 Copyright 2018-2019 Google Inc.
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include "vkex/Application.h"
#include "vkex/Bitmap.h"
#include "vkex/Util.h"
#include "vkex/VulkanUtil.h"

#if defined(VKEX_LINUX_WAYLAND)
#elif defined(VKEX_LINUX_XCB)
# include <X11/Xlib-xcb.h>
#elif defined(VKEX_LINUX_XLIB)
#endif

#if defined(VKEX_LINUX)
# include <sys/utsname.h>
#endif

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_vulkan.h>

#include <map>

namespace vkex {

const VkFormat          kDefaultSwapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
const VkColorSpaceKHR   kDefaultColorSpace      = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
const VkPresentModeKHR  kDefaultPresentMode     = VK_PRESENT_MODE_IMMEDIATE_KHR;

enum {
  kDefaultPhysicalDeviceIndex = 0,
  kDefaultQueueIndex          = 0,
  kDefaultInFlightFrameCount  = 2,
};

static std::map<int32_t, int32_t> sKeyboardMapGlfwToVkex = {
  { GLFW_KEY_SPACE,               kKeySpace	          },
  { GLFW_KEY_APOSTROPHE,          kKeyApostrophe      }, /* ' */
  { GLFW_KEY_COMMA,               kKeyComma	          }, /*,  */
  { GLFW_KEY_MINUS,               kKeyMinus	          }, /* - */
  { GLFW_KEY_PERIOD,              kKeyPeriod	        }, /* . */
  { GLFW_KEY_SLASH,               kKeySlash	          }, /* / */
  { GLFW_KEY_0,                   kKey0	              },
  { GLFW_KEY_1,                   kKey1	              },
  { GLFW_KEY_2,                   kKey2	              },
  { GLFW_KEY_3,                   kKey3	              },
  { GLFW_KEY_4,                   kKey4	              },
  { GLFW_KEY_5,                   kKey5	              },
  { GLFW_KEY_6,                   kKey6	              },
  { GLFW_KEY_7,                   kKey7	              },
  { GLFW_KEY_8,                   kKey8	              },
  { GLFW_KEY_9,                   kKey9	              },
  { GLFW_KEY_SEMICOLON,           kKeySemicolon       }, /* ; */
  { GLFW_KEY_EQUAL,               kKeyEqual	          }, /* = */
  { GLFW_KEY_A,                   kKeyA	              },
  { GLFW_KEY_B,                   kKeyB	              },
  { GLFW_KEY_C,                   kKeyC	              },
  { GLFW_KEY_D,                   kKeyD	              },
  { GLFW_KEY_E,                   kKeyE	              },
  { GLFW_KEY_F,                   kKeyF	              },
  { GLFW_KEY_G,                   kKeyG	              },
  { GLFW_KEY_H,                   kKeyH	              },
  { GLFW_KEY_I,                   kKeyI	              },
  { GLFW_KEY_J,                   kKeyJ	              },
  { GLFW_KEY_K,                   kKeyK	              },
  { GLFW_KEY_L,                   kKeyL	              },
  { GLFW_KEY_M,                   kKeyM	              },
  { GLFW_KEY_N,                   kKeyN	              },
  { GLFW_KEY_O,                   kKeyO	              },
  { GLFW_KEY_P,                   kKeyP	              },
  { GLFW_KEY_Q,                   kKeyQ	              },
  { GLFW_KEY_R,                   kKeyR	              },
  { GLFW_KEY_S,                   kKeyS	              },
  { GLFW_KEY_T,                   kKeyT	              },
  { GLFW_KEY_U,                   kKeyU	              },
  { GLFW_KEY_V,                   kKeyV	              },
  { GLFW_KEY_W,                   kKeyW	              },
  { GLFW_KEY_X,                   kKeyX	              },
  { GLFW_KEY_Y,                   kKeyY	              },
  { GLFW_KEY_Z,                   kKeyZ	              },
  { GLFW_KEY_LEFT_BRACKET,        kKeyLeftBracket     }, /* [ */
  { GLFW_KEY_BACKSLASH,           kKeyBackslash       }, /* \ */
  { GLFW_KEY_RIGHT_BRACKET,       kKeyRightBracket    }, /* ] */
  { GLFW_KEY_GRAVE_ACCENT,        kKeyGraveAccent     }, /* ` */
  { GLFW_KEY_WORLD_1,             kKeyWorld_1         }, /* non-US #1 */
  { GLFW_KEY_WORLD_2,             kKeyWorld_2         }, /* non-US #2 */
  { GLFW_KEY_ESCAPE,              kKeyEscape	        },
  { GLFW_KEY_ENTER,               kKeyEnter	          },
  { GLFW_KEY_TAB,                 kKeyTab	            },
  { GLFW_KEY_BACKSPACE,           kKeyBackspace       },
  { GLFW_KEY_INSERT,              kKeyInsert	        },
  { GLFW_KEY_DELETE,              kKeyDelete	        },
  { GLFW_KEY_RIGHT,               kKeyRight	          },
  { GLFW_KEY_LEFT,                kKeyLeft	          },
  { GLFW_KEY_DOWN,                kKeyDown	          },
  { GLFW_KEY_UP,                  kKeyUp	            },
  { GLFW_KEY_PAGE_UP,             kKeyPageUp	        },
  { GLFW_KEY_PAGE_DOWN,           kKeyPageDown        },
  { GLFW_KEY_HOME,                kKeyHome	          },
  { GLFW_KEY_END,                 kKeyEnd	            },
  { GLFW_KEY_CAPS_LOCK,           kKeyCapsLock        },
  { GLFW_KEY_SCROLL_LOCK,         kKeyScrollLock      },
  { GLFW_KEY_NUM_LOCK,            kKeyNumLock         },
  { GLFW_KEY_PRINT_SCREEN,        kKeyPrintScreen     },
  { GLFW_KEY_PAUSE,               kKeyPause	          },
  { GLFW_KEY_F1,                  kKeyF1	            },
  { GLFW_KEY_F2,                  kKeyF2	            },
  { GLFW_KEY_F3,                  kKeyF3	            },
  { GLFW_KEY_F4,                  kKeyF4	            },
  { GLFW_KEY_F5,                  kKeyF5	            },
  { GLFW_KEY_F6,                  kKeyF6	            },
  { GLFW_KEY_F7,                  kKeyF7	            },
  { GLFW_KEY_F8,                  kKeyF8	            },
  { GLFW_KEY_F9,                  kKeyF9	            },
  { GLFW_KEY_F10,                 kKeyF10	            },
  { GLFW_KEY_F11,                 kKeyF11	            },
  { GLFW_KEY_F12,                 kKeyF12	            },
  { GLFW_KEY_F13,                 kKeyF13	            },
  { GLFW_KEY_F14,                 kKeyF14	            },
  { GLFW_KEY_F15,                 kKeyF15	            },
  { GLFW_KEY_F16,                 kKeyF16	            },
  { GLFW_KEY_F17,                 kKeyF17	            },
  { GLFW_KEY_F18,                 kKeyF18	            },
  { GLFW_KEY_F19,                 kKeyF19	            },
  { GLFW_KEY_F20,                 kKeyF20	            },
  { GLFW_KEY_F21,                 kKeyF21	            },
  { GLFW_KEY_F22,                 kKeyF22	            },
  { GLFW_KEY_F23,                 kKeyF23	            },
  { GLFW_KEY_F24,                 kKeyF24	            },
  { GLFW_KEY_F25,                 kKeyF25	            },
  { GLFW_KEY_KP_0,                kKeyKeyPad0         },
  { GLFW_KEY_KP_1,                kKeyKeyPad1         },
  { GLFW_KEY_KP_2,                kKeyKeyPad2         },
  { GLFW_KEY_KP_3,                kKeyKeyPad3         },
  { GLFW_KEY_KP_4,                kKeyKeyPad4         },
  { GLFW_KEY_KP_5,                kKeyKeyPad5         },
  { GLFW_KEY_KP_6,                kKeyKeyPad6         },
  { GLFW_KEY_KP_7,                kKeyKeyPad7         },
  { GLFW_KEY_KP_8,                kKeyKeyPad8         },
  { GLFW_KEY_KP_9,                kKeyKeyPad9         },
  { GLFW_KEY_KP_DECIMAL,          kKeyKeyPadDecimal   },
  { GLFW_KEY_KP_DIVIDE,           kKeyKeyPadDivide    },
  { GLFW_KEY_KP_MULTIPLY,         kKeyKeyPadMultiply  },
  { GLFW_KEY_KP_SUBTRACT,         kKeyKeyPadSubtract  },
  { GLFW_KEY_KP_ADD,              kKeyKeyPadAdd       },
  { GLFW_KEY_KP_ENTER,            kKeyKeyPadEnter     },
  { GLFW_KEY_KP_EQUAL,            kKeyKeyPadEqual     },
  { GLFW_KEY_LEFT_SHIFT,          kKeyLeftShift       },
  { GLFW_KEY_LEFT_CONTROL,        kKeyLeftControl     },
  { GLFW_KEY_LEFT_ALT,            kKeyLeftAlt         },
  { GLFW_KEY_LEFT_SUPER,          kKeyLeftSuper       },
  { GLFW_KEY_RIGHT_SHIFT,         kKeyRightShift      },
  { GLFW_KEY_RIGHT_CONTROL,       kKeyRightControl    },
  { GLFW_KEY_RIGHT_ALT,           kKeyRightAlt        },
  { GLFW_KEY_RIGHT_SUPER,         kKeyRightSuper      },
  { GLFW_KEY_MENU,                kKeyMenu	          },  
};

 // =================================================================================================
// WindowEvents
// =================================================================================================
struct WindowEvents {
  static std::unordered_map<GLFWwindow*, Application*> s_windows;

  static void MoveCallback(GLFWwindow* window, int event_x, int event_y) {
    auto it = s_windows.find(window);
    if (it == s_windows.end()) {
      return;
    }

    Application* p_application = it->second;
    p_application->MoveCallback(
      static_cast<int32_t>(event_x),
      static_cast<int32_t>(event_y));
  }
  
  static void ResizeCallback(GLFWwindow* window, int event_width, int event_height) {
    auto it = s_windows.find(window);
    if (it == s_windows.end()) {
      return;
    }

    Application* p_application = it->second;
    p_application->ResizeCallback(
      static_cast<uint32_t>(event_width),
      static_cast<uint32_t>(event_height));
  }

  static void MouseButtonCallback(GLFWwindow* window, int event_button, int event_action, int event_mods) {
    auto it = s_windows.find(window);
    if (it == s_windows.end()) {
      return;
    }

    uint32_t buttons = 0;
    if (event_button == GLFW_MOUSE_BUTTON_LEFT) {
      buttons |= vkex::MouseButton::Left;
    }
    if (event_button == GLFW_MOUSE_BUTTON_RIGHT) {
      buttons |= vkex::MouseButton::Right;
    }
    if (event_button == GLFW_MOUSE_BUTTON_MIDDLE) {
      buttons |= vkex::MouseButton::Middle;
    }

    double event_x;
    double event_y;
    glfwGetCursorPos(window, &event_x, &event_y);

    Application* p_application = it->second;
    if (event_action == GLFW_PRESS) {
      p_application->MouseDownCallback(
        static_cast<int32_t>(event_x),
        static_cast<int32_t>(event_y),
        buttons);
    }
    else if (event_action == GLFW_RELEASE) {
      p_application->MouseUpCallback(
        static_cast<int32_t>(event_x),
        static_cast<int32_t>(event_y),
        buttons);
    }
    ImGui_ImplGlfw_MouseButtonCallback(window, event_button, event_action, event_mods);
  }

  static void MouseMoveCallback(GLFWwindow* window, double event_x, double event_y) {
    auto it = s_windows.find(window);
    if (it == s_windows.end()) {
      return;
    }

    uint32_t buttons = 0;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ) {
      buttons |= vkex::MouseButton::Left;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ) {
      buttons |= vkex::MouseButton::Right;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS ) {
      buttons |= vkex::MouseButton::Middle;
    }

    Application* p_application = it->second;
    p_application->MouseMoveCallback(
      static_cast<int32_t>(event_x),
      static_cast<int32_t>(event_y),
      buttons);
  }

  static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto it = s_windows.find(window);
    if (it == s_windows.end()) {
      return;
    }
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset); 
  }

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto it = s_windows.find(window);
    if (it == s_windows.end()) {
      return;
    }

    bool has_key = (sKeyboardMapGlfwToVkex.find(key) != sKeyboardMapGlfwToVkex.end());
    if (!has_key) {
      VKEX_LOG_WARN("GLFW key not supported, key=" << key);
    }

    if (has_key) {
      KeyboardInput appKey = static_cast<KeyboardInput>(sKeyboardMapGlfwToVkex[key]);

      Application* p_application = it->second;
      if (action == GLFW_PRESS) {
        p_application->KeyDownCallback(appKey);
      } else if (action == GLFW_RELEASE) {
        p_application->KeyUpCallback(appKey);
      }
    }

    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
  }

  static void CharCallback(GLFWwindow* window, unsigned int c) {
    auto it = s_windows.find(window);
    if (it == s_windows.end()) {
      return;
    }
    ImGui_ImplGlfw_CharCallback(window,  c);
  }

  static vkex::Result RegisterWindowEvents(GLFWwindow* window, Application* application)
  {
    auto it = s_windows.find(window);
    if (it != s_windows.end()) {
      return vkex::Result::ErrorWindowEventsAlreadyRegistered;
    }

    glfwSetWindowPosCallback(window, WindowEvents::MoveCallback);
    glfwSetWindowSizeCallback(window, WindowEvents::ResizeCallback);
    glfwSetMouseButtonCallback(window, WindowEvents::MouseButtonCallback);
    glfwSetCursorPosCallback(window, WindowEvents::MouseMoveCallback);
    glfwSetScrollCallback(window, WindowEvents::ScrollCallback);
	  glfwSetKeyCallback(window, WindowEvents::KeyCallback);
    glfwSetCharCallback(window, WindowEvents::CharCallback);

    s_windows[window] = application;

    return vkex::Result::Success;
  }
};

std::unordered_map<GLFWwindow*, Application*> WindowEvents::s_windows;

// =================================================================================================
// Application::RenderData
// =================================================================================================
Application::RenderData::RenderData()
{
}

Application::RenderData::~RenderData()
{
}
       
vkex::Result Application::RenderData::InternalCreate(vkex::Device device, uint32_t frame_index, vkex::CommandBuffer cmd)
{
  m_device = device;
  m_frame_index = frame_index;
  m_work_cmd = cmd;

  // Work complete semaphore
  {
    vkex::SemaphoreCreateInfo semaphore_create_info = {};
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateSemaphore(semaphore_create_info, &m_work_complete_semaphore)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  return vkex::Result::Success;
}

vkex::Result Application::RenderData::InternalDestroy()
{
  // Work complete semaphore
  if (m_work_complete_semaphore != nullptr) {
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroySemaphore(m_work_complete_semaphore);
    );
    if (!vkex_result) {
      return vkex_result;
    }   
  }

  return vkex::Result::Success;
}

// =================================================================================================
// Application::PresentData
// =================================================================================================
Application::PresentData::PresentData()
{
}

Application::PresentData::~PresentData()
{
}
       
vkex::Result Application::PresentData::InternalCreate(vkex::Device device, uint32_t frame_index, vkex::CommandBuffer cmd)
{
  m_device = device;
  m_frame_index = frame_index;
  m_work_cmd = cmd;

  // Image acquired semaphore
  {
    vkex::SemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.object_name = "present_data:image_acquired_semaphore:" + std::to_string(frame_index);
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateSemaphore(semaphore_create_info, &m_image_acquired_sempahore)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }
  // Work complete semaphore
  {
    vkex::SemaphoreCreateInfo semaphore_create_info = {};
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateSemaphore(semaphore_create_info, &m_work_complete_semaphore)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  return vkex::Result::Success;
}

vkex::Result Application::PresentData::InternalDestroy()
{
  // Image acquired semaphore
  if (m_image_acquired_sempahore != nullptr) {
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroySemaphore(m_image_acquired_sempahore);
    );
    if (!vkex_result) {
      return vkex_result;
    }   
  }
  // Work complete semaphore
  if (m_work_complete_semaphore != nullptr) {
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroySemaphore(m_work_complete_semaphore);
    );
    if (!vkex_result) {
      return vkex_result;
    }   
  }

  return vkex::Result::Success;
}

void Application::PresentData::SetRenderPass(vkex::RenderPass render_pass)
{
  m_render_pass = render_pass; 
}
 
// =================================================================================================
// Application
// =================================================================================================
static Application* s_instance = nullptr;

Application::Application(const std::string& name)
{
  s_instance = this;

  m_configuration.name = name;
  if (m_configuration.name.empty()) {
    m_configuration.name = "VKEX Unnamed Application";
  }

  m_configuration.window.width  = 640;
  m_configuration.window.height = 480;
  m_configuration.window.cursor_mode = vkex::CursorMode::Visible;

  m_configuration.swapchain.color_format  = kDefaultSwapchainFormat;
  m_configuration.swapchain.color_space   = kDefaultColorSpace;
  m_configuration.swapchain.present_mode  = kDefaultPresentMode;

  m_configuration.enable_imgui = true;
  m_configuration.enable_screen_shot = false;

  InitializeAssetDirs();
}

Application::Application(uint32_t width, uint32_t height, const std::string& name)
{
  s_instance = this;

  m_configuration.name = name;
  if (m_configuration.name.empty()) {
    m_configuration.name = "VKEX Unnamed Application";
  }

  m_configuration.window.width  = width;
  m_configuration.window.height = height;

  m_configuration.swapchain.color_format  = kDefaultSwapchainFormat;
  m_configuration.swapchain.color_space   = kDefaultColorSpace;
  m_configuration.swapchain.present_mode  = kDefaultPresentMode;

  m_configuration.enable_imgui = true;
  m_configuration.enable_screen_shot = false;

  InitializeAssetDirs();
}

Application::~Application()
{
  s_instance = nullptr;
}

Application* Application::Get()
{
  return s_instance;
}

void Application::InitializeAssetDirs()
{
  fs::path app_path = GetApplicationPath();
  VKEX_LOG_INFO("Application path: " << app_path);
  fs::path base_dir = app_path.parent();  
  AddAssetDir(base_dir);
  size_t n = base_dir.part_count();
  for (size_t i = 0; i < n; ++i) {
    fs::path asset_dir = base_dir / "assets";
    if (fs::exists(asset_dir)) {
      AddAssetDir(asset_dir);
      VKEX_LOG_INFO("Added asset path: " << asset_dir);
    }
    base_dir = base_dir.parent();
  }
}

static const char* ToString(vkex::CursorMode value)
{
  switch (value) {
    case vkex::CursorMode::Visible  : return "Visible"; break;
    case vkex::CursorMode::Hidden   : return "Hidden"; break;
    case vkex::CursorMode::Captured : return "Captured"; break;
  }
  return "<UNKNOWN CURSOR MODE>";
}

vkex::Result Application::InitializeWindow()
{
  if (m_window != nullptr) {
    return vkex::Result::Success;
  }

  // Size
  {
    if (m_configuration.window.width <= 0) {
      return vkex::Result::ErrorInvalidWindowWidth;
    }

    if (m_configuration.window.height <= 0) {
      return vkex::Result::ErrorInvalidWindowHeight;
    }
  }

  // GLFW
  if (IsApplicationModeWindow()) {
    if (!glfwInit()) {    
      return vkex::Result::ErrorGlfwInitializeFailed;;
    }

    // Needed for Vulkan and D3D12
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Resize
    glfwWindowHint(GLFW_RESIZABLE, 
      m_configuration.window.resizeable ? true : false);

    m_window = glfwCreateWindow(
      static_cast<int>(m_configuration.window.width), 
      static_cast<int>(m_configuration.window.height), 
      m_configuration.name.c_str(),
      nullptr, 
      nullptr);

    if (m_window == nullptr) {
      vkex::Result vkex_result = vkex::Result::ErrorGlfwWindowCreateFailed;
      VKEX_ASSERT_MSG(vkex_result, "GLFW window creation failed");
      return vkex_result;
    }

    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      WindowEvents::RegisterWindowEvents(
        m_window, 
        this)
    );
    if (!vkex_result) {
      glfwDestroyWindow(m_window);
      return vkex_result;
    }

    // Cursor mode
    SetCursorMode(m_configuration.window.cursor_mode);

    // Log window creation
    if (IsApplicationModeWindow()) {
      VKEX_LOG_INFO("");
      VKEX_LOG_INFO("Application is running in WINDOW mode");
      VKEX_LOG_INFO("");
      VKEX_LOG_INFO("Window created:");
      VKEX_LOG_INFO("   " << "Name       : " << m_configuration.name);
      VKEX_LOG_INFO("   " << "Size       : " << m_configuration.window.width << "x" << m_configuration.window.height);
      VKEX_LOG_INFO("   " << "Resizable  : " << m_configuration.window.resizeable);
      VKEX_LOG_INFO("   " << "Borderless : " << m_configuration.window.borderless);
      VKEX_LOG_INFO("   " << "Cursor     : " << ToString(m_configuration.window.cursor_mode));
      VKEX_LOG_INFO("");
    }
  }
  else if (IsApplicationModeHeadless()) {
    VKEX_LOG_INFO("");
    VKEX_LOG_INFO("Application is running in HEADLESS mode");
    VKEX_LOG_INFO("");
  }

  return vkex::Result::Success;
}

vkex::Result Application::InitializeVkexDevice()
{
  // Find device
  vkex::PhysicalDevice physical_device = m_instance->FindPhysicalDevice(m_configuration.device_criteria);
  VKEX_ASSERT_MSG(physical_device != nullptr, "unable to find device with specified criteria");

  // Find graphics queue
  uint32_t graphics_queue_family_index = UINT32_MAX;
  {
    auto& queue_family_properties = physical_device->GetQueueFamilyProperties();
    const uint32_t count = CountU32(queue_family_properties);
    for (uint32_t i = 0; i < count; ++i) {
      auto& properties = queue_family_properties[i].queueFamilyProperties;
      if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        graphics_queue_family_index = i;
        break;
      }
    }

    if ((graphics_queue_family_index != UINT32_MAX) && 
        (queue_family_properties[graphics_queue_family_index].queueFamilyProperties.queueCount < 1))
    {
      return vkex::Result::ErrorInvalidQueueCount;
    }
  }

  // Device
  {
    vkex::DeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.queue_type = VK_QUEUE_GRAPHICS_BIT;
    queue_create_info.queue_family_index = graphics_queue_family_index;
    queue_create_info.queue_count = 1;

    vkex::DeviceCreateInfo device_create_info = {};
    device_create_info.physical_device  = physical_device;
    device_create_info.safe_values      = true;
    device_create_info.queue_create_infos.push_back(queue_create_info);
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_instance->CreateDevice(device_create_info, &m_device);
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Queues
  {
    vkex::Result vkex_result = vkex::Result::Undefined;
    // Graphics
    {
      vkex::Result vkex_result = vkex::Result::Undefined;
      VKEX_RESULT_CALL(
        vkex_result,
        m_device->GetQueue(
          VK_QUEUE_GRAPHICS_BIT, 
          graphics_queue_family_index, 
          kDefaultQueueIndex, 
          &m_graphics_queue)
      );
      if (!vkex_result) {
        return vkex_result;
      }
    }
    // Compute
    {
      vkex::Result vkex_result = vkex::Result::Undefined;
      VKEX_RESULT_CALL(
        vkex_result,
        m_device->GetQueue(
          VK_QUEUE_COMPUTE_BIT, 
          graphics_queue_family_index, 
          kDefaultQueueIndex, 
          &m_compute_queue)
      );
      if (!vkex_result) {
        return vkex_result;
      }
    }
    // Transfer
    {
      vkex::Result vkex_result = vkex::Result::Undefined;
      VKEX_RESULT_CALL(
        vkex_result,
        m_device->GetQueue(
          VK_QUEUE_TRANSFER_BIT, 
          graphics_queue_family_index, 
          kDefaultQueueIndex, 
          &m_transfer_queue)
      );
      if (!vkex_result) {
        return vkex_result;
      }
    }
    // Present
    if (IsApplicationModeWindow()) {
      // Paranoid set to nullptr
      m_present_queue = nullptr;
      // Get display info
      vkex::DisplayInfo display_info = {};
      // Note WIN32 doesn't require any connection information.
#if defined(VKEX_LINUX_WAYLAND)
# error "not implemented"
#elif defined(VKEX_LINUX_XCB)
      { 
        // Get connection
        xcb_connection_t* connection = XGetXCBConnection(glfwGetX11Display());
        // Get screen for visual id
        const xcb_setup_t* setup = xcb_get_setup(connection);
        xcb_screen_t* screen = (xcb_setup_roots_iterator(setup)).data;        
        // Display info
        display_info.connection = connection;
        display_info.visual_id  = screen->root_visual;
      }
#elif defined(VKEX_LINUX_XLIB)
# error "not implemented"
#endif      
      
      // Look for a queue that can present
      // Graphics
      if ((m_present_queue == nullptr) && (m_graphics_queue != nullptr)) {
        VkBool32 supported = m_graphics_queue->SupportsPresent(display_info);
        if (supported == VK_TRUE) {
          m_present_queue = m_graphics_queue;
        }
      }
      // Compute
      if ((m_present_queue == nullptr) && (m_compute_queue != nullptr)) {
        VkBool32 supported = m_compute_queue->SupportsPresent(display_info);
        if (supported == VK_TRUE) {
          m_present_queue = m_compute_queue;
        }
      }
      // Transfer
      if ((m_present_queue == nullptr) && (m_transfer_queue != nullptr)) {
        VkBool32 supported = m_transfer_queue->SupportsPresent(display_info);
        if (supported == VK_TRUE) {
          m_present_queue = m_transfer_queue;
        }
      }
    }
  }

  return vkex::Result::Success;
}

vkex::Result Application::InitializeVkexSwapchainImageMemoryPool()
{
  // 
  // Why is there a separate memory pool for swap chain images?
  //
  // VKEX by default uses VMA's default memory pool for objects 
  // that it needs to allocate internally, such as swapchain DSV 
  // images. This means that the swapchain DSV images and any
  // buffer or image an app creates can land in the same pool
  // and chunk if the VMA's default pool is used. Normally, this
  // isn't a problem. But on some platforms, such as Intel, if
  // the app maps the chunk that's shared with the swapchain 
  // DSV images, it can trigger a validation warning.
  //
  // To avoid these validation warnings, swapchain DSV images
  // uses a separate memory pool.
  // 

  if (m_configuration.swapchain.depth_stencil_format != VK_FORMAT_UNDEFINED) {
    ImageUsageFlags usage = {};
    usage.bits.transfer_src = true;
    usage.bits.transfer_dst = true;
    usage.bits.sampled      = true;
    usage.bits.depth_stencil_attachment = true;

    VkImageCreateInfo image_create_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    //image_create_info.flags                 = m_create_info.create_flags;
    image_create_info.imageType             = VK_IMAGE_TYPE_2D;
    image_create_info.format                = m_configuration.swapchain.depth_stencil_format;
    image_create_info.extent                = { m_configuration.window.width, m_configuration.window.height, 1 };
    image_create_info.mipLevels             = 1;
    image_create_info.arrayLayers           = 1;
    image_create_info.samples               = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling                = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage                 = usage.flags;
    image_create_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices   = nullptr;
    image_create_info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocate_info = {};
    allocate_info.usage = VMA_MEMORY_USAGE_GPU_ONLY; 

    VmaPoolCreateInfo create_info = {};
    VkResult vk_result = vmaFindMemoryTypeIndexForImageInfo(
      GetDevice()->GetVmaAllocator(),
      &image_create_info,
      &allocate_info,
      &create_info.memoryTypeIndex);
    VKEX_ASSERT_MSG(vk_result == VK_SUCCESS, "Unable to find memory type for swapchain image memory pool");
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }

    vk_result = vmaCreatePool(
      GetDevice()->GetVmaAllocator(),
      &create_info,
      &m_swapchain_image_memory_pool);
    VKEX_ASSERT_MSG(vk_result == VK_SUCCESS, "Failed to create swapchain image memory pool");
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  return vkex::Result::Success;
}

vkex::Result Application::InitializeVkexSwapchain()
{
  // Surface
  {
    vkex::SurfaceCreateInfo surface_create_info = {};
    surface_create_info.physical_device = m_device->GetPhysicalDevice();
#if defined(VKEX_WIN32)
    surface_create_info.hinstance = ::GetModuleHandle(nullptr);
    surface_create_info.hwnd = glfwGetWin32Window(m_window);
#elif defined(VKEX_LINUX_WAYLAND)
# error "not implemented"
#elif defined(VKEX_LINUX_XCB)
    surface_create_info.connection = XGetXCBConnection(glfwGetX11Display());
    surface_create_info.window = glfwGetX11Window(m_window);
#elif defined(VKEX_LINUX_XLIB)
# error "not implemented"
#elif defined(VKEX_LINUX_GGP)
#endif
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_instance->CreateSurface(surface_create_info, &m_surface)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Image count
  {
    auto& vk_surface_capabilities = m_surface->GetVkSurfaceCapabilities2();
    uint32_t min_image_count = vk_surface_capabilities.surfaceCapabilities.minImageCount;
    uint32_t max_image_count = vk_surface_capabilities.surfaceCapabilities.maxImageCount;

    uint32_t image_count = m_configuration.swapchain.image_count;
    if (image_count == 0) {
      image_count = m_configuration.frame_count;
    }
    image_count = std::max(std::min(image_count, max_image_count), min_image_count);
    if (image_count != m_configuration.swapchain.image_count) {
      if (m_configuration.swapchain.image_count != 0) {
        VKEX_LOG_WARN("Swapchain image count readjusted from " << 
                       m_configuration.swapchain.image_count << " to " << image_count);
      }
      m_configuration.swapchain.image_count = image_count;
    }
  }

  // Extents
  {
    auto& vk_surface_capabilities = m_surface->GetVkSurfaceCapabilities2();
    uint32_t max_width = vk_surface_capabilities.surfaceCapabilities.maxImageExtent.width;
    uint32_t max_height = vk_surface_capabilities.surfaceCapabilities.maxImageExtent.height;
    if ((max_width < m_configuration.window.width) || (max_height < m_configuration.window.height)) {
      VKEX_LOG_WARN("Swapchain extents readjusted from "
                     << m_configuration.window.width << "x" << m_configuration.window.height
                     << " to " 
                     << max_width << "x" << max_height);
    }
    m_configuration.window.width = std::min(m_configuration.window.width, max_width);
    m_configuration.window.height = std::min(m_configuration.window.height, max_height);
  }

  // Swapchain
  {
    vkex::SwapchainCreateInfo swapchain_create_info = {};
    swapchain_create_info.surface               = m_surface;
    swapchain_create_info.image_count           = m_configuration.swapchain.image_count;
    swapchain_create_info.color_format          = m_configuration.swapchain.color_format;
    swapchain_create_info.color_space           = m_configuration.swapchain.color_space;
    swapchain_create_info.depth_stencil_format  = m_configuration.swapchain.depth_stencil_format;
    swapchain_create_info.width                 = m_configuration.window.width;
    swapchain_create_info.height                = m_configuration.window.height;
    swapchain_create_info.present_mode          = m_configuration.swapchain.present_mode;
    swapchain_create_info.queue                 = m_present_queue;
    swapchain_create_info.image_memory_pool     = m_swapchain_image_memory_pool;
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateSwapchain(swapchain_create_info, &m_swapchain)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Color image views
  {
    uint32_t image_count = m_swapchain->GetImageCount();
    for (uint32_t image_index = 0; image_index < image_count; ++image_index) {
      // Get image
      vkex::Image image = nullptr;
      {
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_RESULT_CALL(
          vkex_result,
          m_swapchain->GetColorImage(image_index, &image);
        );
        if (!vkex_result) {
          return vkex_result;
        }
      }
      // Create image view
      {
        // Image format
        VkFormat image_format = image->GetFormat();
        // Image aspect
        VkImageAspectFlags image_aspect = vkex::DetermineAspectMask(image_format);
        // Create info
        vkex::ImageViewCreateInfo image_view_create_info = {};
        {
          image_view_create_info.create_flags       = 0;
          image_view_create_info.image              = image;
          image_view_create_info.view_type          = VK_IMAGE_VIEW_TYPE_2D;
          image_view_create_info.format             = image_format;
          image_view_create_info.samples            = VK_SAMPLE_COUNT_1_BIT;
          image_view_create_info.components         = vkex::ComponentMappingRGBA();
          image_view_create_info.subresource_range  = vkex::ImageSubresourceRange(image_aspect);
        }
        // Image view
        vkex::ImageView image_view = nullptr;
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_RESULT_CALL(
          vkex_result,
          m_device->CreateImageView(
            image_view_create_info, 
            &image_view)
        );
        if (!vkex_result) {
          return vkex_result;
        }
        // Add to list
        m_color_image_views.push_back(image_view);
      }
    }
  }

  // Depth stencil image views
  if (m_configuration.swapchain.depth_stencil_format != VK_FORMAT_UNDEFINED) {
    uint32_t image_count = m_swapchain->GetImageCount();
    for (uint32_t image_index = 0; image_index < image_count; ++image_index) {
      // Get image
      vkex::Image image = nullptr;
      {
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_RESULT_CALL(
          vkex_result,
          m_swapchain->GetDepthStencilImage(image_index, &image);
        );
        if (!vkex_result) {
          return vkex_result;
        }
      }
      // Create image view
      {
        // Image format
        VkFormat image_format = image->GetFormat();
        // Image aspect
        VkImageAspectFlags image_aspect = vkex::DetermineAspectMask(image_format);
        // Create info
        vkex::ImageViewCreateInfo image_view_create_info = {};
        {
          image_view_create_info.create_flags       = 0;
          image_view_create_info.image              = image;
          image_view_create_info.view_type          = VK_IMAGE_VIEW_TYPE_2D;
          image_view_create_info.format             = image_format;
          image_view_create_info.samples            = VK_SAMPLE_COUNT_1_BIT;
          image_view_create_info.components         = vkex::ComponentMappingRGBA();
          image_view_create_info.subresource_range  = vkex::ImageSubresourceRange(image_aspect);
        }
        // Image view
        vkex::ImageView image_view = nullptr;
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_RESULT_CALL(
          vkex_result,
          m_device->CreateImageView(
            image_view_create_info, 
            &image_view)
        );
        if (!vkex_result) {
          return vkex_result;
        }
        // Add to list
        m_depth_stencil_image_views.push_back(image_view);
      }
    }
  }

  // Render passes
  {
    uint32_t image_count = m_swapchain->GetImageCount();
    for (uint32_t image_index = 0; image_index < image_count; ++image_index) {
      // RTV
      vkex::RenderTargetView rtv = nullptr;
      {
        // Image view
        vkex::ImageView image_view = m_color_image_views[image_index];
        // Fill out RTV
        vkex::RenderTargetViewCreateInfo create_info ={};
        create_info.format        = image_view->GetFormat();
        create_info.samples       = image_view->GetSamples();
        create_info.load_op       = VK_ATTACHMENT_LOAD_OP_CLEAR;
        create_info.store_op      = VK_ATTACHMENT_STORE_OP_STORE;
        create_info.final_layout  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        create_info.clear_value   = m_configuration.swapchain.rtv_clear_value;
        create_info.attachment      = image_view;
        create_info.resolve       = nullptr;
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_RESULT_CALL(
          vkex_result,
          m_device->CreateRenderTargetView(
            create_info, 
            &rtv)
        );
        if (!vkex_result) {
          return vkex_result;
        }
      }

      // DSV
      bool has_depth_stencil = m_configuration.swapchain.depth_stencil_format != VK_FORMAT_UNDEFINED;
      vkex::DepthStencilView dsv = nullptr;
      if (has_depth_stencil) {
        // Image view
        vkex::ImageView image_view = m_depth_stencil_image_views[image_index];
        // Fill out DSV
        vkex::DepthStencilViewCreateInfo create_info = {};
        create_info.format            = image_view->GetFormat();
        create_info.samples           = image_view->GetSamples();
        create_info.depth_load_op     = VK_ATTACHMENT_LOAD_OP_CLEAR;
        create_info.depth_store_op    = VK_ATTACHMENT_STORE_OP_STORE;
        create_info.stencil_load_op   = VK_ATTACHMENT_LOAD_OP_CLEAR;
        create_info.stencil_store_op  = VK_ATTACHMENT_STORE_OP_STORE;
        create_info.final_layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        create_info.clear_value       = m_configuration.swapchain.dsv_clear_value;
        create_info.attachment     = image_view;
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_RESULT_CALL(
          vkex_result,
          m_device->CreateDepthStencilView(
            create_info, 
            &dsv)
        );
        if (!vkex_result) {
          return vkex_result;
        }
      }
      // Create info
      vkex::RenderPassCreateInfo render_pass_create_info = {};
      render_pass_create_info.flags   = 0;
      render_pass_create_info.rtvs    = { rtv };
      render_pass_create_info.dsv     = dsv;
      render_pass_create_info.extent  = { m_configuration.window.width, m_configuration.window.height };
      // Create render pass
      vkex::RenderPass render_pass = nullptr;
      vkex::Result vkex_result = vkex::Result::Undefined;
      VKEX_RESULT_CALL(
        vkex_result,
        m_device->CreateRenderPass(render_pass_create_info, &render_pass)
      );
      if (!vkex_result) {
        return vkex_result;
      }

      m_render_passes.push_back(render_pass);
    }
  }

  // Screenshot buffer
  if (m_configuration.enable_screen_shot) {
    uint64_t size = vkex::RoundUp<uint64_t>(m_configuration.window.width, 4) *
                    vkex::RoundUp<uint64_t>(m_configuration.window.height, 4) *
                    vkex::FormatSize(m_configuration.swapchain.color_format);
    vkex::BufferCreateInfo buffer_create_info = {};
    buffer_create_info.size                           = size;
    buffer_create_info.usage_flags.bits.transfer_dst  = true;
    buffer_create_info.committed                      = true;
    buffer_create_info.host_visible                   = true;
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateBuffer(buffer_create_info, &m_screenshot_buffer)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Log swapchain creation
  {
    VKEX_LOG_INFO("");
    VKEX_LOG_INFO("Swapchain created : " << vkex::ToHexString(m_swapchain->GetVkObject()));
    VKEX_LOG_INFO("   " << "Image Count      : " << m_configuration.swapchain.image_count);
    VKEX_LOG_INFO("   " << "Format           : " << vkex::ToString(m_configuration.swapchain.color_format));
    VKEX_LOG_INFO("   " << "Color Space      : " << vkex::ToString(m_configuration.swapchain.color_space));
    VKEX_LOG_INFO("   " << "Size             : " << m_configuration.window.width << "x" << m_configuration.window.height);
    VKEX_LOG_INFO("   " << "Present Mode     : " << vkex::ToString(m_configuration.swapchain.present_mode));
    VKEX_LOG_INFO("   " << "Paced Frame Rate : " << m_configuration.swapchain.paced_frame_rate);
    VKEX_LOG_INFO("");
  }

  return vkex::Result::Success;
}

vkex::Result Application::InitializeVkexPerFrameRenderData()
{
  // Command pool
  {
    vkex::CommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.flags.bits.reset_command_buffer = true;
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateCommandPool(command_pool_create_info, &m_render_command_pool)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }
  // Command buffers
  std::vector<vkex::CommandBuffer> command_buffers;
  {
    vkex::CommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.command_buffer_count = m_configuration.frame_count;
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_render_command_pool->AllocateCommandBuffers(
        command_buffer_allocate_info,
        &command_buffers);
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Per frame data
  for (uint32_t frame_index = 0; frame_index < m_configuration.frame_count; ++frame_index) {
    // Present data
    {
      RenderDataPtr data = std::make_unique<RenderData>();
      if (!data) {
        return vkex::Result(vkex::Result::ErrorAllocationFailed);
      }

      vkex::CommandBuffer cmd = command_buffers[frame_index];
      vkex::Result vkex_result = vkex::Result::Undefined;
      VKEX_RESULT_CALL(
        vkex_result,
        data->InternalCreate(m_device, frame_index, cmd)
      );
      if (!vkex_result) {
        return vkex_result;
      }

      m_per_frame_render_data.push_back(std::move(data));
    }
  }

  return vkex::Result::Success;
}

vkex::Result Application::InitializeVkexPerFramePresentData()
{
  // Command pool
  {
    vkex::CommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.flags.bits.reset_command_buffer = true;
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateCommandPool(command_pool_create_info, &m_present_command_pool)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }
  // Command buffers
  std::vector<vkex::CommandBuffer> command_buffers;
  {
    vkex::CommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.command_buffer_count = m_configuration.frame_count;
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_present_command_pool->AllocateCommandBuffers(
        command_buffer_allocate_info,
        &command_buffers);
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Per frame data
  for (uint32_t frame_index = 0; frame_index < m_configuration.frame_count; ++frame_index) {
    // Present data
    {
      PresentDataPtr data = std::make_unique<PresentData>();
      if (!data) {
        return vkex::Result(vkex::Result::ErrorAllocationFailed);
      }

      vkex::CommandBuffer cmd = command_buffers[frame_index];
      vkex::Result vkex_result = vkex::Result::Undefined;
      VKEX_RESULT_CALL(
        vkex_result,
        data->InternalCreate(m_device, frame_index, cmd)
      );
      if (!vkex_result) {
        return vkex_result;
      }

      m_per_frame_present_data.push_back(std::move(data));
    }
  }

  return vkex::Result::Success;
}

vkex::Result Application::InitializeVkex()
{
  // Instance
  {
    vkex::InstanceCreateInfo instance_create_info = {};
    instance_create_info.application_info.application_name  = m_configuration.name;
    instance_create_info.debug_utils.enable                 = m_configuration.graphics_debug.enable;
    instance_create_info.debug_utils.message_severity       = m_configuration.graphics_debug.message_severity;
    instance_create_info.debug_utils.message_type           = m_configuration.graphics_debug.message_type;
    instance_create_info.enable_swapchain                   = (m_configuration.mode == APPLICATION_MODE_WINDOW) ? true : false;

    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      vkex::CreateInstanceVKEX(instance_create_info, &m_instance)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Device
  {
    vkex::Result vkex_result = InitializeVkexDevice();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Swapchain memory pool
  if (IsApplicationModeWindow()) {
    vkex::Result vkex_result = InitializeVkexSwapchainImageMemoryPool();
    if (!vkex_result) {
      return vkex_result;
    }
  }
    
  // Swapchain
  if (IsApplicationModeWindow()) {
    vkex::Result vkex_result = InitializeVkexSwapchain();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Per frame render data
  {
    vkex::Result vkex_result = InitializeVkexPerFrameRenderData();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Per frame present data
  if (IsApplicationModeWindow()) {
    vkex::Result vkex_result = InitializeVkexPerFramePresentData();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Frame fence
  if (IsApplicationModeWindow()) {
    vkex::FenceCreateInfo create_info = {};
    create_info.flags.bits.signaled = true;
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateFence(create_info, &m_frame_fence);
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  return vkex::Result::Success;
}

vkex::Result Application::InitializeImGui()
{
  // Setup Dear ImGui binding
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  // Setup GLFW binding
  ImGui_ImplGlfw_InitForVulkan(m_window, false);

  { 
    vkex::DescriptorPoolCreateInfo create_info = {};
    create_info.pool_sizes.combined_image_sampler = 1;
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateDescriptorPool(create_info, &m_imgui_descriptor_pool)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  {
    // Setup Vulkan binding
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance        = *m_instance;
    init_info.PhysicalDevice  = *(m_device->GetPhysicalDevice());
    init_info.Device          = *m_device;
    init_info.QueueFamily     = m_graphics_queue->GetVkQueueFamilyIndex();
    init_info.Queue           = *m_graphics_queue;
    init_info.PipelineCache   = VK_NULL_HANDLE;
    init_info.DescriptorPool  = *m_imgui_descriptor_pool;
    init_info.MinImageCount   = m_configuration.swapchain.image_count;
    init_info.ImageCount      = m_configuration.swapchain.image_count;
    init_info.Allocator       = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = nullptr;
    bool result = ImGui_ImplVulkan_Init(
      &init_info, 
      *m_render_passes[0]);
    if (!result) {
      return vkex::Result::ErrorImGuiInitializeFailed;
    }
  }

  //ImGui::StyleColorsClassic();
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();

  // Upload Fonts
  {
    // Use any command queue
    VkCommandPool command_pool = *m_present_command_pool;
    VkCommandBuffer command_buffer = *(m_per_frame_present_data[0]->GetCommandBuffer());

    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, vkex::ResetCommandPool(*m_device, command_pool, 0));
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }    

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, vkex::BeginCommandBuffer(command_buffer, &begin_info));
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }    

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &command_buffer;
    vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, vkex::EndCommandBuffer(command_buffer));
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }    

    vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, vkex::QueueSubmit(*m_graphics_queue, 1, &end_info, VK_NULL_HANDLE));
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }   

    vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, vkex::DeviceWaitIdle(*m_device));
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }   

    ImGui_ImplVulkan_DestroyFontUploadObjects();
  }

  return vkex::Result::Success;
}

vkex::Result Application::InternalCreate()
{
  if (m_swapchain != nullptr) {
    return vkex::Result::Success;
  }
  
  // Window
  {
    vkex::Result vkex_result = InitializeWindow();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // VKEX
  {
    vkex::Result vkex_result = InitializeVkex();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // ImGui
  if (IsApplicationModeWindow()) {
    vkex::Result vkex_result = InitializeImGui();
    if (!vkex_result) {
      return vkex_result;
    }
  }
  
  return vkex::Result::Success;
}

vkex::Result Application::DestroyVkexSwapchainImageMemoryPool()
{
  if (m_swapchain_image_memory_pool != VK_NULL_HANDLE) {
    vmaDestroyPool(
      GetDevice()->GetVmaAllocator(),
      m_swapchain_image_memory_pool);
    m_swapchain_image_memory_pool = VK_NULL_HANDLE;
  }
  
  return vkex::Result::Success;
}

vkex::Result Application::DestroyVkexSwapchain()
{
  // Render passes
  for (auto& render_pass : m_render_passes) {
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroyRenderPass(render_pass)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }
  m_render_passes.clear();

  // Color image views
  for (auto& image_view : m_color_image_views) {
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroyImageView(image_view)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }
  m_color_image_views.clear();

  // Color image views
  for (auto& image_view : m_depth_stencil_image_views) {
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroyImageView(image_view)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }
  m_depth_stencil_image_views.clear();

  // Swapchain
  VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
  if (m_swapchain != nullptr) {
    vk_swapchain = *m_swapchain;
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroySwapchain(m_swapchain)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Surface
  if (m_surface != nullptr) {
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_instance->DestroySurface(m_surface)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Screenshot buffer
  if (m_screenshot_buffer) {
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroyBuffer(m_screenshot_buffer)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  VKEX_LOG_INFO("");
  VKEX_LOG_INFO("Swapchain destroyed : " << vkex::ToHexString(vk_swapchain));
  VKEX_LOG_INFO("");

  return vkex::Result::Success;
}

vkex::Result Application::DestroyImGui()
{
  if (m_imgui_descriptor_pool != nullptr) {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroyDescriptorPool(m_imgui_descriptor_pool)
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  return vkex::Result::Success;
}

vkex::Result Application::InternalDestroy()
{
  // ImGui
  {
    vkex::Result vkex_result = DestroyImGui();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Swapchain
  if (IsApplicationModeWindow()) {
    vkex::Result vkex_result = DestroyVkexSwapchain();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Swapchain memory pool
  if (IsApplicationModeWindow()) {
    vkex::Result vkex_result = DestroyVkexSwapchainImageMemoryPool();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Command pools
  {
    vkex::Result vkex_result = m_device->DestroyCommandPool(m_render_command_pool);
    if (!vkex_result) {
      return vkex_result;
    }

    vkex_result = m_device->DestroyCommandPool(m_present_command_pool);
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Render data
  {
    for (auto& data : m_per_frame_render_data) {
      vkex::Result vkex_result = data->InternalDestroy();
      if (!vkex_result) {
        return vkex_result;
      }
    }
    m_per_frame_render_data.clear();
  }
  
  // Present data
  if (IsApplicationModeWindow()) {
    for (auto& data : m_per_frame_present_data) {
      vkex::Result vkex_result = data->InternalDestroy();
      if (!vkex_result) {
        return vkex_result;
      }
    }
    m_per_frame_present_data.clear();
  }

  // Frame fence
  if (IsApplicationModeWindow()) {
    vkex::Result vkex_result = m_device->DestroyFence(m_frame_fence);
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Device
  if (m_device != nullptr) {
    vkex::Result vkex_result = m_instance->DestroyDevice(m_device);
    if (!vkex_result) {
      return vkex_result;
    }
    m_device = nullptr;
  }

  // Instance
  if (m_instance != nullptr) {
    vkex::Result vkex_result = vkex::DestroyInstanceVKEX(m_instance);
    if (!vkex_result) {
      return vkex_result;
    }
    m_instance = nullptr;
  }

  if (m_window != nullptr) {
    glfwDestroyWindow(m_window);
    m_window = nullptr;
  }

  return vkex::Result::Success;
}

vkex::Result Application::RecreateVkexSwapchain()
{
  if (!IsApplicationModeWindow()) {
    return vkex::Result::ErrorInvalidApplicationMode;
  }

  VkResult vk_result = vkex::QueueWaitIdle(*m_graphics_queue);
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  vk_result = vkex::QueueWaitIdle(*m_present_queue);
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  vkex::Result vkex_result = DestroyVkexSwapchain();
  if (!vkex_result) {
    return vkex_result;
  }

  vkex_result = InitializeVkexSwapchain();
  if (!vkex_result) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

void Application::MoveCallback(int32_t x, int32_t y)
{
  if (!IsApplicationModeWindow()) {
    return;
  }

  Move(x, y);
}

void Application::ResizeCallback(uint32_t width, uint32_t height)
{
  if (!IsApplicationModeWindow()) {
    return;
  }

  bool width_changed = (width != m_configuration.window.width);
  bool height_changed = (height != m_configuration.window.height);
  if (width_changed || height_changed) {
    m_configuration.window.width = width;
    m_configuration.window.height = height;
    m_recreate_swapchain = true;

    Resize(width, height);
  }
}

void Application::MouseDownCallback(int32_t x, int32_t y, uint32_t buttons)
{
  if (!IsApplicationModeWindow()) {
    return;
  }

  DispatchCallMouseDown(x, y, buttons);
}

void Application::MouseUpCallback(int32_t x, int32_t y, uint32_t buttons)
{
  if (!IsApplicationModeWindow()) {
    return;
  }
  DispatchCallMouseUp(x, y, buttons);

}

void Application::MouseMoveCallback(int32_t x, int32_t y, uint32_t buttons)
{
  if (!IsApplicationModeWindow()) {
    return;
  }

  DispatchCallMouseMove(x, y, buttons);
}

void Application::KeyUpCallback(KeyboardInput key) 
{
  if (!IsApplicationModeWindow()) {
    return;
  }

  m_keys[key] = false;
  DispatchCallKeyUp(key);
}

void Application::KeyDownCallback(KeyboardInput key) 
{
  if (!IsApplicationModeWindow()) {
    return;
  }

  m_keys[key] = true;
  DispatchCallKeyDown(key);

  if (key == kKeyPrintScreen) {
    m_screen_shot = true;
  }
}

vkex::Result Application::CheckConfiguration()
{
  if (m_configuration.frame_count == 0) {
    m_configuration.frame_count = kDefaultInFlightFrameCount;
  }

  return vkex::Result::Success;
}

vkex::Result Application::ProcessFrameFence()
{
  if (!IsApplicationModeWindow()) {
    return vkex::Result::ErrorInvalidApplicationMode;
  }


  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    m_frame_fence->WaitForFence()
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    m_frame_fence->ResetFence()
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  return vkex::Result::Success;
}

vkex::Result Application::AcquireNextImage(Application::PresentData* p_data, uint32_t* p_swapchain_image_index)
{
  if (!IsApplicationModeWindow()) {
    return vkex::Result::ErrorInvalidApplicationMode;
  }

  // Vulkan objects
  VkSemaphore vk_image_acquired_semaphore = *(p_data->GetImageAcquiredSemaphore());
  VkFence vk_image_acquired_fence = VK_NULL_HANDLE;

  // Acquire next image
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    m_swapchain->AcquireNextImage(
      UINT64_MAX, 
      vk_image_acquired_semaphore, 
      vk_image_acquired_fence, 
      p_swapchain_image_index)
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }
 
  return vkex::Result::Success;
}

void Application::Quit()
{
  m_running = false;
  if (IsApplicationModeWindow()) {
    glfwSetWindowShouldClose(m_window, 1);
  }
}

const std::string& Application::GetName() const
{
  return m_configuration.name;
}

std::string Application::GetPlatformName() const
{
  std::stringstream ss;
#if defined(VKEX_WIN32)
  ss << "Windows";
  // TODO: Do we actually want to print out specific Windows version?
  // If so, move over to VerifyVersionInfo
  //OSVERSIONINFO osvi = {};
  //osvi.dwOSVersionInfoSize = sizeof(osvi);
  //if (GetVersionEx(&osvi) != 0) {
  //  ss << " ";
  //  switch (osvi.dwPlatformId)
  //  {
  //  case VER_PLATFORM_WIN32s:
  //    ss << "3.x";
  //    break;
  //  case VER_PLATFORM_WIN32_WINDOWS:
  //    ss << (osvi.dwMinorVersion == 0 ? "95" : "98");
  //    break;
  //  case VER_PLATFORM_WIN32_NT:
  //    ss << "NT";
  //    break;
  //  default:
  //    ss << "Unknown";
  //    break;
  //  }

  //  ss << " " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
    //ss << " " << "(Build " << osvi.dwBuildNumber <<")";
  //}
#elif defined(VKEX_LINUX)
# if defined(VKEX_LINUX_GGP)
  ss << "Stadia";
# else
  ss << "Generic Linux";
# endif
  //struct utsname uts = {};
  //uname(&uts);
  //ss << "(" << uts.release << ")";
#endif
  return ss.str();
}

ApplicationMode Application::GetApplicationMode() const
{
  return m_configuration.mode;
}

bool Application::IsApplicationModeWindow() const
{
  bool is_mode = (m_configuration.mode == APPLICATION_MODE_WINDOW);
  return is_mode;
}

bool Application::IsApplicationModeHeadless() const
{
  bool is_mode = (m_configuration.mode == APPLICATION_MODE_HEADLESS);
  return is_mode;
}

uint32_t Application::GetProcessId() const
{
  uint32_t pid = UINT32_MAX;
#if defined(VKEX_WIN32)
  pid = static_cast<uint32_t>(::GetCurrentProcessId());
#elif defined(VKEX_LINUX)
  pid = static_cast<uint32_t>(getpid());
#endif
  return pid;
}

fs::path Application::GetApplicationPath() const
{
  fs::path path;
#if defined(VKEX_WIN32)
  HMODULE this_win32_module = GetModuleHandleA(nullptr);
  char buf[MAX_PATH];
  std::memset(buf, 0, MAX_PATH);
  GetModuleFileNameA(this_win32_module, buf, MAX_PATH);
  path = fs::path(buf);
#elif defined(VKEX_LINUX)
  char buf[PATH_MAX];
  std::memset(buf, 0, PATH_MAX);
  readlink("/proc/self/exe", buf, PATH_MAX);
  path = fs::path(buf);  
#else
# error "not implemented"
#endif
  return path;
}

const std::vector<fs::path>& Application::GetAssetDirs() const
{
  return m_asset_dirs;
}

fs::path Application::GetAssetPath(const std::string& file_name) const
{
  fs::path file_path;
  for (auto& asset_dir : m_asset_dirs) {
    fs::path path = asset_dir / file_name;
    if (fs::exists(path)) {
      file_path = path;
      break;
    }
  }
  return file_path;
}

void Application::AddAssetDir(const fs::path& path)
{
  auto it = vkex::Find(m_asset_dirs, path);
  if (it != std::end(m_asset_dirs)) {
    return;
  }

  if (!fs::is_directory(path)) {
    return;
  }

  m_asset_dirs.push_back(path);
}

void Application::DispatchCallAddArgs(vkex::ArgParser& args)
{
  AddArgs(args);
}

void Application::DispatchCallConfigure(const vkex::ArgParser& args, vkex::Configuration& configuration)
{
  Configure(args, configuration);
}

void Application::DispatchCallSetup()
{
  Setup();
}

void Application::DispatchCallDestroy()
{
  Destroy();
}

void Application::DispatchCallMove(int32_t x, int32_t y)
{
  Move(x, y);
}

void Application::DispatchCallResize(uint32_t width, uint32_t height)
{
  Resize(width, height);
}

void Application::DispatchCallMouseDown(int32_t x, int32_t y, uint32_t buttons)
{
  MouseDown(x, y, buttons);
}

void Application::DispatchCallMouseUp(int32_t x, int32_t y, uint32_t buttons)
{
  MouseUp(x, y, buttons);
}

void Application::DispatchCallMouseMove(int32_t x, int32_t y, uint32_t buttons)
{
  MouseMove(x, y, buttons);
}

void Application::DispatchCallKeyUp(KeyboardInput key)
{
  KeyUp(key);
}

void Application::DispatchCallKeyDown(KeyboardInput key)
{
  KeyDown(key);
}

void Application::DispatchCallUpdate(double frame_elapsed_time)
{
  Update(frame_elapsed_time);
}

void Application::DispatchCallRender(Application::RenderData* p_data)
{
  Render(p_data);
}

void Application::DispatchCallPresent(Application::PresentData* p_data)
{
  Present(p_data);
}

bool Application::IsKeyPressed(KeyboardInput key)
{
  // KeyUp/KeyDown callbacks are not suitable for multi-key and "repeat" key tracking
  // Therefore per-key tracking is necessary
  return m_keys[key];
}

bool Application::IsMouseDown(int button) const
{
  return false;
}

void Application::SetCursorMode(vkex::CursorMode cursor_mode)
{
  m_configuration.window.cursor_mode = cursor_mode;
  switch (m_configuration.window.cursor_mode) {
    case CursorMode::Visible: {
      glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    break;

    case CursorMode::Hidden: {
      glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    break;

    case CursorMode::Captured: {
      glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    break;
  }
}

vkex::Result Application::SubmitRender(Application::RenderData* p_data)
{
  // Update submitted flag
  {
    m_render_submitted = true;
  }

  return vkex::Result::Success;
}

void Application::DrawImGui(vkex::CommandBuffer cmd)
{
  // Rendering
  ImGui::Render();
	// Record Imgui Draw Data and draw funcs into command buffer
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);
}

vkex::Result Application::SubmitPresent(Application::PresentData* p_data)
{
  if (IsApplicationModeHeadless()) {
    return vkex::Result::ErrorInvalidApplicationMode;
  }

  // Vulkan objects
  VkSemaphore vk_image_acquired_semaphore = *(p_data->GetImageAcquiredSemaphore());
  VkCommandBuffer vk_command_buffer       = *(p_data->GetCommandBuffer());
  VkPipelineStageFlags vk_pipeline_stage  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  VkSemaphore vk_work_complete_semaphore  = *(p_data->GetWorkCompleteSemaphore());
  VkFence vk_work_complete_fence          = *m_frame_fence;
  VkSwapchainKHR vk_swapchain             = *m_swapchain;
  uint32_t vk_swapchain_image_index       = m_current_swapchain_image_index;

  // Submit present work
  {
    // Containers
    std::vector<VkSemaphore> vk_wait_semaphores           = { vk_image_acquired_semaphore };
    std::vector<VkCommandBuffer> vk_command_buffers       = { vk_command_buffer };
    std::vector<VkPipelineStageFlags> vk_pipeline_stages  = { vk_pipeline_stage };
    std::vector<VkSemaphore> vk_signal_semaphores         = { vk_work_complete_semaphore };
    
    // Add wait for render work if submitted
    if (m_render_submitted) {
      VkSemaphore vk_render_work_completed_semaphore = *(m_current_render_data->GetWorkCompleteSemaphore());
      vk_wait_semaphores.push_back(vk_render_work_completed_semaphore);
      vk_pipeline_stages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }

    // Submit info
    VkSubmitInfo vk_submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    vk_submit_info.waitSemaphoreCount   = CountU32(vk_wait_semaphores);
    vk_submit_info.pWaitSemaphores      = DataPtr(vk_wait_semaphores);
    vk_submit_info.pWaitDstStageMask    = DataPtr(vk_pipeline_stages);
    vk_submit_info.commandBufferCount   = CountU32(vk_command_buffers);
    vk_submit_info.pCommandBuffers      = DataPtr(vk_command_buffers);
    vk_submit_info.signalSemaphoreCount = CountU32(vk_signal_semaphores);
    vk_submit_info.pSignalSemaphores    = DataPtr(vk_signal_semaphores);
    // Queue submit
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::QueueSubmit(
        *m_graphics_queue,
        1,
        &vk_submit_info,
        vk_work_complete_fence)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Submit present request
  {
    // Containers
    std::vector<VkSemaphore> vk_wait_semaphores       = { vk_work_complete_semaphore };
    std::vector<VkSwapchainKHR> vk_swapchains         = { vk_swapchain };
    std::vector<uint32_t> vk_swapchain_image_indices  = { vk_swapchain_image_index };

    // Present info
    VkPresentInfoKHR vk_present_info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    vk_present_info.waitSemaphoreCount  = CountU32(vk_wait_semaphores);
    vk_present_info.pWaitSemaphores     = DataPtr(vk_wait_semaphores);
    vk_present_info.swapchainCount      = CountU32(vk_swapchains);
    vk_present_info.pSwapchains         = DataPtr(vk_swapchains);
    vk_present_info.pImageIndices       = DataPtr(vk_swapchain_image_indices);
    vk_present_info.pResults            = nullptr;

#if defined(VKEX_LINUX_GGP)
    // TODO: Add frame token support to be a good citizen...
    // Though, it isn't really needed for this sample
#endif

    // Time start
    TimeRange time_range = {};
    time_range.start = static_cast<float>(GetElapsedTime());

    // Queue present
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::QueuePresentKHR(
        *m_present_queue,
        &vk_present_info)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }

    // End time
    time_range.end = static_cast<float>(GetElapsedTime());
    time_range.diff = time_range.end - time_range.start;
    m_vk_queue_present_times.push_back(time_range);
    // Average queue present time
    {
      m_average_vk_queue_present_time = 0;
      const size_t n = m_vk_queue_present_times.size();
      for (size_t i = 0; i < n; ++i) {
        m_average_vk_queue_present_time += m_vk_queue_present_times[i].diff;
      }
      m_average_vk_queue_present_time *= 1.0f / static_cast<float>(n);
    }
  }

  // Write screenshot if flag is set and option is not disabled
  if (m_screen_shot && m_configuration.enable_screen_shot) {
    // Wait for queue idle
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, vkex::QueueWaitIdle(*m_graphics_queue));

    // Build command buffer
    vkex::CommandBuffer command_buffer = p_data->GetCommandBuffer();
    {
      auto rtvs = p_data->GetRenderPass()->GetRtvs();
      vkex::Image image = rtvs[0]->GetResource()->GetImage();
      command_buffer->Begin();
      // Transition image
      command_buffer->CmdTransitionImageLayout(
        *image, image->GetAspectFlags(), 
        0, 
        image->GetMipLevels(), 
        0, 
        image->GetArrayLayers(), 
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT);
      // Copy
      VkBufferImageCopy region                = {};
      region.bufferOffset                     = 0;
      region.bufferRowLength                  = m_configuration.window.width;
      region.bufferImageHeight                = m_configuration.window.height;
      region.imageSubresource.aspectMask      = image->GetAspectFlags();
      region.imageSubresource.mipLevel        = 0;
      region.imageSubresource.baseArrayLayer  = 0;
      region.imageSubresource.layerCount      = 1;
      region.imageOffset                      = { 0, 0, 0 };
      region.imageExtent                      = { m_configuration.window.width, m_configuration.window.height, 1 };
      command_buffer->CmdCopyImageToBuffer(*image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *m_screenshot_buffer, 1, &region);
      // Transition image
      command_buffer->CmdTransitionImageLayout(
        *image, image->GetAspectFlags(), 
        0, 
        image->GetMipLevels(), 
        0, 
        image->GetArrayLayers(), 
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
      command_buffer->End();
    }

    // Get Vulkan command buffer handle
    VkCommandBuffer vk_command_buffer = *command_buffer;

    // Submit info
    VkSubmitInfo vk_submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    vk_submit_info.pWaitDstStageMask    = &vk_pipeline_stage;
    vk_submit_info.commandBufferCount   = 1;
    vk_submit_info.pCommandBuffers      = &vk_command_buffer;
    
    // Queue submit
    vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::QueueSubmit(*m_graphics_queue, 1, &vk_submit_info, VK_NULL_HANDLE)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }

    // Wait for queue idle again
    VKEX_VULKAN_RESULT_CALL(vk_result, vkex::QueueWaitIdle(*m_graphics_queue));

    // Write image
    {
      std::stringstream file_name;
      file_name << "screenshot_" << std::setfill('0') << std::setw(6) << m_elapsed_frame_count << ".jpg";
      fs::path file_path = GetApplicationPath().parent() / file_name.str();
      uint32_t component_count = FormatComponentCount(m_configuration.swapchain.color_format);
      uint32_t pixel_stride = FormatSize(m_configuration.swapchain.color_format);
      uint32_t row_stride = m_configuration.window.width * pixel_stride;

      void* mapped_address = nullptr;
      m_screenshot_buffer->MapMemory(&mapped_address);

      // Swap channels and fill alpha
      if (m_configuration.swapchain.color_format == VK_FORMAT_B8G8R8A8_UNORM) {
        uint8_t* row = static_cast<uint8_t*>(mapped_address);
        for (uint32_t y = 0; y < m_configuration.window.height; ++y) {
          uint8_t* pixel = row;
          for (uint32_t x = 0; x < m_configuration.window.width; ++x) {
            std::swap(pixel[0], pixel[2]);
            pixel += pixel_stride;
          }
          row += row_stride;
        }
      }
      
      // Write JPEG since STB's PNG write is *really* slow.
      vkex::Result vkex_result = Bitmap::WriteJPG(
        file_path,
        m_configuration.window.width,
        m_configuration.window.height,
        component_count,
        row_stride,
        mapped_address);
    }

    // Clear screenshot flag
    m_screen_shot = false;
  }

  return vkex::Result::Success;
}

vkex::Result Application::WaitAllQueuesIdle()
{
  // Graphics
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, m_graphics_queue->WaitIdle());
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Compute
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, m_compute_queue->WaitIdle());
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Transfer
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, m_transfer_queue->WaitIdle());
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Present - will be null if application is in headless mode
  if (m_present_queue != nullptr) {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, m_present_queue->WaitIdle());
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  return vkex::Result::Success;
}

bool Application::IsRunning() const
{
  bool is_running = m_running;
  if (IsApplicationModeWindow()) {
    is_running = (glfwWindowShouldClose(m_window) == 0);
  }
  return is_running;
}

vkex::Result Application::Run(int argn, const char* const* argv)
{
  // Add args
  DispatchCallAddArgs(m_args);

  // Parse args
  {
    bool parsed = m_args.Parse(argn, argv, std::cout);
    if (!parsed) {
      return vkex::Result::ErrorArgsParseFailed;
    }
  }

  // Call app configure
  DispatchCallConfigure(m_args, m_configuration);

  // Check configuration
  vkex::Result vkex_result = CheckConfiguration();
  if (!vkex_result) {
    return vkex_result;
  }

  vkex_result = InternalCreate();
  if (!vkex_result) {
    return vkex_result;
  }

  // Call app setup
  DispatchCallSetup();

  // Set time to 0
  if (IsApplicationModeWindow()) {
    glfwSetTime(0);
  }
  
  // -----------------------------------------------------------------------------------------------
  // Main loop [BEGIN]
  // -----------------------------------------------------------------------------------------------
  m_running = true;
  while(IsRunning()) {
    // Poll GLFW events
    if (IsApplicationModeWindow()) {
      glfwPollEvents();
    }

    // Frame fence, time, total, average, rate
    {
      if (IsApplicationModeWindow()) {
        vkex::Result vkex_result = ProcessFrameFence();
        if (!vkex_result) {
          return vkex_result;
        }
      }

      // Current time
      double current_time = GetElapsedTime();

      // Update time stats
      if (m_elapsed_frame_count > 0) {
        m_frame_start_time_delta = (current_time - m_frame_start_time);
        m_total_frame_time += m_frame_start_time_delta;
        m_average_frame_time = (m_total_frame_time / static_cast<double>(m_elapsed_frame_count));
        m_frames_per_second = static_cast<double>(m_elapsed_frame_count) / current_time;
      }

      // Calculate elapsed time since last frame
      m_frame_elapsed_time = current_time - m_frame_start_time;
    
      // Update frame start time
      m_frame_start_time = current_time;
    }

    // Recreate the swapchain if needed
    if (m_recreate_swapchain) {
      vkex::Result vkex_result = RecreateVkexSwapchain();
      if (!vkex_result) {
        return vkex_result;
      }
      m_recreate_swapchain = false;
    }

    // Update current per frame data
    m_current_render_data = m_per_frame_render_data[m_frame_index].get();
    if (IsApplicationModeWindow()) {
      m_current_present_data = m_per_frame_present_data[m_frame_index].get();
    }

    // Start the Dear ImGui frame
    if (IsApplicationModeWindow() && m_configuration.enable_imgui) {
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
    }

    // Call app update
    {
      double start_time = GetElapsedTime();
      DispatchCallUpdate(m_frame_elapsed_time);
      double end_time = GetElapsedTime();
      m_update_fn_time = end_time - start_time;
    }

    if (m_window_frame_count >= kWindowFrames) {
      m_max_window_frame_time = 0;
      m_min_window_frame_time = std::numeric_limits<double>::max();
      m_window_frame_count = 0;
    } else {
      m_max_window_frame_time = std::max(m_max_window_frame_time, m_frame_elapsed_time);
      m_min_window_frame_time = std::min(m_min_window_frame_time, m_frame_elapsed_time);
      m_window_frame_count++;
    }

 
    // Call app render
    {
      double start_time = GetElapsedTime();
      DispatchCallRender(m_current_render_data);
      double end_time = GetElapsedTime();
      m_render_fn_time = end_time - start_time;
    }

    // Acquire next image
    m_current_swapchain_image_index = UINT32_MAX;
    if (IsApplicationModeWindow()) {
      vkex::Result vkex_result = vkex::Result::Undefined;
      VKEX_RESULT_CALL(
        vkex_result,
        AcquireNextImage(m_current_present_data, &m_current_swapchain_image_index)
      );
      if (!vkex_result) {
        return vkex_result;
      }
    }

    // Pace fames - if needed
    if (m_configuration.swapchain.paced_frame_rate > 0) {
      if (m_elapsed_frame_count > 0) {
        double current_time  = GetElapsedTime();
        double paced_fps     = 1.0 / static_cast<double>(m_configuration.swapchain.paced_frame_rate);
        double expected_time = m_frame_0_time + (m_elapsed_frame_count * paced_fps);
        double diff = expected_time - current_time;
        if (diff > 0) {
          vkex::Timer::SleepSeconds(diff);
        }
      }
      else {
        m_frame_0_time = GetElapsedTime();
      }
    }

    // Set present render pass
    if (IsApplicationModeWindow()) {
      vkex::RenderPass render_pass = m_render_passes[m_current_swapchain_image_index];
      m_current_present_data->SetRenderPass(render_pass);
    }

    // Call app present
    if (IsApplicationModeWindow()) {
      double start_time = GetElapsedTime();
      DispatchCallPresent(m_current_present_data);
      double end_time = GetElapsedTime();
      m_present_fn_time = end_time - start_time;
    }
    
    // Increment present count
    m_elapsed_frame_count += 1;
    // In flight image index
    m_frame_index = (m_elapsed_frame_count % m_configuration.frame_count);
  }
  // -----------------------------------------------------------------------------------------------
  // Main loop [END]
  // -----------------------------------------------------------------------------------------------

  // Wait for all queues to become idle
  vkex_result = WaitAllQueuesIdle();
  if (!vkex_result) {
    return vkex_result;
  }

  // Wait for device to become idle
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(vk_result, m_device->WaitIdle());
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Call app destroy
  DispatchCallDestroy();

  // Destroy everything
  vkex_result = InternalDestroy();
  if (!vkex_result) {
    return vkex_result;
  }

  VKEX_LOG_INFO("");
  VKEX_LOG_INFO("Application exited cleanly");
  VKEX_LOG_INFO("");

  return vkex::Result::Success;
}

const vkex::Configuration& Application::GetConfiguration() const
{
  return m_configuration;
}

uint32_t Application::GetFrameCount() const
{
  return m_configuration.frame_count;
}

uint32_t Application::GetFrameIndex() const
{
  return m_frame_index;
}

float Application::GetElapsedTime() const
{
  double elapsed_seconds = glfwGetTime();
  return static_cast<float>(elapsed_seconds);
}

void Application::SetSwapchainFormat(VkFormat format, VkColorSpaceKHR color_space)
{
  bool is_format_different = (format != m_configuration.swapchain.color_format);
  bool is_color_space_different = (color_space != m_configuration.swapchain.color_space);
  if (is_format_different || is_color_space_different) {
    m_configuration.swapchain.color_format = format;
    m_configuration.swapchain.color_space = color_space;
    m_recreate_swapchain = true;
  }
}

void Application::SetSwapchainPresentMode(VkPresentModeKHR present_mode)
{
  if (m_configuration.swapchain.present_mode == present_mode) {
    m_configuration.swapchain.present_mode = present_mode;
    m_recreate_swapchain = true;
  }
}

vkex::Queue Application::GetGraphicsQueue() const
{
  return m_graphics_queue;
}

void Application::DrawDebugApplicationInfo()
{
  if (!m_configuration.enable_imgui) {
    return;
  }

  auto& configuration = GetConfiguration();
  auto& gpu_properties = GetDevice()->GetPhysicalDevice()->GetPhysicalDeviceProperties().properties;

  if (ImGui::Begin("Application Info")) {
    {
      ImGui::Columns(2);
      // Application PID
      {
        ImGui::Text("Application PID"); 
        ImGui::NextColumn(); 
        ImGui::Text("%d", GetProcessId()); 
        ImGui::NextColumn(); 
      }
      // Application Name
      {
        ImGui::Text("Application Name"); 
        ImGui::NextColumn(); 
        ImGui::Text("%s", configuration.name.c_str()); 
        ImGui::NextColumn(); 
      }
      // GPU
      {
        ImGui::Text("GPU"); 
        ImGui::NextColumn(); 
        ImGui::Text("%s", GetDevice()->GetDescriptiveName()); 
        ImGui::NextColumn(); 
      }
      // GPU Type
      {
        ImGui::Text("GPU Type"); 
        ImGui::NextColumn(); 
        ImGui::Text("%s", vkex::ToStringShort(gpu_properties.deviceType).c_str()); 
        ImGui::NextColumn(); 
      }
      ImGui::Columns(1);
    }

    ImGui::Separator();

    {
      ImGui::Columns(2);
      // Debug Utils
      {
        ImGui::Text("Vulkan Debug Utils"); 
        ImGui::NextColumn(); 
        ImGui::Text("%d", m_configuration.graphics_debug.enable); 
        ImGui::NextColumn(); 
      }
    }

    ImGui::Separator();

    {
      ImGui::Columns(2);
      // Average Frame Time
      {
        ImGui::Text("Average Frame Time"); 
        ImGui::NextColumn(); 
        ImGui::Text("%f ms", (GetAverageFrameTime() * 1000.0)); 
        ImGui::NextColumn(); 
      }
      // Current Frame Time
      {
        ImGui::Text("Current Frame Time");
        ImGui::NextColumn();
        ImGui::Text("%f ms", GetFrameElapsedTime() * 1000.0f);
        ImGui::NextColumn();
      }
      // Max Frame Time
      {
        ImGui::Text("Max Past %d Frames Time", kWindowFrames);
        ImGui::NextColumn();
        ImGui::Text("%f ms", GetMaxWindowFrameTime() * 1000.0f);
        ImGui::NextColumn();
      }
      // Min Frame Time
      {
        ImGui::Text("Min Past %d Frames Time", kWindowFrames);
        ImGui::NextColumn();
        ImGui::Text("%f ms", GetMinWindowFrameTime() * 1000.0f);
        ImGui::NextColumn();
      }
      // Frames Per Second
      {
        ImGui::Text("Frames Per Second"); 
        ImGui::NextColumn(); 
        ImGui::Text("%f fps", GetFramesPerSecond()); 
        ImGui::NextColumn(); 
      }
      // Total Frames
      {
        ImGui::Text("Total Frames"); 
        ImGui::NextColumn(); 
        ImGui::Text("%llu frames", static_cast<unsigned long long>(GetElapsedFrames())); 
        ImGui::NextColumn(); 
      }
      // Elapsed Time
      {
        ImGui::Text("Elapsed Time (s)"); 
        ImGui::NextColumn(); 
        ImGui::Text("%f seconds", GetElapsedTime()); 
        ImGui::NextColumn(); 
      }
      ImGui::Columns(1);
    }

    ImGui::Separator();

    // Function call times
    {
      ImGui::Columns(2);
      // Update Function Call Time
      {
        ImGui::Text("Update Call Time"); 
        ImGui::NextColumn(); 
        ImGui::Text("%f ms", m_update_fn_time * 1000.0); 
        ImGui::NextColumn(); 
      }
      // Render Function Call Time
      {
        ImGui::Text("Render Call Time");
        ImGui::NextColumn();
        ImGui::Text("%f ms", m_render_fn_time * 1000.0f);
        ImGui::NextColumn();
      }
      // Present Function Call Time
      {
        ImGui::Text("Present Call Time");
        ImGui::NextColumn();
        ImGui::Text("%f ms", m_present_fn_time * 1000.0f);
        ImGui::NextColumn();
      }
      ImGui::Columns(1);
    }

    ImGui::Separator();

    // Swapchain
    {
      ImGui::Columns(2);
      // Image count
      {
        ImGui::Text("Swapchain Image Count"); 
        ImGui::NextColumn(); 
        ImGui::Text("%u", configuration.swapchain.image_count); 
        ImGui::NextColumn(); 
      }
      // Format
      {
        ImGui::Text("Swapchain Format"); 
        ImGui::NextColumn(); 
        ImGui::Text("%s", vkex::ToStringShort(configuration.swapchain.color_format).c_str()); 
        ImGui::NextColumn(); 
      }
      // Color space
      {
        ImGui::Text("Swapchain Color Space"); 
        ImGui::NextColumn(); 
        ImGui::Text("%s", vkex::ToStringShort(configuration.swapchain.color_space).c_str()); 
        ImGui::NextColumn(); 
      }
      // Size
      {
        ImGui::Text("Swapchain Size"); 
        ImGui::NextColumn(); 
        ImGui::Text("%ux%u", configuration.window.width, configuration.window.height); 
        ImGui::NextColumn();
      }
      // Size
      {
        ImGui::Text("Present Mode"); 
        ImGui::NextColumn(); 
        ImGui::Text("%s", vkex::ToStringShort(configuration.swapchain.present_mode).c_str());
        ImGui::NextColumn();
      } 
      ImGui::Columns(1);
    }

    ImGui::Separator();

    // Vulkan call times
    {
      ImGui::Columns(2);
      // vkQueuePresentKHR
      {
        ImGui::Text("vkQueuePresentKHR"); 
        ImGui::NextColumn(); 
        ImGui::Text("%f", m_average_vk_queue_present_time); 
        ImGui::NextColumn(); 
      }
      ImGui::Columns(1);

      //ImGui::PlotLines(
      //  "0 to 100us",
      //  (float*)m_queue_present_times.data() + 2,
      //  m_queue_present_times.size(),
      //  0,
      //  nullptr,
      //  0,
      //  100 * 0.000001f, // microseconds
      //  ImVec2(0, 64),
      //  sizeof(TimeRange));
    }
  }
  ImGui::End();
}

} // namespace vkex
