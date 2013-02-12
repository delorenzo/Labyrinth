
varying vec3 normal, lightDir, viewDir;

varying vec3 N;
varying vec3 v;
void main()
{

   vec3 L = normalize(gl_LightSource[0].position.xyz - v);   
   vec3 E = normalize(-v); // we are in Eye Coordinates, so EyePos is (0,0,0)  
   vec3 R = normalize(-reflect(L,N));  
 
   
   vec4 Iamb = vec4(0.3,0.3,0.3,1.0);  

    
   vec4 Idiff = vec4(0.2,0.2,0.2,1.0) * max(dot(N,L), 0.0);
   Idiff = clamp(Idiff, 0.0, 1.0);  
   
  
   vec4 Ispec = vec4(1,1,1,1) * pow(max(dot(R,E),0.0),0.3*);
   Ispec = clamp(Ispec, 0.0, 1.0); 
   
   gl_FragColor = Iamb + Idiff + Ispec;     
}
