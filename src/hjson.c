#include "cross_util.c"


// json types
#define HJson_Invalid (0)
#define HJson_False   (1 << 0)
#define HJson_True    (1 << 1)
#define HJson_Number  (1 << 2)
#define HJson_String  (1 << 3)
#define HJson_Array   (1 << 4)
#define HJson_Object  (1 << 5)
#define HJson_Null    (1 << 6)
#define HJson_Refrence (3 << 4)


#define HJson_NewAlloc() (HJson*)calloc(1, sizeof(HJson))
#define HJson_MoveBufEndinput_buf(input_buf) ((*input_buf) += strlen(*input_buf))


#define HJson_indentLevel 4
#define HJson_printWidth 112


// HJson structure
typedef struct HJson {
    // next for walking on array/object linked list
    struct HJson *next; // or only child as array?

    // Item name string for object childs
    u8* name;

    u8 type;

    union {
        f64 number;
        u8* string;
        // An array or object item will have a child pointer pointing to a chain of the items in the array/object.
        struct HJson *child;
    };
} HJson;

typedef struct {
    void* allocated;
} SaveAllocation;

typedef u8* ParseBuffer;


// Lib headers Api
HJson* HJson_parse(u8* src);
void HJson_free(HJson* json);
u8* HJson_stringify(const HJson* json);
u32 HJson_GetArrayLen(const HJson* array);


// Api

bool HJson_IsFalse(const HJson* json) {
    return json && json->type == HJson_False;
}
bool HJson_IsTrue(const HJson* json) {
    return json && json->type == HJson_True;
}
bool HJson_IsBool(const HJson* json) {
    return json && (json->type & HJson_False | HJson_True);
}
bool HJson_IsNumber(const HJson* json) {
    return json && json->type == HJson_Number;
}
bool HJson_IsString(const HJson* json) {
    return json && json->type == HJson_String;
}
bool HJson_IsArray(const HJson* json) {
    return json && json->type == HJson_Array;
}
bool HJson_IsObject(const HJson* json) {
    return json && json->type == HJson_Object;
}
bool HJson_IsHaveChild(const HJson* json) {
    return json && (json->type & (HJson_Array | HJson_Object)) && json->child;
}

u32 HJson_ArrayLen(const HJson* array) {
    HJson *child = NULL;
    u32 size = 0;

    if(array == NULL) {
        return 0;
    }

    child = array->child;

    for(HJson *child = array->child; child != NULL; child = child->next) {
        size++;
    }

    return size;
}



HJson* HJson_ArrayAt(const HJson* array, u32 index) {
    if(array == NULL) {
        return NULL;
    }

    HJson* current_child = array->child;
    while((current_child != NULL) && (index > 0)) {
        index--;
        current_child = current_child->next;
    }

    return current_child;
}


HJson* HJson_ObjectAtKey(const HJson* object, const u8* name) {
    if ((object == NULL) || (name == NULL)) {
        return NULL;
    }

    HJson *current_element = object->child;
    while(current_element != NULL && current_element->name != NULL && strcmp(name, current_element->name) != 0) {
        current_element = current_element->next;
    }

    return current_element;
}

// Utils
// HJson* HJson_CreateNumber(f64 num);
// HJson* HJson_CreateString(u8* str);

static HJson* parse_value(ParseBuffer* const input_buf);



// Helpers
// inline void skipWhitespace(ParseBuffer* const str);
static inline void skipWhitespace(ParseBuffer* const input_buf) {
    while(**input_buf != '\0' && (**input_buf == ' ' || **input_buf == '\n')) {
        (*input_buf)++;
    }
}


// HJson


// inline HJson* parse_number(ParseBuffer* const input_buf);
static inline HJson* parse_number(ParseBuffer* const input_buf) {
    u8 len = 1;
    u8 c;
    while(c = (*input_buf)[len]) {
        switch(c) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case '.':
            case 'e':
                len++;
                break;
            default:
                goto loop_end;
        }
    }

