#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput samplerposition;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput samplerNormal;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput samplerAlbedo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

//layout (constant_id = 0) const int NUM_LIGHTS = 64;

struct Light {
	vec4 position;
	vec3 color;
	float radius;
};

layout(set = 1,binding = 0) uniform UniformCameraObject{
	vec3 viewPos;
} camera;

layout(set = 1,binding = 1) uniform UniformLightObject{
	int NUM_LIGHT;
} l;

layout (set = 1, binding = 2) buffer readonly LIGHT{
	Light lights[];
};


void main() 
{
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = subpassLoad(samplerposition).rgb;
	vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);
	
	#define ambient 0.15
	
	// Ambient part
	vec3 fragcolor  = albedo.rgb * ambient;
	
	for(int i = 0; i < l.NUM_LIGHT; ++i)
	{
		// Vector to light
		vec3 L = lights[i].position.xyz - fragPos;
		// Distance from light to fragment position
		float dist = length(L);

		// Viewer to fragment
		vec3 V = camera.viewPos.xyz - fragPos;
		V = normalize(V);
		
		// Light to fragment
		L = normalize(L);

		// Attenuation
		float atten = lights[i].radius / (pow(dist, 2.0) + 1.0);

		// Diffuse part
		vec3 N = normalize(normal);
		float NdotL = max(0.0, dot(N, L));
		vec3 diff = lights[i].color * albedo.rgb * NdotL * atten;

		// Specular part
		// Specular map values are stored in alpha of albedo mrt
		vec3 R = reflect(-L, N);
		float NdotR = max(0.0, dot(R, V));
		//vec3 spec = ubo.lights[i].color * albedo.a * pow(NdotR, 32.0) * atten;

		fragcolor += diff;// + spec;	
	}    	
   
	outColor = vec4(fragcolor, 1.0);
}