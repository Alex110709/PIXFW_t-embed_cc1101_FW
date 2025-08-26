/**
 * @file mjs.c
 * @brief Simplified mJS JavaScript Engine Implementation
 */

#include "mjs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Simple mJS structure
struct mjs {
    char *error_msg;
    mjs_error_handler_t error_handler;
    void *error_user_data;
    mjs_val_t global_object;
    // Global variables storage (simplified)
    struct {
        char *name;
        mjs_val_t value;
    } globals[32];
    int global_count;
};

struct mjs *mjs_create(void)
{
    struct mjs *mjs = calloc(1, sizeof(struct mjs));
    if (!mjs) {
        return NULL;
    }
    
    mjs->global_object = MJS_NULL;
    mjs->global_count = 0;
    
    return mjs;
}

void mjs_destroy(struct mjs *mjs)
{
    if (!mjs) return;
    
    if (mjs->error_msg) {
        free(mjs->error_msg);
    }
    
    // Free global variables
    for (int i = 0; i < mjs->global_count; i++) {
        if (mjs->globals[i].name) {
            free(mjs->globals[i].name);
        }
    }
    
    free(mjs);
}

static void set_error(struct mjs *mjs, const char *msg)
{
    if (mjs->error_msg) {
        free(mjs->error_msg);
    }
    mjs->error_msg = strdup(msg);
    
    if (mjs->error_handler) {
        mjs->error_handler(mjs, msg, mjs->error_user_data);
    }
}

// Simple JavaScript parser and executor (very basic implementation)
static mjs_val_t eval_expression(struct mjs *mjs, const char *expr)
{
    // Skip whitespace
    while (isspace(*expr)) expr++;
    
    // Numbers
    if (isdigit(*expr) || *expr == '-') {
        double num = strtod(expr, NULL);
        return mjs_mk_number(mjs, num);
    }
    
    // Strings
    if (*expr == '"' || *expr == '\'') {
        char quote = *expr++;
        const char *start = expr;
        while (*expr && *expr != quote) expr++;
        if (*expr != quote) {
            set_error(mjs, "Unterminated string");
            return MJS_NULL;
        }
        int len = expr - start;
        return mjs_mk_string(mjs, start, len);
    }
    
    // Boolean literals
    if (strncmp(expr, "true", 4) == 0) {
        return MJS_TRUE;
    }
    if (strncmp(expr, "false", 5) == 0) {
        return MJS_FALSE;
    }
    
    // null
    if (strncmp(expr, "null", 4) == 0) {
        return MJS_NULL;
    }
    
    // undefined
    if (strncmp(expr, "undefined", 9) == 0) {
        return MJS_UNDEFINED;
    }
    
    // Variable lookup (simplified)
    const char *name_start = expr;
    while (isalnum(*expr) || *expr == '_') expr++;
    int name_len = expr - name_start;
    
    if (name_len > 0) {
        char name[64];
        strncpy(name, name_start, name_len);
        name[name_len] = '\0';
        return mjs_get_global(mjs, name);
    }
    
    set_error(mjs, "Syntax error");
    return MJS_NULL;
}

mjs_val_t mjs_exec(struct mjs *mjs, const char *code, const char *filename)
{
    if (!mjs || !code) {
        return MJS_NULL;
    }
    
    // Very simple JavaScript execution
    // This is a placeholder implementation that handles basic expressions
    
    // Look for simple console.log statements
    if (strstr(code, "console.log")) {
        // Extract the argument and print it (simplified)
        const char *start = strstr(code, "(");
        const char *end = strstr(code, ")");
        if (start && end && start < end) {
            start++; // Skip opening parenthesis
            char arg[256];
            int len = end - start;
            if (len < sizeof(arg) - 1) {
                strncpy(arg, start, len);
                arg[len] = '\0';
                
                // Evaluate the argument
                mjs_val_t result = eval_expression(mjs, arg);
                
                // Print the result (simplified)
                printf("JS Console: ");
                if (result == MJS_NULL) {
                    printf("null\n");
                } else if (result == MJS_UNDEFINED) {
                    printf("undefined\n");
                } else if (result == MJS_TRUE) {
                    printf("true\n");
                } else if (result == MJS_FALSE) {
                    printf("false\n");
                } else {
                    // Try to get as string or number
                    size_t str_len;
                    const char *str = mjs_get_string(mjs, result, &str_len);
                    if (str) {
                        printf("%.*s\n", (int)str_len, str);
                    } else {
                        double num = mjs_get_double(mjs, result);
                        printf("%g\n", num);
                    }
                }
            }
        }
        return MJS_UNDEFINED;
    }
    
    // Look for variable assignments (var x = value;)
    if (strstr(code, "var ") || strstr(code, "let ") || strstr(code, "const ")) {
        const char *var_start = code;
        if (strncmp(var_start, "var ", 4) == 0) var_start += 4;
        else if (strncmp(var_start, "let ", 4) == 0) var_start += 4;
        else if (strncmp(var_start, "const ", 6) == 0) var_start += 6;
        
        // Extract variable name
        while (isspace(*var_start)) var_start++;
        const char *name_start = var_start;
        while (isalnum(*var_start) || *var_start == '_') var_start++;
        int name_len = var_start - name_start;
        
        if (name_len > 0) {
            char name[64];
            strncpy(name, name_start, name_len);
            name[name_len] = '\0';
            
            // Look for assignment
            while (isspace(*var_start)) var_start++;
            if (*var_start == '=') {
                var_start++; // Skip =
                while (isspace(*var_start)) var_start++;
                
                // Find end of expression (semicolon or end of string)
                const char *expr_end = strchr(var_start, ';');
                if (!expr_end) expr_end = var_start + strlen(var_start);
                
                char expr[256];
                int expr_len = expr_end - var_start;
                if (expr_len < sizeof(expr) - 1) {
                    strncpy(expr, var_start, expr_len);
                    expr[expr_len] = '\0';
                    
                    // Evaluate expression and store
                    mjs_val_t value = eval_expression(mjs, expr);
                    mjs_set_global(mjs, name, value);
                }
            }
        }
        return MJS_UNDEFINED;
    }
    
    // Default: try to evaluate as expression
    return eval_expression(mjs, code);
}

