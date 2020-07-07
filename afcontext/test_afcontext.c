#include <stdio.h>
#include <stdlib.h>
#include "afcontext.h"

af_context_t ctx_main, ctx_1, ctx_2;

struct arg_dummy_int {
    int arg1;
    int arg2;
};

void dummy_double(void* arg)
{
    struct arg_dummy_int input_2 = {9, 6};

    double input = *((double*)arg);
    double sum = 0.0;
    sum += input;

    printf("dummy_double: %lf\n", sum);
    jump_afcontext(&ctx_1, ctx_main, (void*)(&sum));

    jump_afcontext(&ctx_1, ctx_2, (void*)(&input_2));
}

void dummy_int(void* arg)
{
    int arg1 = ((struct arg_dummy_int*) arg)->arg1;
    int arg2 = ((struct arg_dummy_int*) arg)->arg2;

    printf("dummy_int: %d\n", arg1 + arg2);
    jump_afcontext(&ctx_2, ctx_main, NULL);
}

int main()
{
    double sum, input = 1.7;

    size_t stack_size = 1 << 15;
    void* sp_1 = malloc(stack_size);
    void* sp_2 = malloc(stack_size);
    ctx_1 = make_afcontext(sp_1, stack_size, (void (*)(void))dummy_double);
    ctx_2 = make_afcontext(sp_2, stack_size, (void (*)(void))dummy_int);

    sum = *((double*) jump_afcontext(&ctx_main, ctx_1, (void*)(&input)));
    printf("main: %lf\n", sum);

    jump_afcontext(&ctx_main, ctx_1, NULL);
    printf("main: end\n");

    free(sp_1);
    free(sp_2);
}