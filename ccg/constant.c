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

#include <ctype.h>
#include "ccg.h"

static const char *hexdigits = "0123456789ABCDEF";
static const char * const floatpositivebase[_floattypemax] = {
    "3.40282347", "1.79769313486231571", "1.79769313486231571"
};
static const char * const floatnegativebase[_floattypemax] = {
    "1.175494351", "2.22507385850720138", "2.22507385850720138"
};
static const char * const floatexponent[_floattypemax] = {
    "38", "308", "308"
};

char *makeHexadecimalValue(unsigned digitNumber)
{
    size_t i;
    char *ret = xcalloc(digitNumber + 1, 1);

    for(i = 0; i < digitNumber; ++i)
        ret[i] = hexdigits[rand() % 16];

    return ret;
}

/* For now, the generated integers are strictly positive */
Constant *makeIntegerConstant(unsigned bits)
{
    Constant *ret = xmalloc(sizeof(*ret));
    char *hexvalue = ((rand() % 3) ? makeHexadecimalValue(bits / 4) : "0"); /* 30% of the time, a zero is generated */

    ret->value   = xcalloc(4 + strlen(hexvalue), 1);
    ret->bitness = bits;

    sprintf(ret->value, "0x%s", hexvalue);

    if(strlen(hexvalue) > 1)
        free(hexvalue);

    return ret;
}

static char *makeRandomStringOfDigits(const char *magicstr) 
{
    size_t i;
    size_t length = strlen(magicstr);
    char *ret = xcalloc(length + 1, 1);
    
    for(i = 0; i < length; ++i) {
        char c = magicstr[i];
        if (c == '.') {
            ret[i] = '.';
            continue;
        }

        assert(isdigit(c) && "Invalid Digit String!");
        int digit = c - '0' + 1;
        assert((digit <= 10) && "Invalid Digit!");
        ret[i] = rand() % digit + '0';
    }
    return ret;
}

static Constant *makeZeroFloatConstant(void)
{
    Constant *ret = xmalloc(sizeof(*ret));
    bool positive = rand() % 2;
    char *sign = positive ? "+" : "-";
    ret->value = xcalloc(5, 1); /* [+-]0.0\0 */
    sprintf(ret->value, "%s0.0", sign);
    return ret;
}

static Constant *makeSmallFloatConstant(void)
{
    Constant *ret = xmalloc(sizeof(*ret));
    bool positive = rand() % 2;
    char *sign = positive ? "+" : "-";

    const char *smallbasemagic = "99.99";
    ret->value = xcalloc(strlen(smallbasemagic) + 2, 1);
    char *base = makeRandomStringOfDigits(smallbasemagic);
    sprintf(ret->value, "%s%s", sign, base);
    free(base);
    return ret;
}

static Constant *makeFullFloatConstant(FloatType type)
{
    Constant *ret = xmalloc(sizeof(*ret));
    bool positive = rand() % 2;
    char *sign = positive ? "+" : "-";

    const char *basemagic = positive ? floatpositivebase[type] : 
                                  floatnegativebase[type];
    const char *expmagic = floatexponent[type];
    ret->value = xcalloc(strlen(basemagic) + strlen(expmagic) + 3, 1);
    char *base = makeRandomStringOfDigits(basemagic);
    char *exp = makeRandomStringOfDigits(expmagic);
    sprintf(ret->value, "%se%s%s", base, sign, exp);
    free(base);
    free(exp);
    
    return ret;
}

Constant *makeFloatConstant(FloatType type)
{
    if (rand() % 5) {
        if (rand() % 2)
            return makeSmallFloatConstant();
        else 
            return makeFullFloatConstant(type);
    }
    else {
        return makeZeroFloatConstant();
    }
}

Constant *makeRandomFloatConstant(void)
{
   FloatType type = rand() % _floattypemax;
   return makeFloatConstant(type);
}

void printConstant(Constant *constant)
{
    fputs(constant->value, outputstream);
    /* And... we are done. */
}
