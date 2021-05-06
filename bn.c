#include "bn.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

int max(int a, int b){
    if (a > b) return a;
    else return b;
}

const int base = 100000000;

struct bn_s {
    int *body; // Тело большого числа
    int bodysize; // Размер массива body
    int sign; // Знак числа
};

int cut(bn *t) {
    int i = 0;
    int s = t->bodysize;
    while (t->body[s - 1 - i] == 0 && i < s - 1) ++i;
    t->bodysize = s - i;
    t->body = (int *) realloc(t->body, t->bodysize * sizeof(int));
    return 0;
}
int bn_cmp_body(bn const *left, bn const *right){
    if (left->bodysize < right->bodysize) return -1;
    else if (left->bodysize > right->bodysize) return 1;
    else {
        int s = left ->bodysize;
        for (int i = s - 1; i > -1; --i){
            if (left->body[i] > right->body[i]) return 1;
            else if (left->body[i] < right->body[i]) return -1;
        }
        return 0;
    }
}

bn *bn_new() {
    bn *r = malloc(sizeof(bn));
    if (r == NULL) return NULL;
    r->bodysize = 1;
    r->sign = 0;
    r->body = malloc(sizeof(int) * r->bodysize);
    if (r->body == NULL) {
        free(r);
        return NULL;
    }
    r->body[0] = 0;
    return r;
}
bn *bn_init(bn const *orig){
    bn *copy = bn_new();
    if (copy == NULL){
        return NULL;
    }
    copy -> bodysize = orig -> bodysize;
    copy -> sign = orig -> sign;
    copy -> body = realloc(copy -> body, sizeof(int) * copy->bodysize);
    if (copy -> body == NULL){
        free(copy -> body);
        free(copy);
        return NULL;
    }
    for (int i = 0; i < copy -> bodysize; ++i){
        copy -> body[i] = orig -> body[i];
    }
    return copy;
}

int bn_init_string(bn *t, const char *init_string) {
    if (init_string[0] == '-') t->sign = -1;
    else if (init_string[0] == '0') t->sign = 0;
    else t->sign = 1;
    int l = strlen(init_string);
    if (t->sign == -1) l -= 1;
    t->bodysize = (l + 7) / 8;
    t->body =(int *) realloc(t->body, sizeof(int) * t->bodysize);
    if (t->body == NULL) {
        free(t);
        return 1;
    }
    int x;
    l = strlen(init_string);
    for (int i = 0; i < t->bodysize; ++i) {
        x = 0;
        int index;
        for (int j = 0; j < 8; j++) {
            index = l - 8 * (i + 1) + j;
            if (index >= 0 && index < l) {
                if (init_string[index] != '-')
                    x = x * 10 + (init_string[index] - '0');
            }
        }

        t->body[i] = x;

    }
    return 0;
}


int bn_init_int(bn *t,int init_int){
    if (init_int == 0){
        t->sign = 0;
        t->bodysize = 1;
        t->body =(int *) realloc(t->body, sizeof(int) * t->bodysize);
        t->body[0] = 0;
        return 0;
    }
    if (init_int > 0) t -> sign = 1;
    else {
        t ->sign = -1;
        init_int *= -1;
    }
    int n = init_int / base;
    while (n != 0){
        t -> bodysize += 1;
        n /= base;
    }
    t->body =(int *) realloc(t->body, sizeof(int) * t->bodysize);
    if (t->body == NULL) {
        free(t);
        return 1;
    }
    for (int i = 0; i < t -> bodysize; ++i){
        t -> body[i] = init_int % base;
        init_int /= base;
    }
    return 0;
}

int bn_delete(bn *t) {
    if (t == NULL) return 0;
    if (t->body != NULL) {
        free(t->body);
    }
    free(t);
    return 0;
}

int bn_cmp(bn const *left, bn const *right){
    int res;
    bn* x = bn_sub(left, right);
    res = x->sign;
    bn_delete(x);
    return res;
}
int bn_neg(bn *t) {
    if (t == NULL) return 1;
    else{
        t ->sign *= -1;
        return 0;
    }
}
int bn_abs(bn *t){
    if (t == NULL) return 1;
    else {
        t ->sign *= t->sign;
        return 0;
    }
}
int bn_sign(bn const *t){
    return t->sign;
}