loop_end:

    u8 buf[len + 1];

    memcpy(buf, *input_buf, len);
    buf[len] = '\0';

    *input_buf += len;

    HJson* node = HJson_NewAlloc();
    node->type = HJson_Number;
    node->number = atof(buf);
    return node;
}


// Need validated string
// TODO utf chars like \u3243
// TODO src_i => input_buf inline char* parse_string(ParseBuffer* const input_buf);
// inline char* parse_string(ParseBuffer* const input_buf);
static inline u8* parse_string(ParseBuffer* const input_buf) {
    (*input_buf)++;
    // allocate bigger? and realocate?
    u32 skipped_bytes = 0;
    u32 src_len = 0;
    while((*input_buf)[src_len] != '"') {
        if((*input_buf)[src_len] == '\\') {
            skipped_bytes++;
            src_len++;
        }
        src_len++;
    }

    u8* out = malloc(src_len - skipped_bytes + 1);

    u8* out_ptr = out;
    u8* src_end = (*input_buf) + src_len;
    while(*input_buf < src_end) {
        if(**input_buf != '\\') {
            *(out_ptr++) = *(*input_buf)++;
        } else { // escape sequence
            switch ((*input_buf)[1]) {
                case 'n':
                    *(out_ptr++) = '\n';
                    break;
                case 't':
                    *(out_ptr++) = '\t';
                    break;
                case '"':
                    *(out_ptr++) = '"';
                    break;
                case '\\':
                    *(out_ptr++) = '\\';
                    break;
                case 'u':
                    printf("Error at parse_string (HJson): unicode not supported yet!\n");
                    HJson_MoveBufEndinput_buf(input_buf);
                    return NULL;

                default:
                    printf("Error at parse_string (HJson): invalid escape char: %c\n", (*input_buf)[1]);
                    HJson_MoveBufEndinput_buf(input_buf);
                    return NULL;
            }
            (*input_buf) += 2;
        }
    }

    (*input_buf)++;
    (*out_ptr) = '\0';

    return out;
}


// // cJSON_parse_string
// // Parse the input text into an unescaped cinput, and populate item.
// static inline u8* _parse_string(ParseBuffer* const input_buf) {
//     (*input_buf)++;

//     const u8 *input_pointer = *input_buf;
//     const u8 *input_end = *input_buf;
//     u8 *output_pointer = NULL;
//     u8 *output = NULL;
//     u32 offset = 0;

//     {
//         // calculate approximate size of the output (overestimate)
//         u32 allocation_length = 0;
//         u32 skipped_bytes = 0;
//         while (*input_end != '"') {
//             /* is escape sequence */
//             if (input_end[0] == '\\') {
//                 skipped_bytes++;
//                 input_end++;
//             }
//             input_end++;
//         }
//         // if (((u32)(input_end - input_buf->content) >= input_buf->length) || (*input_end != '\"')) {
//         //     goto fail; /* string ended unexpectedly */
//         // }

//         /* This is at most how much we need for the output */
//         allocation_length = (u32)(input_end - (const u8 *)*input_buf) - skipped_bytes;
//         output = malloc(allocation_length + sizeof(""));
//         if (output == NULL) {
//             return NULL; /* allocation failure */
//         }
//     }

//     output_pointer = output;
//     /* loop through the string literal */
//     while (input_pointer < input_end) {
//         if (*input_pointer != '\\') {
//             *output_pointer++ = *input_pointer++;
//         }
//         /* escape sequence */
//         else {
//             u8 sequence_length = 2;
//             if ((input_end - input_pointer) < 1) {
//                 return NULL;
//             }

//             switch (input_pointer[1]) {
//                 case 'b':
//                     *output_pointer++ = '\b';
//                     break;
//                 case 'f':
//                     *output_pointer++ = '\f';
//                     break;
//                 case 'n':
//                     *output_pointer++ = '\n';
//                     break;
//                 case 'r':
//                     *output_pointer++ = '\r';
//                     break;
//                 case 't':
//                     *output_pointer++ = '\t';
//                     break;
//                 case '\"':
//                 case '\\':
//                 case '/':
//                     *output_pointer++ = input_pointer[1];
//                     break;

