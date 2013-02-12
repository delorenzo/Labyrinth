
varying vec3 N;
varying vec3 v; 
varying vec3 normal, lightDir, viewDir;

void main()
{	
	lightDir = normalize(vec3(gl_LightSource[0].position));
	normal = normalize(gl_NormalMatrix * gl_Normal);
	viewDir = normalize(vec3(gl_ModelViewMatrix * gl_Vertex));
	gl_Position = ftransform();

	v = vec3(gl_ModelViewMatrix * gl_Vertex);       
   N = normalize(gl_NormalMatrix * gl_Normal);
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;  
}