bn* bn_add(bn const *left, bn const *right) {
    bn *res = bn_new();
    if (res == NULL) return NULL;
    int l_size = left->bodysize;
    int r_size = right->bodysize;
    if (left->sign == right->sign) {
        res->sign = left->sign;
        res->bodysize = max(l_size, r_size) + 1;
        res->body = (int *) realloc(res->body, sizeof(int) * res->bodysize);
        if (res->body == NULL) {
            free(res);
            return NULL;
        }
        int i = 0;
        int x = 0;
        while (i < res->bodysize) {
            if (i < l_size && i < r_size) {
                res->body[i] = (left->body[i] + right->body[i] + x) % base;
                x = (left->body[i] + right->body[i] + x) / base;
            } else if (i < l_size) {
                res->body[i] = (left->body[i] + x) % base;
                x = (left->body[i] + x) / base;
            } else if (i < r_size) {
                res->body[i] = (right->body[i] + x) % base;
                x = (right->body[i] + x) / base;
            } else {
                res->body[i] = x % base;
                x = x / base;
            }
            i += 1;
        }
    } else {
        int l_sign = left->sign;
        int r_sign = right->sign;

        if (bn_cmp_body(left, right) == -1) {
            res->sign = r_sign;
            res->bodysize = r_size;
            res->body = (int *) realloc(res->body, sizeof(int) * res->bodysize);
            if (res->body == NULL) {
                free(res);
                return NULL;
            }
            int x = 0;
            for (int i = 0; i < res->bodysize; ++i) {
                if (i >= left->bodysize) {
                    res->body[i] = (right->body[i] - x + base) % base;
                    if (right->body[i] - x < 0) x = 1;
                    else x = 0;
                } else {
                    res->body[i] = (right->body[i] - x - left->body[i] + base) % base;
                    if (right->body[i] - x < left->body[i]) x = 1;
                    else x = 0;
                }
            }
        } else {
            res->sign = l_sign;
            res->bodysize = l_size;
            res->body = (int *)  realloc(res->body, sizeof(int) * res->bodysize);
            if (res->body == NULL) {
                free(res);
                return NULL;
            }
            int x = 0;
            for (int i = 0; i < res->bodysize; ++i) {
                if (i >= right->bodysize) {
                    res->body[i] = (left->body[i] - x + base) % base;
                    if (left->body[i] - x < 0) x = 1;
                    else x = 0;
                } else {
                    res->body[i] = (left->body[i] - x - right->body[i] + base) % base;
                    if (left->body[i] - x < right->body[i]) x = 1;
                    else x = 0;
                }
            }
        }
    }
    cut(res);
    if (res->body[0] == 0 && res->bodysize == 1) res->sign = 0;
    return res;
}
bn* bn_sub(bn const *left, bn const *right){
    bn *res = bn_new();
    if (res == NULL) return NULL;
    int l_size = left->bodysize;
    int r_size = right->bodysize;
    int l_sign = left->sign;
    int r_sign = (-1) * right->sign;
    if (l_sign == r_sign) {
        res->sign = l_sign;
        res->bodysize = max(l_size, r_size) + 1;

        res->body = (int *) realloc(res->body, sizeof(int) * res->bodysize);

        if (res->body == NULL) {
            free(res);
            return NULL;
        }

        int i = 0;
        int x = 0;
        while (i < res->bodysize) {
            if (i < l_size && i < r_size) {
                res->body[i] = (left->body[i] + right->body[i] + x) % base;
                x = (left->body[i] + right->body[i] + x) / base;
            } else if (i < l_size) {
                res->body[i] = (left->body[i] + x) % base;
                x = (left->body[i] + x) / base;
            } else if (i < r_size) {
                res->body[i] = (right->body[i] + x) % base;
                x = (right->body[i] + x) / base;
            } else {
                res->body[i] = x % base;
                x = x / base;
            }
            i += 1;
        }
    } else {
        if (bn_cmp_body(left, right) == -1) {
            res->sign = r_sign;
            res->bodysize = r_size;
            res->body = (int *) realloc(res->body, sizeof(int) * res->bodysize);
            if (res->body == NULL) {
                free(res);
                return NULL;
            }
            int x = 0;
            for (int i = 0; i < res->bodysize; ++i) {
                if (i >= left->bodysize) {
                    res->body[i] = (right->body[i] - x + base) % base;
                    if (right->body[i] - x < 0) x = 1;
                    else x = 0;
                } else {
                    res->body[i] = (right->body[i] - x - left->body[i] + base) % base;
                    if (right->body[i] - x < left->body[i]) x = 1;
                    else x = 0;
                }
            }
        } else {
            res->sign = l_sign;
            res->bodysize = l_size;
            res->body = (int *)  realloc(res->body, sizeof(int) * res->bodysize);
            if (res->body == NULL) {
                free(res);
                return NULL;
            }
            int x = 0;
            for (int i = 0; i < res->bodysize; ++i) {
                if (i >= right->bodysize) {
                    res->body[i] = (left->body[i] - x + base) % base;
                    if (left->body[i] - x < 0) x = 1;
                    else x = 0;
                } else {
                    res->body[i] = (left->body[i] - x - right->body[i] + base) % base;
                    if (left->body[i] - x < right->body[i]) x = 1;
                    else x = 0;
                }
            }
        }
    }
    cut(res);
    if (res->body[0] == 0 && res->bodysize == 1) res->sign = 0;
    return res;
}
bn* bn_mul(bn const *left, bn const *right){
    bn* res = bn_new();
    bn* t = bn_new();
    res->sign = 1;
    for(int i = 0; i < right->bodysize; ++i){
        int r = right->body[i];
        t->sign = 1;
        t->bodysize = left->bodysize + 1 + i;
        t->body = (int *) realloc(t->body, sizeof(int) * t->bodysize);
        if (t->body == NULL) {
            free(t);
            return NULL;
        }
        unsigned long long x = 0;
        unsigned long long y;
        for (int k = 0; k < i; ++k){
            t->body[k] = 0;
        }
        for (int k = 0; k < t->bodysize - i; ++k){
            if (k < left->bodysize) {
                y = (unsigned long long) left->body[k] * r + x;
                t->body[k + i] = y % base;
                x = y / base;
            } else{
                t->body[k + i] = x % base;
                x = x / base;
            }
        }
        cut(t);
        bn_add_to(res, t);
    }
    res->sign = left->sign * right->sign;
    cut(res);
    bn_delete(t);
    return res;
}

