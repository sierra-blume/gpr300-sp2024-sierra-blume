#version 450
out vec4 FragColor; //The color of this fragment

in VS_OUT{
	vec3 WorldPos; //Vertex position in world space
	vec3 WorldNormal; //Vertex normal in world space
	vec2 TexCoord;
	vec4 FragPosLightSpace;
}fs_in;

uniform sampler2D _MainTex; //2D texture sampler
uniform sampler2D _ShadowMap;

uniform vec3 _LightPos;
uniform vec3 _EyePos;
//Light pointing straight down
//uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0); //White light
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};

uniform Material _Material;

float ShadowCalculation(vec4 fragPosLightSpace)
{
	//Perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	//Transform the NDC coordinates to the range [0,1]
	projCoords = projCoords * 0.5 + 0.5;
	//Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(_ShadowMap, projCoords.xy).r;
	//Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	//Check whether current frag pos is in shadow
	float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

	return shadow;
}

void main(){
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	//Make sure fragment normal is still length 1 after interpolation
	vec3 normal = normalize(fs_in.WorldNormal);
	//Light pointing straight down
	vec3 toLight = normalize(_LightPos - fs_in.WorldPos);
	float diffuseFactor = 0.5 * max(dot(normal, toLight), 0.0);
	//Direction towards eye
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	//Blinn-phong uses half angle
	vec3 halfwayDir = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, halfwayDir), 0.0), _Material.Shininess);

	/*//Combination of specular and diffuse reflection
	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
	//Add some ambient light
	lightColor += _AmbientColor * _Material.Ka;*/

	//Calculate shadow
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
	vec3 lighting = ((_AmbientColor * _Material.Ka) + (1.0 - shadow) * (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor)) * objectColor;

	FragColor = vec4(lighting, 1.0);
}