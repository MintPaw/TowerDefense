precision highp float;

#if __VERSION__ >= 300
in vec2 v_texCoord;
out vec4 fragColor;
#define TEXTURE2D texture
#elif __VERSION__ >= 100
varying vec2 v_texCoord;
vec4 fragColor;
#define TEXTURE2D texture2D
#endif

uniform sampler2D u_texture;

uniform vec4 u_tint;
uniform bool u_bleed;

void main(void) { 
	vec4 textureFrag = TEXTURE2D(u_texture, v_texCoord);

	fragColor = (u_tint - textureFrag)*u_tint.a + textureFrag;
	if (u_bleed == false) fragColor.a = textureFrag.a;

#if __VERSION__ == 100
	gl_FragColor = fragColor;
#endif
}
