#ifndef LUNAR_IR_HPP
#define LUNAR_IR_HPP

#include "lunar_common.hpp"
#include "lunar_string.hpp"

#include <vector>
#include <unordered_map>

/*
 * -----------------------------------------------------------------------------
 *
 * IR  := TOP*
 * TOP := FUNC | STRUCT | UNION | DATA | GLOBAL | EXPORT | IMPORT
 * STATEMENT := LET | COND | WHILE | BREAK | SELECT | RETURN | SCHEDULE
 * GLOBAL := ( global ( ( TYPE (IDENTIFIER+) EXPRIDENTLIT )+ ) )
 * EXPORT := ( export IDENTIFIER+ )
 * IMPORT := ( import STR32+ )
 *
 * -----------------------------------------------------------------------------
 *
 * TYPE  := TYPE0 | ( OWNERSHIP TYPE0 )
 * TYPE0 := SCALAR | VECTOR | STRING | BINARY | LIST | STRUCT | DICT | SET | DATA |
 *          FUNCTYPE | RSTREAM | WSTREAM | PTR | UNION | PARSEC | IDENTIFIER
 *
 * OWNERSHIP := unique | shared | ref
 *
 * SCALAR     := SCALARTYPE INITSCALAR | SCALARTYPE
 * SCALARTYPE := bool | u64 | s64 | u32 | s32 | u16 | s16 | u8 | s8 | double | float | char | ATOM
 *
 * ATOM := `IDENTIFIER
 *
 * VECTOR := ( vector TYPE SIZE )
 * SIZE := An integer greater than or equal to 0
 *
 * STRING := string
 *
 * BINARY := binary
 *
 * LIST := ( list TYPE )
 *
 * STRUCT := ( struct IDENTIFIER? ( TYPE IDENTIFIER )+ )
 *
 * DATA := ( data IDENTIFIER? ( TYPE IDENTIFIER )+ )
 *
 * UNION := ( union IDENTIFIER? ( TYPE IDENTIFIER )+ )
 *
 * DICT := ( dict TYPE TYPE )
 *
 * SET := ( set TYPE )
 *
 * FUNCTYPE := ( func ( TYPE* ) ( TYPE* ) )
 *
 * RSTREAM := ( rstrm TYPE )
 * WSTREAM := ( wstrm TYPE )
 *
 * PTR := (ptr TYPE ) | ( ptr PTR )
 *
 * PARSEC := parsec
 *
 * -----------------------------------------------------------------------------
 *
 * FUNC := ( defun IDENTIFIER ( TYPE* ) ( TYPE IDENTIFIER )* STEXPR* )
 *
 * -----------------------------------------------------------------------------
 *
 * STEXPR := STATMENT | EXPR
 *
 * LET := ( let ( (TYPE IDENTIFIER)+ EXPRIDENTLIT )+ STEXPR\* )
 *
 * COND := ( cond ( EXPRIDENTLIT STEXPR* )+ ( else STEXPR* )? )
 *
 * WHILE := ( while EXPRIDENTLIT STEXPR* )
 *
 * SELECT := ( select ( EXPRIDENT STEXPR*)* ( timeout EXPRIDENTLIT STEXPR* )? )
 *
 * BREAK := ( break )
 *
 * RETURN := ( return EXPRIDENTLIT* )
 *
 * SCHEDULE := ( schedule )
 *
 * -----------------------------------------------------------------------------
 *
 * EXPRIDENT := EXPR | IDENTIFIER
 *
 * SPAWN := ( spawn EXPRIDENTLIT EXPRIDENT EXPRIDENTLIT )
 *
 * THREAD := ( thread EXPRIDENTLIT TYPE EXPRIDENTLIT EXPRIDENT EXPRIDENTLIT )
 *
 * COPY := ( copy! EXPRIDENT EXPRIDENTLIT )
 *
 * ASSOC := ( assoc! EXPRIDENT EXPRIDENT )
 *
 * INCCNT := ( inccnt EXPRIDENT )
 * DECCNT := ( deccnt EXPRIDENT )
 *
 * IF := ( if EXPRIDENTLIT EXPRIDENTLIT EXPRIDENTLIT )
 *
 * LAMBDA := ( lambda ( TYPE* ) ( TYPE IDENTIFIER )* STEXPR* )
 *
 * NEW := ( new TYPE )
 *
 * CALLFUNC := ( EXPRIDENT EXPRIDENTLIT* )
 *
 * TYPEOF := ( type TYPE0 IDENTIFIER )
 *
 * MKSTREAM := ( mkstream TYPE SIZE )
 *
 * PUSH := ( push! EXPRIDENTLIT )
 *
 * POP := ( pop! EXPRIDENTLIT )
 * 
 * SPIN_LOCK_INIT := ( spin_lock_init EXPRIDENT )
 * SPIN_LOCK      := ( spin_lock EXPRIDENT )
 * SPIN_TRY_LOCK  := ( spin_try_lock EXPRIDENT )
 * SPIN_UNLOCK    := ( spin_unlock EXPRIDENT )
 *
 * PARSECINIT   := ( parser_init string EXPRIDENT ) | ( parsec_init binary EXPRIDENT )
 * PARSEC       := ( parse EXPRIDENT PARSECOPS EXPRIDENTLIT* )
 * PARSECOPS    := PARSECCHAR | PARSECMANY | PARSECMANY1 | PARSECTRY | PARSECTRYEND | PARSECLA | PARSECLAEND | PARSECDIGIT | PARSECHEX | PARSECOCT | PARSECSPACE | PARSECSATIS | PARSECSTR 
 * PARSECCHAR   := character
 * PARSECTRY    := try
 * PARSERTRYEND := try_end
 * PARSECLA     := look_ahead
 * PARSECLAEND  := look_ahead_end
 * PARSECDIGT   := digit
 * PARSECHEX    := hex
 * PARSECOCT    := oct
 * PARSECSPACE  := space
 * PARSECSATIS  := satisfy
 * PARSECSTR    := string
 * PARSECRESULT := result
 *
 * CCALL := ( ccall IDENTIFIER EXPRIDENTLIT* )
 *
 * DLOPEN := ( dlopen EXPRIDENTLIT )
 *
 * TOPTR := ( toptr EXPRIDENT )
 * DEREF := ( deref EXPRIDENT )
 *
 * ADD   := (+ EXPRIDENTLIT EXPRIDENTLIT+ )
 * MINUS := (- EXPRIDENTLIT EXPRIDENTLIT+ )
 * MULTI := (* EXPRIDENTLIT EXPRIDENTLIT+ )
 * DIV   := (/ EXPRIDENTLIT EXPRIDENTLIT+ )
 * MOD   := (mod EXPRIDENTLIT EXPRIDENTLIT+ )
 *
 * PRINT := ( print EXPRIDENTLIT )
 *
 * TOSTR := ( tostr EXPRIDENTLIT )
 *
 * -----------------------------------------------------------------------------
 *
 * LITERAL := STR32 | STR8 | CHAR32 | CHAR8 | INT | FLOAT | HEX | OCT | BIN
 *
 * STR32  := " CHARS* "
 * STR8   := b " CHARS* "
 * ESCAPE := \a | \b | \f | \r | \n | \t | \v | \\ | \? | \' | \" | \0 | \UXXXXXXXX | \uXXXX
 * CHARS  := ESCAPE | ESCAPE以外の文字
 *
 * CHAR32 := ' CHARS '
 * CHAR8  := ' CHARS '
 *
 * INT     := -? DIGIT
 * DIGIT   := NUM1to9 NUM0to9* | 0
 * NUM1to9 := 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
 * NUM0to9 := 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
 *
 * FLOAT := NUM . EXP
 * EXP   := EE SIGN NUM+
 * EE    := e | E
 * SIGN  := - | +
 *
 * HEX     := 0x HEXNUM2\* | 0X HEXNUM2\*
 * HEXNUM2 := HEXNUM HEXNUM
 * HEXNUM  := 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | a | A | b | B | c | C | d | D | f | F
 *
 * OCT    := 0 OCTNUM*
 * OCTNUM := 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7
 *
 * BIN    := b BINNUM\* | B BINNUM\*
 * BINNUM := 0 | 1
 *
 * TRUE  := true
 * FALSE := false
 */

