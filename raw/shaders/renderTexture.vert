#if __VERSION__ >= 300
in vec2 a_position;
in vec2 a_texCoord;
out vec2 v_texCoord;
#elif __VERSION__ >= 100
attribute vec2 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
#endif

uniform mat3 u_matrix;

void main(void) {
	v_texCoord = a_texCoord;
	gl_Position = vec4((u_matrix * vec3(a_position, 1)).xy, 0, 1);
}