int bn_mul_ten(bn* t){
    bn* ten = bn_new();
    bn_init_int(ten, 10);
    bn_mul_to(t, ten);
    bn_delete(ten);
    return 0;
}

bn* bn_div(bn const *left, bn const *right) {
    bn* res = bn_new();
    bn* one = bn_new();
    bn_init_int(one, 1);
    bn* c_left = bn_init(left);
    bn* c_right = bn_init(right);
    bn_abs(c_left);
    bn_abs(c_right);
    bn* d = bn_new();
    if (left->sign * right->sign == -1) bn_sub_to(c_left, one);
    int k = (left->bodysize - right->bodysize + 2) * 8;
    if (k < 0) k = 0;
    bn **list = malloc(sizeof(bn*) * k);
    int n = 0;
    while (bn_cmp(c_left, c_right) >= 0){
        list[n] = bn_init(c_right);
        bn_mul_ten(c_right);
        n += 1;
    }
    int q = n;
    while (n > 0){
        int x = 0;
        n -= 1;
        bn_mul_ten(res);
        while(bn_cmp(c_left, list[n]) >= 0){
            bn_sub_to(c_left, list[n]);
            x += 1;
        }
        d->body[0] = x;
        bn_add_to(res, d);
    }
    bn_delete(c_right);
    bn_delete(c_left);
    bn_delete(d);
    for (int i = 0; i < q; ++ i){
        bn_delete(list[i]);
    }
    free(list);
    res->sign = right->sign * left->sign;
    cut(res);
    if (res->body[0] == 0 && res->bodysize == 1) res->sign = 0;
    if (left->sign * right->sign == -1) bn_sub_to(res, one);
    bn_delete(one);
    return res;
}
bn* bn_mod(bn const *left, bn const *right){
    bn* one = bn_new();
    bn_init_int(one, 1);
    bn* c_left = bn_init(left);
    bn* c_right = bn_init(right);
    bn_abs(c_left);
    bn_abs(c_right);
    int k = (left->bodysize - right->bodysize + 2) * 8;
    if (k < 0) k = 0;
    bn **list = malloc(sizeof(bn*) * k);
    int n = 0;
    while (bn_cmp(c_left, c_right) >= 0){
        list[n] = bn_init(c_right);
        bn_mul_ten(c_right);
        n += 1;
    }
    int q = n;
    while (n > 0){
        int x = 0;
        n -= 1;
        while(bn_cmp(c_left, list[n]) >= 0){
            bn_sub_to(c_left, list[n]);
            x += 1;
        }
    }
    if (c_left->sign != 0) {
        c_left->sign = right->sign;
    }
    if (left->sign * right->sign == -1 && c_left->sign != 0){
        bn* z = bn_sub(right, c_left);
        bn_delete(c_left);
        c_left = bn_init(z);
        bn_delete(z);
    }
    bn_delete(c_right);
    bn_delete(one);
    for (int i = 0; i < q; ++ i){
        bn_delete(list[i]);
    }
    free(list);
    return c_left;
}

