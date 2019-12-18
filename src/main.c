#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include "parser.tab.h"

const char *argp_program_version = "yproto 1.0";
const char *argp_program_bug_address = "<liaotonglang@gmail.com>";
static char doc[] =
"yproto -- yet another binary encoder and decoder like protobuf.";

static char args_doc[] = "FILE";

static struct argp_option options[] = {
    {"verbose",  'v', 0,      0,  "Produce verbose output" },
    {"quiet",    'q', 0,      0,  "Don't produce any output" },
    {"silent",   's', 0,      OPTION_ALIAS },
    { 0 }
};

struct arguments
{
    int silent, verbose;
    char *input_file;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
        case 'q': case 's':
            arguments->silent = 1;
            break;
        case 'v':
            arguments->verbose = 1;
            break;

        case ARGP_KEY_ARG:
            if (state->arg_num >= 1)
                argp_usage(state);
            arguments->input_file = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1)
                argp_usage(state);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

extern FILE *yyin;

int main(int argc, char *argv[]) {
    struct arguments arguments;

    arguments.silent = 0;
    arguments.verbose = 0;
    arguments.input_file = NULL;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    printf("OUTPUT_FILE = %s\n"
            "VERBOSE = %s\nSILENT = %s\n",
            arguments.input_file,
            arguments.verbose ? "yes" : "no",
            arguments.silent ? "yes" : "no");

    yyin = fopen(arguments.input_file, "r");
    if (yyin == NULL) {
        fprintf(stderr, "cannot open file: %s\n", arguments.input_file);
        exit(1);
    }
    int r = yyparse();
    fclose(yyin);
    return r;
}
