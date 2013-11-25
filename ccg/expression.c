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

static const ExpressionKind exprarray[10] = {_ternaryexpr, _functioncallexpr, _functioncallexpr, _operationexpr, _operationexpr, _operationexpr, _testexpr, _testexpr, _assignmentexpr, _assignmentexpr};

char const * const testop2str[_testopmax] = {"==", "<=", ">=", "<", ">", "!="};
char const * const arithop2str[_arithopmax] = {"+", "-", "/", "%", "*"};
char const * const bitwiseop2str[_bitwiseopmax] = {"&", "|", "^"};
char const * const logicalop2str[_logicalopmax] = {"&&", "||"};
char const * const assignop2str[_assignopmax] = {"+=", "-=", "/=", "%=", "*=", "&=", "|=", "^=", "="};

void buildOperand(Expression*, Context*, unsigned);
void buildTernary(Expression *expr, Context*, unsigned);
void buildOperation(Expression*, Context*, unsigned);
void buildTest(Expression*, Context*, unsigned);
void buildAssignment(Expression*, Context*, unsigned);
void buildFunctionCall(Expression*, Context*, unsigned);

static void (*buildfunctions[_expressionkindmax])(Expression*, Context*, unsigned) =
{
    [_operandexpr] = buildOperand,
    [_ternaryexpr] = buildTernary,
    [_operationexpr] = buildOperation,
    [_testexpr] = buildTest,
    [_assignmentexpr] = buildAssignment,
    [_functioncallexpr] = buildFunctionCall
};

void addExpressionToList(Expression *expression, ExpressionList **list)
{
    if(!*list)
    {
        *list = xmalloc(sizeof(**list));
        (*list)->expression = expression;
        (*list)->next = NULL;
    }

    else
    {
        ExpressionList *e;

        for(e = *list; e->next; e = e->next);
        e->next = xmalloc(sizeof(*e->next));
        e->next->expression = expression;
        e->next->next = NULL;
    }
}

#define EXPRESSION_IS_INVALID(t) (((t) == _functioncallexpr && program.numfunctions >= cmdline.max_functions)\
				  || ((t) == _ternaryexpr && cmdline.noternaryexprs)\
				  || ((t) == _functioncallexpr && cmdline.nofunctioncallexprs)\
				  || ((t) == _operationexpr && cmdline.nooperationexprs)\
				  || ((t) == _testexpr && cmdline.notestexprs)\
				  || ((t) == _assignmentexpr && cmdline.noassignmentexprs))

Expression *makeExpression(Context *context, unsigned nesting)
{
    Expression *expression = xmalloc(sizeof(*expression));
    expression->type = _exprtypemax;

    if(nesting >= cmdline.max_expression_nesting)
        expression->kind = _operandexpr;
    else
    {
        unsigned int ecount = 0;
        do {
            expression->kind = exprarray[rand() % (sizeof(exprarray) / sizeof(*exprarray))];
	    ecount++;
	    assert(ecount < 10000);
	} while(EXPRESSION_IS_INVALID(expression->kind));
    }

    (buildfunctions[expression->kind])(expression, context, nesting + 1);

    return expression;
}

ExpressionType makeExpressionType(bool disallow_float)
{
    if (disallow_float) {
        if (cmdline.nointegers)
            assert(0 && "Can't get return type!");
        else
            return _integer_expr;
    }

    if (!cmdline.nointegers && !cmdline.nofloats) {
        if (rand() % 4)
            return _integer_expr;
        else
            return _float_expr;
    }
    else if (!cmdline.nointegers) {
        return _integer_expr;
    }
    else if (!cmdline.nofloats) {
        return _float_expr;
    }
    else {
        assert(0 && "Can't make ExpressionType!");
    }
}

void buildOperand(Expression *expression, Context *context, unsigned nesting)
{
    expression->expr.operand = selectOperand(context);
    expression->type = (expression->expr.operand)->type;
}

void buildTest(Expression *expression, Context *context, unsigned nesting)
{
    struct TestExpression *te = xmalloc(sizeof(*te));

    te->op = rand() % _testopmax;
    te->lefthand = makeExpression(context, nesting + 1), te->righthand = makeExpression(context, nesting + 1);

    expression->expr.testexpr = te;
    expression->type = _integer_expr;
}

static ExpressionType getBinaryExpressionType(Expression *left, Expression *right)
{
    if (left) {
        assert((left->type != _exprtypemax) && "Invalid type of left expr!");
        if (left->type == _float_expr)
            return _float_expr;    
    }

    assert(right && "NULL right expr!");
    assert((right->type != _exprtypemax) && "Invalid type of right expr!");
    return (right->type == _float_expr) ? _float_expr : _integer_expr;
}

void buildTernary(Expression *expression, Context *context, unsigned nesting)
{
    struct TernaryExpression *te = xmalloc(sizeof(*te));

    bool build_trupath = rand() % 4;
    bool old_disallow_float = context->disallow_float;
    if (build_trupath) {
        context->disallow_float = false;
        te->test = makeExpression(context, nesting + 1);
        context->disallow_float = old_disallow_float;
    }
    else {
        te->test = makeExpression(context, nesting + 1);
        te->truepath = NULL;
    }

    if (build_trupath) {
        te->truepath = makeExpression(context, nesting + 1);
    }
    te->falsepath = makeExpression(context, nesting + 1);
    expression->expr.ternexpr = te;
    expression->type = getBinaryExpressionType(te->truepath, te->falsepath);
}