bool mjs_is_error(mjs_val_t val)
{
    return val == MJS_NULL && false; // Simplified error checking
}

const char *mjs_get_error_message(struct mjs *mjs)
{
    return mjs ? (mjs->error_msg ? mjs->error_msg : "No error") : "Invalid mJS instance";
}

void mjs_set_error_handler(struct mjs *mjs, mjs_error_handler_t handler, void *user_data)
{
    if (mjs) {
        mjs->error_handler = handler;
        mjs->error_user_data = user_data;
    }
}

mjs_val_t mjs_mk_number(struct mjs *mjs, double num)
{
    // Encode double as mjs_val_t (simplified)
    union { double d; uint64_t u; } u;
    u.d = num;
    return u.u;
}

mjs_val_t mjs_mk_string(struct mjs *mjs, const char *str, int len)
{
    // Simplified string encoding - just return a special value
    // In real implementation, this would allocate and track strings
    return 0x1000000000000000ULL | ((uint64_t)str & 0xFFFFFFFFULL);
}

mjs_val_t mjs_mk_boolean(struct mjs *mjs, bool val)
{
    return val ? MJS_TRUE : MJS_FALSE;
}

double mjs_get_double(struct mjs *mjs, mjs_val_t val)
{
    union { double d; uint64_t u; } u;
    u.u = val;
    return u.d;
}

const char *mjs_get_string(struct mjs *mjs, mjs_val_t val, size_t *len)
{
    // Simplified string decoding
    if ((val & 0xF000000000000000ULL) == 0x1000000000000000ULL) {
        const char *str = (const char *)(uintptr_t)(val & 0xFFFFFFFFULL);
        if (len && str) {
            *len = strlen(str);
        }
        return str;
    }
    return NULL;
}

bool mjs_get_bool(mjs_val_t val)
{
    return val == MJS_TRUE;
}

void mjs_set_global(struct mjs *mjs, const char *name, mjs_val_t val)
{
    if (!mjs || !name) return;
    
    // Look for existing variable
    for (int i = 0; i < mjs->global_count; i++) {
        if (mjs->globals[i].name && strcmp(mjs->globals[i].name, name) == 0) {
            mjs->globals[i].value = val;
            return;
        }
    }
    
    // Add new variable if space available
    if (mjs->global_count < 32) {
        mjs->globals[mjs->global_count].name = strdup(name);
        mjs->globals[mjs->global_count].value = val;
        mjs->global_count++;
    }
}

mjs_val_t mjs_get_global(struct mjs *mjs, const char *name)
{
    if (!mjs || !name) return MJS_UNDEFINED;
    
    for (int i = 0; i < mjs->global_count; i++) {
        if (mjs->globals[i].name && strcmp(mjs->globals[i].name, name) == 0) {
            return mjs->globals[i].value;
        }
    }
    
    return MJS_UNDEFINED;
}

void mjs_set_ffi_func(struct mjs *mjs, const char *name, mjs_func_ptr_t func)
{
    if (!mjs || !name || !func) return;
    
    // Store function pointer as a special value
    mjs_val_t func_val = 0x2000000000000000ULL | ((uint64_t)func & 0xFFFFFFFFULL);
    mjs_set_global(mjs, name, func_val);
}