namespace lunar {

enum LANG_OWNERSHIP {
    OWN_UNIQUE,
    OWN_SHARED,
    OWN_IMMOVABLE,
    OWN_REF,
};

enum LANG_SCALAR {
    SC_BOOL,
    SC_U64,
    SC_S64,
    SC_U32,
    SC_S32,
    SC_U16,
    SC_S16,
    SC_U8,
    SC_S8,
    SC_DOUBLE,
    SC_FLOAT,
    SC_CHAR,
    SC_ATOM,
};

enum LANG_BASIC_TYPE {
    BT_SCALAR,
    BT_VECTOR,
    BT_STRING,
    BT_BINARY,
    BT_LIST,
    BT_STRUCT,
    BT_DICT,
    BT_SET,
    BT_DATA,
    BT_UNION,
    BT_FUNCTYPE,
    BT_RSTREAM,
    BT_WSTREAM,
    BT_PTR,
    BT_PARSEC,
};

enum LANG_LITERAL {
    LIT_STR32,
    LIT_STR8,
    LIT_CHAR32,
    LIT_CHAR8,
    LIT_INT,
    LIT_FLOAT,
    LIT_HEX,
    LIT_OCT,
    LIT_BIN,
};

class lunar_ir_type {
public:
    lunar_ir_type(LANG_BASIC_TYPE type, LANG_OWNERSHIP owner_ship) : m_type(type), m_owner_ship(owner_ship) { }
    virtual ~lunar_ir_type() { }

protected:
    LANG_BASIC_TYPE m_type;
    LANG_OWNERSHIP  m_owner_ship;
};

class lunar_ir_expr {
public:
    lunar_ir_expr() { }
    virtual ~lunar_ir_expr() { }
};

class lunar_ir_statement {
public:
    lunar_ir_statement() { }
    virtual ~lunar_ir_statement() { }
};

class lunar_ir_stexpr {
public:
    lunar_ir_stexpr(std::unique_ptr<lunar_ir_expr> expr) : m_expr(std::move(expr)), m_is_expr(true) { }
    lunar_ir_stexpr(std::unique_ptr<lunar_ir_statement> statement) : m_statement(std::move(statement)), m_is_expr(false) { }
    virtual ~lunar_ir_stexpr() { }

