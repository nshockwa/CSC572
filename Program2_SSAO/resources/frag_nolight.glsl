#version 330 core
out vec4 FragColor;
  
in vec2 fragTex;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};
uniform Light light;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, fragTex).rgb;
    vec3 Normal = texture(gNormal, fragTex).rgb;
    vec3 Diffuse = texture(gAlbedo, fragTex).rgb;
    vec3 AmbientOcclusion = texture(ssao, fragTex).rgb;
    AmbientOcclusion.x = pow(AmbientOcclusion.x,2);
	AmbientOcclusion.y = pow(AmbientOcclusion.y,2);
	AmbientOcclusion.z = pow(AmbientOcclusion.z,2);
    // blinn-phong (in view-space)
    vec3 ambient = vec3(0.3 * Diffuse); // here we add occlusion factor
    vec3 lighting  = ambient*AmbientOcclusion; 
    vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0) in view-space
    // diffuse
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.Color * spec;
    // attenuation
    float dist = length(light.Position - FragPos);
   // float attenuation = 1.0 / (1.0 + light.Linear * dist + light.Quadratic * dist * dist);
    //diffuse  *= attenuation;
    //specular *= attenuation;
	float diffusefact = (0.2 + 0.8*max(dot(Normal, lightDir), 0.0));

    lighting = diffusefact*AmbientOcclusion*Diffuse + specular;

    FragColor = vec4(lighting, 1.0);
}