int bn_add_to(bn *t, bn const *right){
    bn*res = bn_add(t, right);
    t->sign = res->sign;
    t->bodysize = res->bodysize;
    t->body = (int *) realloc(t->body, sizeof(int) * t->bodysize);
    for (int i = 0; i < t->bodysize; ++i){
        t->body[i] = res->body[i];
    }
    bn_delete(res);
    return 0;
}
int bn_sub_to(bn *t, bn const *right){
    bn*res = bn_sub(t, right);
    t->sign = res->sign;
    t->bodysize = res->bodysize;
    t->body = (int *) realloc(t->body, sizeof(int) * t->bodysize);
    for (int i = 0; i < t->bodysize; ++i){
        t->body[i] = res->body[i];
    }
    bn_delete(res);
    return 0;
}
int bn_mul_to(bn *t, bn const *right){
    bn*res = bn_mul(t, right);
    t->sign = res->sign;
    t->bodysize = res->bodysize;
    t->body = (int *) realloc(t->body, sizeof(int) * t->bodysize);
    for (int i = 0; i < t->bodysize; ++i){
        t->body[i] = res->body[i];
    }
    bn_delete(res);
    return 0;
}
int bn_div_to(bn* t, bn const *right) {
    bn*res = bn_div(t, right);
    t->sign = res->sign;
    t->bodysize = res->bodysize;
    t->body = (int *) realloc(t->body, sizeof(int) * t->bodysize);
    for (int i = 0; i < t->bodysize; ++i){
        t->body[i] = res->body[i];
    }
    bn_delete(res);
    return 0;
}
int bn_mod_to(bn*t, bn const *right){
    bn*res = bn_mod(t, right);
    t->sign = res->sign;
    t->bodysize = res->bodysize;
    t->body = (int *) realloc(t->body, sizeof(int) * t->bodysize);
    for (int i = 0; i < t->bodysize; ++i){
        t->body[i] = res->body[i];
    }
    bn_delete(res);
    return 0;
}

int deg(int const d){
    int res = 1;
    int n = d;
    while (n > 1){
        n /= 2;
        res += 1;
    }
    return res;
}
int int_pow(int x, int degree){
    int n = deg(degree);
    int* list = malloc(sizeof(int) * n);
    int y = x;
    for (int i = 0; i < n; ++i){
        list[i] = y;
        y *= y;
    }
    x = 1;
    int i = 0;
    while (degree > 0) {
        if (degree % 2 == 1){
            x *= list[i];
        }
        degree /= 2;
        i += 1;
    }
    free(list);
    return x;
}

int bn_pow_to(bn *t, int degree){
    int n = deg(degree);
    bn **list = malloc(sizeof(bn*) * n);
    bn *x = bn_init(t);
    if (degree == 0){
        bn_init_int(t, 1);
        return 0;
    }
    else if (degree < 0){
        bn_init_int(t, 0);
        return 0;
    }
    for (int i = 0; i < n; ++i){
        list[i] = bn_init(x);
        bn_mul_to(x, x);
    }
    bn_init_int(t, 1);
    int i = 0;
    while (degree > 0) {
        if (degree % 2 == 1){
            bn_mul_to(t, list[i]);
        }
        degree /= 2;
        i += 1;
    }
    bn_delete(x);
    for (int i = 0; i < n; ++ i){
        bn_delete(list[i]);
    }
    free(list);
    return 0;
}

