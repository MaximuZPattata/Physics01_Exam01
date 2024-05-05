#include "cControlGameEngine.h"
#include "cVAOManager.h"
#include "cShaderManager.h"
#include "GLWF_CallBacks.h"
#include "cMesh.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <sstream>

//-------------------------------------------------Private Functions-----------------------------------------------------------------------

cMesh* cControlGameEngine::g_pFindMeshByFriendlyName(std::string friendlyNameToFind)
{
    for (unsigned int index = 0; index != g_vec_pMeshesToDraw.size(); index++)
    {
        if (g_vec_pMeshesToDraw[index]->friendlyName == friendlyNameToFind)
        {
            return g_vec_pMeshesToDraw[index];
        }
    }

    std::cout << "Cannot find mesh model for the name provided : " << friendlyNameToFind << std::endl;

    return NULL;
}

sModelDrawInfo* cControlGameEngine::g_pFindModelInfoByFriendlyName(std::string friendlyNameToFind)
{
    for (unsigned int index = 0; index != newMeshAdd.size(); index++)
    {
        if (newMeshAdd[index]->friendlyName == friendlyNameToFind)
        {
            return newMeshAdd[index];
        }
    }

    std::cout << "Cannot find model info for the name provided : " << friendlyNameToFind << std::endl;

    return NULL;
}

sPhysicsProperties* cControlGameEngine::FindPhysicalModelByName(std::string modelName)
{
    for (unsigned int index = 0; index != gModelPhysicalProps.size(); index++)
    {
        if (gModelPhysicalProps[index]->modelName == modelName)
        {
            return gModelPhysicalProps[index];
        }
    }

    std::cout << "Cannot find physical mesh model for the name provided : " << modelName << std::endl;

    return NULL;
}

void cControlGameEngine::DrawObject(cMesh* pCurrentMesh, glm::mat4 matModelParent, GLuint shaderProgramID)
{
    //--------------------------Calculate Matrix Model Transformation--------------------------------

    glm::mat4 matModel = matModelParent;

    glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f),
        glm::vec3(pCurrentMesh->drawPosition.x, pCurrentMesh->drawPosition.y, pCurrentMesh->drawPosition.z));

    glm::mat4 matRotation = glm::mat4(pCurrentMesh->get_qOrientation());

    glm::mat4 matScale = glm::scale(glm::mat4(1.0f),
        glm::vec3(pCurrentMesh->drawScale.x, pCurrentMesh->drawScale.y, pCurrentMesh->drawScale.z));

    matModel = matModel * matTranslate;

    matModel = matModel * matRotation;

    matModel = matModel * matScale;

    //-------------------------Get Model Info--------------------------------------------------------

    GLint matModel_UL = glGetUniformLocation(shaderProgramID, "matModel");
    glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(matModel));

    glm::mat4 matModel_InverseTranspose = glm::inverse(glm::transpose(matModel));

    GLint matModel_IT_UL = glGetUniformLocation(shaderProgramID, "matModel_IT");
    glUniformMatrix4fv(matModel_IT_UL, 1, GL_FALSE, glm::value_ptr(matModel_InverseTranspose));

    // ---------------------Check Light and Wireframe-------------------------------------------------

    GLint bDoNotLight_UL = glGetUniformLocation(shaderProgramID, "bDoNotLight");

    if (pCurrentMesh->bIsWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (pCurrentMesh->bDoNotLight)
        glUniform1f(bDoNotLight_UL, (GLfloat)GL_TRUE);
    else
        glUniform1f(bDoNotLight_UL, (GLfloat)GL_FALSE);

    //-------------------------Get Debug Color from Shader----------------------------------------

    GLint bUseDebugColour_UL = glGetUniformLocation(shaderProgramID, "bUseDebugColour");

    if (pCurrentMesh->bUseDebugColours)
    {
        glUniform1f(bUseDebugColour_UL, (GLfloat)GL_TRUE);

        GLint debugColourRGBA_UL = glGetUniformLocation(shaderProgramID, "debugColourRGBA");
        glUniform4f(debugColourRGBA_UL,
            pCurrentMesh->wholeObjectDebugColourRGBA.r,
            pCurrentMesh->wholeObjectDebugColourRGBA.g,
            pCurrentMesh->wholeObjectDebugColourRGBA.b,
            pCurrentMesh->wholeObjectDebugColourRGBA.a);
    }
    else
        glUniform1f(bUseDebugColour_UL, (GLfloat)GL_FALSE);

    //-------------------------Find Model Info and Draw----------------------------------------

    sModelDrawInfo modelInfo;

    if (gMeshManager->FindDrawInfoByModelName(pCurrentMesh->meshName, modelInfo))
    {
        // Found it!!!

        glBindVertexArray(modelInfo.VAO_ID);
        glDrawElements(GL_TRIANGLES,
            modelInfo.numberOfIndices,
            GL_UNSIGNED_INT,
            0);
        glBindVertexArray(0);

    }

    //-------------------------Remove Scaling----------------------------------------

    glm::mat4 matRemoveScaling = glm::scale(glm::mat4(1.0f),
        glm::vec3(1.0f / pCurrentMesh->drawScale.x, 1.0f / pCurrentMesh->drawScale.y, 1.0f / pCurrentMesh->drawScale.z));

    matModel = matModel * matRemoveScaling;


    return;
}

