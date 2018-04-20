GAME_NAME=TowerDefense
PACKER=cmd /c "C:\Program Files (x86)\SpriteSheet Packer\SpriteSheetPacker.exe"
MONTAGE=magick montage

all:
	$(MAKE) b
	$(MAKE) r

ifneq (, $(findstring MSYS_NT, $(shell uname))) # ------------------------------------------------- Windows

b:
	$(MAKE) buildAssets
	cmd /c "cl -MTd -Zi -EHsc -nologo -Iinclude/win32 -Iinclude/json lib/win32/*.lib src/one.cpp -Fdbin/one.pdb -Fobin/one.obj -link /DEBUG -out:bin/${GAME_NAME}.exe"
	cp lib/win32/SDL2-d.dll bin/SDL2.dll
	cp lib/win32/glew32.dll bin
	cp lib/win32/OpenAl32-d.dll bin/OpenAl32.dll
	cp lib/win32/libcurl.dll bin
	cp lib/win32/libcurl-d.dll bin

r:
	@echo .
	@echo .
	@echo .
	@(cd bin; ./${GAME_NAME}.exe)

debugC:
	cmd /c "devenv /Run bin\${GAME_NAME}.exe"

endif

ifeq ($(shell uname), Linux) # ------------------------------------------------------------------- Linux

all:
	$(MAKE) bLinux
	$(MAKE) rLinux

endif

buildAssets:
	rm -rf bin/assets
	mkdir -p bin/assets/tilesets
	$(MONTAGE) -mode concatenate -channel rgba -background transparent -tile 23x raw/tiles/* bin/assets/tilesets/tileset.png
	cp -r raw/sprites bin/assets
	cp -r raw/maps bin/assets
	cp -r raw/shaders bin/assets
	# -cmd /c "D:\_tools\SpriteSheetPacker\SpriteSheetPacker.exe --powerOf2 --format json raw/sprites assets"
