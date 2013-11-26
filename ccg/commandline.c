/* C Code Generator
 *
 * Copyright (C) 2012, Antoine Balestrat <antoine.balestrat@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include "ccg.h"

extern char *optarg;

CommandlineOpt cmdline;
static char outputfilename[512];
FILE *outputstream;

#define MAX_CMDSTR_LEN 2048
char cmdstr[MAX_CMDSTR_LEN + 1];

static void printHelp(void)
{
    puts("  -h, --help\t\t\tShow this help");
    puts("  -v, --version\t\t\tPrint the version of CCG");
    puts("  --output <file>: output generated code to <file> rather than stdout");
    puts("  --seed\t\t\tUse a custom seed (instead of a random one)");
    puts("  --max-functions\t\tMax number of functions to be generated (default 12)");
    puts("  --max-localvars\t\tMax number of variables in a block (default 4)");
    puts("  --max-function-parameters\tMax number of parameters for a function (default 4)");
    puts("  --max-statements-per-block\tMax number of statements in each block (default 7)");
    puts("  --max-expression-nesting\tControl the complexity of the expressions (default 8)");
    puts("  --max-block-nesting\t\tControl the nesting of the blocks (default 7)");
    puts("  --max-pointer-depth\t\tMaximum depth of a pointer (default 2)");
    puts("  --no-jumps\t\t\tDisables jumps (enabled by default)");
    puts("  --no-ternaryexprs\t\t\tDisables ternary expressions (enabled by default)");
    puts("  --no-functioncallexprs\t\t\tDisables function call expressions (enabled by default)");
    puts("  --no-operationexprs\t\t\tDisables operation expressions (enabled by default)");
    puts("  --no-testexprs\t\t\tDisables test expressions (enabled by default)");
    puts("  --no-assignmentexprss\t\t\tDisables assignment expressions (enabled by default)");
    puts("  --no-ifs\t\t\tDisables ifs (enabled by default)");
    puts("  --no-fors\t\t\tDisables fors (enabled by default)");
    puts("  --no-assignments\t\t\tDisables assignments (enabled by default)");
    puts("  --no-ptrassignments\t\t\tDisables pointer assignments (enabled by default)");
    puts("  --no-functioncalls\t\t\tDisables function calls (enabled by default)");
    puts("  --no-returns\t\t\tDisables returns (enabled by default)");
    puts("  --no-integers\t\t\tDisables integers (enabled by default)");
    puts("  --no-floats\t\t\tDisables floats (enabled by default)");
    puts("  --no-pointers\t\t\tDisables pointers (enabled by default)");
    puts("  --swarm\t\t\tEnable swarm testing (disabled by default)");
    puts("  --swarm-replay\t\t\tUsed for re-generating a swarm-ed program");
    exit(EXIT_SUCCESS);
}

static void printVersion(void)
{
    puts("This is ccg version " CCG_VERSION);
    exit(EXIT_SUCCESS);
}

static const struct option longopt[] =
{
    {"seed", required_argument, NULL, 0},
    {"max-functions", required_argument, NULL, 0},
    {"max-localvars", required_argument, NULL, 0},
    {"max-function-parameters", required_argument, NULL, 0},
    {"max-statements-per-block", required_argument, NULL, 0},
    {"max-expression-nesting", required_argument, NULL, 0},
    {"max-block-nesting", required_argument, NULL, 0},
    {"max-pointer-depth", required_argument, NULL, 0},
    {"no-jumps", no_argument, NULL, 0},
    {"help", no_argument, NULL, 0},
    {"version", no_argument, NULL, 0},
    {"output", required_argument, NULL, 0},
    {"no-ternaryexprs", no_argument, NULL, 0},
    {"no-functioncallexprs", no_argument, NULL, 0},
    {"no-operationexprs", no_argument, NULL, 0},
    {"no-testexprs", no_argument, NULL, 0},
    {"no-assignmentexprs", no_argument, NULL, 0},
    {"no-ifs", no_argument, NULL, 0},
    {"no-fors", no_argument, NULL, 0},
    {"no-assignments", no_argument, NULL, 0},
    {"no-ptrassignments", no_argument, NULL, 0},
    {"no-functioncalls", no_argument, NULL, 0},
    {"no-returns", no_argument, NULL, 0},
    {"no-integers", no_argument, NULL, 0},
    {"no-floats", no_argument, NULL, 0},
    {"no-pointers", no_argument, NULL, 0},
    {"swarm", no_argument, NULL, 0},
    {"swarm-replay", no_argument, NULL, 0},
    {NULL, 0, NULL, 0}
};

static int opt_length = (sizeof(longopt) / sizeof(struct option)) - 1;
static unsigned * index2member[(sizeof(longopt) / sizeof(struct option)) - 1];
static SwarmKind index2swarmkind[(sizeof(longopt) / sizeof(struct option)) - 1];

static void setopt(int index)
{
    assert((index < opt_length) && "Invalid option index!");
    if(index >= 0 && index <= 7)
        *(index2member[index]) = strtol(optarg, NULL, 10);
    else if(index == 9)
        printHelp();
    else if(index == 10)
        printVersion();
    else if(index == 11)
        strcpy(outputfilename, optarg);
    else
      *(index2member[index]) = true;

    /* Sanity check */
    if(cmdline.max_pointer_depth == 0)
        die("the maximum pointer depth can't be 0 !");
}