//                 // UTF-16 literal
//                 case 'u':
//                     // sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
//                     // if (sequence_length == 0) {
//                     //     /* failed to convert UTF16-literal to UTF-8 */
//                     //     goto fail;
//                     // }
//                     break;

//                 default:
//                     return NULL;
//             }
//             input_pointer += sequence_length;
//         }
//     }

//     // zero terminate the output
//     *output_pointer = '\0';


//     *input_buf += input_end - *input_buf;
//     // input_buf->offset = input_end - input_buf->content;
//     // input_buf->offset++;

//     return output;
// }



// TODO (array/object).child as array with last index HJson_Invalid
// inline HJson* parse_array(ParseBuffer* const input_buf);
static inline HJson* parse_array(ParseBuffer* const input_buf) {
    (*input_buf)++; // skip '['
    HJson* array = HJson_NewAlloc();
    array->type = HJson_Array;
    skipWhitespace(input_buf);
    HJson* prev = NULL;
    HJson* cur_item = NULL;

    while(**input_buf != ']' && **input_buf != '\0') {
        cur_item = parse_value(input_buf);
        if(prev == NULL) {
            array->child = cur_item;
        } else {
            prev->next = cur_item;
        }

        skipWhitespace(input_buf);
        if(**input_buf == ',') (*input_buf)++;
        skipWhitespace(input_buf);
        prev = cur_item;
    }

    (*input_buf)++; // skip ']'

    return array;
}


// TODO see parse_array TODO
// inline HJson* parse_object(ParseBuffer* const input_buf);
static inline HJson* parse_object(ParseBuffer* const input_buf) {
    (*input_buf)++; // skip '{'
    HJson* object = HJson_NewAlloc();
    object->type = HJson_Object;
    skipWhitespace(input_buf);
    HJson* prev = NULL;
    HJson* cur_item = NULL;

    while(**input_buf != '}' && **input_buf != '\0') {
        u8* name = parse_string(input_buf);
        (*input_buf)++;
        skipWhitespace(input_buf);
        cur_item = parse_value(input_buf);
        cur_item->name = name;
        // allocate string name
        if(prev == NULL) {
            object->child = cur_item;
        } else {
            prev->next = cur_item;
        }

        skipWhitespace(input_buf);
        if(**input_buf == ',') (*input_buf)++;
        skipWhitespace(input_buf);
        prev = cur_item;
    }

    (*input_buf)++; // skip '}'

    return object;
}


static HJson* parse_value(ParseBuffer* const input_buf) {
    skipWhitespace(input_buf);

    // string
    if(**input_buf == '"') {
        HJson* node = HJson_NewAlloc();
        node->type = HJson_String;
        node->string = parse_string(input_buf);
        return node;
    }

    // object
    if(**input_buf == '{') {
        // TODO
        return parse_object(input_buf);
    }

    // array
    if(**input_buf == '[') {
        return parse_array(input_buf);
    }

    // number
    if('0' <= **input_buf && **input_buf <= '9' || **input_buf == '-') {
        return parse_number(input_buf);
    }

    // true
    if(strncmp((const u8*)(*input_buf), "true", 4) == 0) {
        (*input_buf) += 4;
        HJson* node = HJson_NewAlloc();
        node->type = HJson_True;
        return node;
    }

    // false
    if(strncmp((const u8*)(*input_buf), "false", 5) == 0) {
        (*input_buf) += 5;
        HJson* node = HJson_NewAlloc();
        node->type = HJson_False;
        return node;
    }

    // null
    if(strncmp((const u8*)(*input_buf), "null", 4) == 0) {
        (*input_buf) += 4;
        HJson* node = HJson_NewAlloc();
        node->type = HJson_False;
        return node;
    } else {
        HJson* node = HJson_NewAlloc();
        node->type = HJson_Invalid;
        HJson_MoveBufEndinput_buf(input_buf);
        return node;
    }
}


// HJson_stringify