int cControlGameEngine::InitializeShader()
{
    pShaderThing = new cShaderManager();

    pShaderThing->setBasePath("Assets/Shaders");

    vertexShader.fileName = "vertexShader01.glsl";

    fragmentShader.fileName = "fragmentShader01.glsl";

    if (!pShaderThing->createProgramFromFile("shader01", vertexShader, fragmentShader))
    {
        std::cout << "Error: Couldn't compile or link:" << std::endl;
        std::cout << pShaderThing->getLastError();
        return -1;
    }

    shaderProgramID = pShaderThing->getIDFromFriendlyName("shader01");

    return 0;
}

//---------------------------------------------------Public Functions-----------------------------------------------------------------------

float getRandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

//--------------------------------------Camera Controls----------------------------------------------------------------

void cControlGameEngine::MoveCameraPosition(float translate_x, float translate_y, float translate_z)
{
    g_cameraEye = glm::vec3(translate_x, translate_y, translate_z);
}

void cControlGameEngine::MoveCameraTarget(float translate_x, float translate_y, float translate_z)
{
    g_cameraTarget = glm::vec3(translate_x, translate_y, translate_z);
}

glm::vec3 cControlGameEngine::GetCurrentCameraPosition()
{
    return g_cameraEye;
}

glm::vec3 cControlGameEngine::GetCurrentCameraTarget()
{
    return g_cameraTarget;
}

//--------------------------------------Mesh Controls-----------------------------------------------------------------

void cControlGameEngine::ScaleModel(std::string modelName, float scale_value)
{
    cMesh* meshToBeScaled = g_pFindMeshByFriendlyName(modelName);

    meshToBeScaled->setUniformDrawScale(scale_value);
}

void cControlGameEngine::MoveModel(std::string modelName, float translate_x, float translate_y, float translate_z)
{
    cMesh* meshToBeTranslated = g_pFindMeshByFriendlyName(modelName);

    const glm::vec3& position = glm::vec3(translate_x, translate_y, translate_z);

    meshToBeTranslated->setDrawPosition(position);
}

glm::vec3 cControlGameEngine::GetModelPosition(std::string modelName)
{
    cMesh* meshPosition = g_pFindMeshByFriendlyName(modelName);

    return meshPosition->getDrawPosition();
}

void cControlGameEngine::RotateMeshModel(std::string modelName, float scalar, float rotate_x, float rotate_y, float rotate_z)
{
    cMesh* meshToBeRotated = g_pFindMeshByFriendlyName(modelName);

    glm::vec3 rotation = glm::vec3(rotate_x, rotate_y, rotate_z);

    //meshToBeRotated->setRotationFromEuler(rotation);
}

void cControlGameEngine::TurnVisibilityOn(std::string modelName)
{
    cMesh* meshVisibility = g_pFindMeshByFriendlyName(modelName);

    if (meshVisibility->bIsVisible != true)
        meshVisibility->bIsVisible = true;
    else
        meshVisibility->bIsVisible = false;
}

