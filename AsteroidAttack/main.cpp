#include <cControlGameEngine.h>
#include "GLWF_CallBacks.h"
#include "cMesh.h"

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

//---------------------------Global Objects-----------------------------------------------

GLFWwindow* window;
cControlGameEngine gameEngine;

int main()
{
    //-----------------------------------Initialize Window--------------------------------------

    int result = 0;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    //--------------------------------Initialize Game Engine----------------------------------------

    result = gameEngine.InitializeGameEngine();

    if (result != 0)
        return -1;

    //----------------------------Set Camera and default Lighting------------------------------------

    gameEngine.MoveCameraPosition(318.0f, 44.0f, 15.0f);
    
    int lightId = 0;

    gameEngine.CreateLight(lightId, 0.0f, 30.0f, 0.0f);
    gameEngine.ChangeLightType(lightId, 2.0f);
    gameEngine.ChangeLightIntensity(lightId, 0.1f, 0.0f);
    gameEngine.ChangeLightAngle(lightId, 0.50f, 0.100f);

    //--------------------------------Load Models-----------------------------------------------------

    //----------------------------Asteroid Model----------------------------------------------

    gameEngine.CreateAsteroidField();

    //----------------------------Ship Model(Question 1)--------------------------------------------------

    std::string addingNewModel = "Ship";

    gameEngine.LoadModelsInto3DSpace("SM_Ship_Massive_Transport_01_xyz_n_rgba_uv_flatshaded_xyz_n_rgba.ply", addingNewModel, 0.0f, 0.0f, 0.0f);

    gameEngine.ScaleModel(addingNewModel, 0.005f);

    //-------------------------------Frame loop---------------------------------------------

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        //--------------------Run Engine----------------------------------------------------

        gameEngine.RunGameEngine(window);

        //----------------------------Physics stuff-----------------------------------------

        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;

        const double LARGEST_DELTA_TIME = 1.0f / 30.0f;

        if (deltaTime > LARGEST_DELTA_TIME)
            deltaTime = LARGEST_DELTA_TIME;

        lastTime = currentTime;

        gameEngine.CheckForPhysicalModel(deltaTime, "Ship");


        //----------------------------Check for red sphere occurence(Part of Question 3)----------------------------

        gameEngine.CheckForRedSphereOccurence();

        //----------------------------Check Asteroid boundary-----------------------------------

        gameEngine.CheckAsteroidBoundary();

        //-------------------Make Asteroids move towards Spaceship(Question 6)-------------------

        gameEngine.MoveTowardsSpaceship(deltaTime);
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    exit(EXIT_SUCCESS);

    return 0;
}