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

#define IS_INVALID ((type == _integer && cmdline.nointegers)\
                    || (type == _float && cmdline.nofloats)\
                    || (type == _pointer && cmdline.nopointers))

Variable *makeVariable(Context *context, VariableType type)

{
    Variable *ret = xmalloc(sizeof(Variable));

    if(type != _randomvartype) {
        ret->type = type;
    }
    else
    {
        unsigned int tcount = 0;
        do {
            type = rand() % _vartypemax;
            tcount++;
            assert((tcount < 10000) && "Can't find valid type after 10,000 tries");
        } while(IS_INVALID);

        ret->type = type;
    }

    if(ret->type == _integer)
        makeInteger(ret, context);
    else if(ret->type == _float)
        makeFloat(ret, context);
    else
        makePointer(ret, context);
    return ret;
}

void printVariableDecl(Variable *var)
{
    if(var->type == _integer)
        printIntegerDecl(var);
    else if (var->type == _float)
        printFloatDecl(var);
    else
        printPointerDecl(var);
}

void printVariableUltimateType(Variable *var)
{
    var = ultimateVariable(var);
    if(var->type == _integer) {
        fprintf(outputstream, "%s", inttype2str[var->intvar.type]);
    }
    else if(var->type == _float) {
        fprintf(outputstream, "%s", floattype2str[var->floatvar.type]);
    }
    else{
        assert(0 && "Invalid variable type!");
    }
}

void printVariableType(Variable *var)
{
    printVariableUltimateType(var);

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

static size_t getNumberOfPointersForType(Context *context, VariableType type)
{
    size_t n = 0;
    Variable *ultimatevar = NULL;
    VariableList *v;

    foreach(v, context->scope)
    {
        if (v->variable->type != _pointer)
            continue;
        ultimatevar = ultimateVariable(v->variable);
        if(ultimatevar->type == type)
            n++;
    }
    return n;
}

bool hasIntegerVariables(Context *context)
{
    VariableList *v;

    foreach(v, context->scope)
    {
        if (v->variable->type == _integer) {
            return true;
        }
        else if (v->variable->type == _pointer) {
            Variable *ultimatevar = ultimateVariable(v->variable);
            if(ultimatevar->type == _integer)
                return true;
        }
    }
    return false;
}

Variable *selectVariable(Context *context, VariableType type)
{
    VariableList *v;
    size_t n, t = 0;

    if(type == _randomvartype) {
        if(context->disallow_float) {
            assert(!cmdline.nointegers && "can't select integer variable!");
            size_t np = getNumberOfPointersForType(context, _float);
            assert((context->nvars >= (context->nfloats + np)) && "Bad var counts!");
            n = context->nvars - context->nfloats - np;
            assert(n && "Can't find valid integer!");
            n = rand() % n;
        }
        else {
            n = rand() % context->nvars;
        }
    }
    else if(type == _integer) {
        n = rand() % context->nintegers;
    }
    else if(type == _float) {
        assert(context->disallow_float && "can't have a float type here!");
        n = rand() % context->nfloats;
    }
    else {
        n = rand() % (context->nvars - context->nintegers - context->nfloats);
    }

    foreach(v, context->scope)
    {
        if (context->disallow_float) {
            Variable *ultimatevar = ultimateVariable(v->variable);
            if (ultimatevar->type == _float)
                continue;
        }
        if(v->variable->type == type || type == _randomvartype) {
            if(t++ == n)
                return v->variable;
        }
    }

    die("selectVariable: unreachable!");
    return NULL;
}

VariableType makeVariableNonPointerType(void)
{
    if (!cmdline.nointegers && !cmdline.nofloats) {
        if(rand() % 4) 
            return _integer;
        else
            return _float;
    }
    else if (cmdline.nofloats) {
        return _integer;
    }
    else if (cmdline.nointegers) {
        return _float;
    }
    else {
        assert(0 && "can't happen (disallowed by checkCommandlineOptions!");
    }
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
        VariableType t = makeVariableNonPointerType();
        if (t == _integer)
            program.numglobalintvars++;
        else if (t == _float)
            program.numglobalfloatvars++;
        else
            assert(0 && "Invalid variable type!");
        
        addVariableToList(makeVariable(c, t), &program.globalvars);
        c->nvars++;
    }

    free(c);
}
