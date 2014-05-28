#include "mpc.h"

static char input[2048];

enum { LVAL_NUM, LVAL_ERR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };
typedef union {
    long num;
    int err;
}tval;
typedef struct {
    int type;
    tval val;
}lval;

lval lval_num(long x){
    lval ret;
    ret.type = LVAL_NUM;
    ret.val.num = x;
    return ret;
}

lval lval_err(int err){
    lval ret;
    ret.type = LVAL_ERR;
    ret.val.err = err;
    return ret;
}

lval eval_op(lval x, char *op, lval y){
    if (strcmp(op, "+") == 0) { return lval_num(x.val.num + y.val.num); }
    if (strcmp(op, "-") == 0) { return lval_num(x.val.num - y.val.num); }
    if (strcmp(op, "*") == 0) { return lval_num(x.val.num * y.val.num); }
    if (strcmp(op, "/") == 0) {
        if (y.val.num == 0) return lval_err(LERR_DIV_ZERO);
        else return lval_num(x.val.num / y.val.num);
    }
    if (strcmp(op, "%") == 0) { return lval_num(x.val.num % y.val.num); }
    if (strcmp(op, "^") == 0) { return lval_num(x.val.num ^ y.val.num); }

    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t){
    if (strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    char *op = t->children[1]->contents;

    lval x = eval(t->children[2]);

    int i = 3;
    int single = 1;
    while (strstr(t->children[i]->tag, "expr")){
        x = eval_op(x, op, eval(t->children[i]));
        ++i;
        single = 0;
    }
    if (single && strcmp(op, "-") == 0 && x.type == LVAL_NUM)
        x.val.num = - x.val.num;
    return x;
}

int lval_print(lval v){
    switch (v.type){
        case LVAL_NUM:
            printf("%li\n", v.val.num);
            break;
        case LVAL_ERR:
            if (LERR_DIV_ZERO == v.val.err) printf("Error: Division By Zero!\n");
            if (LERR_BAD_OP == v.val.err) printf("Error: Invalid Operator!\n");
            if (LERR_BAD_NUM == v.val.err) printf("Error: Invalid Number!\n");
            break;
    }
    return 0;
}

int main(){
    mpc_parser_t* Number    = mpc_new("number");
    mpc_parser_t* Operator  = mpc_new("operator");
    mpc_parser_t* Expr      = mpc_new("expr");
    mpc_parser_t* Lispy     = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
            "                                           \
            number      : /-?[0-9.]+/ ;              \
            operator    : '+' | '-' | '*' | '/' | '%' | '^';   \
            expr        : <number> | '(' <operator> <expr>+ ')' ;   \
            lispy       : /^/ <operator> <expr>+ /$/ ;              \
            ",
            Number, Operator, Expr, Lispy);

    while(1){
        fgets(input, 2048, stdin);
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            lval result = eval(r.output);
            lval_print(result);
            //mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        }
        else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);

    return 0;
}
