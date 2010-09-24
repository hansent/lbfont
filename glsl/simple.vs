
varying vec4 color;
varying vec3 texCoord;

void main()
{
	color = gl_Color;
	texCoord = gl_MultiTexCoord0.xyz;
	gl_Position = ftransform();
}
