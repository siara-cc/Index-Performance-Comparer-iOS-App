//
//  apple_invoker.cpp
//  Index Research
//
//  Created by Arundale Ramanathan on 31/03/18.
//  Copyright © 2018 Siara Logics (cc). All rights reserved.
//

#include "apple_invoker.hpp"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <sys/time.h>
#include <unordered_map>
#include "art.h"
#include "linex.h"
#include "basix.h"
#include "dfox.h"
#include "dfos.h"
#include "dfqx.h"
#include "bft.h"
#include "dft.h"
#include "bfos.h"
#include "bfqs.h"
#include "univix_util.h"
#include "rb_tree.h"
#include <sys/stat.h>

using namespace std;

#define CS_PRINTABLE 1
#define CS_ALPHA_ONLY 2
#define CS_NUMBER_ONLY 3
#define CS_ONE_PER_OCTET 4
#define CS_255_RANDOM 5
#define CS_255_DENSE 6

static volatile int IMPORT_FILE = 0;
static volatile long NUM_ENTRIES = 100;
static volatile int CHAR_SET = 2;
static volatile int KEY_LEN = 8;
static volatile int VALUE_LEN = 4;
static volatile int USE_HASHTABLE = 0;
static volatile int IDX2_SEL = 1;
static volatile int IDX3_SEL = 2;
static volatile int ctr = 0;

void print(const char *s, FN_SIG) {
    cb((const unsigned char *) s);
}

size_t insert(unordered_map<string, string>& m, byte *data_buf, FN_SIG) {
    char k[KEY_LEN + 1];
    char v[VALUE_LEN + 1];
    size_t ret = 0;
    srand48(time(NULL));
    for (unsigned long l = 0; l < NUM_ENTRIES; l++) {
        if (CHAR_SET == CS_PRINTABLE) {
            for (int i = 0; i < KEY_LEN; i++)
                k[i] = (char) (32 + (rand() % 95));
            k[KEY_LEN] = 0;
        } else if (CHAR_SET == CS_ALPHA_ONLY) {
            for (int i = 0; i < KEY_LEN; i++)
                k[i] = (char) (97 + (rand() % 26));
            k[KEY_LEN] = 0;
        } else if (CHAR_SET == CS_NUMBER_ONLY) {
            for (int i = 0; i < KEY_LEN; i++)
                k[i] = (char) (48 + (rand() % 10));
            k[KEY_LEN] = 0;
        } else if (CHAR_SET == CS_ONE_PER_OCTET) {
            for (int i = 0; i < KEY_LEN; i++)
                k[i] = (char) (((rand() % 32) << 3) | 0x07);
            k[KEY_LEN] = 0;
        } else if (CHAR_SET == CS_255_RANDOM) {
            for (int i = 0; i < KEY_LEN; i++)
                k[i] = (char) ((rand() % 255));
            k[KEY_LEN] = 0;
            for (int i = 0; i < KEY_LEN; i++) {
                if (k[i] == 0)
                    k[i] = (char) (i + 1);
            }
        } else if (CHAR_SET == CS_255_DENSE) {
            KEY_LEN = 4;
            k[0] = (char) ((l >> 24) & 0xFF);
            k[1] = (char) ((l >> 16) & 0xFF);
            k[2] = (char) ((l >> 8) & 0xFF);
            k[3] = (char) ((l & 0xFF));
            if (k[0] == 0)
                k[0]++;
            if (k[1] == 0)
                k[1]++;
            if (k[2] == 0)
                k[2]++;
            if (k[3] == 0)
                k[3]++;
            k[4] = 0;
        }
        for (int16_t i = 0; i < VALUE_LEN; i++)
            v[VALUE_LEN - i - 1] = k[i];
        v[VALUE_LEN] = 0;
        //itoa(rand(), v, 10);
        //itoa(rand(), v + strlen(v), 10);
        //itoa(rand(), v + strlen(v), 10);
        if (l == 0) {
            char out_str[200];
            sprintf(out_str, "Key:'%s', Value: '%s'\n", k, v);
            print(out_str, cb);
        }
        if (USE_HASHTABLE)
            m.insert(pair<string, string>(k, v));
        else {
            data_buf[ret++] = KEY_LEN;
            memcpy(data_buf + ret, k, KEY_LEN);
            ret += KEY_LEN;
            data_buf[ret++] = 0;
            data_buf[ret++] = VALUE_LEN;
            memcpy(data_buf + ret, v, VALUE_LEN);
            ret += VALUE_LEN;
            NUM_ENTRIES++;
        }
    }
    if (USE_HASHTABLE)
        NUM_ENTRIES = m.size();
    return ret;
}

