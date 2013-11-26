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

#include "ccg.h"

Context context;

void addFunctionToList(Function *function, FunctionList **list)
{
    if(!*list)
    {
        *list = xmalloc(sizeof(**list));
        (*list)->function = function;
        (*list)->next = NULL;
    }

    else
    {
        FunctionList *v;

        for(v = *list; v->next; v = v->next);
        v->next = xmalloc(sizeof(*v->next));
        v->next->function = function;
        v->next->next = NULL;
    }
}

char *makeFunctionName(void)
{
    char *name;
    char buff[8];

    sprintf(buff, "%zu", program.numfunctions);
    name = malloc(strlen(buff) + 6);
    sprintf(name, "func_%zu", program.numfunctions);

    return name;
}

static void makeReturnType(Function *func, bool disallow_float)
{
    func->returntypekind = makeExpressionType(disallow_float);
    if (func->returntypekind == _float_expr)
            func->returntype.floattype = rand() % _floattypemax;
    else
            func->returntype.inttype = rand() % _inttypemax;
}

/* At this point, only global variables are available */
Function *makeFunction(bool params, bool disallow_float)
{
    short i, numparams = 0;
    Function *ret = xmalloc(sizeof(*ret));

    Context *context = xmalloc(sizeof(*context));
    context->scope = params ? NULL : program.globalvars;
    context->currfunc = ret;

    ret->paramlist = NULL;
    makeReturnType(ret, disallow_float);
    ret->name = makeFunctionName();
    ret->numlabels = 0;
    ret->labels = NULL;

    context->nvars = program.numglobalvars;
    context->nintegers = program.numglobalintvars;
    context->nfloats = program.numglobalfloatvars;
    context->disallow_float = false;
    numparams = (params ? rand() % (cmdline.max_function_parameters + 1) : 0);

    program.numfunctions++;

    if(params)
    {
        copyVariableList(program.globalvars, &context->scope);

        for(i = 0; i < numparams; ++i)
        {
            VariableType t = makeVariableNonPointerType();
            Variable *v = makeVariable(context, t);
            context->nvars++;

            if(v->type == _integer)
                context->nintegers++;
            else if(v->type == _float)
                context->nfloats++;
            else {
                assert(0 && "Invalid Variable Type!");
            }

            addVariableToList(v, &ret->paramlist);
            addVariableToList(v, &context->scope);
        }
    }

    ret->body = makeBlock(context, 0);

    addFunctionToList(ret, &program.functions);

    if(params)
        freeVariableList(context->scope);

    free(context);

    return ret;
}

void printFunctionPrototype(Function *function)
{
    VariableList *v;

    if (function->returntypekind == _integer_expr) {
        fprintf(outputstream, "%s %s(", inttype2str[function->returntype.inttype], function->name);
    }
    else {
        fprintf(outputstream, "%s %s(", floattype2str[function->returntype.floattype], function->name);
    }

    foreach(v, function->paramlist)
    {
        fprintf(outputstream, v == function->paramlist ? "" : ", ");
        printVariableType(v->variable);
        fprintf(outputstream, " %s", v->variable->name);
    }

    fprintf(outputstream, ")");
}

void printFunction(Function *function)
{
    printFunctionPrototype(function);
    putc('\n', outputstream);
    printBlock(function->body);
}
