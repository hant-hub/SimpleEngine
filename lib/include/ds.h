#ifndef DS_H
#include <cutils.h>

/*
    Dynamic arrays
*/

#define dynArray(type)                                                         \
    struct {                                                                   \
        Allocator a;                                                           \
        u64 size;                                                              \
        u64 cap;                                                               \
        type *data;                                                            \
    }

#define dynPush(array, val)                                                    \
    ((array.size + 1 > array.cap)                                              \
         ? (array.data = Realloc(                                              \
                array.a, array.data, array.cap * sizeof(array.data[0]),        \
                (array.cap ? array.cap * 2 : 4) * sizeof(array.data[0])),      \
            array.cap = array.cap ? array.cap * 2 : 4)                         \
         : 0,                                                                  \
     array.data[array.size++] = val)

#define dynReserve(array, size)                                                \
    ((array.cap < size)                                                        \
         ? (array.data = Realloc(array.a, array.data,                          \
                                 array.cap * sizeof(array.data[0]),            \
                                 size * sizeof(array.data[0])),                \
            array.cap = size)                                                  \
         : 0)

// INFO(ELI): This macro can't be included in expressions and must be
// on its own line. This is due to the loops which I could fix if
// I added more global functions but I decided I preferred to not
// do that.
#define dynResize(array, newsize)                                              \
    do {                                                                       \
        if (array.cap < newsize) {                                             \
            u64 oldsize = array.cap;                                           \
            while (array.cap < newsize) {                                      \
                array.cap = array.cap ? array.cap * 2 : 4;                     \
            }                                                                  \
            array.data =                                                       \
                Realloc(array.a, array.data, oldsize * sizeof(array.data[0]),  \
                        array.cap * sizeof(array.data[0]));                    \
        }                                                                      \
                                                                               \
        if (array.size < newsize) {                                            \
            memset(&array.data[array.size], 0,                                 \
                   (newsize - array.size) * sizeof(array.data[0]));            \
        }                                                                      \
        array.size = newsize;                                                  \
    } while (0);

#define dynBack(array) (array.data[array.size - 1])

#define dynFree(array)                                                         \
    (Free(array.a, array.data, array.cap * sizeof(array.data[0])))

// INFO(ELI): These Macros cannot be part of expressions since they require
// loops.

#define dynIns(array, idx, val)                                                \
    do {                                                                       \
        dynResize(array, array.size + 1);                                      \
        for (i64 i = array.size - 1; i >= idx; i--) {                          \
            array.data[i + 1] = array.data[i];                                 \
        }                                                                      \
        array.data[idx] = val;                                                 \
    } while (0)

#define dynDel(array, idx)                                                     \
    do {                                                                       \
        for (u32 i = idx; i < ints.size - 1; i++) {                            \
            array.data[i] = array.data[i + 1];                                 \
        }                                                                      \
        dynResize(array, array.size - 1);                                      \
    } while (0)

#define dynExt(array, vals, num)                                               \
    do {                                                                       \
        dynResize(array, array.size + num);                                    \
        for (u64 i = array.size - num; i < array.size; i++) {                  \
            array.data[i] = vals[i - array.size + num];                        \
        }                                                                      \
    } while (0)

/*
    HashMap

    One is for non string keys, other is for arbitrary sized string
    keys
*/

#define MAX_LOAD 0.9
#define MIN_CAP 16

#define DefHashMapDecl(name, key, val)                                         \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        u32 cap;                                                               \
        u32 size;                                                              \
        struct {                                                               \
            key k;                                                             \
            i32 m;                                                             \
        } *keys;                                                               \
        val *vals;                                                             \
    } name;                                                                    \
    void name##Ins(name *, key, val);                                          \
    val *name##Get(name *, key);                                               \
    void name##Del(name *, key);                                               \
    void name##Clear(name *);                                                  \
    void name##Free(name *);

