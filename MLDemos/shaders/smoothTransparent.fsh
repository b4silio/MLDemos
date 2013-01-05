varying vec3 normal, lightDir;
varying vec4 diffuse;

void main()
{
  vec4 color;
  float dotProduct;

  dotProduct = dot(normal, lightDir);

  color = dotProduct * diffuse;

  gl_FragColor = color;
}
