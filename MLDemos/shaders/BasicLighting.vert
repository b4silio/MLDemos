#version 120

attribute vec3 position;
attribute vec3 normal;

uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;
uniform vec4 materialColor;
uniform vec4 lightPosition;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform float specularity;
uniform float shininess;

varying vec4 fragColor;

void main()
{
    // Transform vertex position and normal
    vec3 posEye = vec3(mvMatrix * vec4(position, 1.0));
    vec3 normEye = normalize(normalMatrix * normal);

    // Calculate lighting vectors
    vec3 lightDir;
    if (lightPosition.w == 0.0) {
        // Directional light
        lightDir = normalize(vec3(lightPosition));
    } else {
        // Point light
        lightDir = normalize(vec3(lightPosition) - posEye);
    }

    // Ambient component
    vec4 ambient = lightAmbient * materialColor;

    // Diffuse component
    float NdotL = max(dot(normEye, lightDir), 0.0);
    vec4 diffuse = lightDiffuse * materialColor * NdotL;

    // Specular component
    vec4 specular = vec4(0.0);
    if (NdotL > 0.0) {
        vec3 viewDir = normalize(-posEye);
        vec3 halfDir = normalize(lightDir + viewDir);
        float NdotH = max(dot(normEye, halfDir), 0.0);
        specular = lightSpecular * vec4(specularity, specularity, specularity, 1.0) * pow(NdotH, shininess);
    }

    // Final color
    fragColor = ambient + diffuse + specular;
    fragColor.a = materialColor.a;

    // Transform position
    gl_Position = mvpMatrix * vec4(position, 1.0);
}