#define DefHashMapImpl(name, key, val)                                         \
    void name##Ins(name *map, key ky, val vl) {                                \
        if (map->cap * MAX_LOAD <= map->size + 1) {                            \
            u64 oldcap = map->cap;                                             \
            map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      \
                                                                               \
            typeof(map->keys) oldkeys = map->keys;                             \
            typeof(map->vals) oldvals = map->vals;                             \
                                                                               \
            map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));        \
            map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));        \
                                                                               \
            memset(map->keys, -1, map->cap * sizeof(map->keys[0]));            \
            for (u32 i = 0; i < oldcap; i++) {                                 \
                if (oldkeys[i].m == -1)                                        \
                    continue;                                                  \
                                                                               \
                typeof(map->keys[0].k) tempk = oldkeys[i].k;                   \
                typeof(map->vals[0]) tempv = oldvals[i];                       \
                u32 hash = FNVHash32((u8 *)&tempk, sizeof(tempk));             \
                                                                               \
                u32 idx = hash % map->cap;                                     \
                u32 dist = 0;                                                  \
                                                                               \
                for (u64 i = 0; i < map->cap; i++) {                           \
                    if (map->keys[idx].m == -1) {                              \
                        map->keys[idx].k = tempk;                              \
                        map->keys[idx].m = dist;                               \
                        map->vals[idx] = tempv;                                \
                        break;                                                 \
                    }                                                          \
                    if (map->keys[idx].m < dist) {                             \
                        typeof(map->keys[0].k) tmpkey = map->keys[idx].k;      \
                        typeof(map->vals[0]) tmpval = map->vals[idx];          \
                        u32 tmpdist = map->keys[idx].m;                        \
                                                                               \
                        map->keys[idx].k = tempk;                              \
                        map->keys[idx].m = dist;                               \
                        map->vals[idx] = tempv;                                \
                                                                               \
                        tempk = tmpkey;                                        \
                        tempv = tmpval;                                        \
                        dist = tmpdist;                                        \
                    }                                                          \
                                                                               \
                    if (map->keys[idx].k == tempk) {                           \
                        map->vals[idx] = tempv;                                \
                        break;                                                 \
                    }                                                          \
                    idx = (idx + 1) % map->cap;                                \
                    dist++;                                                    \
                }                                                              \
            }                                                                  \
                                                                               \
            Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));              \
            Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));              \
        }                                                                      \
                                                                               \
        typeof(map->keys[0].k) tempk = ky;                                     \
        typeof(map->vals[0]) tempv = vl;                                       \
        u32 hash = FNVHash32((u8 *)&tempk, sizeof(tempk));                     \
                                                                               \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                map->keys[idx].k = tempk;                                      \
                map->keys[idx].m = dist;                                       \
                map->vals[idx] = tempv;                                        \
                map->size++;                                                   \
                break;                                                         \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                typeof(map->keys[0].k) tmpkey = map->keys[idx].k;              \
                typeof(map->vals[0]) tmpval = map->vals[idx];                  \
                u32 tmpdist = map->keys[idx].m;                                \
                                                                               \
                map->keys[idx].k = tempk;                                      \
                map->keys[idx].m = dist;                                       \
                map->vals[idx] = tempv;                                        \
                                                                               \
                tempk = tmpkey;                                                \
                tempv = tmpval;                                                \
                dist = tmpdist;                                                \
            }                                                                  \
                                                                               \
            if (map->keys[idx].k == tempk) {                                   \
                map->vals[idx] = tempv;                                        \
                break;                                                         \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
    }                                                                          \
    val *name##Get(name *map, key ky) {                                        \
        typeof(map->keys[0].k) tempk = ky;                                     \
        u32 hash = FNVHash32((u8 *)&tempk, sizeof(tempk));                     \
                                                                               \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                return NULL;                                                   \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                return NULL;                                                   \
            }                                                                  \
                                                                               \
            if (map->keys[idx].k == tempk) {                                   \
                return &map->vals[idx];                                        \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
                                                                               \
        return NULL;                                                           \
    }                                                                          \
    void name##Del(name *map, key ky) {                                        \
                                                                               \
        typeof(map->keys[0].k) tempk = ky;                                     \
                                                                               \
        u32 hash = FNVHash32((u8 *)&tempk, sizeof(tempk));                     \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                return;                                                        \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                return;                                                        \
            }                                                                  \
                                                                               \
            if (map->keys[idx].k == tempk) {                                   \
                map->size--;                                                   \
                idx = (idx + 1) % map->cap;                                    \
                break;                                                         \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
                                                                               \
        u32 prev = (idx + map->cap - 1) % map->cap;                            \
        while (map->keys[idx].m != -1 && map->keys[idx].m > 0) {               \
            map->keys[prev] = map->keys[idx];                                  \
            map->keys[prev].m -= 1;                                            \
                                                                               \
            map->vals[prev] = map->vals[idx];                                  \
            idx = (idx + 1) % map->cap;                                        \
            prev = (idx + map->cap - 1) % map->cap;                            \
        }                                                                      \
                                                                               \
        map->keys[prev].m = -1;                                                \
    }                                                                          \
    void name##Reserve(name *map, u64 size) {                                  \
        u32 oldcap = map->cap;                                                 \
        while (map->cap < size) {                                              \
            map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      \
        }                                                                      \
                                                                               \
        typeof(map->keys) oldkeys = map->keys;                                 \
        typeof(map->vals) oldvals = map->vals;                                 \
                                                                               \
        map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));            \
        map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));            \
                                                                               \
        memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                \
        for (u32 i = 0; i < oldcap; i++) {                                     \
            if (oldkeys[i].m == -1)                                            \
                continue;                                                      \
                                                                               \
            typeof(map->keys[0].k) tempk = oldkeys[i].k;                       \
            typeof(map->vals[0]) tempv = oldvals[i];                           \
            u32 hash = FNVHash32((u8 *)&tempk, sizeof(tempk));                 \
                                                                               \
            u32 idx = hash % map->cap;                                         \
            u32 dist = 0;                                                      \
                                                                               \
            for (u64 i = 0; i < map->cap; i++) {                               \
                if (map->keys[idx].m == -1) {                                  \
                    map->keys[idx].k = tempk;                                  \
                    map->keys[idx].m = dist;                                   \
                    map->vals[idx] = tempv;                                    \
                    break;                                                     \
                }                                                              \
                if (map->keys[idx].m < dist) {                                 \
                    typeof(map->keys[0].k) tmpkey = map->keys[idx].k;          \
                    typeof(map->vals[0]) tmpval = map->vals[idx];              \
                    u32 tmpdist = map->keys[idx].m;                            \
                                                                               \
                    map->keys[idx].k = tempk;                                  \
                    map->keys[idx].m = dist;                                   \
                    map->vals[idx] = tempv;                                    \
                                                                               \
                    tempk = tmpkey;                                            \
                    tempv = tmpval;                                            \
                    dist = tmpdist;                                            \
                }                                                              \
                                                                               \
                if (map->keys[idx].k == tempk) {                               \
                    map->vals[idx] = tempv;                                    \
                    break;                                                     \
                }                                                              \
                idx = (idx + 1) % map->cap;                                    \
                dist++;                                                        \
            }                                                                  \
        }                                                                      \
                                                                               \
        Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));                  \
        Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));                  \
    }                                                                          \
    void name##Clear(name *map) {                                              \
        map->size = 0;                                                         \
        memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                \
    }                                                                          \
    void name##Free(name *map) {                                               \
        Free(map->a, map->keys, map->cap * sizeof(map->keys[0]));              \
        Free(map->a, map->vals, map->cap * sizeof(map->vals[0]));              \
    }

