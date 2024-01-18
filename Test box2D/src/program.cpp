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
// structure for holding shape data for drawing
// p1: top-left corner, p2: bottom-right corner, color: shape color

  typedef struct Shape {
        std::string name;
        ImVec2 p1;
        ImVec2 p2;
        ImVec4 color;
        b2BodyType type;
        float area; 
        float rotation;

        void reset() {
            p1 = ImVec2(110.0f, 0.0f);
            p2 = ImVec2(0.0f, 0.0f);
            area = -1.0f;
        }
    } Shape_t ;

// callback for registering the pressed keys
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
// callback for resizing the viewport when the window is resized
void windowSizeCallback(GLFWwindow* window, int width, int height);
// callback for registering the mouse buttons
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
// limits the FPS to a given amount
void limitFPS(double* last_time, float targetFPS);
// save a canva sketch to file
void saveCanvasFile(const std::string& filePath, const std::map<int, ImVector<Shape_t>>& shapes);
// loads a canva sketch form file
void loadCanvasFile(const std::string& filePath, std::map<int, ImVector<Shape_t>>* shapes);
// creates a box Box2D object from a canva sketch
void createBoxObject(const ImVec2 origin, const Shape_t& shape);
// creates a static Box2D object 
void createWallObject(const ImVec2 origin, const Shape_t& shape);
// creates a circle Box2D object
void createCircleObject(const ImVec2 origin, const Shape_t& shape);
// checks if two points in the canva are overlapping. Returns true if they overlap
bool checkPointsOverlapping(ImVec2 p1, ImVec2 p2);
// checks if a point is inside a given rectangluar area
bool isPointInGivenArea(Shape_t area, ImVec2 point);
// draw a rotated shae in the canva
void drawRotatedQuad(ImDrawList* draw_list, ImVec2 origin, Shape_t shape);



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
bool mouse_keys[3];
bool mouse_keys_processed[3];

