#ifndef _Particle_H_
#define _Particle_H_

#include "Core.h"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

/**
    This class define each individual particle
*/

class Particle
{
private:
    GLuint shader;

    glm::mat4 individualModelMatrix;
    glm::vec3 color;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float lifeLeft;

    float distanceLeft;

public:
    Particle(GLuint shader, glm::mat4 modelMatrix, glm::vec3 color, 
       glm::vec3 initialVelocity, glm::vec3 acceleration, int lifeSpan, float maxDistance);
    ~Particle();

    void draw(glm::mat4 model, const glm::mat4& viewProjMat, GLuint parentVAO);
    // Update all the children and check if the die
    void update(float timeDifference);
    bool isAlive();
};

#endif

