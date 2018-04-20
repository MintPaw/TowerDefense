precision mediump float;

#if __VERSION__ >= 300
in vec2 v_texCoord;
out vec4 fragColor;
#define TEXTURE2D texture
#elif __VERSION__ >= 100
varying vec2 v_texCoord;
vec4 fragColor;
#define TEXTURE2D texture2D
#endif

uniform vec4 u_colour;

void main(void) { 
	if ((v_texCoord.x - 0.5)*(v_texCoord.x - 0.5) + (v_texCoord.y - 0.5)*(v_texCoord.y - 0.5) > 0.25) discard;

	fragColor = u_colour;

#if __VERSION__ == 100
	gl_FragColor = fragColor;
#endif
}
