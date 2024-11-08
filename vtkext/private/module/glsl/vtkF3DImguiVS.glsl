//VTK::System::Dec

uniform vec2 Scale;
uniform vec2 Shift;

in vec2 Position;
in vec2 UV;
in vec4 Color;

out vec2 Frag_UV;
out vec4 Frag_Color;

void main()
{
    Frag_UV = UV;
    Frag_Color = Color;
    gl_Position = vec4(Scale * Position + Shift, 0, 1);
}