static u8* HJson_stringify_value(const HJson* json, u8 indent) {
    // false
    if(json->type == HJson_False) {
        u8* buf = malloc(6);
        strcpy(buf, "false");
        return buf;
    }

    // true
    if(json->type == HJson_True) {
        u8* buf = malloc(5);
        strcpy(buf, "true");
        return buf;
    }

    // null
    if(json->type == HJson_Null) {
        u8* buf = malloc(5);
        strcpy(buf, "null");
        return buf;
    }

    // number
    if(json->type == HJson_Number) {
        u8* buf = malloc(32);
        snprintf(buf, 32, "%f", json->number);
        u8 *p = buf + strlen(buf) - 1;
        while (*p == '0') *p-- = '\0';

        // remove trailing dot if needed
        if (*p == '.') *p = '\0';
        return buf;
    }

    // string
    if(json->type == HJson_String) {
        u32 len = strlen(json->string);
        u8* buf = malloc(len + 3);
        for(u32 i = 0; i < len; i++) {
            buf[i + 1] = json->string[i];
        }
        buf[0] = buf[len + 1] = '"';
        buf[len + 2] = '\0';
        return buf;
    }
    
    // array
    if(json->type == HJson_Array) {
        if(json->child == NULL) {
            u8* buf = malloc(3);
            strcpy(buf, "[]");
            return buf;
        }

        u32 len = 1;
        u8* buf = malloc(len + 2);
        buf[0] = '[';
        for(HJson* child = json->child; child != NULL; child = child->next) {
            u8* value = HJson_stringify_value(child, indent);
            u32 len2 = strlen(value);
            buf = (u8*)realloc(buf, len + len2 + 2);

            // TODO if to long add \n and indent => realocate

            // Value
            for(u32 j = 0; j < len2; j++) {
                buf[len + j] = value[j];
            }
            buf[len + len2] = ',';
            buf[len + len2 + 1] = ' ';
            len += len2 + 2;

            // Free
            free(value);
        }

        buf[len - 2] = ']';
        buf[len - 1] = '\0';
        return buf;
    }

    // object
    if(json->type == HJson_Object) {
        if(json->child == NULL) {
            u8* buf = malloc(3);
            strcpy(buf, "{}");
            return buf;
        }

        u32 len = 2;
        u8* buf = malloc(len + indent);
        buf[0] = '{';
        buf[1] = '\n';
        for(HJson* child = json->child; child != NULL; child = child->next) {
            u8* name = child->name;
            u32 name_len = strlen(name);
            u8* value = HJson_stringify_value(child, indent + HJson_indentLevel);
            u32 value_len = strlen(value);
            buf = (u8*)realloc(buf, len + indent + value_len + name_len +  2 + 2 + 2 + 1);

            // Indent
            memset(&buf[len], ' ', indent);
            len += indent;

            // Name
            buf[len] = '"';
            for(u32 j = 0; j < name_len; j++) {
                buf[len + j + 1] = name[j];
            }
            buf[len + name_len + 1] = '"';
            buf[len + name_len + 2] = ':';
            buf[len + name_len + 3] = ' ';

            // Value
            for(u32 j = 0; j < value_len; j++) {
                buf[len + name_len + 4 + j] = value[j];
            }
            buf[len + name_len + value_len + 4] = ',';
            buf[len + name_len + value_len + 5] = ' ';
            buf[len + name_len + value_len + 6] = '\n';
            len += name_len + value_len + 7;

            // Free
            free(value);
        }

        indent -= HJson_indentLevel;
        buf[len - 3] = '\n';
        memset(&buf[len - 2], ' ', indent);
        buf[len + indent - 2] = '}';
        buf[len + indent - 1] = '\0';
        return buf;
    }
}


// Init allocate in memory
HJson* HJson_parse(u8* src) {
    return parse_value(&src);
}


// Stringify allocate json string
u8* HJson_stringify(const HJson* json) {
    return HJson_stringify_value(json, HJson_indentLevel);
}


// Free memory
void HJson_free(HJson* json) {
    if(json->name) free(json->name);
    if(json->type & HJson_Refrence && json->child) HJson_free(json->child);
    if(json->type == HJson_String) free(json->string);
    if(json->next) {
        HJson_free(json->next);
        free(json->next);
    }
}
