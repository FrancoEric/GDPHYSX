#version 330 core

in vec2 texCoord;

out vec4 FragColor;

uniform vec3 objectColor;
uniform sampler2D tex0;

uniform bool useTexture;

void main()
{
	if(useTexture)
        FragColor = texture(tex0, texCoord);
    else
        FragColor = vec4(objectColor, 1.0);
}