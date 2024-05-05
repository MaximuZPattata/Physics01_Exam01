#include "sPhysicsProperties.h"



sPhysicsProperties::sPhysicsProperties()
{
    this->collisionPoint = glm::vec3(0.0f);
    this->velocity = glm::vec3(0.0f);
    this->acceleration = glm::vec3(0.0f);
    this->inverse_mass = 1.0f / 10.0f;
}



