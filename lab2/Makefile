CFLAGS:=-Wall -pedantic -std=c11 -g -Isrc
build_targets:=prepareBuildFolders 	
os:=lin

ifeq (${OS},Windows_NT)
	os:=win
	bf=build/${os}
	build_targets:=${build_targets} ${bf}/main_win.exe ${bf}/childManager_win.exe ${bf}/child_win.exe
else
	os:=lin
	bf=build/${os}
	build_targets:=${build_targets} ${bf}/main_lin ${bf}/child_lin	
endif

.PHONY: prepareBuildFolders run clean prepareEnv

lab2: ${build_targets}

# win instructions
${bf}/main_win.exe: src/main.c src/win/implementation_win.c
	gcc ${CFLAGS} -o ${bf}/main_win.exe $^

${bf}/child_win.exe: src/win/child_win.c
	gcc ${CFLAGS} -o ${bf}/child_win.exe $^

${bf}/childManager_win.exe: src/stack/stack.c src/win/childManager.c 
	gcc ${CFLAGS} -o ${bf}/childManager_win.exe $^


# lin instruction
${bf}/main_lin: src/main.c src/lin/implementation_lin.c src/stack/stack.c
	gcc ${CFLAGS} -o ${bf}/main_lin $^

${bf}/child_lin: src/lin/child_lin.c
	gcc ${CFLAGS} -o ${bf}/child_lin $^

# auxiliary targets
run: 
	@cd build/${os}&&./main_${os}



prepareBuildFolders:
	@mkdir -p build/win
	@mkdir -p build/lin

clean:
	@rm -r ./build