void buildOperation(Expression *expression, Context *context, unsigned nesting)
{
    struct OperationExpression *oe = xmalloc(sizeof(*oe));

    bool old_disallow_float = context->disallow_float;
    oe->kind = rand() % _operationkindmax;

    if(oe->kind == _arithmetic) {
        oe->operator.arithop = rand() % _arithopmax;
        if (oe->operator.arithop == _mod)
            context->disallow_float = true;
    }
    else if(oe->kind == _bitwise) {
        oe->operator.bitwiseop = rand() % _bitwiseopmax;
        context->disallow_float = true;
    }
    else {
        oe->operator.logicalop = rand() % _logicalopmax;
    }

    oe->lefthand = makeExpression(context, nesting + 1), oe->righthand = makeExpression(context, nesting + 1);
    context->disallow_float = old_disallow_float;
    expression->expr.opexpr = oe;
    expression->type = getBinaryExpressionType(oe->lefthand, oe->righthand);
}

#define ASSIGNMENT_OP_IS_INVALID(oprtr, left, right) (((left->type == _float || (IS_FLOATING_POINT_VARIABLE(right))) && (oprtr == _assignmod)))

void buildAssignment(Expression *expression, Context *context, unsigned nesting)
{
    struct AssignmentExpression *ae = xmalloc(sizeof(*ae));
    bool old_disallow_float = context->disallow_float;

    ae->op = rand() % _assignopmax;
    if ((ae->op == _assignmod) || (ae->op == _assignand) ||
            (ae->op == _assignor) || (ae->op == _assignxor)) {
        context->disallow_float = true;
    }

    ae->lvalue = selectVariable(context, _randomvartype);
    ae->rvalue = makeExpression(context, nesting + 1);

    context->disallow_float = old_disallow_float;
    expression->expr.assignexpr= ae;
    expression->type = IS_INTEGER_VARIABLE(ae->lvalue) ? _integer_expr : _float_expr;
}

void buildFunctionCall(Expression *expression, Context *context, unsigned nesting)
{
    struct FunctionCallExpression *fce = xmalloc(sizeof(*fce));
    VariableList *v;

    fce->paramlist = NULL;
    fce->function = makeFunction(true, context->disallow_float);

    foreach(v, fce->function->paramlist)
        addExpressionToList(makeExpression(context, nesting + 1), (ExpressionList**) &fce->paramlist);

    expression->expr.funccallexpr = fce;
    expression->type = fce->function->returntypekind;
}

static void printOperand(Operand *op)
{
    if(op->kind == _variable)
        fputs(USABLE_ID(op->op.variable), stdout);
    else
        printConstant(op->op.constant);
}

void printTest(struct TestExpression *te)
{
    putchar('(');
    printExpression(te->lefthand);
    printf(" %s ", testop2str[te->op]);
    printExpression(te->righthand);
    putchar(')');
}

static void printTernary(struct TernaryExpression *te)
{
    putchar('(');
    printExpression(te->test);
    fputs(" ? ", stdout);

    if(te->truepath)
        printExpression(te->truepath);

    fputs(" : ", stdout);
    printExpression(te->falsepath);
    putchar(')');
}

static void printOperation(struct OperationExpression *oe)
{
    putchar('(');
    printExpression(oe->lefthand);
    printf(" %s ", oe->kind == _arithmetic ? arithop2str[oe->operator.arithop] : (oe->kind == _bitwise ? bitwiseop2str[oe->operator.bitwiseop] : logicalop2str[oe->operator.logicalop]));
    printExpression(oe->righthand);
    putchar(')');
}

static void printAssignment(struct AssignmentExpression *ae)
{
    putchar('(');
    printf("%s %s ", USABLE_ID(ae->lvalue), assignop2str[ae->op]);
    printExpression(ae->rvalue);
    putchar(')');
}

static void printFunctionCall(struct FunctionCallExpression *fce)
{
    ExpressionList *e;

    printf("%s(", fce->function->name);

    foreach(e, fce->paramlist)
    {
        printExpression(e->expression);

        if(e->next)
            fputs(", ", stdout);
    }

    putchar(')');
}

void printExpression(Expression *expression)
{
    assert(expression && "NULL expression!");
    assert((expression->type != _exprtypemax) && "Invalid expression type!");

    switch(expression->kind)
    {
        case _operandexpr: printOperand(expression->expr.operand); break;
        case _ternaryexpr: printTernary(expression->expr.ternexpr); break;
        case _operationexpr: printOperation(expression->expr.opexpr); break;
        case _testexpr: printTest(expression->expr.testexpr); break;
        case _assignmentexpr: printAssignment(expression->expr.assignexpr); break;
        case _functioncallexpr: printFunctionCall(expression->expr.funccallexpr); break;
        default: die("are you... mad ?");
    }
}
