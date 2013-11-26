
#include "ccg.h"

static char const * const floattype2varid[_floattypemax] = {"f", "d", "ld"};
char const * const floattype2str[_floattypemax] = {"float", "double", "long double"};

static char *makeFloatName(FloatType type, Context *context)
{
    char buffer[32] = {0}, *ret;

    sprintf(buffer, "%s_%u", floattype2varid[type], context->nvars);
    ret = xmalloc(strlen(buffer) + 1);
    strcpy(ret, buffer);

    return ret;
}

void makeFloat(Variable *var, Context *context)
{
    var->floatvar.type = rand() % _floattypemax;
    var->floatvar.initializer = makeFloatConstant(var->floatvar.type);
    var->name = makeFloatName(var->floatvar.type, context);
}

bool compatibleFloats(FloatType t1, FloatType t2)
{
    if (t1 == _f)
        return true;
    if (t1 == _d)
        return (t2 == _d || t2 == _ld);
    if (t1 == _ld)
        return (t2 == _ld);
    return false;
}

void printFloatDecl(Variable *var)
{
    fprintf(outputstream, "%s %s = %s;\n", floattype2str[var->floatvar.type], var->name, var->floatvar.initializer->value);
}

