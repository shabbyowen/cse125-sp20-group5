////////////////////////////////////////
// Particle.glsl
////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#ifdef VERTEX_SHADER
//#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 viewProjMat;
uniform mat4 model;

out float sampleExtraOutput;

////////////////////////////////////////
// Vertex shader
////////////////////////////////////////

void main()
{
    gl_Position = viewProjMat * model * vec4(position, 1.0);
    sampleExtraOutput = 1.0f;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef FRAGMENT_SHADER
//#version 330 core

in float sampleExtraOutput;

uniform vec3 color;

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////
// Fragment shader
////////////////////////////////////////

void main()
{
    fragColor = vec4(color, sampleExtraOutput);
}

#endif

////////////////////////////////////////////////////////////////////////////////