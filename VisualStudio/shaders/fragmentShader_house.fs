#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;  // 02_-_Default
uniform sampler2D texture2;  // 07_-_Default
uniform sampler2D texture3;  // Abendmahl_O_Material_u1_v1.001
uniform sampler2D texture4;  // Curtain_Mat
uniform sampler2D texture5;  // Curtain_Rod_Mat
uniform sampler2D texture6;  // Drugs
uniform sampler2D texture7;  // None
uniform sampler2D texture8;  // Picture
uniform sampler2D texture9;  // material_0
uniform sampler2D texture10; // initialShadingGroup
uniform sampler2D texture11; // potted_plant_01_leaves
uniform sampler2D texture12; // potted_plant_01_pot

void main()
{
    // Obtener los colores difusos de cada textura
    vec4 color1 = texture(texture1, TexCoords);
    vec4 color2 = texture(texture2, TexCoords);
    vec4 color3 = texture(texture3, TexCoords);
    vec4 color4 = texture(texture4, TexCoords);
    vec4 color5 = texture(texture5, TexCoords);
    vec4 color6 = texture(texture6, TexCoords);
    vec4 color7 = texture(texture7, TexCoords);
    vec4 color8 = texture(texture8, TexCoords);
    vec4 color9 = texture(texture9, TexCoords);
    vec4 color10 = texture(texture10, TexCoords);
    vec4 color11 = texture(texture11, TexCoords);
    vec4 color12 = texture(texture12, TexCoords);

    // Combinar todas las texturas (puedes ajustar la mezcla según sea necesario)
    vec4 combinedColor = (color1 + color2 + color3 + color4 + color5 + color6 + color7 + color8 + color9 + color10 + color11 + color12) / 12.0;

    FragColor = combinedColor;
}
