GAME_NAME=TowerDefense
PACKER=cmd /c "C:\Program Files (x86)\SpriteSheet Packer\SpriteSheetPacker.exe"

all:
	$(MAKE) b
	$(MAKE) r

ifneq (, $(findstring MSYS_NT, $(shell uname)))
b:
	$(MAKE) packSprites
	$(MAKE) bWindows

r:
	$(MAKE) rWindows

debugC:
	cmd /c "devenv /Run bin\${GAME_NAME}.exe"

packSprites:
	-cmd /c "D:\_tools\SpriteSheetPacker\SpriteSheetPacker.exe --powerOf2 --format json raw/sprites assets"

endif

ifeq ($(shell uname), Linux)
all:
	$(MAKE) bLinux
	$(MAKE) rLinux
endif

bWindows:
	cmd /c "cl -MTd -Zi -EHsc -nologo -Iinclude/win32 -Iinclude/json lib/win32/*.lib src/one.cpp -Fdbin/one.pdb -Fobin/one.obj -link /DEBUG -out:bin/${GAME_NAME}.exe"
	cp lib/win32/SDL2-d.dll bin/SDL2.dll
	cp lib/win32/glew32.dll bin
	cp lib/win32/OpenAl32-d.dll bin/OpenAl32.dll
	cp lib/win32/libcurl.dll bin
	cp lib/win32/libcurl-d.dll bin
	cp -r assets bin

rWindows:
	@echo .
	@echo .
	@echo .
	@(cd bin; ./${GAME_NAME}.exe)