    bool is_expr() { return m_is_expr; }

private:
    std::unique_ptr<lunar_ir_expr>      m_expr;
    std::unique_ptr<lunar_ir_statement> m_statement;
    bool m_is_expr;
};


class lunar_ir_scalar : public lunar_ir_type {
public:
    lunar_ir_scalar(LANG_OWNERSHIP owner_ship, LANG_SCALAR scalar)
        : lunar_ir_type(BT_SCALAR, owner_ship), m_scalar(scalar) { }

    LANG_SCALAR get_type() { return m_scalar; }

private:
    LANG_SCALAR m_scalar;
};

class lunar_ir_vector : public lunar_ir_type {
public:
    lunar_ir_vector(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> type, uint64_t size)
        : lunar_ir_type(BT_VECTOR, owner_ship),
          m_type(std::move(type)),
          m_size(size) { }
    
    virtual ~lunar_ir_vector() { }

    uint64_t size() { return m_size; }

private:
    std::unique_ptr<lunar_ir_type> m_type;
    uint64_t m_size;
};

class lunar_ir_list : public lunar_ir_type {
public:
    lunar_ir_list(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_LIST, owner_ship),
          m_type(std::move(type)) { }
    
    virtual ~lunar_ir_list() { }

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_struct : public lunar_ir_type {
public:
    lunar_ir_struct(LANG_OWNERSHIP owner_ship, const std::string &name)
        : lunar_ir_type(BT_STRUCT, owner_ship), m_name(name) { }
    
    virtual ~lunar_ir_struct() { }

    void add_member(std::unique_ptr<lunar_ir_type> type, const std::string &name)
    {
        m_member_types.push_back(std::move(type));
        m_member_names.push_back(name);
    }

private:
    std::vector<std::unique_ptr<lunar_ir_type>> m_member_types;
    std::vector<std::string> m_member_names;
    std::string m_name;
};

class lunar_ir_union : public lunar_ir_type {
public:
    lunar_ir_union(LANG_OWNERSHIP owner_ship, const std::string &name)
        : lunar_ir_type(BT_UNION, owner_ship), m_name(name) { }
    