void cControlGameEngine::TurnWireframeModeOn(std::string modelName)
{
    cMesh* meshWireframe = g_pFindMeshByFriendlyName(modelName);

    if (meshWireframe->bIsWireframe == true)
        meshWireframe->bIsWireframe = false;
    else
        meshWireframe->bIsWireframe = true;
}

void cControlGameEngine::TurnMeshLightsOn(std::string modelName)
{
    cMesh* meshLights = g_pFindMeshByFriendlyName(modelName);

    if (meshLights->bDoNotLight == true)
        meshLights->bDoNotLight = false;
    else
        meshLights->bDoNotLight = true;
}

void cControlGameEngine::DeleteMesh(std::string modelName)
{
    cMesh* meshModel = g_pFindMeshByFriendlyName(modelName);

    sPhysicsProperties* physicalModel = FindPhysicalModelByName(modelName);

    sModelDrawInfo* modelInfo = g_pFindModelInfoByFriendlyName(modelName);

    if (meshModel != NULL)
        g_vec_pMeshesToDraw.erase(std::remove(g_vec_pMeshesToDraw.begin(), g_vec_pMeshesToDraw.end(), meshModel), g_vec_pMeshesToDraw.end());

    if (physicalModel != NULL)
        gModelPhysicalProps.erase(std::remove(gModelPhysicalProps.begin(), gModelPhysicalProps.end(), physicalModel), gModelPhysicalProps.end());

    if (modelInfo != NULL)
        newMeshAdd.erase(std::remove(newMeshAdd.begin(), newMeshAdd.end(), modelInfo), newMeshAdd.end());
}

//--------------------------------------Lights Controls-----------------------------------------------------------------

void cControlGameEngine::CreateLight(int lightId, float initial_x, float initial_y, float initial_z)
{
    if (lightId > 15)
    {
        std::cout << "Light Id is more than 15 ! Cannot create light !" << std::endl;
        return;
    }
    std::cout << "Light : " << lightId << " Created !" << std::endl;

    gTheLights->SetUniformLocations(shaderProgramID, lightId);

    gTheLights->theLights[lightId].param2.x = 1.0f; // Turn on

    gTheLights->theLights[lightId].param1.x = 2.0f;   // 0 = point light , 1 = spot light , 2 = directional light

    gTheLights->theLights[lightId].param1.y = 50.0f; // inner angle

    gTheLights->theLights[lightId].param1.z = 50.0f; // outer angle

    gTheLights->theLights[lightId].position.x = initial_x;

    gTheLights->theLights[lightId].position.y = initial_y;

    gTheLights->theLights[lightId].position.z = initial_z;

    gTheLights->theLights[lightId].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);

    gTheLights->theLights[lightId].atten.x = 0.0f;        // Constant attenuation

    gTheLights->theLights[lightId].atten.y = 0.1f;        // Linear attenuation

    gTheLights->theLights[lightId].atten.z = 0.0f;        // Quadratic attenuation
}

void cControlGameEngine::TurnOffLight(int lightId, bool turnOff)
{
    if (turnOff)
        gTheLights->theLights[lightId].param2.x = 0.0f;
    else
        gTheLights->theLights[lightId].param2.x = 1.0f;
}

void cControlGameEngine::PositionLight(int lightId, float translate_x, float translate_y, float translate_z)
{
    gTheLights->theLights[lightId].position.x = translate_x;

    gTheLights->theLights[lightId].position.y = translate_y;

    gTheLights->theLights[lightId].position.z = translate_z;
}

void cControlGameEngine::ChangeLightIntensity(int lightId, float linearAttentuation, float quadraticAttentuation)
{
    gTheLights->theLights[lightId].atten.y = linearAttentuation;

    gTheLights->theLights[lightId].atten.z = quadraticAttentuation;
}

void cControlGameEngine::ChangeLightType(int lightId, float lightType)
{
    gTheLights->theLights[lightId].param1.x = lightType;
}

void cControlGameEngine::ChangeLightAngle(int lightId, float innerAngle, float outerAngle)
{
    gTheLights->theLights[lightId].param1.y = innerAngle; // inner angle

    gTheLights->theLights[lightId].param1.z = outerAngle; // outer angle
}

void cControlGameEngine::ChangeLightDirection(int lightId, float direction_x, float direction_y, float direction_z)
{
    gTheLights->theLights[lightId].direction = glm::vec4(direction_x, direction_y, direction_z, 1.0f);

}

