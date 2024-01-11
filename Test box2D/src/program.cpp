#define _CRT_SECURE_NO_WARNINGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <box2d/box2d.h>
#include "box2DObject.h"
#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <fstream>
#include <cctype>
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "texture.h"
#include <glm/glm.hpp> 
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "framebuffer.h"
#include "simulation_manager.h"
#include "ImGuiFileBrowser.h"

#define PI atan(1) * 4
// structure for hloding shape data for drawing
// p1: top-left corner, p2: bottom-right corner, color: shape color
namespace MyShape
{
    struct Shape {
        std::string name;
        ImVec2 p1;
        ImVec2 p2;
        ImVec4 color;
        b2BodyType type;
        float area; 
    };
}
// callback for registering the pressed keys
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
// callback for resizing the viewport when the window is resized
void window_size_callback_static(GLFWwindow* window, int width, int height);
// callback for registering the mouse buttons
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
// limits the FPS to a given amount
void limitFPS(double* lastTime, float targetFPS);
// save a canva sketch to file
void saveCanvasFile(const std::string& filePath, const std::map<int, ImVector<MyShape::Shape>>& pts);
// loads a canva sketch form file
void loadCanvasFile(const std::string& filePath, std::map<int, ImVector<MyShape::Shape>>* pts);
// creates a box Box2D object from a canva sketch
void createBoxObject(const ImVec2 origin, const MyShape::Shape& shape);
// creates a static Box2D object 
void createWallObject(const ImVec2 origin, const MyShape::Shape& shape);
// creates a circle Box2D object
void createCircleObject(const ImVec2 origin, const MyShape::Shape& shape);
// checks if two points in the canva are overlapping. Returns true if they overlap
bool checkPointsOverlapping(ImVec2 p1, ImVec2 p2);

// screen dimentions
const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 800;
// scaling factor for transforming Box2D dimensions in rendering dimensions
const float RENDER_SCALE = 30.0f;
// maximum FPS
const float TARGET_FPS = 60.0f;