// Instanciate a custom framebuffer and a simulation manager. The former is used for rendering the simultation, while the latter
// manages the simulation objects and parameters 
FrameBuffer scene_buffer = FrameBuffer();
SimulationManager simulation_manager = SimulationManager(RENDER_SCALE, SCREEN_WIDTH, SCREEN_HEIGHT);

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
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

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
    ResourceManager::getShader("sprite").use().setInteger("image", 0);
    ResourceManager::getShader("sprite").use().setMatrix4("projection", proj);

    double last_time = glfwGetTime();
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
    
    ImGuiWindowFlags rendering_window_flags = 0;
    ImGuiWindowFlags canva_window_flags = 0;

    /*rendering_window_flags |= ImGuiWindowFlags_NoResize;
    rendering_window_flags |= ImGuiWindowFlags_NoMove;
    rendering_window_flags |= ImGuiWindowFlags_NoCollapse;
    rendering_window_flags |= ImGuiWindowFlags_NoScrollbar;
    rendering_window_flags |= ImGuiWindowFlags_NoTitleBar;*/

    canva_window_flags |= ImGuiWindowFlags_MenuBar;

    // file dialog initializaxtino for saving and opening files
    imgui_addons::ImGuiFileBrowser file_dialog;

    // buffer initialization for rendering the simulation
    scene_buffer.init(SCREEN_WIDTH, SCREEN_HEIGHT);

    // ImGUI initial color settings
    ImVec4 clear_color = ImVec4(0.3f, 0.4f, 0.8f, 1.0f);
    ImVec4 shape_color = ImVec4(1.0f, 0.0f, 0.5f, 1.0f);

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
            simulation_manager.play = true;
            simulation_manager.simulate = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            simulation_manager.simulate = false;
        }
       
        if (ImGui::Button("Reset")) 
        {
            simulation_manager.reset = true;
            simulation_manager.play = false;
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 130.0f);
        if (ImGui::Checkbox("Enable gravity", &simulation_manager.gravity_on))
            simulation_manager.enableGravity();

        ImGui::ColorEdit3("background color", (float*)&clear_color);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        // Canva window
        // ------------
        ImGui::Begin("Canva", nullptr, canva_window_flags);

        // Canvas variables initialization
        static std::map<int, ImVector<Shape_t>> shapes;
        std::map<int, ImVector<Shape_t>>::iterator shapes_it;
        static Shape_t selection_shape;
        static bool selection_shape_active = false;
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
        ImGui::ColorEdit3("shape color", (float*)&shape_color); ImGui::SameLine();
        static float rotate_amount;
        ImGui::InputFloat("Degrees", &rotate_amount, 0.0f, 2 * PI);
        ImGui::Combo("Shape selection", &current_item, items, IM_ARRAYSIZE(items)); ImGui::SameLine();
        // flag for drawing static and dynamic objects in the canva
        static int is_object_static;
        ImGui::RadioButton("dynamic", &is_object_static, 0); ImGui::SameLine();
        ImGui::RadioButton("static", &is_object_static, 1);
        static bool select_shape;
        static int modify_shape;
        ImGui::Checkbox("Select shape", &select_shape); ImGui::SameLine();
        ImGui::RadioButton("Rotate", &modify_shape, 0); ImGui::SameLine();
        ImGui::RadioButton("Move", &modify_shape, 1);

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
            if (select_shape)
            {
                if (!isPointInGivenArea(selection_shape, mouse_pos_in_canva))
                {
                    rotate_amount = 0.0f;
                    selection_shape.p1 = mouse_pos_in_canva;
                    selection_shape.p2 = mouse_pos_in_canva;
                    selection_shape.color = ImVec4(1.0f, 1.0f, 1.0f, 0.15f);
                    selection_shape_active = false;
                }
            }
            else
            {
                Shape_t shape;
                shape.p1 = mouse_pos_in_canva;
                shape.p2 = mouse_pos_in_canva;
                shape.color = shape_color;
                shape.type = is_object_static == 0 ? b2_dynamicBody : b2_staticBody;
                shape.rotation = 0.0f;
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
                shapes[current_item].push_back(shape);
            }
            
            adding_line = true;
        }
        if (adding_line)
        {
            if (select_shape)
            {
                if (!selection_shape_active)
                    selection_shape.p2 = mouse_pos_in_canva;
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) // left mouse button released
                {
                    adding_line = false;
                    selection_shape.area = abs(selection_shape.p1.x - selection_shape.p2.x) * abs(selection_shape.p1.y - selection_shape.p2.y);
                    selection_shape_active = true;
                }
            }
            else
            {
                shapes[current_item].back().p2 = mouse_pos_in_canva;
                if (shapes[current_item].back().name == "Ball")
                    shapes[current_item].back().area = PI * pow(sqrt(pow(shapes[current_item].back().p1.x - shapes[current_item].back().p2.x, 2) + pow((shapes[current_item].back().p1.y - shapes[current_item].back().p2.y), 2)), 2);
                else
                    shapes[current_item].back().area = abs(shapes[current_item].back().p1.x - shapes[current_item].back().p2.x) * abs(shapes[current_item].back().p1.y - shapes[current_item].back().p2.y);

                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) // left mouse button released
                {
                    adding_line = false;
                    // delete figure if area too small or p1 and p2 overlap
                    if (shapes[current_item].back().area > 100 || !checkPointsOverlapping(shapes[current_item].back().p1, shapes[current_item].back().p2))
                    {
                        switch (is_object_static)
                        {
                        case 0:
                            // generates dynamic Box2D objects from the objects in the canva
                            // we only need to generate the last object added and not all the ones in the canva
                            switch (current_item)
                            {
                            case 0:
                                createBoxObject(origin, shapes[current_item].back());
                                break;
                            case 1:
                                createBoxObject(origin, shapes[current_item].back());
                                break;
                            case 2:
                                createCircleObject(origin, shapes[current_item].back());
                                break;
                            }
                            break;
                        case 1:
                            switch (current_item)
                            {
                            case 0:
                                createWallObject(origin, shapes[current_item].back());
                                break;
                            case 1:
                                createWallObject(origin, shapes[current_item].back());
                                break;
                            case 2:
                                createCircleObject(origin, shapes[current_item].back());
                                break;
                            }
                            break;
                        }
                    }
                    else
                        shapes[current_item].resize(shapes[current_item].size() - 1);
                }
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
                shapes[current_item].resize(shapes.size() - 1);
            adding_line = false;
            // remove last item added in the canva
            if (ImGui::MenuItem("Remove one", NULL, false, shapes.size() > 0))
            {
                shapes[current_item].resize(shapes[current_item].size() - 1);
                simulation_manager.clearLastObject();
            }
            // remove all the items in the canva
            if (ImGui::MenuItem("Remove all", NULL, false, shapes.size() > 0))
            {
                for (shapes_it = shapes.begin(); shapes_it !=shapes.end(); shapes_it++)
                    shapes_it->second.clear();
                simulation_manager.clearObjects();
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

        // adjust rotation with mousewheel
        if (io.MouseWheel == 1.0)
            rotate_amount += 2.5f;
        else if (io.MouseWheel == -1.0f)
            rotate_amount -= 2.5f;

        // draw figures to the canva
        for (shapes_it = shapes.begin(); shapes_it !=shapes.end(); shapes_it++)
        {
            for (int n = 0; n < shapes_it->second.Size; n++)
            {
                switch (shapes_it->first)
                {
                case 0:
                    draw_list->AddLine(ImVec2(origin.x + shapes[0][n].p1.x, origin.y + shapes[0][n].p1.y), ImVec2(origin.x + shapes[0][n].p2.x, origin.y + shapes[0][n].p2.y), ImGui::ColorConvertFloat4ToU32(shapes[0][n].color), 2.0f);
                    break;
                case 1:
                    if (select_shape && isPointInGivenArea(selection_shape, shapes_it->second[n].p1) && isPointInGivenArea(selection_shape, shapes_it->second[n].p2) && selection_shape_active)
                    {
                        if (modify_shape == 0)
                        {
                            shapes_it->second[n].rotation = rotate_amount;

                            // save rotation to simulation
                            simulation_manager.m_objects[1][n].setRotation(glm::radians(-rotate_amount));
                        }
                        else if (modify_shape == 1)
                        {
                            if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, mouse_threshold_for_pan))
                            {
                                shapes_it->second[n].p1.x += io.MouseDelta.x;
                                shapes_it->second[n].p1.y += io.MouseDelta.y;
                                shapes_it->second[n].p2.x += io.MouseDelta.x;
                                shapes_it->second[n].p2.y += io.MouseDelta.y;

                                selection_shape.p1.x += io.MouseDelta.x;
                                selection_shape.p1.y += io.MouseDelta.y;
                                selection_shape.p2.x += io.MouseDelta.x;
                                selection_shape.p2.y += io.MouseDelta.y;
                            }
                        }
                    }
                    drawRotatedQuad(draw_list, origin, shapes[1][n]);
                    break;
                case 2:
                    draw_list->AddCircle(ImVec2(origin.x + shapes[2][n].p1.x, origin.y + shapes[2][n].p1.y), sqrt(pow(shapes[2][n].p1.x - shapes[2][n].p2.x, 2) + pow(shapes[2][n].p1.y - shapes[2][n].p2.y, 2)), ImGui::ColorConvertFloat4ToU32(shapes[2][n].color));
                    break;
                }
            }
        }
        // draw selection shape if needed
        if (select_shape)
            if (selection_shape.area > 10 || selection_shape.area == -1.0f)
                draw_list->AddRectFilled(ImVec2(origin.x + selection_shape.p1.x, origin.y + selection_shape.p1.y), ImVec2(origin.x + selection_shape.p2.x, origin.y + selection_shape.p2.y), ImGui::ColorConvertFloat4ToU32(selection_shape.color));
        else
            selection_shape.reset();

        draw_list->PopClipRect();

        if (show_file_open) ImGui::OpenPopup("Open file");
        if (show_file_save) ImGui::OpenPopup("Save file");
        
        // load canva from file is needed
        if (file_dialog.showFileDialog("Open file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), &show_file_open))
        {
            loadCanvasFile(file_dialog.selected_path, &shapes);

            for (shapes_it = shapes.begin(); shapes_it !=shapes.end(); shapes_it++)
            {
                for (int n = 0; n < shapes_it->second.Size; n++)
                {
                    switch (shapes_it->second[n].type)
                    {
                    case b2_dynamicBody:
                        if (shapes_it->second[n].name == "Box")
                            createBoxObject(origin, shapes_it->second[n]);
                        else if (shapes_it->second[n].name == "Box")
                            createBoxObject(origin, shapes_it->second[n]);
                        else if (shapes_it->second[n].name == "Ball")
                            createCircleObject(origin, shapes_it->second[n]);
                        break;
                    case b2_staticBody:
                        if (shapes_it->second[n].name == "Wall")
                            createWallObject(origin, shapes_it->second[n]);
                        else if (shapes_it->second[n].name == "Wall")
                            createWallObject(origin, shapes_it->second[n]);
                        else if (shapes_it->second[n].name == "Ball")
                            createCircleObject(origin, shapes_it->second[n]);
                        break;
                    }
                }
            }
        }
        // save current canva to file
        if (file_dialog.showFileDialog("Save file", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), &show_file_save))
            saveCanvasFile(file_dialog.selected_path, shapes);

        ImGui::End();

        // Rendering window
        // ----------------
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(2 * SCREEN_WIDTH / 3, 2 * SCREEN_HEIGHT / 3), ImGuiCond_Once);
        style.WindowPadding = ImVec2(0.0f, 0.0f);

        ImGui::Begin("Rendering", nullptr, rendering_window_flags);
        if (simulation_manager.reset)
        {
            simulation_manager.clearObjects();

            for (shapes_it = shapes.begin(); shapes_it != shapes.end(); shapes_it++)
            {
                for (int n = 0; n < shapes_it->second.Size; n++)
                {
                    switch (shapes_it->second[n].type)
                    {
                    case b2_dynamicBody:
                        if (shapes_it->second[n].name == "Box")
                            createBoxObject(origin, shapes_it->second[n]);
                        else if (shapes_it->second[n].name == "Box")
                            createBoxObject(origin, shapes_it->second[n]);
                        else if (shapes_it->second[n].name == "Ball")
                            createCircleObject(origin, shapes_it->second[n]);
                        break;
                    case b2_staticBody:
                        if (shapes_it->second[n].name == "Wall")
                            createWallObject(origin, shapes_it->second[n]);
                        else if (shapes_it->second[n].name == "Wall")
                            createWallObject(origin, shapes_it->second[n]);
                        else if (shapes_it->second[n].name == "Ball")
                            createCircleObject(origin, shapes_it->second[n]);
                        break;
                    }
                }
            }
            simulation_manager.reset = false;
        }
        if (simulation_manager.play)
        {
            ImGui::Image(
                (ImTextureID)scene_buffer.getFrameTexture(),
                ImGui::GetContentRegionAvail(),
                ImVec2(0, 1),
                ImVec2(1, 0));

            // write to the custom framebuffer
            scene_buffer.bind();
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);

            // reder all the objects in the scene
            std::map<int, std::vector<Box2DObject>>::iterator it;
            for (it = simulation_manager.m_objects.begin(); it != simulation_manager.m_objects.end(); it++)
            {
                for (int n = 0; n < it->second.size(); n++)
                {
                    if (it->second[n].getName() == "Box")
                    {
                        glm::vec2 pos = glm::vec2(it->second[n].getBody()->GetPosition().x, it->second[n].getBody()->GetPosition().y);
                        glm::vec2 size = it->second[n].getDimensions();
                        renderer->drawSpriteBox2D(RENDER_SCALE, ResourceManager::getTexture("container"), pos, size, glm::degrees(it->second[n].getBody()->GetAngle()), it->second[n].getColor());
                    }
                    else if (it->second[n].getName() == "Wall")
                    {
                        glm::vec2 pos = glm::vec2(it->second[n].getBody()->GetPosition().x, it->second[n].getBody()->GetPosition().y);
                        glm::vec2 size = it->second[n].getDimensions();
                        renderer->drawSpriteBox2D(RENDER_SCALE, ResourceManager::getTexture("bricks"), pos, size, glm::degrees(it->second[n].getBody()->GetAngle()), it->second[n].getColor());
                    }
                    else if (it->second[n].getName() == "Ball")
                    {
                        glm::vec2 pos = glm::vec2(it->second[n].getBody()->GetPosition().x, it->second[n].getBody()->GetPosition().y);
                        glm::vec2 size = it->second[n].getDimensions();
                        renderer->drawSpriteBox2D(RENDER_SCALE, ResourceManager::getTexture("ball"), pos, size, glm::degrees(it->second[n].getBody()->GetAngle()), it->second[n].getColor());
                    }
                }
            }

            scene_buffer.unbind();

            // perform a step in the simulation
            if (simulation_manager.simulate)
                simulation_manager.m_world->Step(1.0f / 60.0f, 6, 2);
        }
        else
        {
            ImGui::Image(
                (ImTextureID)scene_buffer.getFrameTexture(),
                ImGui::GetContentRegionAvail(),
                ImVec2(0, 1),
                ImVec2(1, 0));

            // write to the custom framebuffer
            scene_buffer.bind();
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            scene_buffer.unbind();
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
        limitFPS(&last_time, TARGET_FPS);
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

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) 
{
    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) 
    {
        if (action == GLFW_PRESS)
            mouse_keys[button] = true;
        else if (action == GLFW_RELEASE) 
        {
            mouse_keys[button] = false;
            mouse_keys_processed[button] = false;
        }
    }
}