void cControlGameEngine::ChangeLightColour(int lightId, float color_r, float color_g, float color_b)
{
    gTheLights->theLights[lightId].diffuse = glm::vec4(color_r, color_g, color_b, 1.0f);
}

//--------------------------------------Physics Controls---------------------------------------------------------------

void cControlGameEngine::DoPhysics(sPhysicsProperties* physicsModel, std::string Model2, double deltaTime) //Question 3
{
    //---------------------Calculate acceleration & velocity-------------------------------

    glm::vec3 velocityChange = physicsModel->acceleration * (float)deltaTime;

    physicsModel->velocity += velocityChange;

    glm::vec3 positionChange = physicsModel->velocity * (float)deltaTime;

    physicsModel->position.x += positionChange.x;
    physicsModel->position.y += positionChange.y;
    physicsModel->position.z += positionChange.z;

    //---------------------Set sphere's position based on new velocity--------------------

    cMesh* sphereMesh = g_pFindMeshByFriendlyName(physicsModel->modelName);

    sphereMesh->setDrawPosition(physicsModel->position);

    //---------------------Get ship's position--------------------------------------------

    cMesh* shipMesh = g_pFindMeshByFriendlyName(Model2);

    shipMesh->getDrawPosition();

    sModelDrawInfo* modelInfo = g_pFindModelInfoByFriendlyName(Model2);

    //----------------------Check for Collision--------------------------------------------

    bool result = false;

    if (physicsModel->position.x < shipMesh->drawPosition.x + 70.0f && physicsModel->position.x > shipMesh->drawPosition.x - 70.0f &&
        physicsModel->position.y < shipMesh->drawPosition.y + 70.0f && physicsModel->position.y > shipMesh->drawPosition.y - 70.0f &&
        physicsModel->position.z < shipMesh->drawPosition.z + 200.0f && physicsModel->position.z > shipMesh->drawPosition.z - 200.0f)
    {
        result = gPhysics->CheckForCollision(gMeshManager, modelInfo->meshFileName, modelInfo, physicsModel->position, physicsModel->radius, shipMesh, physicsModel);
    }

    if (result == true)
    {
        CollisionResponse(physicsModel);
    }
}

void cControlGameEngine::ChangeModelPhysicsPosition(std::string modelName, float newPositionX, float newPositionY, float newPositionZ)
{
    sPhysicsProperties* physicalModelFound = FindPhysicalModelByName(modelName);

    physicalModelFound->position.x = newPositionX;
    physicalModelFound->position.y = newPositionY;
    physicalModelFound->position.z = newPositionZ;
}

void cControlGameEngine::AddPhysicsToMesh(std::string modelName, float obejctRadius)
{
    sPhysicsProperties* newPhysicsModel = new sPhysicsProperties();

    cMesh* meshDetails = g_pFindMeshByFriendlyName(modelName);

    glm::vec3 modelPosition = meshDetails->getDrawPosition();

    newPhysicsModel->modelName = modelName;

    newPhysicsModel->radius = obejctRadius;

    newPhysicsModel->position = modelPosition;

    gModelPhysicalProps.push_back(newPhysicsModel);
}

void cControlGameEngine::ChangeModelPhysicsVelocity(std::string modelName, glm::vec3 velocityChange)
{
    sPhysicsProperties* physicalModelFound = FindPhysicalModelByName(modelName);

    physicalModelFound->velocity = velocityChange;
}

void cControlGameEngine::ChangeModelPhysicsAcceleration(std::string modelName, glm::vec3 accelerationChange)
{
    sPhysicsProperties* physicalModelFound = FindPhysicalModelByName(modelName);

    physicalModelFound->acceleration = accelerationChange;
}

//-----------------------------------Asteroid Controls-----------------------------------------------------------------