#define DefStringMapDecl(name, val)                                            \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        u32 cap;                                                               \
        u32 size;                                                              \
        struct {                                                               \
            SString k;                                                         \
            i32 m;                                                             \
        } *keys;                                                               \
        val *vals;                                                             \
    } name;                                                                    \
    void nameIns(name *, SString, val);                                        \
    val *nameGet(name *, SString);                                             \
    void nameDel(name *, SString);                                             \
    void nameFree(name *);

#define DefStringMapImpl(name, val)                                            \
    void name##Ins(name *map, SString ky, val vl) {                            \
        if (map->cap * MAX_LOAD <= map->size + 1) {                            \
            u64 oldcap = map->cap;                                             \
            map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      \
                                                                               \
            typeof(map->keys) oldkeys = map->keys;                             \
            typeof(map->vals) oldvals = map->vals;                             \
                                                                               \
            map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));        \
            map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));        \
                                                                               \
            memset(map->keys, -1, map->cap * sizeof(map->keys[0]));            \
            for (u32 i = 0; i < oldcap; i++) {                                 \
                if (oldkeys[i].m == -1)                                        \
                    continue;                                                  \
                                                                               \
                typeof(map->keys[0].k) tempk = oldkeys[i].k;                   \
                typeof(map->vals[0]) tempv = oldvals[i];                       \
                u32 hash = FNVHash32((u8 *)tempk.data, tempk.len);             \
                                                                               \
                u32 idx = hash % map->cap;                                     \
                u32 dist = 0;                                                  \
                                                                               \
                for (u64 i = 0; i < map->cap; i++) {                           \
                    if (map->keys[idx].m == -1) {                              \
                        map->keys[idx].k = tempk;                              \
                        map->keys[idx].m = dist;                               \
                        map->vals[idx] = tempv;                                \
                        break;                                                 \
                    }                                                          \
                    if (map->keys[idx].m < dist) {                             \
                        typeof(map->keys[0].k) tmpkey = map->keys[idx].k;      \
                        typeof(map->vals[0]) tmpval = map->vals[idx];          \
                        u32 tmpdist = map->keys[idx].m;                        \
                                                                               \
                        map->keys[idx].k = tempk;                              \
                        map->keys[idx].m = dist;                               \
                        map->vals[idx] = tempv;                                \
                                                                               \
                        tempk = tmpkey;                                        \
                        tempv = tmpval;                                        \
                        dist = tmpdist;                                        \
                    }                                                          \
                                                                               \
                    if (Sstrcmp(map->keys[idx].k, tempk)) {                    \
                        map->vals[idx] = tempv;                                \
                        break;                                                 \
                    }                                                          \
                    idx = (idx + 1) % map->cap;                                \
                    dist++;                                                    \
                }                                                              \
            }                                                                  \
                                                                               \
            Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));              \
            Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));              \
        }                                                                      \
                                                                               \
        typeof(map->keys[0].k) tempk = ky;                                     \
        typeof(map->vals[0]) tempv = vl;                                       \
        u32 hash = FNVHash32((u8 *)tempk.data, tempk.len);                     \
                                                                               \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                map->keys[idx].k = tempk;                                      \
                map->keys[idx].m = dist;                                       \
                map->vals[idx] = tempv;                                        \
                map->size++;                                                   \
                break;                                                         \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                typeof(map->keys[0].k) tmpkey = map->keys[idx].k;              \
                typeof(map->vals[0]) tmpval = map->vals[idx];                  \
                u32 tmpdist = map->keys[idx].m;                                \
                                                                               \
                map->keys[idx].k = tempk;                                      \
                map->keys[idx].m = dist;                                       \
                map->vals[idx] = tempv;                                        \
                                                                               \
                tempk = tmpkey;                                                \
                tempv = tmpval;                                                \
                dist = tmpdist;                                                \
            }                                                                  \
                                                                               \
            if (Sstrcmp(map->keys[idx].k, tempk)) {                            \
                map->vals[idx] = tempv;                                        \
                break;                                                         \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
    }                                                                          \
    val *name##Get(name *map, SString ky) {                                    \
        typeof(map->keys[0].k) tempk = ky;                                     \
        u32 hash = FNVHash32((u8 *)tempk.data, tempk.len);                     \
                                                                               \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                return NULL;                                                   \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                return NULL;                                                   \
            }                                                                  \
                                                                               \
            if (Sstrcmp(map->keys[idx].k, tempk)) {                            \
                return &map->vals[idx];                                        \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
                                                                               \
        return NULL;                                                           \
    }                                                                          \
    void name##Del(name *map, SString ky) {                                    \
                                                                               \
        typeof(map->keys[0].k) tempk = ky;                                     \
                                                                               \
        u32 hash = FNVHash32((u8 *)tempk.data, tempk.len);                     \
        u32 idx = hash % map->cap;                                             \
        u32 dist = 0;                                                          \
                                                                               \
        for (u64 i = 0; i < map->cap; i++) {                                   \
            if (map->keys[idx].m == -1) {                                      \
                return;                                                        \
            }                                                                  \
            if (map->keys[idx].m < dist) {                                     \
                return;                                                        \
            }                                                                  \
                                                                               \
            if (Sstrcmp(map->keys[idx].k, tempk)) {                            \
                map->size--;                                                   \
                idx = (idx + 1) % map->cap;                                    \
                break;                                                         \
            }                                                                  \
            idx = (idx + 1) % map->cap;                                        \
            dist++;                                                            \
        }                                                                      \
                                                                               \
        u32 prev = (idx + map->cap - 1) % map->cap;                            \
        while (map->keys[idx].m != -1 && map->keys[idx].m > 0) {               \
            map->keys[prev] = map->keys[idx];                                  \
            map->keys[prev].m -= 1;                                            \
                                                                               \
            map->vals[prev] = map->vals[idx];                                  \
            idx = (idx + 1) % map->cap;                                        \
            prev = (idx + map->cap - 1) % map->cap;                            \
        }                                                                      \
                                                                               \
        map->keys[prev].m = -1;                                                \
    }                                                                          \
    void name##Reserve(name *map, u64 size) {                                  \
        u32 oldcap = map->cap;                                                 \
        while (map->cap < size) {                                              \
            map->cap = map->cap ? map->cap * 2 : MIN_CAP;                      \
        }                                                                      \
                                                                               \
        typeof(map->keys) oldkeys = map->keys;                                 \
        typeof(map->vals) oldvals = map->vals;                                 \
                                                                               \
        map->keys = Alloc(map->a, map->cap * sizeof(map->keys[0]));            \
        map->vals = Alloc(map->a, map->cap * sizeof(map->vals[0]));            \
                                                                               \
        memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                \
        for (u32 i = 0; i < oldcap; i++) {                                     \
            if (oldkeys[i].m == -1)                                            \
                continue;                                                      \
                                                                               \
            typeof(map->keys[0].k) tempk = oldkeys[i].k;                       \
            typeof(map->vals[0]) tempv = oldvals[i];                           \
            u32 hash = FNVHash32((u8 *)tempk.data, tempk.len);                 \
                                                                               \
            u32 idx = hash % map->cap;                                         \
            u32 dist = 0;                                                      \
                                                                               \
            for (u64 i = 0; i < map->cap; i++) {                               \
                if (map->keys[idx].m == -1) {                                  \
                    map->keys[idx].k = tempk;                                  \
                    map->keys[idx].m = dist;                                   \
                    map->vals[idx] = tempv;                                    \
                    break;                                                     \
                }                                                              \
                if (map->keys[idx].m < dist) {                                 \
                    typeof(map->keys[0].k) tmpkey = map->keys[idx].k;          \
                    typeof(map->vals[0]) tmpval = map->vals[idx];              \
                    u32 tmpdist = map->keys[idx].m;                            \
                                                                               \
                    map->keys[idx].k = tempk;                                  \
                    map->keys[idx].m = dist;                                   \
                    map->vals[idx] = tempv;                                    \
                                                                               \
                    tempk = tmpkey;                                            \
                    tempv = tmpval;                                            \
                    dist = tmpdist;                                            \
                }                                                              \
                                                                               \
                if (Sstrcmp(map->keys[idx].k, tempk)) {                        \
                    map->vals[idx] = tempv;                                    \
                    break;                                                     \
                }                                                              \
                idx = (idx + 1) % map->cap;                                    \
                dist++;                                                        \
            }                                                                  \
        }                                                                      \
                                                                               \
        Free(map->a, oldkeys, oldcap * sizeof(map->keys[0]));                  \
        Free(map->a, oldvals, oldcap * sizeof(map->vals[0]));                  \
    }                                                                          \
    void name##Clear(name *map) {                                              \
        map->size = 0;                                                         \
        memset(map->keys, -1, map->cap * sizeof(map->keys[0]));                \
    }                                                                          \
    void name##Free(name *map) {                                               \
        Free(map->a, map->keys, map->cap * sizeof(map->keys[0]));              \
        Free(map->a, map->vals, map->cap * sizeof(map->vals[0]));              \
    }