void limitFPS(double* last_time, const float targetFPS) 
{
    while (glfwGetTime() < *last_time + 1.0f / targetFPS) { ; }
    *last_time += 1.0f / targetFPS;
}

void windowSizeCallback(GLFWwindow* window, int width, int height) 
{
    glViewport(0, 0, width, height);
    scene_buffer.rescaleFrameBuffer(width, height);
}

void saveCanvasFile(const std::string& filePath, const std::map<int,ImVector<Shape_t>>& shapes)
{
    std::ofstream outfile(filePath, std::ios_base::app);

    outfile << "{" << '\n';

    std::map<int, ImVector<Shape_t>>::const_iterator shapes_it;
    for (shapes_it = shapes.begin(); shapes_it !=shapes.end(); shapes_it++)
    {
        outfile << "\t[" << shapes_it->first << "]:\n\t{\n";
        for (auto& shape : shapes_it->second)
        {
            outfile << "\t\t";
            outfile << shape.name << "," << "(" << shape.p1.x << ", " << shape.p1.y << "), " << "(" << shape.p2.x << ", " << shape.p2.y << "), "
                << "(" << shape.color.x << "," << shape.color.y << "," << shape.color.z << "," << shape.color.w << "),"
                << shape.type << "," << shape.area << "," << shape.rotation << " \n";
        }
        outfile << "\t}\n";
    }
    outfile << "}";
}

