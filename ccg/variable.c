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

void addVariableToList(Variable *variable, VariableList **list)
{
    if(!*list)
    {
        *list = xmalloc(sizeof(**list));
        (*list)->variable = variable;
        (*list)->next = NULL;
    }

    else
    {
        VariableList *v;

        for(v = *list; v->next; v = v->next);
        v->next = xmalloc(sizeof(*v->next));
        v->next->variable = variable;
        v->next->next = NULL;
    }
}

void freeVariableList(VariableList *list)
{
    VariableList *v, *next;

    for(v = list; v; v = next)
    {
        next = v->next;
        free(v);
    }
}

static bool isValidType(VariableType type)
{
    if (type == _pointer && cmdline.nopointers)
        return false;
    return true;
}

Variable *makeVariable(Context *context, VariableType type)
{
    Variable *ret = xmalloc(sizeof(Variable));

    if(type != _randomvartype)
    {
        ret->type = type;

    }
    else
    {
        unsigned int tcount = 0;
        do {
            type = rand() % _vartypemax;
            tcount++;
            assert((tcount < 10000) && "Can't find valid type after 10,000 tries");
        } while(!isValidType(type));
        ret->type = type;
    }

    if(ret->type == _integer)
        makeInteger(ret, context);
    else
        makePointer(ret, context);
    return ret;
}

void printVariableDecl(Variable *var)
{
    if(var->type == _integer)
        printIntegerDecl(var);
    else
        printPointerDecl(var);
}

void printVariableType(Variable *var)
{
    fprintf(outputstream, "%s", inttype2str[ultimateType(var)]);

    if(var->type == _pointer)
    {
        size_t n = pointerDepth(var);

        for(size_t i = 0; i < n; ++i)
            putc('*', outputstream);
    }
}

void copyVariableList(VariableList *src, VariableList **dest)
{
    VariableList *v;

    foreach(v, src)
        addVariableToList(v->variable, dest);
}

Variable *selectVariable(Context *context, VariableType type)
{
    VariableList *v;
    size_t n, t = 0;

    if(type == _randomvartype)
        n = rand() % context->nvars;
    else
        n = rand() % (type == _integer ? context->nintegers : (context->nvars - context->nintegers));

    foreach(v, context->scope)
    {
        if(v->variable->type == type || type == _randomvartype)
            if(t++ == n)
                return v->variable;
    }

    die("unreachable");
    return NULL;
}

void makeGlobalVariables(void)
{
    size_t i;
    Context *c = xmalloc(sizeof(*c));

    c->scope = program.globalvars;
    c->nvars = 0;

    program.numglobalvars = rand() % 10 + 1;

    for(i = 0; i < program.numglobalvars; ++i)
    {
        addVariableToList(makeVariable(c, _integer), &program.globalvars);
        c->nvars++;
    }

    free(c);
}
