#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
	vec3 Normal;
} gs_in[];

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 0.05;
    vec3 direction = normalize(normal) * magnitude; 
    return position + vec4(direction, 0.0);
}

void main() {
	vec3 normal = gs_in[0].Normal;
    gl_Position = explode(gl_in[0].gl_Position, normal);
    EmitVertex();
	normal = gs_in[1].Normal;
    gl_Position = explode(gl_in[1].gl_Position, normal);
    EmitVertex();
	normal = gs_in[2].Normal;
    gl_Position = explode(gl_in[2].gl_Position, normal);
    EmitVertex();
    EndPrimitive();
}  