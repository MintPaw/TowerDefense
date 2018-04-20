precision highp float;

#if __VERSION__ >= 300
in vec2 v_texCoord;
out vec4 fragColor;
#define TEXTURE2D texture
#elif __VERSION__ >= 100
vec4 fragColor;
varying vec2 v_texCoord;
#define TEXTURE2D texture2D
#endif

uniform sampler2D u_tilesetTexture;
uniform sampler2D u_tilemapTexture;
uniform ivec2 u_tilesetSize;
uniform ivec2 u_tilemapSize;
uniform ivec2 u_resultSize;
uniform ivec2 u_pixelRatio;

void main(void) {
	u_tilesetTexture;
	u_tilemapTexture;
	u_tilesetSize;
	u_tilemapSize;
	u_resultSize;
	u_pixelRatio;

	vec2 dataTex;
	dataTex.x = v_texCoord.x / float(u_resultSize.x);
	dataTex.y = v_texCoord.y / float(u_resultSize.y);
	// realTex *= vec2(1, -1); // Flip because default.frag expects all textures to be upside-down
	
	ivec2 tilesetSizeInTiles = u_tilesetSize / u_pixelRatio;

	vec4 dataFrag = TEXTURE2D(u_tilemapTexture, dataTex);
	int tileIndex = 0;
	tileIndex += int(dataFrag.r*256.0);
	tileIndex += 256 * int(dataFrag.g*256.0);
	//tileIndex += (256*2) * int(dataFrag.b*256.0);
	//tileIndex += (256*3) * int(dataFrag.a*256.0);
	tileIndex -= 1;
	if (tileIndex == -1) discard;
	ivec2 tilePos = ivec2(mod(float(tileIndex), float(tilesetSizeInTiles.x)), tileIndex / tilesetSizeInTiles.x);
	vec2 uv = vec2(tilePos) / vec2(u_tilesetSize);
	
	// fragColor = vec4(float(tilePos.x)/10.0, float(tilePos.y)/10.0, float(tileIndex)/100.0, 1);
	// fragColor = vec4(float(u_tilesetSize.y)/100.0, 0.0, 0.0, 1);
	// return;

	// int x = u_tilesetSize.x + u_tilemapSize.x + u_resultSize.x + u_pixelRatio.x;
	// fragColor = vec4(float(u_tilesetSize.x)/10000.0, 0.0, float(x)*0.000001, 1.0);
	// return;

	vec2 offset = vec2(
		fract(dataTex.x * float(u_tilemapSize.x)) / float(u_tilesetSize.x),
		fract(dataTex.y * float(u_tilemapSize.y)) / float(u_tilesetSize.y)
	);
	
	//fragColor = vec4(offset.x*10.0, offset.y*10.0, 0.0, 1);
	//return;

	uv += offset;
	uv *= vec2(u_pixelRatio);
	fragColor = TEXTURE2D(u_tilesetTexture, uv);

#if __VERSION__ == 100
	gl_FragColor = fragColor;
#endif
}
