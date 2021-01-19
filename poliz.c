#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <poliz.h>

long long quotient(long long a, long long b) {
    long long quot = 0;
    
    if (b > 0 && a >= 0) {
        quot = a / b;
    }
    else if (a >= 0) {
        quot = -(a / (-b));
    }
    else if (b > 0) {
        quot = -((-a) / b);
        if ((-a) % b != 0) {
            quot -= 1;
        }
    }
    else {
        quot = (-a) / (-b);
        if ((-a) % (-b) != 0) {
            quot += 1;
        }
    }
    return quot;
}

struct PolizState {
    long *data;
    size_t size;
    size_t capacity;
    int err;
};

int add(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (state->size >= state->capacity - 1) {
        size_t new_capacity = state->capacity * 2;
        if (!new_capacity) new_capacity = 16;
        size_t new_size = new_capacity * sizeof(*state->data);
        
        long *new_data = realloc(state->data, new_size);
        if (new_data == NULL) {
            free(state->data);
            state->err = PE_OUT_OF_MEMORY;
            return -state->err;
        }
        state->capacity = new_capacity;
        state->data = new_data;
    }
    state->data[state->size] = iextra;
    ++state->size;
    state->err = PE_OK;
    return PE_OK;
}

int plus(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (state->size < 2) {
        state->err = PE_STACK_UNDERFLOW;
        return -state->err;
    }
    long long sum = (long long)state->data[state->size - 1] + 
                    (long long)state->data[state->size - 2];
    if ((int32_t)sum != sum) {
        state->err = PE_INT_OVERFLOW;
        return -state->err;
    }
    state->size -= 2;
    return add(state, sum);
}

int minus(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (state->size < 2) {
        state->err = PE_STACK_UNDERFLOW;
        return -state->err;
    }
    long long dif = (long long)state->data[state->size - 2] - 
                    (long long)state->data[state->size - 1];
    if ((int32_t)dif != dif) {
        state->err = PE_INT_OVERFLOW;
        return -state->err;
    }
    state->size -= 2;
    return add(state, dif);
}

int mult(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (state->size < 2) {
        state->err = PE_STACK_UNDERFLOW;
        return -state->err;
    }
    long long prod = 0;
    if (__builtin_mul_overflow(state->data[state->size - 1], 
        state->data[state->size - 2], &prod) || (int32_t)prod != prod) {
        state->err = PE_INT_OVERFLOW;
        return -state->err;
    }
    state->size -= 2;
    return add(state, prod);
}

int dev(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (state->size < 2) {
        state->err = PE_STACK_UNDERFLOW;
        return -state->err;
    }
    if (state->data[state->size - 1] == 0) {
        state->err = PE_DIVISION_BY_ZERO;
        return -state->err;
    }
    long long quot = quotient(state->data[state->size - 2], 
                              state->data[state->size - 1]);
    if ((int32_t)quot != quot) {
        state->err = PE_INT_OVERFLOW;
        return -state->err;
    }
    state->size -= 2;
    return add(state, quot);
}

int mod(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (state->size < 2) {
        state->err = PE_STACK_UNDERFLOW;
        return -state->err;
    }
    if (state->data[state->size - 1] == 0) {
        state->err = PE_DIVISION_BY_ZERO;
        return -state->err;
    }
    long long a = state->data[state->size - 2], 
              b = state->data[state->size - 1];
    long long mod = 0;
    
    if (a < 0) {
        long long quot = quotient(a, b);
        mod = a - b * quot;
    }
    else {
        if (b < 0) {
            b = -b;
        }
        mod = a % b;
    }
    if ((int32_t)mod != mod) {
        state->err = PE_INT_OVERFLOW;
        return -state->err;
    }
    state->size -= 2;
    return add(state, mod);
}

int neg(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (state->size < 1) {
        state->err = PE_STACK_UNDERFLOW;
        return -state->err;
    }
    long long prod = 0;
    if (__builtin_mul_overflow(state->data[state->size - 1], 
        -1, &prod) || (int32_t)prod != prod) {
        state->err = PE_INT_OVERFLOW;
        return -state->err;
    }
    state->size -= 1;
    return add(state, prod);
}

int read(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    long long inp = 0;
    if (scanf("%lld", &inp) != 1) {
        state->err = PE_READ_FAILED;
        return -state->err;
    }
    if ((int32_t)inp != inp) {
        state->err = PE_INT_OVERFLOW;
        return -state->err;
    }
    return add(state, inp);
}

int del(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (state->size < 1) {
        state->err = PE_STACK_UNDERFLOW;
        return -state->err;
    }
    --state->size;
    state->err = PE_OK;
    return PE_OK;
}

