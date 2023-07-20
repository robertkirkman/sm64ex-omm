#ifndef OMM_MODELS_CPP_H
#define OMM_MODELS_CPP_H
#ifdef __cplusplus
#if !defined(DYNOS)

extern "C" {
#include "types.h"
}
#include <initializer_list>
#include <string>

//
// Enums
//

enum {
    OMM_MODELS_DATA_TYPE_NONE = 0,
    OMM_MODELS_DATA_TYPE_LIGHT,
    OMM_MODELS_DATA_TYPE_TEXTURE,
    OMM_MODELS_DATA_TYPE_VERTEX,
    OMM_MODELS_DATA_TYPE_DISPLAY_LIST,
    OMM_MODELS_DATA_TYPE_GEO_LAYOUT,
    OMM_MODELS_DATA_TYPE_ANIMATION_VALUE,
    OMM_MODELS_DATA_TYPE_ANIMATION_INDEX,
    OMM_MODELS_DATA_TYPE_ANIMATION,
    OMM_MODELS_DATA_TYPE_ANIMATION_TABLE,
};

//
// A vector-like array, implemented to be processed really fast but cannot handle C++ complex classes like std::string
//

template <typename T>
class Array {
public:
    inline Array() : buffer(NULL), count(0), capacity(0) {
    }

    inline Array(const std::initializer_list<T> &list) : buffer(NULL), count(0), capacity(0) {
        resize(list.size());
        memcpy(buffer, list.begin(), count * sizeof(T));
    }

    inline Array(const T *begin, const T *end) : buffer(NULL), count(0), capacity(0) {
        resize(end - begin);
        memcpy(buffer, begin, count * sizeof(T));
    }

    inline Array(const Array &other) : buffer(NULL), count(0), capacity(0) {
        resize(other.count);
        memcpy(buffer, other.buffer, count * sizeof(T));
    }

    inline void operator=(const Array &other) {
        resize(other.count);
        memcpy(buffer, other.buffer, count * sizeof(T));
    }

    inline ~Array() {
        clear();
    }

public:
    void resize(s32 new_count) {
        if (new_count > capacity) {
            capacity = MAX(new_count, MAX(16, capacity * 2));
            T *new_buffer = (T *) calloc(capacity, sizeof(T));
            if (buffer) {
                memcpy(new_buffer, buffer, count * sizeof(T));
                free(buffer);
            }
            buffer = new_buffer;
        }
        count = new_count;
    }

    void add(const T& item) {
        resize(count + 1);
        buffer[count - 1] = item;
    }

    void clear() {
        if (buffer) free(buffer);
        buffer = NULL;
        count = 0;
        capacity = 0;
    }

public:
    inline const T *begin() const { return buffer; }
    inline const T *end() const { return buffer + count; }
    inline T *begin() { return buffer; }
    inline T *end() { return buffer + count; }

    inline const T &operator[](s32 index) const { return buffer[index]; }
    inline T &operator[](s32 index) { return buffer[index]; }

    inline s32 get_count() const { return count; }
    inline bool is_empty() const { return count == 0; }

public:
    void read(FILE *file) {
        s32 length = 0; fread(&length, sizeof(s32), 1, file);
        resize(length);
        fread(buffer, sizeof(T), length, file);
    }

private:
    T *buffer;
    s32 count;
    s32 capacity;
};

//
// A fixed-size string that doesn't require heap memory allocation
//

#define STRING_SIZE 127
class String {
public:
    inline String() : count(0) {
        buffer[0] = 0;
    }

    inline String(const char *str) : count(0) {
        if (str) {
            u64 length = strlen(str);
            count = MIN(length, STRING_SIZE - 1);
            memcpy(buffer, str, length);
        }
        buffer[count] = 0;
    }

    template <typename... Args>
    inline String(const char *fmt, Args... args) : count(0) {
        snprintf(buffer, STRING_SIZE, fmt, args...);
        count = (u8) strlen(buffer);
        buffer[count] = 0;
    }

    inline String(const String &other) : count(0) {
        count = other.count;
        memcpy(buffer, other.buffer, count);
        buffer[count] = 0;
    }