void loadCanvasFile(const std::string& filePath, std::map<int, ImVector<Shape_t>>* shapes)
{
    simulation_manager.clearObjects();
    (*shapes).clear();
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
                    element_number = int(buffer[i + 1] - '0'); // converts char to int
                else if (buffer[i] == '\t' && buffer[i + 1] == '\t')
                {
                    std::vector<float> values;
                    std::string b;
                    std::string name;
                    bool proc_string = false;
                    for (auto& el : buffer)
                    {
                        if (std::isdigit(el) || el == '.') // retrieves numbers
                            b += el;
                        else if (!std::isdigit(el) && !b.empty()) // saves the number
                        {
                            values.push_back(stof(b));
                            b.clear();
                        }
                        else if (std::isalpha(el)) // retrieves object's name
                            name += el;
                    }
                    Shape_t shape;
                    shape.name = name;
                    shape.p1 = ImVec2(values[0], values[1]);
                    shape.p2 = ImVec2(values[2], values[3]);
                    shape.color = ImVec4(values[4], values[5], values[6], values[7]);
                    shape.type = (b2BodyType)values[8];
                    shape.area = values[9];
                    shape.rotation = values[10];
                    (*shapes)[element_number].push_back(shape);
                }
            }
        }
    }
}

void createBoxObject(const ImVec2 origin, const Shape_t& shape)
{
    Box box;
    box.init(simulation_manager.m_world, shape.name ,glm::vec2((shape.p1.x + shape.p2.x) / 2 / RENDER_SCALE, (shape.p1.y + shape.p2.y) / 2 / RENDER_SCALE), glm::vec2(abs(shape.p1.x - shape.p2.x) / RENDER_SCALE, abs(shape.p1.y - shape.p2.y) / RENDER_SCALE), b2_dynamicBody, -shape.rotation);
    simulation_manager.m_objects[1].push_back(box);
}

