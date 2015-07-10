#pragma once

#include "types.h"
#include "variant.h"
#include "binary.h"
#include "paramstack.h"
#include "array.h"
#include "container.h"

struct fake;

#define GET_CMD(fb, pos) fb.m_buff[pos]

#define GET_CMD_LINENO(fb, pos) (pos >= 0 && pos < (int)fb.m_lineno_size) ? fb.m_lineno_buff[pos] : (fb.m_lineno_size > 0 ? fb.m_lineno_buff[fb.m_lineno_size - 1] : 0)

#define GET_CONST(v, fb, pos) \
    assert(pos >= 0 && pos < (int)fb.m_const_list_num);\
    v = &fb.m_const_list[pos];

#define GET_CONTAINER(v, s, fb, pos) v = get_container_variant(s, fb, pos)

#define GET_STACK(v, s, pos) \
	assert(pos >= 0 && pos < (int)ARRAY_MAX_SIZE((s).m_stack_variant_list));\
    v = &ARRAY_GET((s).m_stack_variant_list, pos);

#define SET_STACK(v, s, pos) \
	assert(pos >= 0 && pos < (int)ARRAY_MAX_SIZE((s).m_stack_variant_list));\
    ARRAY_GET((s).m_stack_variant_list, pos) = *v;
    
#define GET_VARIANT(s, fb, v, pos) \
    GET_VARIANT_BY_CMD(s, fb, v, GET_CMD(fb, pos))
    
#define GET_VARIANT_BY_CMD(s, fb, v, cmd) \
    command v##_cmd = cmd;\
    assert (COMMAND_TYPE(v##_cmd) == COMMAND_ADDR);\
    int v##_addrtype = ADDR_TYPE(COMMAND_CODE(v##_cmd));\
    int v##_addrpos = ADDR_POS(COMMAND_CODE(v##_cmd));\
    assert (v##_addrtype == ADDR_STACK || v##_addrtype == ADDR_CONST || v##_addrtype == ADDR_CONTAINER);\
    if (v##_addrtype == ADDR_STACK)\
    {\
        GET_STACK(v, s, (v##_addrpos));\
    }\
    else if (v##_addrtype == ADDR_CONST)\
    {\
		GET_CONST(v, fb, (v##_addrpos)); \
    }\
    else if (v##_addrtype == ADDR_CONTAINER)\
    {\
		GET_CONTAINER(v, s, fb, (v##_addrpos)); \
		if (!v) \
		{ \
		    err = true; \
		    break;\
		} \
    }\
    else\
    {\
		v = 0;\
        assert(0);\
        FKERR("next_assign assignaddrtype cannot be %d %d", v##_addrtype, v##_addrpos);\
        err = true;\
        break;\
    }

#define LOG_VARIANT(s, fb, pos, prefix) \
    FKLOG(prefix " variant %d %d", \
        ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, pos))),\
        ADDR_POS(COMMAND_CODE(GET_CMD(fb, pos))));

#define MATH_OPER(s, fb, oper) \
	const variant * left = 0;\
	LOG_VARIANT(s, fb, (s).m_pos, "left");\
    GET_VARIANT(s, fb, left, (s).m_pos);\
    (s).m_pos++;\
    \
	const variant * right = 0;\
	LOG_VARIANT(s, fb, (s).m_pos, "right");\
    GET_VARIANT(s, fb, right, (s).m_pos);\
    (s).m_pos++;\
    \
	variant * dest;\
	if (UNLIKE(!(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, (s).m_pos))) == ADDR_STACK))) \
	{ \
	    err = true; \
	    seterror(fk, efk_run_inter_error, "interpreter math oper error, dest is not stack, type %s", vartypetostring((int)ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, (s).m_pos))))); \
	    break; \
	} \
    LOG_VARIANT(s, fb, (s).m_pos, "dest");\
	GET_VARIANT(s, fb, dest, (s).m_pos);\
    (s).m_pos++;\
    \
	FKLOG("math left %s right %s", (vartostring(left)).c_str(), (vartostring(right)).c_str());\
    \
    V_##oper(dest, left, right);\
    \
    FKLOG("math %s %s", OpCodeStr(code), (vartostring(dest)).c_str());
 
#define MATH_ASSIGN_OPER(s, fb, oper) \
	variant * var = 0;\
    if (UNLIKE(!(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, (s).m_pos))) == ADDR_STACK || ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, (s).m_pos))) == ADDR_CONTAINER)))\
    { \
	    err = true; \
	    seterror(fk, efk_run_inter_error, "interpreter math assign oper error, dest is not stack or container, type %s", vartypetostring((int)ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, (s).m_pos))))); \
	    break; \
    } \
	LOG_VARIANT(s, fb, (s).m_pos, "var");\
    GET_VARIANT(s, fb, var, (s).m_pos);\
    (s).m_pos++;\
    \
	const variant * value = 0;\
	LOG_VARIANT(s, fb, (s).m_pos, "value");\
    GET_VARIANT(s, fb, value, (s).m_pos);\
    (s).m_pos++;\
    \
	FKLOG("math var %s value %s", (vartostring(var)).c_str(), (vartostring(value)).c_str());\
    \
    V_##oper(var, var, value);\
    \
    FKLOG("math %s %s", OpCodeStr(code), (vartostring(var)).c_str());

