precision mediump float;

#if __VERSION__ >= 300
out vec4 fragColor;
#define TEXTURE2D texture
#elif __VERSION__ >= 100
vec4 fragColor;
#define TEXTURE2D texture2D
#endif

uniform vec4 u_colour;

void main(void) { 
	fragColor = u_colour;

#if __VERSION__ == 100
	gl_FragColor = fragColor;
#endif
}
