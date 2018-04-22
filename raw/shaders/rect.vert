#if __VERSION__ >= 300
in vec2 a_position;
#elif __VERSION__ >= 100
attribute vec2 a_position;
#endif

uniform mat3 u_projection;

void main(void) {
	gl_Position = vec4((u_projection * vec3(a_position, 1)).xy, 0, 1);
}
