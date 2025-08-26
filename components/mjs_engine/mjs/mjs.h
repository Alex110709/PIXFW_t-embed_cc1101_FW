/**
 * @file mjs.h
 * @brief Simplified mJS JavaScript Engine Header
 */

#ifndef MJS_H
#define MJS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
struct mjs;
typedef struct mjs mjs_t;

// Value type
typedef uint64_t mjs_val_t;

// Special values
#define MJS_NULL        ((mjs_val_t) 0x7ff8000000000001ULL)
#define MJS_UNDEFINED   ((mjs_val_t) 0x7ff8000000000002ULL)
#define MJS_TRUE        ((mjs_val_t) 0x7ff8000000000003ULL)
#define MJS_FALSE       ((mjs_val_t) 0x7ff8000000000004ULL)

// Error handler callback
typedef void (*mjs_error_handler_t)(struct mjs *mjs, const char *msg, void *user_data);

/**
 * @brief Create mJS instance
 * @return mJS instance or NULL on failure
 */
struct mjs *mjs_create(void);

/**
 * @brief Destroy mJS instance
 * @param mjs mJS instance
 */
void mjs_destroy(struct mjs *mjs);

/**
 * @brief Execute JavaScript code
 * @param mjs mJS instance
 * @param code JavaScript code string
 * @param filename Optional filename for debugging
 * @return Result value
 */
mjs_val_t mjs_exec(struct mjs *mjs, const char *code, const char *filename);

/**
 * @brief Check if value is an error
 * @param val Value to check
 * @return true if error
 */
bool mjs_is_error(mjs_val_t val);

/**
 * @brief Get error message from mJS instance
 * @param mjs mJS instance
 * @return Error message string
 */
const char *mjs_get_error_message(struct mjs *mjs);

/**
 * @brief Set error handler
 * @param mjs mJS instance
 * @param handler Error handler callback
 * @param user_data User data for callback
 */
void mjs_set_error_handler(struct mjs *mjs, mjs_error_handler_t handler, void *user_data);

/**
 * @brief Create JavaScript number
 * @param mjs mJS instance
 * @param num Number value
 * @return JavaScript value
 */
mjs_val_t mjs_mk_number(struct mjs *mjs, double num);

/**
 * @brief Create JavaScript string
 * @param mjs mJS instance
 * @param str String value
 * @param len String length (-1 for null-terminated)
 * @return JavaScript value
 */
mjs_val_t mjs_mk_string(struct mjs *mjs, const char *str, int len);

/**
 * @brief Create JavaScript boolean
 * @param mjs mJS instance
 * @param val Boolean value
 * @return JavaScript value
 */
mjs_val_t mjs_mk_boolean(struct mjs *mjs, bool val);

/**
 * @brief Get number from JavaScript value
 * @param val JavaScript value
 * @return Number value
 */
double mjs_get_double(struct mjs *mjs, mjs_val_t val);

/**
 * @brief Get string from JavaScript value
 * @param mjs mJS instance
 * @param val JavaScript value
 * @param len Pointer to store string length
 * @return String pointer
 */
const char *mjs_get_string(struct mjs *mjs, mjs_val_t val, size_t *len);

/**
 * @brief Get boolean from JavaScript value
 * @param val JavaScript value
 * @return Boolean value
 */
bool mjs_get_bool(mjs_val_t val);

/**
 * @brief Set global variable
 * @param mjs mJS instance
 * @param name Variable name
 * @param val Variable value
 */
void mjs_set_global(struct mjs *mjs, const char *name, mjs_val_t val);

/**
 * @brief Get global variable
 * @param mjs mJS instance
 * @param name Variable name
 * @return Variable value
 */
mjs_val_t mjs_get_global(struct mjs *mjs, const char *name);

// Native function callback
typedef mjs_val_t (*mjs_func_ptr_t)(struct mjs *mjs);

/**
 * @brief Set native function
 * @param mjs mJS instance
 * @param name Function name
 * @param func Function pointer
 */
void mjs_set_ffi_func(struct mjs *mjs, const char *name, mjs_func_ptr_t func);

#ifdef __cplusplus
}
#endif

#endif // MJS_H