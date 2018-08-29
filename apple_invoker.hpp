//
//  apple_invoker.hpp
//  Index Research
//
//  Created by Arundale Ramanathan on 31/03/18.
//  Copyright © 2018 Siara Logics (cc). All rights reserved.
//

#ifndef apple_invoker_hpp
#define apple_invoker_hpp

#define FN_SIG void(*cb)(const unsigned char *)
#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

    void runNative(int data_sel, int idx2_sel, int idx3_sel, int idx_len,
                   long num_entries, int char_set, int key_len, int value_len, int isART,
                   const char *filePath, FN_SIG);

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif
#endif /* apple_invoker_hpp */