void cControlGameEngine::CreateAsteroidField() //Question 2
{
    std::string addingNewModel;

    float random_x_value = 0.0;
    float random_y_value = 0.0;
    float random_z_value = 0.0;

    for (int asteroidCount = 0; asteroidCount < 25; asteroidCount++)
    {
        addingNewModel = "Asteroid_" + std::to_string(asteroidCount);

        random_x_value = getRandomFloat(-150.0f, 150.0f);
        random_y_value = getRandomFloat(-100.0f, 100.0f);
        random_z_value = getRandomFloat(200.0f, 700.0f);

        LoadModelsInto3DSpace("Asteroid_011_x10_flatshaded_xyz_n_rgba.ply", addingNewModel, random_x_value, random_y_value, random_z_value);

        TurnMeshLightsOn(addingNewModel);

        ScaleModel(addingNewModel, 0.01f);

        AddPhysicsToMesh(addingNewModel, 5.9f);

        ChangeModelPhysicsVelocity(addingNewModel, glm::vec3(0.0f, 0.0f, -5.0f));

        ChangeModelPhysicsAcceleration(addingNewModel, glm::vec3(0.0f, 0.0f, -9.81f / 2.0f));
    }
}

void cControlGameEngine::CheckAsteroidBoundary()
{
    if (gModelPhysicalProps.size() > 0)
    {
        cMesh* shipMesh = g_pFindMeshByFriendlyName("Ship");

        float shipPosX = shipMesh->drawPosition.x;
        float shipPosY = shipMesh->drawPosition.y;
        float shipPosZ = shipMesh->drawPosition.z;

        for (int asteroidCount = 0; asteroidCount < gModelPhysicalProps.size(); asteroidCount++)
        {
            if (gModelPhysicalProps[asteroidCount]->position.x > shipPosX + 500.0f || gModelPhysicalProps[asteroidCount]->position.x < shipPosX - 500.0f ||
                gModelPhysicalProps[asteroidCount]->position.y > shipPosY + 500.0f || gModelPhysicalProps[asteroidCount]->position.y < shipPosY - 500.0f ||
                gModelPhysicalProps[asteroidCount]->position.z < shipPosZ - 500.0f)
            {
                DeleteMesh(gModelPhysicalProps[asteroidCount]->modelName);
            }
        }
    }
    else
    {
        CreateAsteroidField();
    }
}

void cControlGameEngine::CheckForRedSphereOccurence() //Question 3
{
    if (gRedSphereModelProps.size() > 0)
    {
        for (int redSphereCount = 0; redSphereCount < gRedSphereModelProps.size(); redSphereCount++)
        {
            if (!gRedSphereModelProps[redSphereCount]->expansionLimitReached)
            {
                cMesh* redSphereMesh = g_pFindMeshByFriendlyName(gRedSphereModelProps[redSphereCount]->modelName);
                gRedSphereModelProps[redSphereCount]->scaleValue += 0.25f;
                redSphereMesh->setUniformDrawScale(gRedSphereModelProps[redSphereCount]->scaleValue);

                if (gRedSphereModelProps[redSphereCount]->scaleValue >= 10.5f)
                    gRedSphereModelProps[redSphereCount]->expansionLimitReached = true;
            }
            else
            {
                cMesh* redSphereMesh = g_pFindMeshByFriendlyName(gRedSphereModelProps[redSphereCount]->modelName);

                std::string modelName = "GreySphere_" + gRedSphereModelProps[redSphereCount]->modelName;
                LoadModelsInto3DSpace("Sphere_grey_xyz_n_rgba.ply", modelName, redSphereMesh->drawPosition.x, redSphereMesh->drawPosition.y, redSphereMesh->drawPosition.z);
                ScaleModel(modelName, 3.0f);

                DeleteMesh(gRedSphereModelProps[redSphereCount]->modelName);
                gRedSphereModelProps.erase(std::remove(gRedSphereModelProps.begin(), gRedSphereModelProps.end(), gRedSphereModelProps[redSphereCount]), gRedSphereModelProps.end());
            }
        }
    }
}

void cControlGameEngine::MoveTowardsSpaceship(double deltaTime) //Question 6
{
    if (gModelPhysicalProps.size() > 0)
    {
        cMesh* shipMesh = g_pFindMeshByFriendlyName("Ship");

        glm::vec3 shipPos = shipMesh->drawPosition;

        for (int asteroidCount = 0; asteroidCount < gModelPhysicalProps.size(); asteroidCount++)
        {

            if (gModelPhysicalProps[asteroidCount]->position.z > (shipPos.z - 300.0f))
            {
                glm::vec3 asteroidDirection = normalize(shipPos - gModelPhysicalProps[asteroidCount]->position);
                glm::vec3 asteroidAcceleration = asteroidDirection * ((float)deltaTime + 0.025f);
                gModelPhysicalProps[asteroidCount]->velocity += asteroidAcceleration;
                gModelPhysicalProps[asteroidCount]->position += gModelPhysicalProps[asteroidCount]->velocity * ((float)deltaTime + 0.025f);
            }
        }
    }

}

