#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <box2d/box2d.h>
#include "Box.h"
#include <iostream>
#include <vector>
#include <map>
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "texture.h"
#include <glm/glm.hpp> 
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <random>
#include "framebuffer.h"
#include "simulation_manager.h"
#include "ImGuiFileBrowser.h"

// structure for hloding shape data for drawing
// p1: top-left corner, p2: bottom-right corner, color: shape color
namespace MyShape
{
    struct Shape {
        ImVec2 p1;
        ImVec2 p2;
        ImVec4 color;
    };
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void window_size_callback_static(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void limitFPS(double* lastTime, float targetFPS);
bool hoverImGUIWindow(const char* windowName);

void showFileOpen();
void showFileSave();

// screen dimentions
const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 800;
const float RENDER_SCALE = 30.0f;
const float TARGET_FPS = 60.0f;

// Arrays for storing pressed and processed keys
bool keys[1024];
bool keysProcessed[1024];
bool mouseKeys[3];
bool mouseKeysProcessed[3];

// Flags for canvas menu
bool show_file_open = false;
bool show_file_save = false;

// Instanciate a custom framebuffer and a simulation manager. The former is used for rendering the simultation, while the latter
// manages the simulation objects and parameters 
FrameBuffer sceneBuffer = FrameBuffer();
SimulationManager simulationManager = SimulationManager(RENDER_SCALE, SCREEN_WIDTH, SCREEN_HEIGHT);

int main(int argc, char* argv[]) 
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    //glfwWindowHint(GLFW_RESIZABLE, false);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Test box2D", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    //glfwSwapInterval(1);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, window_size_callback_static);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // load shaders
    ResourceManager::loadShader("shaders source/vertex.vs", "shaders source/fragment.fs", nullptr, "sprite");
    ResourceManager::loadTexture("textures/container.jpg", false, "container");
    ResourceManager::getTexture("container");
    // configure shaders
    glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT),
        0.0f, -1.0f, 0.0f);
    ResourceManager::getShader("sprite").use().SetInteger("image", 0);
    ResourceManager::getShader("sprite").use().SetMatrix4("projection", proj);

    double lastTime = glfwGetTime();
    SpriteRenderer* renderer = new SpriteRenderer(ResourceManager::getShader("sprite"));

    // GUI initialization
    // --------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiStyle ref_saved_style = style;

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    ImGuiWindowFlags renderingWindowFlags = 0;
    ImGuiWindowFlags canvasWindowFlags = 0;

    /*renderingWindowFlags |= ImGuiWindowFlags_NoResize;
    renderingWindowFlags |= ImGuiWindowFlags_NoMove;
    renderingWindowFlags |= ImGuiWindowFlags_NoCollapse;
    renderingWindowFlags |= ImGuiWindowFlags_NoScrollbar;
    renderingWindowFlags |= ImGuiWindowFlags_NoTitleBar;*/

    canvasWindowFlags |= ImGuiWindowFlags_MenuBar;

    // file dialog initializaxtino for saving and opening files
    imgui_addons::ImGuiFileBrowser file_dialog;

    // map connecting wall position and dimensions ---> map<position, dimension>
    std::map<std::vector<float>, std::vector<float>> wallsData = 
    {
        {{(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, (float)SCREEN_HEIGHT / RENDER_SCALE + 10.0f} , {(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, 10.0f}},  // bottom
        {{(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, -10.0f}                                      , {(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, 10.0f}},  // top
        {{-10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}                                     , {10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}}, // left
        {{(float)SCREEN_WIDTH / RENDER_SCALE + 10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}  , {10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}}, // right
    };

    std::map< std::vector<float>, std::vector<float>>::iterator it;
    for (it = wallsData.begin(); it != wallsData.end(); it++) 
    {
        b2BodyDef groundBodyDef;
        groundBodyDef.position.Set(it->first[0], it->first[1]);
        b2Body* groundBody = simulationManager.m_world->CreateBody(&groundBodyDef);
        // ground fixture
        b2PolygonShape groundBox;
        groundBox.SetAsBox(it->second[0], it->second[1]);
        groundBody->CreateFixture(&groundBox, 0.0f);
    }

    // buffer initialization for rendering the simulation
    sceneBuffer.init(SCREEN_WIDTH, SCREEN_HEIGHT);

    // ImGUI initial color settings
    ImVec4 clearColor = ImVec4(0.3f, 0.4f, 0.8f, 1.0f);
    ImVec4 shapeColor = ImVec4(1.0f, 0.0f, 0.5f, 1.0f);

    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
        // imgui frame creation
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 2 * main_viewport->Size.x / 3, main_viewport->WorkPos.y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3), ImGuiCond_Once);
        

        // control window 
        ImGui::Begin("Control");
        if (ImGui::Button("Play")) 
        {
            simulationManager.render = true;
            simulationManager.simulate = true;
            simulationManager.reset = false;
            //populate = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            simulationManager.simulate = false;
        }
       
        if (ImGui::Button("Reset")) 
        {
            simulationManager.reset = true;
            simulationManager.render = false;
            simulationManager.populate = true;
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 130.0f);
        if (ImGui::Checkbox("Enable gravity", &simulationManager.gravityOn))
            simulationManager.enableGravity();

        ImGui::ColorEdit3("background color", (float*)&clearColor);
        ImGui::InputInt("Number of boxes", &simulationManager.boxNumber);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        // window for rendering the simulation

        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(2 * SCREEN_WIDTH / 3, 2 * SCREEN_HEIGHT / 3), ImGuiCond_Once);
        style.WindowPadding = ImVec2(0.0f, 0.0f);
        ImGui::Begin("Rendering", nullptr, renderingWindowFlags);
        if (simulationManager.reset)
        {
           simulationManager.m_boxes.clear();
           // deletes all the dynamic objects inside the world
           for (b2Body* b = simulationManager.m_world->GetBodyList(); b != nullptr;)
           {
               b2Body* next = b->GetNext();
               if (b->GetType() == b2_dynamicBody)
                   simulationManager.m_world->DestroyBody(b);
               b = next;
           }
           /* regenerates the boxes
           for (int i = 0; i < num_boxes; i++)
           {
               Box newBox;
               newBox.init(m_world, glm::vec2(xPos(randGenerator), yPos(randGenerator)), glm::vec2(size(randGenerator), size(randGenerator)));
               newBox.setRotation(glm::radians(rotation(randGenerator)));
               newBox.setColor(glm::vec3(r(randGenerator), g(randGenerator), b(randGenerator)));
               m_boxes.push_back(newBox);
           }*/
        }
        if (simulationManager.render)
        {
            if (simulationManager.populate)
            {
                simulationManager.generateRandomBox(simulationManager.boxNumber);
                simulationManager.populate = false;
            }
            ImGui::Image(
                (ImTextureID)sceneBuffer.getFrameTexture(),
                ImGui::GetContentRegionAvail(),
                ImVec2(0, 1),
                ImVec2(1, 0));

            // write to the custom framebuffer
            sceneBuffer.bind();
            glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
            glClear(GL_COLOR_BUFFER_BIT);

            for (auto& box : simulationManager.m_boxes)
            {
                glm::vec2 pos = glm::vec2(box.getBody()->GetPosition().x, box.getBody()->GetPosition().y);
                glm::vec2 size = box.getDimensions();
                renderer->drawSpriteBox2D(RENDER_SCALE, ResourceManager::getTexture("container"), pos, size, glm::degrees(box.getBody()->GetAngle()), box.getColor());
            }

            sceneBuffer.unbind();
            // perform a step in the simulation
            if (simulationManager.simulate)
                simulationManager.m_world->Step(1.0f / 60.0f, 6, 2);
        }
        else
        {
            ImGui::Image(
                (ImTextureID)sceneBuffer.getFrameTexture(),
                ImGui::GetContentRegionAvail(),
                ImVec2(0, 1),
                ImVec2(1, 0));

            // write to the custom framebuffer
            sceneBuffer.bind();
            glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
            glClear(GL_COLOR_BUFFER_BIT);
            sceneBuffer.unbind();
        }
        ImGui::End();

        // canvas for drawing shapes 
        style.WindowPadding = ref_saved_style.WindowPadding;
        ImGui::Begin("Canvas", nullptr, canvasWindowFlags);

        // Canvas variables initialization
        static std::map<int, ImVector<MyShape::Shape>> pts;
        static ImVec2 scrolling(0.0f, 0.0f);
        static bool opt_enable_grid = true;
        static bool opt_enable_context_menu = true;
        static bool adding_line = false;
        static int current_item = 0;
        const char* items[] = { "Line", "Rectangle", "Circle" };

        // Menu
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("Open", NULL, &show_file_open);
                ImGui::MenuItem("Save", NULL, &show_file_save);

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (show_file_open) ImGui::OpenPopup("Open file");
        if (show_file_save) ImGui::OpenPopup("Save file");

        if (file_dialog.showFileDialog("Open file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), &show_file_open))
        {
            std::cout << file_dialog.selected_fn << std::endl;
            std::cout << file_dialog.selected_path << std::endl;
        }
        if (file_dialog.showFileDialog("Save file", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), &show_file_save))
        {
            std::cout << file_dialog.selected_fn << std::endl;
            std::cout << file_dialog.selected_path << std::endl;
            std::cout << file_dialog.ext << std::endl;
            

        }

        ImGui::Checkbox("Enable grid", &opt_enable_grid);
        ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);
        ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
        ImGui::ColorEdit3("shape color", (float*)&shapeColor); 
        ImGui::Combo("Shape selection", &current_item, items, IM_ARRAYSIZE(items));

        // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
        if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        // Draw border and background color
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool is_hovered = ImGui::IsItemHovered(); // Hovered
        const bool is_active = ImGui::IsItemActive();   // Held
        const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
        const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

        // Add first and second point
        if (is_hovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            MyShape::Shape shape = {mouse_pos_in_canvas, mouse_pos_in_canvas, shapeColor};
            pts[current_item].push_back(shape);
            adding_line = true;
        }
        if (adding_line)
        {
            pts[current_item].back().p2 = mouse_pos_in_canvas;
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                adding_line = false;
        }
        // Pan (we use a zero mouse threshold when there's no context menu)
        // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
        const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
        {
            scrolling.x += io.MouseDelta.x;
            scrolling.y += io.MouseDelta.y;
        }

        // Context menu (under default mouse threshold)
        ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
        if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
            ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
        if (ImGui::BeginPopup("context"))
        {
            if (adding_line)
                pts[current_item].resize(pts.size() - 1);
            adding_line = false;
            if (ImGui::MenuItem("Remove one", NULL, false, pts[current_item].Size > 0)) { pts[current_item].resize(pts[current_item].size() - 1); }

            if (ImGui::MenuItem("Remove all", NULL, false, pts.size() > 0))
            { 
                std::map<int, ImVector<MyShape::Shape>>::iterator it;
                for (it = pts.begin(); it != pts.end(); it++)
                    it->second.clear();
            }
            ImGui::EndPopup();
        }

        // Draw grid + all lines in the canvas
        draw_list->PushClipRect(canvas_p0, canvas_p1, true);
        if (opt_enable_grid)
        {
            const float GRID_STEP = 64.0f;
            for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
                draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
            for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
                draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
            // Highlight the canvas center
            draw_list->AddLine(ImVec2(origin.x - 10.0f, origin.y), ImVec2(origin.x + 10.0f, origin.y), IM_COL32(255, 0, 0, 255));
            draw_list->AddLine(ImVec2(origin.x, origin.y - 10.0f ), ImVec2(origin.x, origin.y + 10.0f), IM_COL32(255, 0, 0, 255));
        }
        std::map<int, ImVector<MyShape::Shape>>::iterator it;
        for (it = pts.begin(); it != pts.end(); it++)
        {
           for (int n = 0; n < it->second.Size; n ++)
           {
               switch (it->first)
               {
               case 0:
                   draw_list->AddLine(ImVec2(origin.x + pts[0][n].p1.x, origin.y + pts[0][n].p1.y), ImVec2(origin.x + pts[0][n].p2.x, origin.y + pts[0][n].p2.y), ImGui::ColorConvertFloat4ToU32(pts[0][n].color), 2.0f);
                   break;
               case 1:
                   draw_list->AddRect(ImVec2(origin.x + pts[1][n].p1.x, origin.y + pts[1][n].p1.y), ImVec2(origin.x + pts[1][n].p2.x, origin.y + pts[1][n].p2.y), ImGui::ColorConvertFloat4ToU32(pts[1][n].color));
                   break;
               case 2:
                   draw_list->AddCircle(ImVec2(origin.x + pts[2][n].p1.x, origin.y + pts[2][n].p1.y), sqrt(pow(pts[2][n].p1.x - pts[2][n].p2.x, 2) + pow(pts[2][n].p1.y - pts[2][n].p2.y, 2)), ImGui::ColorConvertFloat4ToU32(pts[2][n].color));
               }
           }
        }
        draw_list->PopClipRect();
        ImGui::End();
       
        ImGui::ShowDemoWindow();
        ImGui::Render();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
        limitFPS(&lastTime, TARGET_FPS);
    }
    glfwTerminate();
    return 0;
}