int bn_root_to(bn* t, int reciprocal) {
    bn* y = bn_new();
    bn* t0 = bn_init(t);
    bn* r = bn_new();
    bn* r0 = bn_new();
    bn_init_int(r, reciprocal);
    bn_init_int(r0, reciprocal - 1);
    int n = (t->bodysize - 1) * 8;
    int a = t->body[t->bodysize - 1];
    while(a > 0){
        n += 1;
        a /= 10;
    }
    t->bodysize = ((n - 1) / reciprocal) / 8 + 1;
    t->body = (int *) realloc(t->body, sizeof(int) * t->bodysize);
    for(int i = 0; i < t->bodysize - 1; ++i){
        t->body[i] = 0;
    }
    t->body[t->bodysize - 1] = int_pow(10, ((n - 1) / reciprocal) % 8);
    while(bn_cmp(t, y) != 0){
        bn_delete(y);
        y = bn_init(t);
        bn* b = bn_init(t);
        bn_pow_to(b, reciprocal - 1);
        bn* a = bn_div(t0, b);
        bn_mul_to(t, r0);
        bn_add_to(t, a);
        bn_div_to(t, r);
        bn_delete(a);
        bn_delete(b);
    }
    bn_delete(y);
    bn_delete(r0);
    bn_delete(t0);
    bn_delete(r);
    return 0;
}

int bn_print(bn const *t){
    if (t->sign == -1){
        printf("-");
    }
    int size = t -> bodysize;
    for (int i = size - 1; i > - 1; --i){
        unsigned long int x = t ->body[i] ;
        int j = 0;
        while (x * 10 < base && j < 7 && i != size - 1){
            x *= 10;
            ++j;
            printf("0");
        }
       printf("%d", t->body[i]);
    }
    printf("\n");
    return 0;
}

int bn_init_string_radix(bn *t, const char *init_string, int radix){
    bn_init_int(t, 0);
    int n = strlen(init_string);
    bn* x = bn_new();
    bn* r = bn_new();
    bn_init_int(r, radix);
    int start = 0;
    if (init_string[0] == '-') start = 1;
    for (int i = start; i < n; ++i){
        int d = init_string[i] - '0';
        if (d > 9) d = init_string[i] - 'A' + 10;
        bn_init_int(x, d);
        bn_mul_to(t, r);
        bn_add_to(t, x);
    }
    if (init_string[0] == '-') t->sign = -1;
    bn_delete(x);
    bn_delete(r);
    return 0;
}
int reverse(char* r, int size) {
    char* copy = (char* ) malloc(sizeof(char) * size);
    int a = 0;
    if (r[0] == '-') {
        a = 1;
        copy[0] = '-';
    }
    for (int i = a; i < size - 1; ++i){
        copy[i] = r[size - 2 - i + a];
    }
    copy[size - 1] = '\0';
    for (int i = 0; i < size - 1; ++i){
        r[i] = copy[i];
    }
    free(copy);
    return 0;
}
const char *bn_to_string(bn const *t, int radix) {
    if (radix == 10) {
        int size = 8 * t->bodysize + 2;
        int n = t->bodysize;
        int r_size = 0;
        char *res = (char *) malloc(size * sizeof(char));
        for (int i = 0; i < size; ++i){
            res[i] = '\0';
        }
        if (t->sign == -1) {
            res[0] = '-';
            r_size += 1;
        }
        for(int i = 0; i < n - 1; ++i){
            int x = t->body[i];
            for(int j = r_size;j < 8 + r_size; ++j){
                res[i * 8 + j] = x % 10 + '0';
                x /= 10;
            }
        }
        int i = (t->bodysize - 1) * 8 + r_size;
        int x = t->body[t->bodysize - 1];
        while (x != 0 || i == r_size){
            res[i] = x % 10 + '0';
            x /= 10;
            i += 1;
        }
        res = (char*) realloc(res, sizeof(char) * (i + 2));
        res[i] = '\0';
        reverse(res, i + 1);
        return res;
    }
    if (t->sign == 0){
        char *res = (char* )malloc(sizeof(char) * 2);
        res[0] = '0';
        res[1] = '\0';
        return res;
    }
    bn* x = bn_init(t);
    bn* r = bn_new();
    bn_init_int(r, radix);
    char *res = (char* )malloc(sizeof(char));
    res[0] = '\0';
    int size = 1;
    if (t->sign == -1) {
        res[0] = '-';
        res = (char *) realloc(res, sizeof(char) * 2);
        res[1] = '\0';
        size += 1;
    }
    bn_abs(x);
    while (x->sign != 0){
        bn* bn_d = bn_mod(x, r);
        int d = bn_d->body[0];
        if (d > 9){
            d -= 10;
            res[size - 1] = d + 'A';
        } else{
            res[size - 1] = d + '0';
        }
        size += 1;
        res = (char* )realloc(res, sizeof(char) * size);
        res[size - 1] = '\0';
        bn_div_to(x, r);
        bn_delete(bn_d);
    }
    reverse(res, size);
    bn_delete(r);
    bn_delete(x);
    return res;
}