void cControlGameEngine::PushAsteroidsAway() //Question 5 
{
    if (gModelPhysicalProps.size() > 0)
    {
        cMesh* shipMesh = g_pFindMeshByFriendlyName("Ship");
        float shipPosX = shipMesh->drawPosition.x;
        float shipPosY = shipMesh->drawPosition.y;
        float shipPosZ = shipMesh->drawPosition.z;

        for (int asteroidCount = 0; asteroidCount < gModelPhysicalProps.size(); asteroidCount++)
        {
            if (gModelPhysicalProps[asteroidCount]->position.x < shipPosX + 50.0f && gModelPhysicalProps[asteroidCount]->position.x > shipPosX - 50.0f &&
                gModelPhysicalProps[asteroidCount]->position.y < shipPosY + 50.0f && gModelPhysicalProps[asteroidCount]->position.y > shipPosY - 50.0f &&
                gModelPhysicalProps[asteroidCount]->position.z < shipPosZ + 150.0f && gModelPhysicalProps[asteroidCount]->position.z > shipPosZ - 150.0f)
            {
                glm::vec3 vertex1;
                glm::vec3 vertex2;
                glm::vec3 vertex3;

                if (gModelPhysicalProps[asteroidCount]->position.x < 0.0f)
                {
                    vertex1 = glm::vec3(-10.0f, -10.0f, 0.0f);
                    vertex2 = glm::vec3(0.0f, -10.0f, -10.0f);
                    vertex3 = glm::vec3(-10.0f, 0.0f, -10.0f);
                }
                else
                {
                    vertex1 = glm::vec3(10.0f, 10.0f, 0.0f);
                    vertex2 = glm::vec3(0.0f, 10.0f, 10.0f);
                    vertex3 = glm::vec3(10.0f, 0.0f, 10.0f);
                }

                glm::vec3 edgeA = vertex2 - vertex1;
                glm::vec3 edgeB = vertex3 - vertex1;

                glm::vec3 triNormal = glm::normalize(glm::cross(edgeA, edgeB));

                glm::vec3 sphereDirection = gModelPhysicalProps[asteroidCount]->velocity;

                sphereDirection = glm::normalize(sphereDirection);
                
                glm::vec3 reflectionVector = glm::reflect(sphereDirection, triNormal);
                
                float sphereSpeed = glm::length(gModelPhysicalProps[asteroidCount]->velocity);
                
                glm::vec3 newVelocity = reflectionVector * sphereSpeed * 10.0f;
                
                gModelPhysicalProps[asteroidCount]->velocity = newVelocity;

            }
        }
        std::cout << "SHIELD ACTIVATED !" << std::endl;
    }

    else
    {
        std::cout << "UNECESSARY SHIELD ACTIVATION !! NO ASTEROIDS AHEAD !! SAVE ENERGY !!" << std::endl;
    }
}

void cControlGameEngine::CollisionResponse(sPhysicsProperties* physicsModel) //Question 3
{
    std::string sphereModelName = "RedSphere_" + physicsModel->modelName;

    //cMesh* asteroidMesh = g_pFindMeshByFriendlyName(physicsModel->modelName);

    //asteroidMesh->bIsVisible = false;

    sRedSphereProperties* newRedSphere = new sRedSphereProperties();

    newRedSphere->modelName = sphereModelName;

    newRedSphere->scaleValue = 1.0f;

    gRedSphereModelProps.push_back(newRedSphere);

    LoadModelsInto3DSpace("Sphere_red_xyz_n_rgba.ply", sphereModelName, physicsModel->collisionPoint.x, physicsModel->collisionPoint.y, physicsModel->collisionPoint.z);

    DeleteMesh(physicsModel->modelName);
}

//--------------------------------------Engine Controls-----------------------------------------------------------------