void fakeSwarmOptions(void)
{
    int index;
    for (index = 0; index < opt_length; index++) {
        SwarmKind kind = index2swarmkind[index];
        if (kind == SK_None)
            continue;
        else if (kind == SK_Flipcoin)
            rand();
        else if (kind == SK_Upto)
            assert(0 && "Not implement yet!");
        else
            assert(0 && "Unsupported SwarmKind!");
    }
}

void enableSwarmOptions(void)
{
    int index;
    for (index = 0; index < opt_length; index++) {
        SwarmKind kind = index2swarmkind[index];
        if (kind == SK_None)
            continue;
        else if (kind ==  SK_Flipcoin)
            *index2member[index] = rand() % 2;
        else if (kind == SK_Upto)
            assert(0 && "Not implement yet!");
        else
            assert(0 && "Unsupported SwarmKind!");
    }
}

void initCommandline(void)
{
    int index = 0;
    int swarm_index = 0;

    index2member[index++] = &cmdline.seed;
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.max_functions; 
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.max_localvars; 
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.max_function_parameters;
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.max_statements_per_block; 
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.max_expression_nesting; 
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.max_block_nesting; 
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.max_pointer_depth;
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.nojumps;
    index2swarmkind[swarm_index++] = SK_None;

    /*index == 9: help */
    index2member[index++] = NULL;
    index2swarmkind[swarm_index++] = SK_None;

    /*index == 10: version */
    index2member[index++] = NULL;
    index2swarmkind[swarm_index++] = SK_None;

    /*index == 11: output */
    index2member[index++] = NULL;
    index2swarmkind[swarm_index++] = SK_None;
    outputfilename[0] = '\0';

    index2member[index++] = &cmdline.noternaryexprs;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.nofunctioncallexprs;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.nooperationexprs;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.notestexprs;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.noassignmentexprs;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.noifs;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.nofors;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.noassignments;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.noptrassignments;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.nofunctioncalls;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.noreturns;
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.nointegers;
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.nofloats;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.nopointers;
    index2swarmkind[swarm_index++] = SK_Flipcoin;

    index2member[index++] = &cmdline.swarm;
    index2swarmkind[swarm_index++] = SK_None;

    index2member[index++] = &cmdline.swarmreplay;
    index2swarmkind[swarm_index++] = SK_None;

    assert((index == opt_length) && "bad index!");
    assert((swarm_index == opt_length) && "bad swarm_index!");
}

void handleSwarm(void)
{
    if (cmdline.swarm) {
        enableSwarmOptions();
    }
    else if (cmdline.swarmreplay) {
        /* 
         * If we want to re-generate the same code as one that was generated
         * with --swarm, we have to run fakeSwarmOptions to run enable rand().
         */
        fakeSwarmOptions();
    }
}

static void handleOneCommandlineOption(int index)
{
    char tmp_str[512];
    if (index2member[index] == NULL)
        return;
    if (!strcmp(longopt[index].name, "swarm"))
        return;
    assert((strlen(cmdstr) <= MAX_CMDSTR_LEN) && "Please increase MAX_CMDSTR_LEN!");

    tmp_str[0] = '\0';
    unsigned val = *index2member[index];
    if (index >= 0 && index <= 7) {
      sprintf(tmp_str, "--%s %u ", longopt[index].name, *index2member[index]);
      strcat(cmdstr, tmp_str);
    }
    else if (val) {
      sprintf(tmp_str, "--%s ", longopt[index].name);
      strcat(cmdstr, tmp_str);
    }
}

void checkCommandlineOptions(void)
{
    assert((!cmdline.nointegers || !cmdline.nofloats) &&
           "Can't disallow both integers and floats!");
}

const char *getCommandlineString(void)
{
    int index;
    cmdstr[0] = '\0';
    if (cmdline.swarm)
        strcat(cmdstr, "--swarm-replay ");
    /* skip seed */
    for (index = 0; index < opt_length; index++)
        handleOneCommandlineOption(index);
    return cmdstr;
}

void openOutputStream(void)
{
    if (outputfilename[0] == '\0') {
      outputstream = stdout;
      return;
    }
    outputstream = fopen(outputfilename, "w+");
    assert(outputstream && "Can't open output file!");
}

void closeOutputStream(void)
{
    if (outputfilename[0] == '\0')
      return;
    fclose(outputstream);
}

void processCommandline(int argc, char **argv)
{
    int c, index = 0;

    while((c = getopt_long(argc, argv, "hv", longopt, &index)) != -1)
    {
        switch(c)
        {
            case 0:
                setopt(index);
                break;

            case 'h':
                printHelp();
                break;

            case 'v':
                printVersion();
                break;

            default:
                break;
        }
    }
}