/*
    Memory Pool implementation

    Used for allocating pools
*/

#define PoolDecl(name, item)                                                   \
    typedef struct name {                                                      \
        Allocator a;                                                           \
        item *slots;                                                           \
        u32 *freelist;                                                         \
        u32 cap;                                                               \
        u32 freesize;                                                          \
    } name;                                                                    \
    u32 name##Alloc(name *p);                                                  \
    void name##Free(name *p, u32 idx);                                         \
    void name##Destroy(name *p);

#define PoolImpl(name, item)                                                   \
    u32 name##Alloc(name *p) {                                                 \
        if (p->freesize == 0) {                                                \
            u32 oldcap = p->cap;                                               \
            p->cap = p->cap ? p->cap * 2 : 8;                                  \
            p->slots = Realloc(p->a, p->slots, oldcap * sizeof(item),          \
                               p->cap * sizeof(item));                         \
            p->freelist = Realloc(p->a, p->freelist, oldcap * sizeof(u32),     \
                                  p->cap * sizeof(u32));                       \
            for (u32 i = oldcap; i < p->cap; i++) {                            \
                p->slots[i] = (item){};                                        \
                p->freelist[p->freesize++] = i;                                \
            }                                                                  \
        }                                                                      \
        return p->freelist[--p->freesize];                                     \
    }                                                                          \
    void name##Free(name *p, u32 idx) { p->freelist[p->freesize++] = idx; }    \
    void name##Destroy(name *p) {                                              \
        Free(p->a, p->slots, p->cap * sizeof(item));                           \
        Free(p->a, p->freelist, p->cap * sizeof(u32));                         \
    }
#endif
