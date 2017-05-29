#define _GNU_SOURCE
#define GETTEXT_PACKAGE "gtk20"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <glib.h>
#include "zox_aio.h"
#include "utilities.h" 

int main(int argc, char* argv[]) {
	void* zox_aio_handle = dlopen("./libzox_aio.so", RTLD_NOW);
	if (zox_aio_handle == NULL) {
		printf("load shared library error: %s",dlerror());
		return EXIT_FAILURE;
	} 
	const struct zox_aio ZOX_AIO = 
			*((const struct zox_aio*)dlsym(zox_aio_handle, "ZOX_AIO")); 
	zox_aio_settings settings;
	settings.target_file = NULL;
	settings.ms_delay = 0;
	settings.buffer_size = 20;	
	settings.is_debug_enabled = 0;	

	// ----- command line arguments parsing -----
	GOptionContext* cla_context = g_option_context_new ("- concat files in exec directory");
	GOptionEntry cla_entries[] = {
		{"file", 'f', 0, G_OPTION_ARG_FILENAME, 
							&(settings.target_file), "output file, required", NULL},
		{"delay", 'd', 0, G_OPTION_ARG_INT, 
							&(settings.ms_delay), "delay in miliseconds", NULL},
		{"buffer", 'b', 0, G_OPTION_ARG_INT, 
							&(settings.buffer_size), "buffer size in bytes", NULL},
		{"debug", 'g', 0, G_OPTION_ARG_NONE, 
							&(settings.is_debug_enabled), "debug mode", NULL},
		{NULL}
	};
    g_option_context_set_help_enabled(cla_context, TRUE);
    g_option_context_add_main_entries(cla_context, cla_entries, GETTEXT_PACKAGE);
    GError *error = NULL;
    if (!g_option_context_parse (cla_context, &argc, &argv, &error)) {
      printf("command line parsing failed: %s\n", error->message);
      exit(EXIT_FAILURE);
    }
    if (settings.target_file == NULL) {
    	gchar* usage = g_option_context_get_help (cla_context, TRUE, NULL);
    	printf("%s", usage);
    	exit(0);
    }
    //printf("%s; %d; %d; %d", settings.target_file, settings.ms_delay, settings.buffer_size, settings.is_debug_enabled);
    g_option_context_free(cla_context);
    // --------------------------------------------

    // ----- getting files in current directory -----
	DIR* currentDir = opendir(".");
	if (currentDir == NULL) {
		perror("openning current directory");
		dlclose(zox_aio_handle);
		return EXIT_FAILURE;
	}
	GArray* source_files = g_array_new(FALSE, FALSE, sizeof(const char*));
	int source_files_count = 0;
	struct dirent* dir_entry = readdir(currentDir);
	while (dir_entry != NULL) {
		const char* entry_name = dir_entry->d_name;
		if (string_ends_with(entry_name, ".part")) {			
			g_array_append_val(source_files, entry_name);
			source_files_count++;
		}		
		dir_entry = readdir(currentDir);
	}
	// ----------------------------------------------	

	// ----- start tasks -----
	zox_aio_context* context = NULL;
	ZOX_AIO.initialize_context(&context, settings);
	GArray* tasks_id = g_array_new(FALSE, FALSE, sizeof(int));
	for (int i = 0; i < source_files_count; i++) {
		const char* part_file = g_array_index(source_files, const char*, i);
		int new_task_id = ZOX_AIO.add_task(context, part_file);
		g_array_append_val(tasks_id, new_task_id);
	}
	// -----------------------

	// ----- progress bar loop -----
	while(1) {
		system("clear");
		int is_completed = 1;
		for (int i = 0; i < source_files_count; i++) {
			int task_id = g_array_index(tasks_id, int, i);
			int progress = ZOX_AIO.get_task_progress(context, task_id);
			const char* task_filename = ZOX_AIO.get_task_filename(context, task_id);
			print_progress_bar(task_filename, progress);
			if (progress < 100) is_completed = 0;
		}
		if (is_completed == 1) break;
		usleep(1000*100);
	}
	// -----------------------------

	g_array_free(tasks_id, TRUE);
	g_array_free(source_files, TRUE);
	ZOX_AIO.destroy_context(context);
	dlclose(zox_aio_handle);
	return EXIT_SUCCESS;
}