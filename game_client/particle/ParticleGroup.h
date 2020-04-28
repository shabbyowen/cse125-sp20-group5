#ifndef _Particle_Group_H_
#define _Particle_Group_H_

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "Particle.h"

/**
    This class would be responsable for creating, managing, rendering a group of particles.
*/
class ParticleGroup
{
private:
    std::vector<Particle*> children;

	GLuint vao; // Vao for paritcles
	GLuint vbos[2]; // Vbo for paricles
    GLuint shader; // The shader that renders the particles

    glm::mat4 groupModelMatrix; // Model matrix of the whole particle group

    glm::vec3 baseColor; // Color of the children particles
    glm::vec3 initialVelocity; // The current velocity of each particle

    glm::vec3 acceleration; // How the velocity of particles changes every update

    int maxParticleNum; // Maximum number of particles rendered
    float particleLifeSpan; // The lifespan of each particle

    // Those variables are used to randomize each particle
    glm::vec3 colorVariance;
    glm::vec3 initialVelocityVariance;

    // Helper functions
    glm::vec3 randomizeVec3(glm::vec3 base, glm::vec3 variance);
public:
	ParticleGroup(GLuint shader, glm::mat4 modelMatrix, glm::vec3 color, glm::vec3 initialVelocity,
        glm::vec3 acceleration,  int initParicleNum, int maxParticleNum, float lifeSpan,
        glm::vec3 colorVariance, glm::vec3 initialVelocityVariance);
	~ParticleGroup();
    
    // Draw the whole particle group
	void draw(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	void update(float timeDifference);
};

#endif

