#version 120

varying vec3 vNormal;
uniform vec4 color;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float intensity = max(dot(normalize(vNormal), lightDir), 0.0);
    vec3 diffuse = intensity * color.rgb;
    vec3 ambient = color.rgb * 0.3;
    gl_FragColor = vec4(ambient + diffuse, color.a);
}