int64_t getImportFileSize(const char *filePath) {
    struct stat st;
    string fileName = string(filePath);
    fileName += "/";
    fileName += (IMPORT_FILE == 1 ? "shuffled_domain_rank.csv" : "unordered_dbpedia_labels.txt");
    stat(fileName.c_str(), &st);
    return st.st_size;
}

size_t loadFile(unordered_map<string, string>& m, byte *data_buf, const char *filePath, FN_SIG) {
    FILE *fp;
    char key[2000];
    char value[255];
    char *buf;
    int ctr = 0;
    size_t ret = 0;
    string fileName = string(filePath);
    fileName += "/";
    fileName += (IMPORT_FILE == 1 ? "shuffled_domain_rank.csv" : "unordered_dbpedia_labels.txt");
    print(fileName.c_str(), cb);
    print("\n", cb);
    fp = fopen(fileName.c_str(), "r");
    if (fp == NULL)
        perror("Error opening file");
    buf = key;
    for (int c = fgetc(fp); c > -1; c = fgetc(fp)) {
        if (c == '\t') {
            buf[ctr] = 0;
            ctr = 0;
            buf = value;
        } else if (c == '\n') {
            buf[ctr] = 0;
            ctr = 0;
            unsigned long len = strlen(key);
            if (len > 0 && len <= KEY_LEN) {
                //if (m[key].length() > 0)
                //    cout << key << ":" << value << endl;
                if (buf == value) {
                    if (USE_HASHTABLE)
                        m.insert(pair<string, string>(key, value));
                    else {
                        data_buf[ret++] = len;
                        memcpy(data_buf + ret, key, len);
                        ret += len;
                        data_buf[ret++] = 0;
                        len = strlen(value);
                        data_buf[ret++] = len;
                        memcpy(data_buf + ret, value, len);
                        ret += len;
                    }
                } else {
                    sprintf(value, "%ld", NUM_ENTRIES);
                    //util::ptrToBytes(NUM_ENTRIES, (byte *) value);
                    //value[4] = 0;
                    if (USE_HASHTABLE)
                        m.insert(pair<string, string>(key, value));
                    else {
                        data_buf[ret++] = len;
                        memcpy(data_buf + ret, key, len);
                        ret += len;
                        data_buf[ret++] = 0;
                        len = strlen(value);
                        data_buf[ret++] = len;
                        memcpy(data_buf + ret, value, len);
                        ret += len;
                    }
                }
                if (NUM_ENTRIES % 100000 == 0) {
                    char out_str[200];
                    sprintf(out_str, "Key:'%s', Value: '%s'\n", key, value);
                    print(out_str, cb);
                }
                NUM_ENTRIES++;
            }
            key[0] = 0;
            value[0] = 0;
            buf = key;
        } else {
            if (c != '\r')
                buf[ctr++] = c;
        }
    }
    if (key[0] != 0) {
        if (USE_HASHTABLE)
            m.insert(pair<string, string>(key, value));
        else {
            int16_t len = strlen(key);
            data_buf[ret++] = len;
            memcpy(data_buf + ret, key, len);
            ret += len;
            data_buf[ret++] = 0;
            len = strlen(value);
            data_buf[ret++] = len;
            memcpy(data_buf + ret, value, len);
            ret += len;
        }
        NUM_ENTRIES++;
    }
    fclose(fp);
    return ret;
}

long getTimeVal() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000) + tv.tv_usec;
}

double timedifference(long t0, long t1) {
    double ret = t1;
    ret -= t0;
    ret /= 1000;
    return ret;
}

void initNative() {
}

void checkValue(const char *key, int key_len, const char *val, int val_len,
                const char *returned_value, int returned_len, int& null_ctr, int& cmp) {
    if (returned_value == null) {
        null_ctr++;
    } else {
        int16_t d = util::compare(val, val_len, returned_value, returned_len);
        if (d != 0) {
            cmp++;
            char value[256];
            strncpy(value, returned_value, returned_len);
            value[returned_len] = 0;
            cout << cmp << ":" << (char *) key << "=========="
            << val << "----------->" << returned_value << endl;
        }
    }
}

