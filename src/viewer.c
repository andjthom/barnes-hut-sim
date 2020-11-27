#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <glad/glad.h>
#include <SDL.h>
#include "./csv.h"
#include "./linear.h"

static int Render_Init(int N);
static void Render_Quit();
static void Render(Vec3 *pos, int N);
static int Render_ProcessInput();
static unsigned int Render_CreateShaderProgram(const char *vertex_path, const char *fragment_path);
static char *ReadEntireFile(const char *path);

static int g_screen_width = 800;
static int g_screen_height = 600;

static SDL_Window *g_window;
static unsigned int g_program;
static unsigned int g_pos_VBO, g_VBO, g_VAO;

static const Uint8 *g_keys;
static int g_first_mouse = 1;
static int g_last_x;
static int g_last_y;
static float g_sensitivity = 0.2f;
static float g_pitch = 0.f;
static float g_yaw = -90.f;

static const float g_camera_speed = 500.0f;
static vec3f g_camera_pos = {0.f, 0.f, 0.f};
static vec3f g_camera_front = {0.f, 0.f, -1.f};
static vec3f g_camera_up = {0.f, 1.f, 0.f};

Vec3 *g_pos;
static int g_step = 0;
static int g_steps_per_epoch = 0;
const char g_pos_dir[1024];
static int N = 0;
static int g_num_steps = 0;

int main(int argc, char *argv[])
{
    int area_size;
    float timestep, init_rot, mean_rand_vel;

    if (argc < 2)
    {
        printf("Usage: %s <directory>\n", argv[0]);
        return 0;
    }

    if (!CSV_ReadPropFile(argv[1], &N, &g_num_steps, &timestep, &g_steps_per_epoch, &area_size, &init_rot,
                &mean_rand_vel))
    {
        printf("Error reading prop file in: %s\n", argv[1]);
        return -1;
    }

    sprintf(g_pos_dir, "%s/pos/", argv[1]);
    g_camera_pos[2] = (float)area_size * 1.5;
    g_pos = malloc(N * sizeof(Vec3));

    if (!CSV_ReadStepFile(g_pos_dir, g_step, N, g_pos))
    {
        printf("Error reading step: %d\n", g_step);
    }

    if (Render_Init(N) != 0)
    {
        printf("SDL error\n");
        return 0;
    }

    do {
        Render(g_pos, N);
    } while (Render_ProcessInput() == 0);

    Render_Quit();
    free(g_pos);
    return 0;
}

static int Render_Init(int N)
{
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        return -1;
    }

    g_window = SDL_CreateWindow("BH Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                g_screen_width, g_screen_height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if (!g_window)
    {
        SDL_Quit();
        return -1;
    }

    g_keys = SDL_GetKeyboardState(NULL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_CreateContext(g_window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return -1;
    }

    g_program = Render_CreateShaderProgram("../data/shader.vert", "../data/shader.frag");

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);

    glGenBuffers(1, &g_pos_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_pos_VBO);
    glBufferData(GL_ARRAY_BUFFER, N * sizeof(Vec3), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &g_VAO);
    glGenBuffers(1, &g_VBO);

    glBindVertexArray(g_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    /* Vertex positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    /* Particle positions */
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, g_pos_VBO);
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(Vec3), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
    return 0;
}

static void Render_Quit()
{
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

static void Render(Vec3 *pos, int num_particles)
{
    void *buffer_data;
    mat4 view;
    mat4 proj;
    vec3f camera_target;

    glBindBuffer(GL_ARRAY_BUFFER, g_pos_VBO);
    buffer_data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(buffer_data, pos, num_particles * sizeof(Vec3));
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_program);
    glBindVertexArray(g_VAO);

    vec3f_add(camera_target, g_camera_pos, g_camera_front);
    mat4_look_at(view, g_camera_pos, camera_target, g_camera_up);
    glUniformMatrix4fv(glGetUniformLocation(g_program, "view"), 1, GL_FALSE, view);

#if 1
    mat4_perspective(proj, (float)M_PI / 2.f, (float)g_screen_width / (float)g_screen_height, 0.01f, 1000000.0f);
    glUniformMatrix4fv(glGetUniformLocation(g_program, "projection"), 1, GL_FALSE, proj);
#else
    mat4_ortho(proj, -45000, 45000, -45000, 45000, 0.1, 100000);
    glUniformMatrix4fv(glGetUniformLocation(g_program, "projection"), 1, GL_FALSE, proj);
#endif

    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, num_particles);
    glBindVertexArray(0);

    SDL_GL_SwapWindow(g_window);
}