    virtual ~lunar_ir_union() { }

    void add_member(std::unique_ptr<lunar_ir_type> type, const std::string &name)
    {
        m_member_types.push_back(std::move(type));
        m_member_names.push_back(name);
    }

private:
    std::vector<std::unique_ptr<lunar_ir_type>> m_member_types;
    std::vector<std::string> m_member_names;
    std::string m_name;
};

class lunar_ir_dict : public lunar_ir_type {
public:
    lunar_ir_dict(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> key, std::unique_ptr<lunar_ir_type> val)
        : lunar_ir_type(BT_DICT, owner_ship), m_key(std::move(key)), m_val(std::move(val)) { }

private:
    std::unique_ptr<lunar_ir_type> m_key;
    std::unique_ptr<lunar_ir_type> m_val;
};

class lunar_ir_set : public lunar_ir_type {
public:
    lunar_ir_set(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> val)
        : lunar_ir_type(BT_SET, owner_ship), m_val(std::move(val)) { }

private:
    std::unique_ptr<lunar_ir_type> m_val;
};

class lunar_ir_data : public lunar_ir_type {
public:
    lunar_ir_data(LANG_OWNERSHIP owner_ship, const std::string &name)
        : lunar_ir_type(BT_DATA, owner_ship), m_name(name) { }
    
    virtual ~lunar_ir_data() { }

    void add_member(std::unique_ptr<lunar_ir_type> type, const std::string &name)
    {
        m_member_types.push_back(std::move(type));
        m_member_names.push_back(name);
    }

private:
    std::vector<std::unique_ptr<lunar_ir_type>> m_member_types;
    std::vector<std::string> m_member_names;
    std::string m_name;
};

class lunar_ir_functype : public lunar_ir_type {
public:
    lunar_ir_functype(LANG_OWNERSHIP owner_ship) : lunar_ir_type(BT_FUNCTYPE, owner_ship) { }
    virtual ~lunar_ir_functype() { }
    
    void add_ret(std::unique_ptr<lunar_ir_type> ret)
    {
        m_ret.push_back(std::move(ret));
    }

    void add_arg(std::unique_ptr<lunar_ir_type> arg)
    {
        m_arg.push_back(std::move(arg));
    }

private:
    std::vector<std::unique_ptr<lunar_ir_type>> m_ret;
    std::vector<std::unique_ptr<lunar_ir_type>> m_arg;
};

class lunar_ir_rstream : public lunar_ir_type {
public:
    lunar_ir_rstream(std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_RSTREAM, OWN_UNIQUE), m_type(std::move(type)) { }