template<class T1, class T2>
void runTests(bool isART, byte *data_buf, int64_t data_sz, bplus_tree_handler<T1> *lx, bplus_tree_handler<T2> *dx, unordered_map<string, string>& m, FN_SIG) {
    char out_msg[200];
    long start, stop;
    unordered_map<string, string>::iterator it1;

    art_tree at;
    if (isART) {
        ctr = 0;
        art_tree_init(&at);
        start = getTimeVal();
        if (USE_HASHTABLE) {
            it1 = m.begin();
            for (; it1 != m.end(); ++it1) {
                //cout << it1->first.c_str() << endl; //<< ":" << it1->second.c_str() << endl;
                art_insert(&at, (unsigned char*) it1->first.c_str(),
                           (int) it1->first.length() + 1, (void *) it1->second.c_str(),
                           (int) it1->second.length());
                ctr++;
            }
        } else {
            for (int64_t pos = 0; pos < data_sz; pos++) {
                byte key_len = data_buf[pos++];
                byte value_len = data_buf[pos + key_len + 1];
                art_insert(&at, data_buf + pos, key_len + 1, data_buf + pos + key_len + 2, value_len);
                pos += key_len + value_len + 1;
                ctr++;
            }
        }
        stop = getTimeVal();
        sprintf(out_msg, "ART Insert Time: %.3lf\n", timedifference(start, stop));
        print(out_msg, cb);
        //getchar();
    }

    if (lx != NULL) {
        ctr = 0;
        start = getTimeVal();
        if (USE_HASHTABLE) {
            it1 = m.begin();
            for (; it1 != m.end(); ++it1) {
                //cout << it1->first.c_str() << endl; //<< ":" << it1->second.c_str() << endl;
                lx->put(it1->first.c_str(), it1->first.length(), it1->second.c_str(),
                        it1->second.length());
                ctr++;
            }
        } else {
            for (int64_t pos = 0; pos < data_sz; pos++) {
                byte key_len = data_buf[pos++];
                byte value_len = data_buf[pos + key_len + 1];
                lx->put((char *) data_buf + pos, key_len, (char *) data_buf + pos + key_len + 2, value_len);
                pos += key_len + value_len + 1;
                ctr++;
            }
        }
        stop = getTimeVal();
        sprintf(out_msg, "Ix1 Insert Time: %.3lf\n", timedifference(start, stop));
        print(out_msg, cb);
        //getchar();
    }

    if (dx != NULL) {
        ctr = 0;
        start = getTimeVal();
        if (USE_HASHTABLE) {
            it1 = m.begin();
            for (; it1 != m.end(); ++it1) {
                //cout << it1->first.c_str() << endl; //<< ":" << it1->second.c_str() << endl;
                dx->put(it1->first.c_str(), it1->first.length(), it1->second.c_str(),
                        it1->second.length());
                ctr++;
            }
        } else {
            for (int64_t pos = 0; pos < data_sz; pos++) {
                byte key_len = data_buf[pos++];
                byte value_len = data_buf[pos + key_len + 1];
                dx->put((char *) data_buf + pos, key_len, (char *) data_buf + pos + key_len + 2, value_len);
                pos += key_len + value_len + 1;
                ctr++;
            }
        }
        stop = getTimeVal();
        sprintf(out_msg, "Ix2 Insert Time: %.3lf\n", timedifference(start, stop));
        print(out_msg, cb);
        //getchar();
    }

    int null_ctr = 0;
    int cmp = 0;

    if (isART) {
        cmp = 0;
        ctr = 0;
        null_ctr = 0;
        start = getTimeVal();
        if (USE_HASHTABLE) {
            it1 = m.begin();
            for (; it1 != m.end(); ++it1) {
                int len;
                char *value = (char *) art_search(&at,
                                                  (unsigned char*) it1->first.c_str(), (int) it1->first.length() + 1,
                                                  &len);
                checkValue(it1->first.c_str(), (int) it1->first.length() + 1,
                           it1->second.c_str(), (int) it1->second.length(), value, len, null_ctr, cmp);
                ctr++;
            }
        } else {
            for (int64_t pos = 0; pos < data_sz; pos++) {
                int len;
                byte key_len = data_buf[pos++];
                byte value_len = data_buf[pos + key_len + 1];
                char *value = (char *) art_search(&at, data_buf + pos, key_len + 1, &len);
                checkValue((char *) data_buf + pos, key_len + 1,
                           (char *) data_buf + pos + key_len + 2, value_len, value, len, null_ctr, cmp);
                pos += key_len + value_len + 1;
                ctr++;
            }
        }
        stop = getTimeVal();
        sprintf(out_msg, "ART Get Time: %.3lf\n", timedifference(start, stop));
        print(out_msg, cb);
        sprintf(out_msg, "Null: %d, Cmp: %d\n", null_ctr, cmp);
        print(out_msg, cb);
        //getchar();
    }

    if (lx != NULL) {
        cmp = 0;
        ctr = 0;
        null_ctr = 0;
        it1 = m.begin();
        start = getTimeVal();
        if (USE_HASHTABLE) {
            for (; it1 != m.end(); ++it1) {
                int16_t len;
                char *value = lx->get(it1->first.c_str(), it1->first.length(), &len);
                checkValue(it1->first.c_str(), (int) it1->first.length() + 1,
                           it1->second.c_str(), (int) it1->second.length(), value, len, null_ctr, cmp);
                ctr++;
            }
        } else {
            for (int64_t pos = 0; pos < data_sz; pos++) {
                int16_t len;
                byte key_len = data_buf[pos++];
                byte value_len = data_buf[pos + key_len + 1];
                char *value = lx->get((char *) data_buf + pos, key_len, &len);
                checkValue((char *) data_buf + pos, key_len,
                           (char *) data_buf + pos + key_len + 2, value_len, value, len, null_ctr, cmp);
                pos += key_len + value_len + 1;
                ctr++;
            }
        }
        stop = getTimeVal();
        
        sprintf(out_msg, "Ix1 Get Time: %.3lf\n", timedifference(start, stop));
        print(out_msg, cb);
        sprintf(out_msg, "Null: %d, Cmp: %d\n", null_ctr, cmp);
        print(out_msg, cb);
        lx->printStats(NUM_ENTRIES);
        lx->printNumLevels();
        lx->printCounts();
        sprintf(out_msg, "Root filled size: %d\n", util::getInt(lx->root_block + 1));
        print(out_msg, cb);
    }

    if (dx != NULL) {
        cmp = 0;
        ctr = 0;
        null_ctr = 0;
        //bfos::count = 0;
        it1 = m.begin();
        start = getTimeVal();
        if (USE_HASHTABLE) {
            for (; it1 != m.end(); ++it1) {
                int16_t len;
                char *value = dx->get(it1->first.c_str(), it1->first.length(), &len);
                checkValue(it1->first.c_str(), (int) it1->first.length() + 1,
                           it1->second.c_str(), (int) it1->second.length(), value, len, null_ctr, cmp);
                ctr++;
            }
        } else {
            for (int64_t pos = 0; pos < data_sz; pos++) {
                int16_t len;
                byte key_len = data_buf[pos++];
                byte value_len = data_buf[pos + key_len + 1];
                char *value = dx->get((char *) data_buf + pos, key_len, &len);
                checkValue((char *) data_buf + pos, key_len,
                           (char *) data_buf + pos + key_len + 2, value_len, value, len, null_ctr, cmp);
                pos += key_len + value_len + 1;
                ctr++;
            }
        }
        stop = getTimeVal();
        sprintf(out_msg, "Ix2 Get Time: %.3lf\n", timedifference(start, stop));
        print(out_msg, cb);
        sprintf(out_msg, "Null: %d, Cmp: %d\n", null_ctr, cmp);
        print(out_msg, cb);
        dx->printStats(NUM_ENTRIES);
        dx->printNumLevels();
        dx->printCounts();
        sprintf(out_msg, "Root filled size: %d\n", util::getInt(dx->root_block + 1));
        print(out_msg, cb);
        //getchar();
    }

}