void createWallObject(const ImVec2 origin, const Shape_t& shape)
{
    Wall wall;
    wall.init(simulation_manager.m_world, shape.name, glm::vec2((shape.p1.x + shape.p2.x) / 2 / RENDER_SCALE, (shape.p1.y + shape.p2.y) / 2 / RENDER_SCALE), glm::vec2(abs(shape.p1.x - shape.p2.x) / RENDER_SCALE, abs(shape.p1.y - shape.p2.y) / RENDER_SCALE), b2_staticBody, -shape.rotation);
    simulation_manager.m_objects[1].push_back(wall);
}

void createCircleObject(const ImVec2 origin, const Shape_t& shape)
{
    Circle circle;
    circle.init(simulation_manager.m_world, shape.name, glm::vec2(shape.p1.x / RENDER_SCALE, shape.p1.y / RENDER_SCALE), sqrt(pow(shape.p1.x - shape.p2.x, 2) + pow(shape.p1.y - shape.p2.y, 2)) / RENDER_SCALE, shape.type);
    simulation_manager.m_objects[2].push_back(circle);
}

bool checkPointsOverlapping(ImVec2 p1, ImVec2 p2)
{
    return !(p1.x - p2.x && p1.y - p2.y);
}

bool isPointInGivenArea(Shape_t area, ImVec2 point)
{
    // reference to https://math.stackexchange.com/questions/190111/how-to-check-if-a-point-is-inside-a-rectangle
    glm::vec2 pt(point.x, point.y);
    glm::vec2 AM(pt - glm::vec2(area.p1.x, area.p1.y));
    glm::vec2 AB(glm::vec2(area.p2.x, area.p1.y) - glm::vec2(area.p1.x, area.p1.y));
    glm::vec2 AD(glm::vec2(area.p1.x, area.p2.y) - glm::vec2(area.p1.x, area.p1.y));
    return 0 < glm::dot(AM, AB) && glm::dot(AM, AB) < glm::dot(AB, AB) && 0 < glm::dot(AM, AD) && glm::dot(AM, AD) < glm::dot(AD, AD);
}

