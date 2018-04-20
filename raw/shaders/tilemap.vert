#if __VERSION__ >= 300
in vec2 a_position;
out vec2 v_texCoord;
#elif __VERSION__ >= 100
attribute vec2 a_position;
varying vec2 v_texCoord;
#endif

uniform mat3 u_matrix;

void main(void) {
	a_position;
	u_matrix;

	v_texCoord = vec2(a_position.x, a_position.y);
	vec2 realPos = (u_matrix * vec3(a_position, 1)).xy;
	gl_Position = vec4(realPos.x, -realPos.y, 0, 1);
}