void runNative(int data_sel, int idx2_sel, int idx3_sel, int idx_len,
               long num_entries, int char_set, int key_len, int value_len, int isART,
               const char *filePath, FN_SIG) {

    IMPORT_FILE = data_sel;
    IDX2_SEL = idx2_sel;
    IDX3_SEL = idx3_sel;
    NUM_ENTRIES = num_entries;
    CHAR_SET = char_set;
    KEY_LEN = (data_sel == 0 ? key_len : idx_len);
    VALUE_LEN = value_len;

    char out_msg[200];
    long start, stop;
    size_t data_alloc_sz = (IMPORT_FILE == 0 ? (KEY_LEN + VALUE_LEN + 3) * NUM_ENTRIES
                            : getImportFileSize(filePath) + 75000000);
    byte *data_buf = (byte *) malloc(data_alloc_sz);
    size_t data_sz = 0;

    print(filePath, cb);

    unordered_map<string, string> m;
    start = getTimeVal();
    if (IMPORT_FILE == 0) {
        data_sz = insert(m, data_buf, cb);
    } else {
        NUM_ENTRIES = 0;
        data_sz = loadFile(m, data_buf, filePath, cb);
    }
    if (data_alloc_sz > data_sz + 1000000)
        data_buf = (byte *) realloc(data_buf, data_sz);
    stop = getTimeVal();
    sprintf(out_msg, "Data load time: %lf, count: %ld, size %lU\n", timedifference(start, stop), NUM_ENTRIES, USE_HASHTABLE ? m.size() : data_sz);
    print(out_msg, cb);
    //getchar();

    unordered_map<string, string>::iterator it;
    unordered_map<string, string>::iterator it1;

    switch ((IDX2_SEL << 4) + IDX3_SEL) {
        case 0x00:
            runTests(isART, data_buf, data_sz, new basix(), new basix(), m, cb);
            break;
        case 0x01:
            runTests(isART, data_buf, data_sz, new basix(), new bfos(), m, cb);
            break;
        case 0x02:
            runTests(isART, data_buf, data_sz, new basix(), new bfqs(), m, cb);
            break;
        case 0x03:
            runTests(isART, data_buf, data_sz, new basix(), new dfox(), m, cb);
            break;
        case 0x04:
            runTests(isART, data_buf, data_sz, new basix(), new dfos(), m, cb);
            break;
        case 0x05:
            runTests(isART, data_buf, data_sz, new basix(), new dfqx(), m, cb);
            break;
        case 0x06:
            runTests(isART, data_buf, data_sz, new basix(), new bft(), m, cb);
            break;
        case 0x07:
            runTests(isART, data_buf, data_sz, new basix(), new dft(), m, cb);
            break;
        case 0x08:
            runTests(isART, data_buf, data_sz, new basix(), new linex(), m, cb);
            break;
        case 0x09:
            runTests(isART, data_buf, data_sz, new basix(), (rb_tree *) NULL, m, cb);
            break;
        case 0x10:
            runTests(isART, data_buf, data_sz, new bfos(), new basix(), m, cb);
            break;
        case 0x11:
            runTests(isART, data_buf, data_sz, new bfos(), new bfos(), m, cb);
            break;
        case 0x12:
            runTests(isART, data_buf, data_sz, new bfos(), new bfqs(), m, cb);
            break;
        case 0x13:
            runTests(isART, data_buf, data_sz, new bfos(), new dfox(), m, cb);
            break;
        case 0x14:
            runTests(isART, data_buf, data_sz, new bfos(), new dfos(), m, cb);
            break;
        case 0x15:
            runTests(isART, data_buf, data_sz, new bfos(), new dfqx(), m, cb);
            break;
        case 0x16:
            runTests(isART, data_buf, data_sz, new bfos(), new bft(), m, cb);
            break;
        case 0x17:
            runTests(isART, data_buf, data_sz, new bfos(), new dft(), m, cb);
            break;
        case 0x18:
            runTests(isART, data_buf, data_sz, new bfos(), new linex(), m, cb);
            break;
        case 0x19:
            runTests(isART, data_buf, data_sz, new bfos(), (rb_tree *) NULL, m, cb);
            break;
        case 0x20:
            runTests(isART, data_buf, data_sz, new bfqs(), new basix(), m, cb);
            break;
        case 0x21:
            runTests(isART, data_buf, data_sz, new bfqs(), new bfos(), m, cb);
            break;
        case 0x22:
            runTests(isART, data_buf, data_sz, new bfqs(), new bfqs(), m, cb);
            break;
        case 0x23:
            runTests(isART, data_buf, data_sz, new bfqs(), new dfox(), m, cb);
            break;
        case 0x24:
            runTests(isART, data_buf, data_sz, new bfqs(), new dfos(), m, cb);
            break;
        case 0x25:
            runTests(isART, data_buf, data_sz, new bfqs(), new dfqx(), m, cb);
            break;
        case 0x26:
            runTests(isART, data_buf, data_sz, new bfqs(), new bft(), m, cb);
            break;
        case 0x27:
            runTests(isART, data_buf, data_sz, new bfqs(), new dft(), m, cb);
            break;
        case 0x28:
            runTests(isART, data_buf, data_sz, new bfqs(), new linex(), m, cb);
            break;
        case 0x29:
            runTests(isART, data_buf, data_sz, new bfqs(), (rb_tree *) NULL, m, cb);
            break;
        case 0x30:
            runTests(isART, data_buf, data_sz, new dfox(), new basix(), m, cb);
            break;
        case 0x31:
            runTests(isART, data_buf, data_sz, new dfox(), new bfos(), m, cb);
            break;
        case 0x32:
            runTests(isART, data_buf, data_sz, new dfox(), new bfqs(), m, cb);
            break;
        case 0x33:
            runTests(isART, data_buf, data_sz, new dfox(), new dfox(), m, cb);
            break;
        case 0x34:
            runTests(isART, data_buf, data_sz, new dfox(), new dfos(), m, cb);
            break;
        case 0x35:
            runTests(isART, data_buf, data_sz, new dfox(), new dfqx(), m, cb);
            break;
        case 0x36:
            runTests(isART, data_buf, data_sz, new dfox(), new bft(), m, cb);
            break;
        case 0x37:
            runTests(isART, data_buf, data_sz, new dfox(), new dft(), m, cb);
            break;
        case 0x38:
            runTests(isART, data_buf, data_sz, new dfox(), new linex(), m, cb);
            break;
        case 0x39:
            runTests(isART, data_buf, data_sz, new dfox(), (rb_tree *) NULL, m, cb);
            break;
        case 0x40:
            runTests(isART, data_buf, data_sz, new dfos(), new basix(), m, cb);
            break;
        case 0x41:
            runTests(isART, data_buf, data_sz, new dfos(), new bfos(), m, cb);
            break;
        case 0x42:
            runTests(isART, data_buf, data_sz, new dfos(), new bfqs(), m, cb);
            break;
        case 0x43:
            runTests(isART, data_buf, data_sz, new dfos(), new dfox(), m, cb);
            break;
        case 0x44:
            runTests(isART, data_buf, data_sz, new dfos(), new dfos(), m, cb);
            break;
        case 0x45:
            runTests(isART, data_buf, data_sz, new dfos(), new dfqx(), m, cb);
            break;
        case 0x46:
            runTests(isART, data_buf, data_sz, new dfos(), new bft(), m, cb);
            break;
        case 0x47:
            runTests(isART, data_buf, data_sz, new dfos(), new dft(), m, cb);
            break;
        case 0x48:
            runTests(isART, data_buf, data_sz, new dfos(), new linex(), m, cb);
            break;
        case 0x49:
            runTests(isART, data_buf, data_sz, new dfos(), (rb_tree *) NULL, m, cb);
            break;
        case 0x50:
            runTests(isART, data_buf, data_sz, new dfqx(), new basix(), m, cb);
            break;
        case 0x51:
            runTests(isART, data_buf, data_sz, new dfqx(), new bfos(), m, cb);
            break;
        case 0x52:
            runTests(isART, data_buf, data_sz, new dfqx(), new bfqs(), m, cb);
            break;
        case 0x53:
            runTests(isART, data_buf, data_sz, new dfqx(), new dfox(), m, cb);
            break;
        case 0x54:
            runTests(isART, data_buf, data_sz, new dfqx(), new dfos(), m, cb);
            break;
        case 0x55:
            runTests(isART, data_buf, data_sz, new dfqx(), new dfqx(), m, cb);
            break;
        case 0x56:
            runTests(isART, data_buf, data_sz, new dfqx(), new bft(), m, cb);
            break;
        case 0x57:
            runTests(isART, data_buf, data_sz, new dfqx(), new dft(), m, cb);
            break;
        case 0x58:
            runTests(isART, data_buf, data_sz, new dfqx(), new linex(), m, cb);
            break;
        case 0x59:
            runTests(isART, data_buf, data_sz, new dfqx(), (rb_tree *) NULL, m, cb);
            break;
        case 0x60:
            runTests(isART, data_buf, data_sz, new bft(), new basix(), m, cb);
            break;
        case 0x61:
            runTests(isART, data_buf, data_sz, new bft(), new bfos(), m, cb);
            break;
        case 0x62:
            runTests(isART, data_buf, data_sz, new bft(), new bfqs(), m, cb);
            break;
        case 0x63:
            runTests(isART, data_buf, data_sz, new bft(), new dfox(), m, cb);
            break;
        case 0x64:
            runTests(isART, data_buf, data_sz, new bft(), new dfos(), m, cb);
            break;
        case 0x65:
            runTests(isART, data_buf, data_sz, new bft(), new dfqx(), m, cb);
            break;
        case 0x66:
            runTests(isART, data_buf, data_sz, new bft(), new bft(), m, cb);
            break;
        case 0x67:
            runTests(isART, data_buf, data_sz, new bft(), new dft(), m, cb);
            break;
        case 0x68:
            runTests(isART, data_buf, data_sz, new bft(), new linex(), m, cb);
            break;
        case 0x69:
            runTests(isART, data_buf, data_sz, new bft(), (rb_tree *) NULL, m, cb);
            break;
        case 0x70:
            runTests(isART, data_buf, data_sz, new dft(), new basix(), m, cb);
            break;
        case 0x71:
            runTests(isART, data_buf, data_sz, new dft(), new bfos(), m, cb);
            break;
        case 0x72:
            runTests(isART, data_buf, data_sz, new dft(), new bfqs(), m, cb);
            break;
        case 0x73:
            runTests(isART, data_buf, data_sz, new dft(), new dfox(), m, cb);
            break;
        case 0x74:
            runTests(isART, data_buf, data_sz, new dft(), new dfos(), m, cb);
            break;
        case 0x75:
            runTests(isART, data_buf, data_sz, new dft(), new dfqx(), m, cb);
            break;
        case 0x76:
            runTests(isART, data_buf, data_sz, new dft(), new bft(), m, cb);
            break;
        case 0x77:
            runTests(isART, data_buf, data_sz, new dft(), new dft(), m, cb);
            break;
        case 0x78:
            runTests(isART, data_buf, data_sz, new dft(), new linex(), m, cb);
            break;
        case 0x79:
            runTests(isART, data_buf, data_sz, new dft(), (rb_tree *) NULL, m, cb);
            break;
        case 0x80:
            runTests(isART, data_buf, data_sz, new linex(), new basix(), m, cb);
            break;
        case 0x81:
            runTests(isART, data_buf, data_sz, new linex(), new bfos(), m, cb);
            break;
        case 0x82:
            runTests(isART, data_buf, data_sz, new linex(), new bfqs(), m, cb);
            break;
        case 0x83:
            runTests(isART, data_buf, data_sz, new linex(), new dfox(), m, cb);
            break;
        case 0x84:
            runTests(isART, data_buf, data_sz, new linex(), new dfos(), m, cb);
            break;
        case 0x85:
            runTests(isART, data_buf, data_sz, new linex(), new dfqx(), m, cb);
            break;
        case 0x86:
            runTests(isART, data_buf, data_sz, new linex(), new bft(), m, cb);
            break;
        case 0x87:
            runTests(isART, data_buf, data_sz, new linex(), new dft(), m, cb);
            break;
        case 0x88:
            runTests(isART, data_buf, data_sz, new linex(), new linex(), m, cb);
            break;
        case 0x89:
            runTests(isART, data_buf, data_sz, new linex(), (rb_tree *) NULL, m, cb);
            break;
        case 0x90:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, new basix(), m, cb);
            break;
        case 0x91:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, new bfos(), m, cb);
            break;
        case 0x92:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, new bfqs(), m, cb);
            break;
        case 0x93:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, new dfox(), m, cb);
            break;
        case 0x94:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, new dfos(), m, cb);
            break;
        case 0x95:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, new dfqx(), m, cb);
            break;
        case 0x96:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, new bft(), m, cb);
            break;
        case 0x97:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, new dft(), m, cb);
            break;
        case 0x98:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, new linex(), m, cb);
            break;
        case 0x99:
            runTests(isART, data_buf, data_sz, (rb_tree *) NULL, (rb_tree *) NULL, m, cb);
            break;
        default:
            runTests(isART, data_buf, data_sz, new basix(), new bfos(), m, cb);
            break;
    }

}