static int Render_ProcessInput()
{
    int quit;
    SDL_Event event;

    quit = 0;
    while (SDL_PollEvent(&event)) {
        switch (event.type)
        {
            case SDL_MOUSEMOTION:
                if ((event.motion.state & SDL_BUTTON_LMASK) == 1)
                {
                    if (g_first_mouse == 1)
                    {
                        g_last_x = event.motion.x;
                        g_last_y = event.motion.y;
                        g_first_mouse = 0;
                    }

                    g_yaw += (event.motion.x - g_last_x) * g_sensitivity;
                    g_pitch -= (event.motion.y - g_last_y) * g_sensitivity;

                    if (g_pitch > 90.f)
                    {
                        g_pitch = 90.f;
                    }
                    else if (g_pitch < -90.f)
                    {
                        g_pitch = -90.f;
                    }

                    g_last_x = event.motion.x;
                    g_last_y = event.motion.y;

                    g_camera_front[0] = cosf(deg_to_rad(g_yaw)) * cosf(deg_to_rad(g_pitch));
                    g_camera_front[1] = sinf(deg_to_rad(g_pitch));
                    g_camera_front[2] = sinf(deg_to_rad(g_yaw)) * cosf(deg_to_rad(g_pitch));
                    vec3f_normalize(g_camera_front, g_camera_front);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    g_first_mouse = 1;
                }
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    g_screen_width = event.window.data1;
                    g_screen_height = event.window.data2;
                    glViewport(0, 0, g_screen_width, g_screen_height);
                }
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_LEFT && event.key.state == SDL_PRESSED
                        && event.key.repeat == 0)
                {
                    g_step -= g_steps_per_epoch;

                    if (g_step < 0)
                    {
                        g_step = 0;
                    }
                    else
                    {
                        CSV_ReadStepFile(g_pos_dir, g_step, N, g_pos);
                        printf("Step: %d\n", g_step);
                    }
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT && event.key.state == SDL_PRESSED
                        && event.key.repeat == 0)
                {
                    g_step += g_steps_per_epoch;

                    if (g_step > g_num_steps)
                    {
                        g_step = g_num_steps;
                    }
                    else
                    {
                        CSV_ReadStepFile(g_pos_dir, g_step, N, g_pos);
                        printf("Step: %d\n", g_step);
                    }
                }
                break;

            case SDL_QUIT:
                quit = 1;
                break;

            default:
                break;
        }
    }

    vec3f camera_move;
    if (g_keys[SDL_SCANCODE_W])
    {
        vec3f_scale(camera_move, g_camera_front, g_camera_speed);
        vec3f_add(g_camera_pos, g_camera_pos, camera_move);
    }
    if (g_keys[SDL_SCANCODE_S])
    {
        vec3f_scale(camera_move, g_camera_front, g_camera_speed);
        vec3f_subtract(g_camera_pos, g_camera_pos, camera_move);
    }
    if (g_keys[SDL_SCANCODE_A])
    {
        vec3f_cross(camera_move, g_camera_front, g_camera_up);
        vec3f_normalize(camera_move, camera_move);
        vec3f_scale(camera_move, camera_move, g_camera_speed);
        vec3f_subtract(g_camera_pos, g_camera_pos, camera_move);
    }
    if (g_keys[SDL_SCANCODE_D])
    {
        vec3f_cross(camera_move, g_camera_front, g_camera_up);
        vec3f_normalize(camera_move, camera_move);
        vec3f_scale(camera_move, camera_move, g_camera_speed);
        vec3f_add(g_camera_pos, g_camera_pos, camera_move);
    }
    if (g_keys[SDL_SCANCODE_SPACE])
    {
        vec3f_scale(camera_move, g_camera_up, g_camera_speed);
        vec3f_add(g_camera_pos, g_camera_pos, camera_move);
    }
    if (g_keys[SDL_SCANCODE_LCTRL])
    {
        vec3f_scale(camera_move, g_camera_up, g_camera_speed);
        vec3f_subtract(g_camera_pos, g_camera_pos, camera_move);
    }
    if (g_keys[SDL_SCANCODE_ESCAPE])
    {
        quit = 1;
    }

    return quit;
}

static unsigned int Render_CreateShaderProgram(const char *vertex_path, const char *fragment_path)
{
    const char *vertex_source;
    const char *fragment_source;
    unsigned int vertex, fragment, program_id;
    int success;
    char info_log[512];

    vertex_source = ReadEntireFile(vertex_path);
    fragment_source = ReadEntireFile(fragment_path);

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_source, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, info_log);
        fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", info_log);
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_source, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, info_log);
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", info_log);
    }

    program_id = glCreateProgram();
    glAttachShader(program_id, vertex);
    glAttachShader(program_id, fragment);
    glLinkProgram(program_id);
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program_id, 512, NULL, info_log);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", info_log);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    free((char *)vertex_source);
    free((char *)fragment_source);

    return program_id;
}

static char *ReadEntireFile(const char *path)
{
    FILE *file;
    size_t size, result;
    char *buffer;

    file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Error reading file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    buffer = (char *)malloc(size + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error allocating memory!\n");
    }

    result = fread(buffer, 1, size, file);
    if (result != size)
    {
        fprintf(stderr, "Reading error while reading: %s\n", path);
    }

    fclose(file);
    buffer[size] = 0;
    return buffer;
}