void cControlGameEngine::LoadModelsInto3DSpace(std::string filePath, std::string modelName, float initial_x, float initial_y, float initial_z)
{
    sModelDrawInfo* newModel = new sModelDrawInfo;

    cMesh* newMesh = new cMesh();

    bool result = gMeshManager->LoadModelIntoVAO(modelName, filePath, *newModel, shaderProgramID);

    if (!result)
    {
        std::cout << "Cannot load model - " << modelName << std::endl;
        return;
    }

    newMeshAdd.push_back(newModel);

    newMesh->meshName = filePath;

    newMesh->friendlyName = modelName;

    newMesh->drawPosition = glm::vec3(initial_x, initial_y, initial_z);

    std::cout << "Loaded: " << newMesh->friendlyName << " | Vertices : " << newModel->numberOfVertices << std::endl;

    g_vec_pMeshesToDraw.push_back(newMesh);
}

int cControlGameEngine::InitializeGameEngine()
{
    //-------------------------------------Shader Initialize----------------------------------------------------------------

    int result = InitializeShader();

    if (result != 0)
        return -1;

    //-------------------------------------VAO Initialize---------------------------------------------------------------------

    gMeshManager = new cVAOManager();

    gMeshManager->setBasePath("Assets/Models");

    gPhysics = new cPhysics();

    gPhysics->setVAOManager(gMeshManager);

    //------------------------------------Lights Initialize-----------------------------------------------------------------------

    gTheLights = new cLightManager();

    // CreateLight(0);

    return 0;
}


void  cControlGameEngine::CheckForPhysicalModel(double deltaTime, std::string modelName)
{

    for (int physicalModelCount = 0; physicalModelCount < gModelPhysicalProps.size(); physicalModelCount++)
    {
        sPhysicsProperties* spherePhysicalModel = FindPhysicalModelByName(gModelPhysicalProps[physicalModelCount]->modelName);

        if (spherePhysicalModel != NULL)
        {
            DoPhysics(spherePhysicalModel, modelName, deltaTime);
        }
    }

}

int cControlGameEngine::RunGameEngine(GLFWwindow* window)
{
        float ratio;
        int width, height;

        glUseProgram(shaderProgramID);

        glfwGetFramebufferSize(window, &width, &height);

        ratio = width / (float)height;

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        glCullFace(GL_BACK);

        //---------------------------Light Values Update----------------------------------------

        gTheLights->UpdateUniformValues(shaderProgramID);

        //---------------------------Camera Values----------------------------------------------

        GLint eyeLocation_UL = glGetUniformLocation(shaderProgramID, "eyeLocation");
        glUniform4f(eyeLocation_UL,
            g_cameraEye.x, g_cameraEye.y, g_cameraEye.z, 1.0f);

        glm::mat4 matProjection = glm::perspective(0.6f, ratio, 0.1f, 1000.0f);

        glm::mat4 matView = glm::lookAt(g_cameraEye, g_cameraTarget, g_upVector);

        GLint matProjection_UL = glGetUniformLocation(shaderProgramID, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, glm::value_ptr(matProjection));

        GLint matView_UL = glGetUniformLocation(shaderProgramID, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(matView));

        //----------------------------Draw all the objects--------------------------------------

        for (unsigned int index = 0; index != g_vec_pMeshesToDraw.size(); index++)
        {
            cMesh* pCurrentMesh = g_vec_pMeshesToDraw[index];

            if (pCurrentMesh->bIsVisible)
            {
                glm::mat4 matModel = glm::mat4(1.0f);

                DrawObject(pCurrentMesh, matModel, shaderProgramID);
            }
        }

        //----------------------------Title Screen Values---------------------------------------------

        std::stringstream ssTitle;

        ssTitle << "Camera Eye(x,y,z): "
            << g_cameraEye.x << ", "
            << g_cameraEye.y << ", "
            << g_cameraEye.z << ") | "
            << "Camera Target(x,y,z): "
            << g_cameraTarget.x << ", "
            << g_cameraTarget.y << ", "
            << g_cameraTarget.z << ")";

        std::string theTitle = ssTitle.str();

        glfwSwapBuffers(window);

        glfwPollEvents();

        glfwSetWindowTitle(window, theTitle.c_str());
}
