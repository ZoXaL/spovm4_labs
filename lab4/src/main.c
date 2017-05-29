#define _POSIX_SOURCE
#define GETTEXT_PACKAGE "gtk20"
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <signal.h>     // sigaction
#include <string.h>     // memset
#include <errno.h>      // errno
#include <fcntl.h>      // open
#include "tm.h"

#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (1)

#define handle_error(msg) \
       do { perror(msg); exit(EXIT_FAILURE); } while (1)

extern gint child_delay;
extern gint child_letters_count;
extern int outputFD;

void preDestroy(int s) { 
    kill_tm();
    printf("Program has been gracefully closed\n");
    exit(EXIT_SUCCESS);
}

int main (int argc, char *argv[]) {
    // setup interrupt handler
    struct sigaction interruptAction;
    memset(&interruptAction, '\0', sizeof(interruptAction));
    interruptAction.sa_handler = preDestroy;
    if (sigaction(SIGINT, &interruptAction, NULL) <= -1) {
        perror("sigaction: ");
        exit(EXIT_FAILURE);
    }
    
    // setup command line arg parser and parse 
    gchar* output = NULL;
    GOptionContext* context;

    GOptionEntry entries[] = {
      { "output", 'o', 0, G_OPTION_ARG_STRING, &output, "Children output destination (e.g. terminal file)", NULL },
      { "delay", 'd', 0, G_OPTION_ARG_INT, &child_delay, "Children delay between letters in miliseconds", NULL },
      { "letters", 'l', 0, G_OPTION_ARG_INT, &child_letters_count, "Children letters count", NULL },
      { NULL }
    };

    GError *error = NULL;
    context = g_option_context_new ("- create and manage spaming threads");
    g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
    g_option_context_set_help_enabled (context, TRUE);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
      g_print ("option parsing failed: %s\n", error->message);
      exit(EXIT_FAILURE);
    }
    g_option_context_free(context);
    if (output != NULL) {
        outputFD = open(output, O_WRONLY);
        if (outputFD == -1) {
            handle_error("open: ");
        }
    }
    if (child_delay < 0 || child_letters_count < 1) {
        handle_error_en(EINVAL, "validation: ");
    }
    
    // create thread manager
    int call_result = start_tm();
    if (call_result != 0) {
        handle_error_en(call_result, "pthread_create: ");
    }

    // user input loop
 	int choise;
 	printf("> ");
 	while(1) { 		
 		choise = getchar();
 		switch (choise) {
 			case '+' : {        // add child
                send_tm('+');
 				printf("> ");
 				break;
 			}
 			case '-' : {        // kill child
                send_tm('-');
 				printf("> ");
 				break;
 			}
 			case 'p' : {        // pause
                send_tm('p');
                printf("> ");
 				break;
 			}
            case 'i' : {        // child info
                print_current_child();
                printf("> ");
                break;
            }
            case 'q' : {        // exit
                preDestroy(0);
                return 0;
            }
 			default : {
 				break;
 			}
 		}
    }
    return EXIT_SUCCESS;
 }
