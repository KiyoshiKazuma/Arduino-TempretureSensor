#include <Arduino.h>
#include <string.h>
#include "common.h"
#include "mystring.h"

/*
 * Function: u1_mystring_init
 * ---------------------------
 * Initializes the MYSTRING structure.
 *
 * p_mystring: Pointer to the MYSTRING structure to initialize.
 *
 * returns: STD_RETURN_OK on success, STD_RETURN_NG on failure.
 */
U1 u1_mystring_init(struct MYSTRING *p_mystring)
{
    U1 u1_t_ret = STD_RETURN_NG;

    if (p_mystring != NULL) {
        p_mystring->u1_length = 0;
        memset(p_mystring->au1_data, 0, MYSTRING_MAX_SIZE);
        u1_t_ret = STD_RETURN_OK;
    }

    return u1_t_ret;
}

/*
 * Function: u1_mystring_push_string
 * ---------------------------------
 * Pushes a string into the MYSTRING structure.
 *
 * pst_a_mystring: Pointer to the MYSTRING structure.
 * au1_a_string: Pointer to the string to push.
 *
 * returns: STD_RETURN_OK on success, STD_RETURN_NG on failure.
 */
U1 u1_mystring_push_string(struct MYSTRING *pst_a_mystring, const U1 *au1_a_string)
{
    U1 u1_t_length  = 0;
    U1 u1_t_ret = STD_RETURN_NG;

    if (pst_a_mystring != NULL && au1_a_string != NULL) {
        u1_t_length = strlen((const char *)au1_a_string);
        if (u1_t_length + pst_a_mystring->u1_length < MYSTRING_MAX_SIZE) {
            memcpy(&pst_a_mystring->au1_data[pst_a_mystring->u1_length], au1_a_string, u1_t_length);
            pst_a_mystring->u1_length += u1_t_length;
            u1_t_ret = STD_RETURN_OK;
        }
    }

    return u1_t_ret;
}
/*
 * Function: u1_mystring_push_data
 * -------------------------------
 * Pushes a single byte of data into the MYSTRING structure.
 *
 * pst_a_mystring: Pointer to the MYSTRING structure.
 * u1_a_data: The byte of data to push.
 *
 * returns: STD_RETURN_OK on success, STD_RETURN_NG on failure.
 */
U1 u1_mystring_push_data(struct MYSTRING *pst_a_mystring, const U4 u4_a_data)
{
    U1 u1_t_ret = STD_RETURN_NG;
    U4 u4_t_data_tmp;
    U1 u1_t_length;
    U1 u1_t_cur;
    U1 au1_t_buffer[MYSTRING_MAX_SIZE];

    /* detect data length */
    u4_t_data_tmp = u4_a_data;
    for (u1_t_length = 0; u1_t_length < MYSTRING_MAX_SIZE; u1_t_length++) {
        u4_t_data_tmp /= 10;
        if (u4_t_data_tmp < 1) {
            break;
        }
    }

    /* exchange to string */
    u4_t_data_tmp = u4_a_data;
    for (u1_t_cur = 0; u1_t_cur <= u1_t_length; u1_t_cur++) {
        au1_t_buffer[u1_t_length-u1_t_cur] = u4_t_data_tmp % 10 + '0';
        u4_t_data_tmp /= 10;
    }
    au1_t_buffer[u1_t_length + 1] = '\0';

    u1_t_ret = u1_mystring_push_string(pst_a_mystring, au1_t_buffer);

    return u1_t_ret;
}