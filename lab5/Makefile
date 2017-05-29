src_folder=src
build_folder=build

CFLAGS = -Wall -pedantic -std=c11 -pthread 
glib_libs=${shell pkg-config --libs glib-2.0}
glib_flags=${shell pkg-config --cflags glib-2.0}

zox_aio_src_folder=${src_folder}/zox_aio
zox_aio_build_folder=${build_folder}/zox_aio

zox_aio_src_pattern:=${zox_aio_src_folder}/*.c ${zox_aio_src_folder}/*/*.c ${zox_aio_src_folder}/*/*/*.c
zox_aio_src_files:=${wildcard ${zox_aio_src_pattern}}

zox_aio_obj_files:=${patsubst %.c,%.o,${zox_aio_src_files}}
zox_aio_obj_files:=${subst ${zox_aio_src_folder},${zox_aio_build_folder},${zox_aio_obj_files}}


sources=${src_folder}/main.c ${src_folder}/utilities/utilities.c

${build_folder}/lab5: ${sources} ${zox_aio_build_folder}/libzox_aio.so
	@echo building $@ ...
	@gcc ${CFLAGS} ${glib_flags} -I${src_folder} -I${src_folder}/utilities \
	-I${zox_aio_src_folder} -o $@ ${sources} \
	-Wl,-Bdynamic -lpthread -lrt ${glib_libs} -ldl
	@cp $@ ./test/lab5

${zox_aio_build_folder}/libzox_aio.so: ${zox_aio_obj_files}
	@echo building $@ ...
	@gcc ${CFLAGS} -shared -o $@ $^ -lpthread -lrt ${glib_libs}	#declare here for the implicit dependencies declaration
	@cp $@ ./test/libzox_aio.so

${zox_aio_build_folder}/%.o: ${zox_aio_src_folder}/%.c
	@echo compiling $@ ...
	@mkdir -p ${dir $@}
	@gcc ${CFLAGS} ${glib_flags} -I${zox_aio_src_folder} -I${zox_aio_src_folder}/utilities \
	-c -fPIC -o $@ -Wl,-Bdynamic -lpthread -lrt ${glib_libs} $<

run:
	@${build_folder}/lab5