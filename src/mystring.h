#ifndef MYSTRING_H
#define	MYSTRING_H

#include "common.h"
#define MYSTRING_MAX_SIZE (32U)

struct MYSTRING {
    U1 au1_data[MYSTRING_MAX_SIZE];
    U1 u1_length;
};

U1 u1_mystring_init(struct MYSTRING *p_mystring);
U1 u1_mystring_push_string(struct MYSTRING *pst_a_mystring, const U1 *au1_a_string);
U1 u1_mystring_push_data(struct MYSTRING *pst_a_mystring, const U4 u4_a_data);


#endif	/* MYSTRING_H */