void drawRotatedQuad(ImDrawList* draw_list, ImVec2 origin, Shape_t shape)
{
    ImVec2 center((shape.p1.x + shape.p2.x) / 2.0f, (shape.p1.y + shape.p2.y) / 2.0f);
    ImVec2 p1 = ImRotate(ImVec2(shape.p1.x - center.x, shape.p1.y - center.y), cos(glm::radians(-shape.rotation)), sin(glm::radians(-shape.rotation)));
    ImVec2 p2 = ImRotate(ImVec2(shape.p1.x - center.x, shape.p2.y - center.y), cos(glm::radians(-shape.rotation)), sin(glm::radians(-shape.rotation)));
    ImVec2 p3 = ImRotate(ImVec2(shape.p2.x - center.x, shape.p2.y - center.y), cos(glm::radians(-shape.rotation)), sin(glm::radians(-shape.rotation)));
    ImVec2 p4 = ImRotate(ImVec2(shape.p2.x - center.x, shape.p1.y - center.y), cos(glm::radians(-shape.rotation)), sin(glm::radians(-shape.rotation)));
    draw_list->AddQuad(ImVec2(origin.x + p1.x + center.x, origin.y + p1.y + center.y), ImVec2(origin.x + p2.x + center.x, origin.y + p2.y + center.y), ImVec2(origin.x + p3.x + center.x, origin.y + p3.y + center.y), ImVec2(origin.x + p4.x + center.x, origin.y + p4.y + center.y), ImGui::ColorConvertFloat4ToU32(shape.color));
}
