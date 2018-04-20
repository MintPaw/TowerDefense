GAME_NAME=TowerDefense
PACKER=cmd /c "C:\Program Files (x86)\SpriteSheet Packer\SpriteSheetPacker.exe"
MONTAGE=magick montage

all:
	$(MAKE) b
	$(MAKE) r

ifneq (, $(findstring MSYS_NT, $(shell uname))) # ------------------------------------------------- Windows

bLibs:
	cmd /c "cl -c -MTd -Zi -EHsc -nologo -Iinclude/win32 -Iinclude/json src/libs.cpp -Fdbin/libs.pdb -Fobin/libs.obj"

b:
	$(MAKE) buildAssets
	cmd /c "cl -MTd -Zi -EHsc -nologo -Iinclude/win32 -Iinclude/json lib/win32/*.lib bin/libs.obj src/one.cpp -Fdbin/one.pdb -Fobin/one.obj -link /DEBUG -out:bin/${GAME_NAME}.exe"
	cp lib/win32/SDL2-d.dll bin/SDL2.dll
	cp lib/win32/glew32.dll bin
	cp lib/win32/OpenAl32-d.dll bin/OpenAl32.dll
	cp lib/win32/libcurl.dll bin
	cp lib/win32/libcurl-d.dll bin

r:
	@echo .
	@echo .
	@echo .
	@(cd bin; ./$(GAME_NAME).exe)

debugC:
	cmd /c "devenv /Run bin\$(GAME_NAME).exe"

endif

ifeq ($(shell uname), Linux) # ------------------------------------------------------------------- Linux

bLibs:
	g++ src/libs.cpp \
		-g -Wall -fpermissive -Wno-unused-variable -Wno-sign-compare -Wno-strict-aliasing -Wno-trigraphs \
		-isystem include/linux -isystem include/json -c -o bin/libs.o -lGL -lGLEW -lopenal -lSDL2

b:
	$(MAKE) buildAssets
	g++ bin/libs.o src/one.cpp \
		-g -Wall -Wno-unused-variable -Wno-sign-compare -Wno-strict-aliasing -Wno-trigraphs \
		-isystem include/linux -isystem include/json -o bin/$(GAME_NAME) -lGL -lGLEW -lopenal -lSDL2

r:
	@echo .
	@echo .
	@echo .
	@(cd bin; ./$(GAME_NAME))

debugC:
	(cd bin; gdb $(GAME_NAME))

endif

buildAssets:
	rm -rf bin/assets
	mkdir -p bin/assets/tilesets
	$(MONTAGE) -mode concatenate -channel rgba -background transparent -tile 23x raw/tiles/* bin/assets/tilesets/tileset.png
	cp -r raw/sprites bin/assets
	cp -r raw/maps bin/assets
	cp -r raw/shaders bin/assets
	cp -r raw/fonts bin/assets
	# -cmd /c "D:\_tools\SpriteSheetPacker\SpriteSheetPacker.exe --powerOf2 --format json raw/sprites assets"