    virtual ~lunar_ir_rstream() { }

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_wstream : public lunar_ir_type {
public:
    lunar_ir_wstream(std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_WSTREAM, OWN_SHARED), m_type(std::move(type)) { }

    virtual ~lunar_ir_wstream() { }

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_string : public lunar_ir_type {
public:
    lunar_ir_string(LANG_OWNERSHIP owner_ship) : lunar_ir_type(BT_STRING, owner_ship) { }
    virtual ~lunar_ir_string() { }
};

class lunar_ir_binary : public lunar_ir_type {
public:
    lunar_ir_binary(LANG_OWNERSHIP owner_ship) : lunar_ir_type(BT_BINARY, owner_ship) { }
    virtual ~lunar_ir_binary() { }
};

class lunar_ir_ptr : public lunar_ir_type {
public:
    lunar_ir_ptr(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_PTR, owner_ship),
          m_type(std::move(type)) { }
    virtual ~lunar_ir_ptr() { }

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_parsec : public lunar_ir_type {
public:
    lunar_ir_parsec(bool is_binary) // binary or string
        : lunar_ir_type(BT_PARSEC, OWN_UNIQUE), m_is_binary(is_binary) { }
    virtual ~lunar_ir_parsec() { }

    bool is_binary() { return m_is_binary; }

private:
    bool m_is_binary;
};

class lunar_ir_var {
public:
    lunar_ir_var(std::unique_ptr<lunar_ir_type> type, const std::string &name)
        : m_type(std::move(type)), m_name(name) { }
    
    std::string& get_name() { return m_name; }

private:
    std::unique_ptr<lunar_ir_type> m_type;
    std::string m_name;
};

class lunar_ir_func {
public:
    lunar_ir_func(const std::string &name) : m_name(name) { }
    virtual ~lunar_ir_func() { }
    
    void add_ret(std::unique_ptr<lunar_ir_type> ret)
    {
        m_ret.push_back(std::move(ret));
    }

    void add_arg(std::unique_ptr<lunar_ir_var> var)
    {
        m_argmap[var->get_name()] = var.get();
        m_args.push_back(std::move(var));
    }

    void add_stexpr(std::unique_ptr<lunar_ir_stexpr> stexpr)
    {
        m_stexprs.push_back(std::move(stexpr));
    }

private:
    std::vector<std::unique_ptr<lunar_ir_type>> m_ret;
    std::vector<std::unique_ptr<lunar_ir_var>>  m_args;
    std::unordered_map<std::string, lunar_ir_var*> m_argmap;
    std::string m_name;
    std::vector<std::unique_ptr<lunar_ir_stexpr>> m_stexprs;
};

class lunar_ir_let : public lunar_ir_statement {
public:
    class def {
    public:
        void add_var(std::unique_ptr<lunar_ir_var> var)
        {
            m_argmap[var->get_name()] = var.get();
            m_vars.push_back(std::move(var));
        }

        void set_expr(std::unique_ptr<lunar_ir_expr> expr)
        {
            m_expr = std::move(expr);
        }

    private:
        std::vector<std::unique_ptr<lunar_ir_var>>     m_vars;
        std::unordered_map<std::string, lunar_ir_var*> m_argmap;
        std::unique_ptr<lunar_ir_expr> m_expr;
    };

    lunar_ir_let() { }
    virtual ~lunar_ir_let() { }

    void add_defs(std::unique_ptr<def> def)
    {
        m_defs.push_back(std::move(def));
    }
    
    void add_stexpr(std::unique_ptr<lunar_ir_stexpr> stexpr)
    {
        m_stexprs.push_back(std::move(stexpr));
    }

private:
    std::vector<std::unique_ptr<def>> m_defs;
    std::vector<std::unique_ptr<lunar_ir_stexpr>> m_stexprs;
};

class lunar_ir_cond : public lunar_ir_statement {
public:
    class cond {
    public:
        void set_cond(std::unique_ptr<lunar_ir_expr> expr)
        {
            m_expr = std::move(expr);
        }

        void add_stexpr(std::unique_ptr<lunar_ir_stexpr> stexpr)
        {
            m_stexprs.push_back(std::move(stexpr));
        }

    private:
        std::unique_ptr<lunar_ir_expr> m_expr; // condition
        std::vector<std::unique_ptr<lunar_ir_stexpr>> m_stexprs;
    };

    lunar_ir_cond() { }
    virtual ~lunar_ir_cond() { }

    void add_cond(std::unique_ptr<cond> c)
    {
        m_conds.push_back(std::move(c));
    }

    void add_else(std::unique_ptr<lunar_ir_stexpr> stexpr)
    {
        m_elses.push_back(std::move(stexpr));
    }

private:
    std::vector<std::unique_ptr<cond>> m_conds;
    std::vector<std::unique_ptr<lunar_ir_stexpr>> m_elses;
};

class lunar_ir_while : public lunar_ir_statement {
public:
    lunar_ir_while() { }
    virtual ~lunar_ir_while() { }

    void set_cond(std::unique_ptr<lunar_ir_expr> expr)
    {
        m_cond = std::move(expr);
    }

    void add_stexpr(std::unique_ptr<lunar_ir_stexpr> stexpr)
    {
        m_stexprs.push_back(std::move(stexpr));
    }

private:
    std::unique_ptr<lunar_ir_expr> m_cond;
    std::vector<std::unique_ptr<lunar_ir_stexpr>> m_stexprs;
};

class lunar_ir_select : public lunar_ir_statement {
public:
    class cond {
    public:
        void set_cond(std::unique_ptr<lunar_ir_expr> expr)
        {
            m_expr = std::move(expr);
        }

        void add_stexpr(std::unique_ptr<lunar_ir_stexpr> stexpr)
        {
            m_stexprs.push_back(std::move(stexpr));
        }

    private:
        std::unique_ptr<lunar_ir_expr> m_expr; // condition
        std::vector<std::unique_ptr<lunar_ir_stexpr>> m_stexprs;
    };

    lunar_ir_select() { }
    virtual ~lunar_ir_select() { }

    void set_timeout(std::unique_ptr<lunar_ir_expr> expr)
    {
        m_timeout = std::move(expr);
    }

    void add_timeout(std::unique_ptr<lunar_ir_stexpr> stexpr)
    {
        m_elses.push_back(std::move(stexpr));
    }

private:
    std::vector<std::unique_ptr<cond>> m_conds;
    std::vector<std::unique_ptr<lunar_ir_stexpr>> m_elses;
    std::unique_ptr<lunar_ir_expr> m_timeout; // no timeout when -1
};

class lunar_ir_break : public lunar_ir_statement {
public:
    lunar_ir_break() { }
    virtual ~lunar_ir_break() { }
};

class lunar_ir_return : public lunar_ir_statement {
public:
    lunar_ir_return() { }
    virtual ~lunar_ir_return() { }

    void add_expr(std::unique_ptr<lunar_ir_expr> expr)
    {
        m_exprs.push_back(std::move(expr));
    }

private:
    std::vector<std::unique_ptr<lunar_ir_expr>> m_exprs;
};

class lunar_ir_schedule : public lunar_ir_statement {
public:
    lunar_ir_schedule() { }
    virtual ~lunar_ir_schedule() { }
};

class lunar_ir_spawn : public lunar_ir_expr {
public:
    lunar_ir_spawn(std::unique_ptr<lunar_ir_expr> ssize,
                   std::unique_ptr<lunar_ir_expr> func,
                   std::unique_ptr<lunar_ir_expr> arg)
        : m_ssize(std::move(ssize)),
          m_func(std::move(func)),
          m_arg(std::move(arg)) { }
    virtual ~lunar_ir_spawn() { }

private:
    std::unique_ptr<lunar_ir_expr> m_ssize;  // stack size
    std::unique_ptr<lunar_ir_expr> m_func;
    std::unique_ptr<lunar_ir_expr> m_arg;
};

class lunar_ir_thread : public lunar_ir_expr {
public:
    lunar_ir_thread(std::unique_ptr<lunar_ir_expr> name,
                    std::unique_ptr<lunar_ir_type> type,
                    std::unique_ptr<lunar_ir_expr> qsize,
                    std::unique_ptr<lunar_ir_expr> func,
                    std::unique_ptr<lunar_ir_expr> arg)
        : m_name(std::move(name)),
          m_type(std::move(type)),
          m_qsize(std::move(qsize)),
          m_func(std::move(func)),
          m_arg(std::move(arg)) { }
    virtual ~lunar_ir_thread() { }

private:
    std::unique_ptr<lunar_ir_expr> m_name;  // name (atom)
    std::unique_ptr<lunar_ir_type> m_type;
    std::unique_ptr<lunar_ir_expr> m_qsize; // size of thread queue
    std::unique_ptr<lunar_ir_expr> m_func;
    std::unique_ptr<lunar_ir_expr> m_arg;
};

class lunar_ir_copy : public lunar_ir_expr {
public:
    lunar_ir_copy(std::unique_ptr<lunar_ir_expr> dst, std::unique_ptr<lunar_ir_expr> src)
        : m_dst(std::move(dst)),
          m_src(std::move(src)) { }
    virtual ~lunar_ir_copy() { }

private:
    std::unique_ptr<lunar_ir_expr> m_dst;
    std::unique_ptr<lunar_ir_expr> m_src;
};

class lunar_ir_assoc : public lunar_ir_expr {
public:
    lunar_ir_assoc(std::unique_ptr<lunar_ir_expr> dst, std::unique_ptr<lunar_ir_expr> src)
        : m_dst(std::move(dst)),
          m_src(std::move(src)) { }
    virtual ~lunar_ir_assoc() { }

private:
    std::unique_ptr<lunar_ir_expr> m_dst;
    std::unique_ptr<lunar_ir_expr> m_src;
};

class lunar_ir_inccnt : public lunar_ir_expr {
public:
    lunar_ir_inccnt(std::unique_ptr<lunar_ir_expr> expr) : m_expr(std::move(expr)) { }
    virtual ~lunar_ir_inccnt() { }

private:
    std::unique_ptr<lunar_ir_expr> m_expr;
};

class lunar_ir_deccnt : public lunar_ir_expr {
public:
    lunar_ir_deccnt(std::unique_ptr<lunar_ir_expr> expr) : m_expr(std::move(expr)) { }
    virtual ~lunar_ir_deccnt() { }

private:
    std::unique_ptr<lunar_ir_expr> m_expr;
};

class lunar_ir_if : public lunar_ir_expr {
public:
    lunar_ir_if(std::unique_ptr<lunar_ir_expr> cond,
                std::unique_ptr<lunar_ir_expr> expr1,
                std::unique_ptr<lunar_ir_expr> expr2)
        : m_cond(std::move(cond)),
          m_expr1(std::move(expr1)),
          m_expr2(std::move(expr2)) { }
    virtual ~lunar_ir_if() { }

private:
    std::unique_ptr<lunar_ir_expr> m_cond;
    std::unique_ptr<lunar_ir_expr> m_expr1;
    std::unique_ptr<lunar_ir_expr> m_expr2;
};

class lunar_ir_lambda : public lunar_ir_expr {
public:
    lunar_ir_lambda() { }
    virtual ~lunar_ir_lambda() { }
    
    void add_ret(std::unique_ptr<lunar_ir_type> ret)
    {
        m_ret.push_back(std::move(ret));
    }

    void add_arg(std::unique_ptr<lunar_ir_var> var)
    {
        m_argmap[var->get_name()] = var.get();
        m_args.push_back(std::move(var));
    }

    void add_stexpr(std::unique_ptr<lunar_ir_stexpr> stexpr)
    {
        m_stexprs.push_back(std::move(stexpr));
    }

private:
    std::vector<std::unique_ptr<lunar_ir_type>> m_ret;
    std::vector<std::unique_ptr<lunar_ir_var>>  m_args;
    std::unordered_map<std::string, lunar_ir_var*> m_argmap;
    std::vector<std::unique_ptr<lunar_ir_stexpr>> m_stexprs;
};

class lunar_ir_new : public lunar_ir_expr {
public:
    lunar_ir_new(std::unique_ptr<lunar_ir_type> type) : m_type(std::move(type)) { }
    virtual ~lunar_ir_new() { }

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_callfunc : public lunar_ir_expr {
public:
    lunar_ir_callfunc(std::unique_ptr<lunar_ir_expr> func) : m_func(std::move(func)) { }
    virtual ~lunar_ir_callfunc() { }

    void add_arg(std::unique_ptr<lunar_ir_expr> arg)
    {
        m_args.push_back(std::move(arg));
    }

private:
    std::unique_ptr<lunar_ir_expr> m_func;
    std::vector<std::unique_ptr<lunar_ir_expr>> m_args;
};

}

#endif // LUNAR_IR_HPP