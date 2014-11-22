#ifndef FIX_CLANG_UNDEF_EAI_H
#define FIX_CLANG_UNDEF_EAI_H

#ifndef __extern_always_inline
// #  define __extern_inline extern __inline __attribute__ ((__gnu_inline__))
#  define __extern_always_inline \
  extern __always_inline __attribute__ ((__gnu_inline__))
#endif


#endif /* FIX_CLANG_UNDEF_EAI_H */