#define MATH_SINGLE_OPER(s, fb, oper) \
	const variant * left = 0;\
	LOG_VARIANT(s, fb, (s).m_pos, "left");\
    GET_VARIANT(s, fb, left, (s).m_pos);\
    (s).m_pos++;\
    \
	variant * dest;\
	if (UNLIKE(!(ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, (s).m_pos))) == ADDR_STACK))) \
	{ \
	    err = true; \
	    seterror(fk, efk_run_inter_error, "interpreter math oper error, dest is not stack, type %s", vartypetostring((int)ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, (s).m_pos))))); \
	    break; \
	} \
    LOG_VARIANT(s, fb, (s).m_pos, "dest");\
	GET_VARIANT(s, fb, dest, (s).m_pos);\
    (s).m_pos++;\
    \
	FKLOG("single math left %s", (vartostring(left)).c_str());\
    \
    V_##oper(dest, left);\
    \
    FKLOG("single math %s %s", OpCodeStr(code), (vartostring(dest)).c_str());
 
struct stack
{
    fake * m_fk;
    // 函数二进制
    const func_binary * m_fb;
    // 当前执行位置
    int m_pos;
    // 当前栈上的变量
	array<variant> m_stack_variant_list;
	// 调用的函数返回本栈的个数和位置
	int m_retnum;
	int m_retvpos[MAX_FAKE_RETURN_NUM];
	// 调用开始时间
	uint32_t m_calltime;
};

#define STACK_DELETE(s) ARRAY_DELETE((s).m_stack_variant_list)
#define STACK_INI(s, fk, fb) (s).m_fk = fk;\
    (s).m_fb = fb;\
    ARRAY_INI((s).m_stack_variant_list, fk);\
    (s).m_pos = 0

struct processor;
struct interpreter
{
public:
    force_inline bool isend() const
    {
        return m_isend;
    }
    
    void call(const variant & func);

    variant * get_container_variant(stack & s, const func_binary & fb, int conpos)
    {
        variant * v = 0;
        assert(conpos >= 0 && conpos < (int)fb.m_container_addr_list_num);
        const container_addr & ca = fb.m_container_addr_list[conpos];
        bool err = false;
	    USE(err);
        variant * conv = 0;
        do {GET_VARIANT_BY_CMD(s, fb, conv, ca.con);}while(0);
        const variant * keyv = 0;
        do {GET_VARIANT_BY_CMD(s, fb, keyv, ca.key);}while(0);

        if (UNLIKE(err))
        {   
            return 0;
        }
    
        if (UNLIKE(!(conv->type == variant::ARRAY || conv->type == variant::MAP)))
        {
    	    seterror(m_fk, efk_run_inter_error, "interpreter get container variant fail, container type error, type %s", vartypetostring(conv->type));
    	    return 0;
        }
        
        if (conv->type == variant::MAP)
        {
            v = con_map_get(m_fk, conv->data.vm, keyv);
        }
        else if (conv->type == variant::ARRAY)
        {
            v = con_array_get(m_fk, conv->data.va, keyv);
        }

        return v;
    }

    force_inline const variant & getret() const
    {
        return m_ret[0];
    }

    force_inline const char * get_running_func_name() const
    {
		return FUNC_BINARY_NAME(*(m_cur_stack->m_fb));
    }

    force_inline const char * get_running_file_name() const
    {
		return FUNC_BINARY_FILENAME(*(m_cur_stack->m_fb));
    }
    
    force_inline int get_running_file_line() const
    {
        const func_binary & fb = *m_cur_stack->m_fb;
        int pos = m_cur_stack->m_pos;
        if (pos < 0 || pos >= (int)FUNC_BINARY_LINENO_SIZE(fb))
        {
		    return 0;
        }
        return GET_CMD_LINENO(fb, pos);
    }
    
    const char * get_running_call_stack() const;
	
    int run(int cmdnum);

public:
    fake * m_fk;
    bool m_isend;
	variant m_ret[MAX_FAKE_RETURN_NUM];
	stack * m_cur_stack;
	array<stack> m_stack_list;
	processor * m_processor;
	uint32_t m_wakeuptime;
	uint32_t m_yieldtime;
	bool m_sleeping;
};

#define INTER_DELETE(inter) \
	for (int i = 0; i < (int)ARRAY_MAX_SIZE((inter).m_stack_list); i++)\
	{\
	    STACK_DELETE(ARRAY_GET((inter).m_stack_list, i));\
	}\
	ARRAY_DELETE((inter).m_stack_list)
	
#define INTER_INI(inter, fk) (inter).m_fk = fk;\
    ARRAY_INI((inter).m_stack_list, fk)
    
#define INTER_CLEAR(inter) (inter).m_isend = false;\
    (inter).m_cur_stack = 0;\
    ARRAY_CLEAR((inter).m_stack_list);\
    (inter).m_sleeping = false;
    
#define INTER_SET_PRO(inter, pro) (inter).m_processor = pro