// Registers the keys inputs
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) 
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key >= 0 && key <= 1024) 
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE) 
        {
            keys[key] = false;
            keysProcessed[key] = false;
        }
    }
}

// Registers the mouse inputs
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) 
{
    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) 
    {
        if (action == GLFW_PRESS)
            mouseKeys[button] = true;
        else if (action == GLFW_RELEASE) 
        {
            mouseKeys[button] = false;
            mouseKeysProcessed[button] = false;
        }
    }
}

// limits the FPS of the application to targetFPS
void limitFPS(double* lastTime, const float targetFPS) 
{
    while (glfwGetTime() < *lastTime + 1.0f / targetFPS) { ; }
    *lastTime += 1.0f / targetFPS;
}

// Resets the viewport dimensions whenevers the main window is resized
void window_size_callback_static(GLFWwindow* window, int width, int height) 
{
    glViewport(0, 0, width, height);
    sceneBuffer.rescaleFrameBuffer(width, height);
}

// returns true if the mose is hovering over a window with name windowName
bool hoverImGUIWindow(const char* windowName) 
{
    ImGuiContext& g = *GImGui;
    return !strcmp(g.HoveredWindow ? g.HoveredWindow->Name : "NULL", windowName);
}

// Shows a window for opening a saved sketch in the canvas window
void showFileOpen() 
{
    ImGui::Begin("Open File", &show_file_open);
    ImGui::Text("Menu for opening a file");
    ImGui::End();
} 

// Shows a window for saving a file in the canvas window 
void showFileSave() 
{
    ImGui::Begin("Save File", &show_file_save);
    

    ImGui::End();
}

