#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPositions[4];
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform float constant;
uniform float linear;
uniform float quadratic;

uniform sampler2D texture_diffuse1;
uniform float opacity;

void main()
{
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    float ambientStrength = 0.1;
    float specularStrength = 0.2;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    for (int i = 0; i < 4; i++)
    {
        vec3 lightDir = normalize(lightPositions[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0) * 0.1 * (i == 3 ? 2.0 : 1.0);  // Luz 4 tiene más peso en el diffuse
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
        float distance = length(lightPositions[i] - FragPos);
        float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
        ambient += ambientStrength * lightColor * attenuation;
        diffuse += diff * lightColor * attenuation;
        specular += specularStrength * spec * lightColor * attenuation;
    }

    vec3 result = (ambient + diffuse + specular) * texture(texture_diffuse1, TexCoords).rgb;
    FragColor = vec4(result, opacity);
}