    inline void operator=(const String &other) {
        count = other.count;
        memcpy(buffer, other.buffer, count);
        buffer[count] = 0;
    }

public:
    void add(char c) {
        if (count == STRING_SIZE - 1) return;
        buffer[count++] = c;
        buffer[count] = 0;
    }

    void clear() {
        count = 0;
        buffer[0] = 0;
    }

public:
    inline const char *begin() const { return buffer; }
    inline const char *end() const { return buffer + count; }
    inline char *begin() { return buffer; }
    inline char *end() { return buffer + count; }

    inline const char &operator[](s32 index) const { return buffer[index]; }
    inline char &operator[](s32 index) { return buffer[index]; }

    inline s32 get_length() const { return (s32) count; }
    inline bool is_empty() const { return count == 0; }

public:
    bool operator==(const char *str) const {
        if (strlen(str) != count) return false;
        for (u8 i = 0; i != count; ++i) {
            if (str[i] != buffer[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator==(const String &other) const {
        if (other.count != count) return false;
        for (u8 i = 0; i != count; ++i) {
            if (other.buffer[i] != buffer[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const char *str) const {
        if (strlen(str) != count) return true;
        for (u8 i = 0; i != count; ++i) {
            if (str[i] != buffer[i]) {
                return true;
            }
        }
        return false;
    }

    bool operator!=(const String &other) const {
        if (other.count != count) return true;
        for (u8 i = 0; i != count; ++i) {
            if (other.buffer[i] != buffer[i]) {
                return true;
            }
        }
        return false;
    }

public:
    void read(FILE *file) {
        fread(&count, sizeof(u8), 1, file);
        fread(buffer, sizeof(char), count, file);
        buffer[count] = 0;
    }

private:
    char buffer[STRING_SIZE];
    u8 count;
};
static_assert(sizeof(String) == (STRING_SIZE + 1), "sizeof(String) must be (STRING_SIZE + 1)");

//
// Types
//

template <typename U, typename V>
using Pair = std::pair<U, V>;

typedef std::string SysPath;

class NoCopy {
protected:
    NoCopy() {}
    ~NoCopy() {}
private:
    NoCopy(const NoCopy &) = delete;
    void operator=(const NoCopy &) = delete;
};

struct TexData : NoCopy {
    Array<u8> png_data;
    Array<u8> raw_data;
    s32 raw_width  = -1;
    s32 raw_height = -1;
    bool uploaded  = false;
};

struct AnimData : NoCopy {
    s16 flags = 0;
    s16 anim_y_trans_div = 0;
    s16 start_frame = 0;
    s16 loop_start = 0;
    s16 loop_end = 0;
    s16 bone_count = 0;
    Pair<String, Array<s16>> values;
    Pair<String, Array<u16>> index;
    u32 length = 0;
};

template <typename T>
struct DataNode : NoCopy {
    String name;
    T* data = NULL;
    u32 size = 0;
};
template <typename T>
using DataNodes = Array<DataNode<T>*>;

template <typename T>
using AnimBuffer = Pair<String, Array<T>>;
struct GfxData : NoCopy {
    DataNodes<Lights1> lights;
    DataNodes<TexData> textures;
    DataNodes<Vtx> vertices;
    DataNodes<Gfx> display_lists;
    DataNodes<GeoLayout> geo_layouts;
    DataNodes<AnimData> animations;
    Array<Pair<String, void *>> animation_table;
    SysPath pack_folder;
};

struct ActorGfx {
    GfxData *gfx_data = NULL;
    GraphNode *graph_node = NULL;
    s32 pack_index = 0;
};

//
// Functions
//

s32 omm_models_get_actor_count();
Array<ActorGfx> &omm_models_get_actor_list();
const char *omm_models_get_actor_name(s32 index);
const void *omm_models_get_actor_layout(s32 index);
s32 omm_models_get_actor_index(const void *geo_layout);
void *omm_models_get_func_pointer(s32 index);
Array<const char *> &omm_models_get_packs();
GfxData *omm_models_load_from_binary(const SysPath &pack_folder, const char *actor_name);

#endif
#endif
#endif
