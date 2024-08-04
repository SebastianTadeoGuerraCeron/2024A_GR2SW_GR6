#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuseTexture;   // Textura difusa
uniform sampler2D normalTexture;    // Textura normal (bump map)
uniform sampler2D specularTexture;  // Textura especular (opcional)

void main()
{
    vec4 diffuse = texture(diffuseTexture, TexCoords);
    vec4 normal = texture(normalTexture, TexCoords); // Usado solo si es necesario
    vec4 specular = texture(specularTexture, TexCoords); // Usado solo si es necesario

    // Aquí solo se utiliza la textura difusa
    FragColor = diffuse;
}