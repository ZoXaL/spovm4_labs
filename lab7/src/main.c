#include "cluster.h"
#include "zox-fs.h"
#include "repl.h"

// 1) mkdir, cd, ls, rm, free, clustmap
// 2) load, extract
// 3) mv, cp

/*
 * 1) Создать папку tmp, mike, перейти в неё и создать папку spovm.
 * Из корня создать папку /mike/spovm/labs. Перейти в неё.
 * удалить папку tmp, создать папку tmp2, создать папку tmp3.
 * Перейти в /mike/spovm, удалить /mike. Перейти в /, удалить /mike, clustmap.
 * Загрузить файл 1 в /. Переместить его в /tmp2/any_data.txt.
 * Скопировать его в /tmp3/any_data.txt. Извлечь в any_data.txt
 * удалить /tmp2, clustmap
 *
 *
 *
 */

int main(int argc, char* argv[]) {
	fs_context_t context = initialize_context(argv[1]);
	start_repl(context);
	fs_context_destroy(context);
	return 0;
}