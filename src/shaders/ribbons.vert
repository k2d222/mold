#version 330 core

uniform samplerBuffer u_atom_color_buffer;
uniform usamplerBuffer u_atom_flags_buffer;

layout (std140) uniform ubo {
    mat4 u_world_to_view;
    mat4 u_world_to_view_normal;
    mat4 u_world_to_clip;
    mat4 u_view_to_clip;
    mat4 u_view_to_world;
    mat4 u_clip_to_view;
    mat4 u_prev_world_to_clip;
    mat4 u_curr_view_to_prev_clip;
    vec4 u_jitter_uv;
    uint u_atom_mask;
    uint _pad0;
    uint _pad1;
    uint _pad2;
    float u_width_scale;
    float u_height_scale;
};

layout (location = 0) in vec3 in_control_point;
layout (location = 1) in uint in_atom_index;
layout (location = 2) in vec3 in_velocity;
layout (location = 3) in uint in_flags;
layout (location = 4) in vec3 in_support_vector;
layout (location = 5) in vec3 in_support_tangent;

out Vertex {
    vec4 control_point;
    vec4 support_vector;
    vec4 support_tangent;
    vec4 color;
    vec3 view_velocity;
    uint picking_idx;
    uint flags;
} out_vert;

void main() {
    out_vert.control_point   = vec4(in_control_point, 1);
    out_vert.support_vector  = vec4(in_support_vector, 0);
    out_vert.support_tangent = vec4(in_support_tangent, 0);
    out_vert.color           = texelFetch(u_atom_color_buffer, int(in_atom_index));
    out_vert.view_velocity   = vec3(u_world_to_view * vec4(in_velocity, 0));
    out_vert.picking_idx     = in_atom_index;
    out_vert.flags           = texelFetch(u_atom_flags_buffer, int(in_atom_index)).x;
}