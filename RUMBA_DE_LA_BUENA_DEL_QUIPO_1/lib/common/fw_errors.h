#ifndef FW_ERRORS_H
#define FW_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FW_ERR_OK = 0,
    FW_ERR_NOT_FOUND = 1,
    FW_ERR_TIMEOUT = 2,
    FW_ERR_BAD_DATA = 3,
    FW_ERR_BAD_PARAM = 4,
    FW_ERR_NOT_INIT = 5
} fw_err_t;

#ifdef __cplusplus
}
#endif

#endif /* FW_ERRORS_H */