int write(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (state->size < 1) {
        state->err = PE_STACK_UNDERFLOW;
        return -state->err;
    }
    printf("%ld", state->data[state->size - 1]);
    return del(state, iextra);
}

int endstr(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    printf("\n");
    state->err = PE_OK;
    return PE_OK;
}

int totop(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (iextra >= state->size || iextra < 0) {
        state->err = PE_INVALID_INDEX;
        return -state->err;
    }
    return add(state, state->data[state->size - iextra - 1]);
}

int swap(struct PolizState *state, int iextra) {
    if (state->err) return -state->err;
    
    if (iextra == 0) {
        state->err = PE_OK;
        return PE_OK;
    }
    if (iextra >= state->size || iextra < 0) {
        state->err = PE_INVALID_INDEX;
        return -state->err;
    }
    long long tmp = state->data[state->size - 1];
    
    state->data[state->size - 1] = state->data[state->size - iextra - 1];
    state->data[state->size - iextra - 1] = tmp;
    state->err = PE_OK;
    return PE_OK;
}

struct PolizItem *poliz_compile(const char *str) {
    struct PolizItem *data = malloc(32 * sizeof(*data));
    if (data == NULL) {
        return NULL;
    }
    data->handler = NULL;
    data->iextra = 0;
    
    size_t size = 0;
    size_t capacity = 32;
    
    char buf[64];
    int pos = 0;
    while (*str != '\0') {
        if (isspace(*str)) {
            pos = 0;
        }
        else {
            buf[pos] = *str;
            ++pos;
            if ((isspace(*(str + 1)) || *(str + 1) == '\0')) {
                buf[pos] = '\0';
                
                if (size >= capacity - 2) {
                    size_t new_capacity = capacity * 2;
                    if (!new_capacity) new_capacity = 16;
                    size_t new_size = new_capacity * sizeof(struct PolizItem);
                    struct PolizItem *new_data = realloc(data, new_size);
                    if (new_data == NULL) {
                        free(data);
                        return NULL;
                    }
                    capacity = new_capacity;
                    data = new_data;
                    data[size].iextra = 0;
                }
                if (!strcmp(buf, "+")) {
                    data[size].handler = &plus;
                }
                else if (!strcmp(buf, "-")) {
                    data[size].handler = &minus;
                }
                else if (!strcmp(buf, "*")) {
                    data[size].handler = &mult;
                }
                else if (!strcmp(buf, "/")) {
                    data[size].handler = &dev;
                }
                else if (!strcmp(buf, "%")) {
                    data[size].handler = &mod;
                }
                else if (!strcmp(buf, "#")) {
                    data[size].handler = &neg;
                }
                else if (!strcmp(buf, "r")) {
                    data[size].handler = &read;
                }
                else if (!strcmp(buf, "w")) {
                    data[size].handler = &write;
                }
                else if (!strcmp(buf, "n")) {
                    data[size].handler = &endstr;
                }
                else if (!strcmp(buf, ";")) {
                    data[size].handler = &del;
                }
                else if (buf[0] == 'd')
                {
                    data[size].handler = &totop;
                    buf[0] = ' ';
                    char *eptr;
                    long x = strtol(buf, &eptr, 10);
                    if (buf[1]) {
                        data[size].iextra = x;
                    }
                    else {
                        data[size].iextra = 0;
                    }
                }
                else if (buf[0] == 's') {
                    data[size].handler = &swap;
                    buf[0] = ' ';
                    char *eptr;
                    long x = strtol(buf, &eptr, 10);
                    if (buf[1]) {
                        data[size].iextra = x;
                    }
                    else {
                        data[size].iextra = 1;
                    }
                }
                else {
                    char *eptr;
                    long x = strtol(buf, &eptr, 10);
                    data[size].handler = &add;
                    data[size].iextra = x;
                }
                ++size;
            }
        }
        ++str;
    }
    data[size].handler = NULL;
    data[size].iextra = 0;
    
    return data;
}

struct PolizState *poliz_new_state(void) {
    struct PolizState *new_state = malloc(sizeof(*new_state));
    if (new_state == NULL) {
        return NULL;
    }
    new_state->data = malloc(sizeof(long));
    if (new_state->data == NULL) {
        return NULL;
    }
    new_state->size = 0;
    new_state->capacity = 1;
    new_state->err = PE_OK;
    
    return new_state;
}
void poliz_free_state(struct PolizState *state) {
    free(state->data);
    free(state);
}
int poliz_last_error(struct PolizState *state) {
    return state->err;
}
