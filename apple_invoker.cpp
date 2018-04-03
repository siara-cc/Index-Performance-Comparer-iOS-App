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
#include "univix_util.h"
#include "rb_tree.h"

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
static volatile int IDX2_SEL = 1;
static volatile int IDX3_SEL = 2;
static volatile int ctr = 0;

void print(const char *s, FN_SIG) {
    cb((const unsigned char *) s);
}

void insert(unordered_map<string, string>& m, FN_SIG) {
    char k[100];
    char v[100];
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
        m.insert(pair<string, string>(k, v));
    }
    NUM_ENTRIES = m.size();
}

void loadFile(unordered_map<string, string>& m, const char *filePath, FN_SIG) {
    FILE *fp;
    char key[2000];
    char value[200];
    char *buf;
    int ctr = 0;
    string fileName = string(filePath);
    fileName += "/";
    fileName += (IMPORT_FILE == 1 ? "domain_rank.csv" : "dbpedia_labels.txt");
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
                if (buf == value)
                    m.insert(pair<string, string>(key, value));
                else {
                    sprintf(value, "%ld", NUM_ENTRIES);
                    //util::ptrToBytes(NUM_ENTRIES, (byte *) value);
                    //value[4] = 0;
                    m.insert(pair<string, string>(key, value));
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
        m.insert(pair<string, string>(key, value));
        NUM_ENTRIES++;
    }
    fclose(fp);
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

void runNative(int data_sel, int idx2_sel, int idx3_sel, int idx_len,
               long num_entries, int char_set, int key_len, int value_len,
               const char *filePath, FN_SIG) {

    IMPORT_FILE = data_sel;
    IDX2_SEL = idx2_sel;
    IDX3_SEL = idx3_sel;
    NUM_ENTRIES = num_entries;
    CHAR_SET = char_set;
    KEY_LEN = (data_sel == 0 ? key_len : idx_len);
    VALUE_LEN = value_len;

    util::generateBitCounts();

    print(filePath, cb);

    unordered_map<string, string> m;
    char out_msg[200];
    long start, stop;
    start = getTimeVal();
    if (IMPORT_FILE == 0)
        insert(m, cb);
    else {
        NUM_ENTRIES = 0;
        loadFile(m, filePath, cb);
    }
    stop = getTimeVal();
    sprintf(out_msg, "HashMap insert time: %lf, count: %ld, size %lU\n", timedifference(start, stop), NUM_ENTRIES, m.size());
    print(out_msg, cb);
    //getchar();

    unordered_map<string, string>::iterator it;
    int null_ctr = 0;
    int cmp = 0;

    unordered_map<string, string>::iterator it1;
    
    ctr = 0;
    art_tree at;
    art_tree_init(&at);
    start = getTimeVal();
    it1 = m.begin();
    for (; it1 != m.end(); ++it1) {
        //cout << it1->first.c_str() << endl; //<< ":" << it1->second.c_str() << endl;
        art_insert(&at, (unsigned char*) it1->first.c_str(),
                   (int) it1->first.length() + 1, (void *) it1->second.c_str(),
                   (int) it1->second.length());
        ctr++;
    }
    stop = getTimeVal();
    sprintf(out_msg, "ART Insert Time: %.3lf\n", timedifference(start, stop));
    print(out_msg, cb);
    //getchar();

    ctr = 0;
    bplus_tree *lx;
    switch (IDX2_SEL) {
        case 0:
            lx = new basix();
            break;
        case 1:
            lx = new bfos();
            break;
        case 2:
            lx = new bfos();
            break;
        case 3:
            lx = new dfox();
            break;
        case 4:
            lx = new dfos();
            break;
        case 5:
            lx = new dfqx();
            break;
        case 6:
            lx = new bft();
            break;
        case 7:
            lx = new dft();
            break;
        case 8:
            lx = new rb_tree();
            break;
        default:
            lx = new basix();
            break;
    }
    it1 = m.begin();
    start = getTimeVal();
    for (; it1 != m.end(); ++it1) {
        //cout << it1->first.c_str() << ":" << it1->second.c_str() << endl;
        lx->put(it1->first.c_str(), (int16_t) it1->first.length(), it1->second.c_str(),
                (int16_t) it1->second.length());
        ctr++;
    }
    stop = getTimeVal();
    sprintf(out_msg, "Ix1 Insert Time: %.3lf\n", timedifference(start, stop));
    print(out_msg, cb);
    //getchar();

    ctr = 0;
    bplus_tree *dx;
    switch (IDX3_SEL) {
        case 0:
            dx = new basix();
            break;
        case 1:
            dx = new bfos();
            break;
        case 2:
            dx = new bfos();
            break;
        case 3:
            dx = new dfox();
            break;
        case 4:
            dx = new dfos();
            break;
        case 5:
            dx = new dfqx();
            break;
        case 6:
            dx = new bft();
            break;
        case 7:
            dx = new dft();
            break;
        case 8:
            dx = new rb_tree();
            break;
        default:
            dx = new basix();
            break;
    }
    it1 = m.begin();
    start = getTimeVal();
    for (; it1 != m.end(); ++it1) {
        dx->put(it1->first.c_str(), (int16_t) it1->first.length(), it1->second.c_str(),
                (int16_t) it1->second.length());
        ctr++;
    }
    stop = getTimeVal();
    sprintf(out_msg, "Ix2 Insert Time: %.3lf\n", timedifference(start, stop));
    print(out_msg, cb);
    //getchar();

    cmp = 0;
    ctr = 0;
    null_ctr = 0;
    it1 = m.begin();
    start = getTimeVal();
    for (; it1 != m.end(); ++it1) {
        int len;
        char *value = (char *) art_search(&at,
                                          (unsigned char*) it1->first.c_str(), (int) it1->first.length() + 1,
                                          &len);
        char v[100];
        if (value == null) {
            null_ctr++;
        } else {
            int16_t d = util::compare(it1->second.c_str(), (int16_t) it1->second.length(),
                                      value, (int16_t) len);
            if (d != 0) {
                cmp++;
                strncpy(v, value, (size_t) len);
                v[it1->first.length()] = 0;
                sprintf(out_msg, "%d:%s:==========:%s:==========>%s\n", cmp, it1->first.c_str(),
                        it1->second.c_str(), v);
                print(out_msg, cb);
            }
        }
        ctr++;
    }
    stop = getTimeVal();
    sprintf(out_msg, "ART Get Time: %.3lf\n", timedifference(start, stop));
    print(out_msg, cb);
    sprintf(out_msg, "Null: %d, Cmp: %d\n", null_ctr, cmp);
    print(out_msg, cb);
    //getchar();

    cmp = 0;
    ctr = 0;
    null_ctr = 0;
    it1 = m.begin();
    start = getTimeVal();
    for (; it1 != m.end(); ++it1) {
        int16_t len;
        char *value = lx->get(it1->first.c_str(), (int16_t) it1->first.length(), &len);
        char v[100];
        if (value == null) {
            null_ctr++;
        } else {
            int16_t d = util::compare(it1->second.c_str(), (int16_t) it1->second.length(),
                                      value, len);
            if (d != 0) {
                cmp++;
                strncpy(v, value, (size_t) len);
                v[len] = 0;
                sprintf(out_msg, "%d:%s:==========:%s:==========>%s\n", cmp, it1->first.c_str(),
                        it1->second.c_str(), v);
                print(out_msg, cb);
            } // else {
            //    strncpy(v, value, len);
            //    v[len] = 0;
            //    cout << v << endl;
            //}
        }
        ctr++;
    }
    stop = getTimeVal();
    sprintf(out_msg, "Ix1 Get Time: %.3lf\n", timedifference(start, stop));
    print(out_msg, cb);
    sprintf(out_msg, "Null: %d, Cmp: %d\n", null_ctr, cmp);
    print(out_msg, cb);
    lx->printMaxKeyCount(NUM_ENTRIES);
    lx->printNumLevels();
    //lx->printCounts();
    sprintf(out_msg, "Root filled size: %d\n", util::getInt(lx->root_data + 1));
    print(out_msg, cb);

    cmp = 0;
    ctr = 0;
    null_ctr = 0;
    //bfos::count = 0;
    it1 = m.begin();
    start = getTimeVal();
    for (; it1 != m.end(); ++it1) {
        int16_t len;
        char *value = dx->get(it1->first.c_str(), (int16_t) it1->first.length(), &len);
        char v[100];
        if (value == null) {
            cout << "Null:" << it1->first.c_str() << endl;
            null_ctr++;
        } else {
            int16_t d = util::compare(it1->second.c_str(), (int16_t) it1->second.length(),
                                      value, len);
            if (d != 0) {
                cmp++;
                strncpy(v, value, (size_t) len);
                v[it1->first.length()] = 0;
                sprintf(out_msg, "%d:%s:==========:%s:==========>%s\n", cmp, it1->first.c_str(),
                        it1->second.c_str(), v);
                print(out_msg, cb);
            }
        }
        ctr++;
    }
    stop = getTimeVal();
    sprintf(out_msg, "Ix2 Get Time: %.3lf\n", timedifference(start, stop));
    print(out_msg, cb);
    sprintf(out_msg, "Null: %d, Cmp: %d\n", null_ctr, cmp);
    print(out_msg, cb);
    sprintf(out_msg, "Trie Size: %d\n", (int) dx->root_data[MAX_PTR_BITMAP_BYTES + 5]);
    print(out_msg, cb);
    sprintf(out_msg, "Root filled size: %d\n", (int) util::getInt(dx->root_data + MAX_PTR_BITMAP_BYTES + 1));
    print(out_msg, cb);
    sprintf(out_msg, "Data Pos: %d\n", (int) util::getInt(dx->root_data + MAX_PTR_BITMAP_BYTES + 3));
    print(out_msg, cb);
    dx->printMaxKeyCount(NUM_ENTRIES);
    dx->printNumLevels();
    //dx->printCounts();

}