// Arrays for storing pressed and processed keys
bool keys[1024];
bool keysProcessed[1024];
bool mouseKeys[3];
bool mouseKeysProcessed[3];

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

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Engine 2D", nullptr, nullptr);
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
    ResourceManager::loadTexture("textures/bricks2.jpg", false, "bricks");
    ResourceManager::loadTexture("textures/awesomeface.png", true, "ball");
    ResourceManager::getTexture("container");
    ResourceManager::getTexture("bricks");
    ResourceManager::getTexture("ball");
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
    ImGuiWindowFlags canvaWindowFlags = 0;

    /*renderingWindowFlags |= ImGuiWindowFlags_NoResize;
    renderingWindowFlags |= ImGuiWindowFlags_NoMove;
    renderingWindowFlags |= ImGuiWindowFlags_NoCollapse;
    renderingWindowFlags |= ImGuiWindowFlags_NoScrollbar;
    renderingWindowFlags |= ImGuiWindowFlags_NoTitleBar;*/

    canvaWindowFlags |= ImGuiWindowFlags_MenuBar;

    // file dialog initializaxtino for saving and opening files
    imgui_addons::ImGuiFileBrowser file_dialog;

    // map connecting wall position and dimensions ---> map<position, dimension>
    /*std::map<std::vector<float>, std::vector<float>> wallsData =
    {
        {{(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, (float)SCREEN_HEIGHT / RENDER_SCALE + 10.0f} , {(float)SCREEN_WIDTH / RENDER_SCALE , 20.0f}},  // bottom
        {{(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, -10.0f}                                      , {(float)SCREEN_WIDTH / RENDER_SCALE , 20.0f}},  // top
        {{-10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}                                     , {20.0f, (float)SCREEN_HEIGHT / RENDER_SCALE}}, // left
        {{(float)SCREEN_WIDTH / RENDER_SCALE + 10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}  , {20.0f, (float)SCREEN_HEIGHT / RENDER_SCALE }}, // right
    };
    std::vector<Wall> walls;
    std::map< std::vector<float>, std::vector<float>>::iterator it;
    for (it = wallsData.begin(); it != wallsData.end(); it++) 
    {
        Wall wall;
        wall.init(simulationManager.m_world, glm::vec2(it->first[0], it->first[1]), glm::vec2(it->second[0], it->second[1]));
        walls.push_back(wall);
    }*/

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
        style.WindowPadding = ref_saved_style.WindowPadding;

        ImGui::Begin("Control");
        if (ImGui::Button("Play")) 
        {
            simulationManager.play = true;
            simulationManager.simulate = true;
            //simulationManager.reset = false;
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
            simulationManager.play = false;
            //simulationManager.populate = true;
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 130.0f);
        if (ImGui::Checkbox("Enable gravity", &simulationManager.gravityOn))
            simulationManager.enableGravity();

        ImGui::ColorEdit3("background color", (float*)&clearColor);
        ImGui::InputInt("Number of boxes", &simulationManager.boxNumber);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        // canva for drawing shapes 
        ImGui::Begin("Canva", nullptr, canvaWindowFlags);

        // Canvas variables initialization
        static std::map<int, ImVector<MyShape::Shape>> pts;
        static ImVec2 scrolling(0.0f, 0.0f);
        static bool opt_enable_grid = true;
        static bool opt_enable_context_menu = true;
        static bool adding_line = false;
        static int current_item = 0;
        const char* items[] = { "Line", "Rectangle", "Circle" };

        // Menu
        // Flags for canva menu
        static bool show_file_open = false;
        static bool show_file_save = false;
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

        ImGui::Checkbox("Enable grid", &opt_enable_grid);
        ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);
        ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
        ImGui::ColorEdit3("shape color", (float*)&shapeColor);
        ImGui::Combo("Shape selection", &current_item, items, IM_ARRAYSIZE(items)); ImGui::SameLine();
        // flag for drawing static and dynamic objects in the canva
        static int isObjectStatic;
        ImGui::RadioButton("dynamic", &isObjectStatic, 0); ImGui::SameLine();
        ImGui::RadioButton("static", &isObjectStatic, 1);

        // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
        ImVec2 canva_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
        ImVec2 canva_sz = ImGui::GetContentRegionAvail();   // Resize canva to what's available
        if (canva_sz.x < 50.0f) canva_sz.x = 50.0f;
        if (canva_sz.y < 50.0f) canva_sz.y = 50.0f;
        ImVec2 canva_p1 = ImVec2(canva_p0.x + canva_sz.x, canva_p0.y + canva_sz.y);

        // Draw border and background color
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canva_p0, canva_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canva_p0, canva_p1, IM_COL32(255, 255, 255, 255));

        // This will catch our interactions
        ImGui::InvisibleButton("canva", canva_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool is_hovered = ImGui::IsItemHovered(); // Hovered
        const bool is_active = ImGui::IsItemActive();   // Held
        const ImVec2 origin(canva_p0.x + scrolling.x, canva_p0.y + scrolling.y); // Lock scrolled origin
        const ImVec2 mouse_pos_in_canva(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

        // Add first and second point
        if (is_hovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            MyShape::Shape shape;
            shape.p1 = mouse_pos_in_canva;
            shape.p2 = mouse_pos_in_canva;
            shape.color = shapeColor;
            shape.type = isObjectStatic == 0 ? b2_dynamicBody : b2_staticBody;
            if (current_item == 0 && shape.type == b2_dynamicBody)
                shape.name = "Box";
            else if (current_item == 0 && shape.type == b2_staticBody)
                shape.name = "Wall";
            else if (current_item == 1 && shape.type == b2_dynamicBody)
                shape.name = "Box";
            else if (current_item == 1 && shape.type == b2_staticBody)
                shape.name = "Wall";
            else if (current_item == 2)
                shape.name = "Ball";

            pts[current_item].push_back(shape);
            adding_line = true;
        }
        if (adding_line)
        {
            pts[current_item].back().p2 = mouse_pos_in_canva;
            if (pts[current_item].back().name == "Ball")
                pts[current_item].back().area = PI * pow(sqrt(pow(pts[current_item].back().p1.x - pts[current_item].back().p2.x, 2) + pow((pts[current_item].back().p1.y - pts[current_item].back().p2.y), 2)),2);
            else
                pts[current_item].back().area = abs(pts[current_item].back().p1.x - pts[current_item].back().p2.x) * abs(pts[current_item].back().p1.y - pts[current_item].back().p2.y);
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) // left mouse button released
            {
                adding_line = false;
                // check for too small areas
                if (pts[current_item].back().area > 100 || !checkPointsOverlapping(pts[current_item].back().p1, pts[current_item].back().p2))
                {
                    switch (isObjectStatic)
                    {
                    case 0:
                        // generates dynamic Box2D objects from the objects in the canva
                        // we only need to generate the last object added and not all the ones in the canva
                        switch (current_item)
                        {
                        case 0:
                            createBoxObject(origin, pts[current_item].back());
                            break;
                        case 1:
                            createBoxObject(origin, pts[current_item].back());
                            break;
                        case 2:
                            createCircleObject(origin, pts[current_item].back());
                            break;
                        }
                        break;
                    case 1:
                        switch (current_item)
                        {
                        case 0:
                            createWallObject(origin, pts[current_item].back());
                            break;
                        case 1:
                            createWallObject(origin, pts[current_item].back());
                            break;
                        case 2:
                            createCircleObject(origin, pts[current_item].back());
                            break;
                        }
                        break;
                    }
                }
                else
                    pts[current_item].resize(pts[current_item].size() - 1);
            }
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
            // remove last item added in the canva
            if (ImGui::MenuItem("Remove one", NULL, false, pts.size() > 0))
            {
                pts[current_item].resize(pts[current_item].size() - 1);
                simulationManager.clearLastObject();
            }
            // remove all the items in the canva
            if (ImGui::MenuItem("Remove all", NULL, false, pts.size() > 0))
            {
                std::map<int, ImVector<MyShape::Shape>>::iterator it;
                for (it = pts.begin(); it != pts.end(); it++)
                    it->second.clear();
                simulationManager.clearObjects();
            }
            ImGui::EndPopup();
        }

        // Draw grid + all lines in the canva
        draw_list->PushClipRect(canva_p0, canva_p1, true);
        if (opt_enable_grid)
        {
            const float GRID_STEP = 64.0f;
            for (float x = fmodf(scrolling.x, GRID_STEP); x < canva_sz.x; x += GRID_STEP)
                draw_list->AddLine(ImVec2(canva_p0.x + x, canva_p0.y), ImVec2(canva_p0.x + x, canva_p1.y), IM_COL32(200, 200, 200, 40));
            for (float y = fmodf(scrolling.y, GRID_STEP); y < canva_sz.y; y += GRID_STEP)
                draw_list->AddLine(ImVec2(canva_p0.x, canva_p0.y + y), ImVec2(canva_p1.x, canva_p0.y + y), IM_COL32(200, 200, 200, 40));
            // Highlight the canva center
            draw_list->AddLine(ImVec2(origin.x - 10.0f, origin.y), ImVec2(origin.x + 10.0f, origin.y), IM_COL32(255, 0, 0, 255));
            draw_list->AddLine(ImVec2(origin.x, origin.y - 10.0f), ImVec2(origin.x, origin.y + 10.0f), IM_COL32(255, 0, 0, 255));
        }
        std::map<int, ImVector<MyShape::Shape>>::iterator it;
        for (it = pts.begin(); it != pts.end(); it++)
        {
            for (int n = 0; n < it->second.Size; n++)
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
                    break;
                }
            }
        }
        draw_list->PopClipRect();

        if (show_file_open) ImGui::OpenPopup("Open file");
        if (show_file_save) ImGui::OpenPopup("Save file");

        if (file_dialog.showFileDialog("Open file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), &show_file_open))
        {
            loadCanvasFile(file_dialog.selected_path, &pts);

            std::map<int, ImVector<MyShape::Shape>>::iterator it;
            for (it = pts.begin(); it != pts.end(); it++)
            {
                for (int n = 0; n < it->second.Size; n++)
                {
                    switch (it->second[n].type)
                    {
                    case b2_dynamicBody:
                        createBoxObject(origin, it->second[n]);
                        break;
                    case b2_staticBody:
                        createWallObject(origin, it->second[n]);
                        break;
                    }
                }
            }
        }
        if (file_dialog.showFileDialog("Save file", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), &show_file_save))
            saveCanvasFile(file_dialog.selected_path, pts);

        ImGui::End();

        // window for rendering the simulation
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(2 * SCREEN_WIDTH / 3, 2 * SCREEN_HEIGHT / 3), ImGuiCond_Once);
        style.WindowPadding = ImVec2(0.0f, 0.0f);

        ImGui::Begin("Rendering", nullptr, renderingWindowFlags);
        if (simulationManager.reset)
        {
            simulationManager.clearObjects();
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
        if (simulationManager.play)
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

            if (simulationManager.reset)
            {
                simulationManager.reset = false;
                /*
                std::map< std::vector<float>, std::vector<float>>::iterator it1;
                for (it1 = wallsData.begin(); it1 != wallsData.end(); it1++)
                {
                    Wall wall;
                    wall.init(simulationManager.m_world, glm::vec2(it1->first[0], it1->first[1]), glm::vec2(it1->second[0], it1->second[1]));
                    walls.push_back(wall);
                }*/
                std::map<int, ImVector<MyShape::Shape>>::iterator it;
                for (it = pts.begin(); it != pts.end(); it++)
                {
                    for (int n = 0; n < it->second.Size; n++)
                    {
                        switch (it->second[n].type)
                        {
                        case b2_dynamicBody:
                            if (it->second[n].name == "Box")
                                createBoxObject(origin, it->second[n]);
                            if (it->second[n].name == "Box")
                                createBoxObject(origin, it->second[n]);
                            if (it->second[n].name == "Ball")
                                createCircleObject(origin, it->second[n]);
                            break;
                        case b2_staticBody:
                            if (it->second[n].name == "Wall")
                                createWallObject(origin, it->second[n]);
                            if (it->second[n].name == "Wall")
                                createWallObject(origin, it->second[n]);
                            if (it->second[n].name == "Ball")
                                createCircleObject(origin, it->second[n]);
                            break;
                        }
                    }
                }
            }

            for (auto& object : simulationManager.m_objects)
            {
                if (object.getName() == "Box")
                {
                    glm::vec2 pos = glm::vec2(object.getBody()->GetPosition().x, object.getBody()->GetPosition().y);
                    glm::vec2 size = object.getDimensions();
                    renderer->drawSpriteBox2D(RENDER_SCALE, ResourceManager::getTexture("container"), pos, size, glm::degrees(object.getBody()->GetAngle()), object.getColor());
                }
                else if (object.getName() == "Wall")
                {
                    glm::vec2 pos = glm::vec2(object.getBody()->GetPosition().x, object.getBody()->GetPosition().y);
                    glm::vec2 size = object.getDimensions();
                    renderer->drawSpriteBox2D(RENDER_SCALE, ResourceManager::getTexture("bricks"), pos, size, glm::degrees(object.getBody()->GetAngle()), object.getColor());
                }
                else if (object.getName() == "Ball")
                {
                    glm::vec2 pos = glm::vec2(object.getBody()->GetPosition().x, object.getBody()->GetPosition().y);
                    glm::vec2 size = object.getDimensions();
                    renderer->drawSpriteBox2D(RENDER_SCALE, ResourceManager::getTexture("ball"), pos, size, glm::degrees(object.getBody()->GetAngle()), object.getColor());
                }
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

        //ImGui::ShowDemoWindow();
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

void limitFPS(double* lastTime, const float targetFPS) 
{
    while (glfwGetTime() < *lastTime + 1.0f / targetFPS) { ; }
    *lastTime += 1.0f / targetFPS;
}

void window_size_callback_static(GLFWwindow* window, int width, int height) 
{
    glViewport(0, 0, width, height);
    sceneBuffer.rescaleFrameBuffer(width, height);
}

void saveCanvasFile(const std::string& filePath, const std::map<int,ImVector<MyShape::Shape>>& pts)
{
    std::ofstream outfile(filePath, std::ios_base::app);

    outfile << "{" << '\n';

    std::map<int, ImVector<MyShape::Shape>>::const_iterator it;
    for (it = pts.begin(); it != pts.end(); it++)
    {
        outfile << "\t[" << it->first << "]:\n\t{\n";
        for (auto& shape : it->second)
        {
            outfile << "\t\t";
            outfile << "(" << shape.p1.x << "," << shape.p1.y << ")," << "(" << shape.p2.x << "," << shape.p2.y << "),"
                << "(" << shape.color.x << "," << shape.color.y << "," << shape.color.z << "," << shape.color.w << "),"
                << shape.type << "," << shape.area << " \n";
        }
        outfile << "\t}\n";
    }
    outfile << "}";
}

void loadCanvasFile(const std::string& filePath, std::map<int, ImVector<MyShape::Shape>>* pts)
{
    (*pts).clear();
    std::ifstream infile(filePath, std::ios_base::in);
    std::string buffer;

    int element_number{};
    if (infile.is_open())
    {
        while (infile)
        {
            std::getline(infile, buffer);
            for (unsigned int i = 0; i < buffer.length(); i++)
            {
                if (buffer[i] == '[')
                    element_number = int(buffer[i + 1] - '0');
                else if (buffer[i] == '\t' && buffer[i + 1] == '\t')
                {
                    std::vector<float> values;
                    std::string b;
                    bool proc_string = false;
                    for (auto& el : buffer)
                    {
                        if (std::isdigit(el) || el == '.')
                            b += el;
                        else if (!std::isdigit(el) && !b.empty())
                        {
                            values.push_back(stof(b));
                            b.clear();
                        }
                    }
                    MyShape::Shape shape = {
                        "pippo",
                        ImVec2(values[0], values[1]), // p1
                        ImVec2(values[2], values[3]), // p2
                        ImVec4(values[4], values[5], values[6], values[7]), // color
                        (b2BodyType)values[8], // body type
                        values[9] // area
                    };  
                    (*pts)[element_number].push_back(shape);
                }
            }
        }
    }
}

void createBoxObject(const ImVec2 origin, const MyShape::Shape& shape)
{
    Box box;
    box.init(simulationManager.m_world, shape.name ,glm::vec2((shape.p1.x + shape.p2.x) / 2 / RENDER_SCALE, (shape.p1.y + shape.p2.y) / 2 / RENDER_SCALE), glm::vec2(abs(shape.p1.x - shape.p2.x) / RENDER_SCALE, abs(shape.p1.y - shape.p2.y) / RENDER_SCALE), b2_dynamicBody);
    simulationManager.m_objects.push_back(box);
}

void createWallObject(const ImVec2 origin, const MyShape::Shape& shape)
{
    Wall wall;
    wall.init(simulationManager.m_world, shape.name, glm::vec2((shape.p1.x + shape.p2.x) / 2 / RENDER_SCALE, (shape.p1.y + shape.p2.y) / 2 / RENDER_SCALE), glm::vec2(abs(shape.p1.x - shape.p2.x) / RENDER_SCALE, abs(shape.p1.y - shape.p2.y) / RENDER_SCALE), b2_staticBody);
    simulationManager.m_objects.push_back(wall);
}

void createCircleObject(const ImVec2 origin, const MyShape::Shape& shape)
{
    Circle circle;
    circle.init(simulationManager.m_world, shape.name, glm::vec2(shape.p1.x / RENDER_SCALE, shape.p1.y / RENDER_SCALE), sqrt(pow(shape.p1.x - shape.p2.x, 2) + pow(shape.p1.y - shape.p2.y, 2)) / RENDER_SCALE, shape.type);
    simulationManager.m_objects.push_back(circle);
}

bool checkPointsOverlapping(ImVec2 p1, ImVec2 p2)
{
    return !(p1.x - p2.x && p1.y - p2.y);
}
