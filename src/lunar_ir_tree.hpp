#ifndef LUNAR_IR_TREE
#define LUNAR_IR_TREE

#include <stdint.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

/*
 * -----------------------------------------------------------------------------
 *
 * IR           := TOP*
 * TOP          := FUNC | GLOBAL | THREADLOCAL | IMPORT | EXPR | STATEMENT
 * TOPSTATEMENT := LET | COND | WHILE | SELECT | SCHEDULE | STRUCT | CUNION | UNION
 * STATEMENT    := LET | COND | WHILE | BREAK | SELECT | RETURN | SCHEDULE | STRUCT | CUNION | UNION | BLOCK | LEAP
 * GLOBAL       := ( global ( ( TYPE (IDENTIFIER+) EXPRIDENTLIT )+ ) )
 * THREADLOCAL  := ( threadlocal ( ( TYPE ( IDENTIFIER+ ) EXPRIDENTLIT )+ ) )
 * IMPORT       := ( import STR32+ )
 *
 * -----------------------------------------------------------------------------
 *
 * TYPE  := TYPE0 | ( OWNERSHIP TYPE0 )
 * TYPE0 := SCALAR | ARRAY | STRING | BINARY | LIST | STRUCT | DICT | SET | UNION | FUNCTYPE |
 *          RSTREAM | WSTREAM | RFILESTREAM | WFILESTREAM | RSOCKSTREAM | WSOCKSTREAM |
 *          RSIGSTREAM | RTHREADSTREAM | WTHREADSTREAM | PTR | CUNION | PARSEC | IDENTIFIER
 *
 * OWNERSHIP := unique | shared | ref
 *
 * SCALAR     := SCALARTYPE INITSCALAR | SCALARTYPE
 * SCALARTYPE := bool | u64 | s64 | u32 | s32 | u16 | s16 | u8 | s8 | double | float | char | atom
 *
 * ARRAY := ( array TYPE SIZE ) | ( array TYPE )
 * SIZE  := INT | HEX | OCT | BIN
 *
 * STRING := string
 *
 * BINARY := binary
 *
 * LIST := ( list TYPE )
 *
 * STRUCT := ( struct IDENTIFIER? ( TYPE IDENTIFIER )* )
 *
 * UNION := ( union IDENTIFIER? ( TYPE IDENTIFIER )* )
 *
 * CUNION := ( cunion IDENTIFIER? ( TYPE IDENTIFIER )* )
 *
 * DICT := ( dict TYPE TYPE )
 *
 * SET := ( set TYPE )
 *
 * FUNCTYPE := ( func ( TYPE* ) ( TYPE* ) )
 *
 * RSTREAM := ( rstrm TYPE )
 * WSTREAM := ( wstrm TYPE )
 * RFILESTREAM := rfilestrm
 * WFILESTREAM := wfilestrm
 * RSOCKSTREAM := rsockstrm
 * WSOCKSTREAM := wsockstrm
 * RSIGSTREAM  := rsigstrm
 * RTHREADTREAM := ( rthreadstrm TYPE )
 * WTHREADTREAM := ( wthreadstrm TYPE )
 *
 * PTR := (ptr TYPE )
 *
 * PARSEC := ( parsec string ) | ( parsec binary)
 *
 * -----------------------------------------------------------------------------
 *
 * FUNC := ( defun IDENTIFIER ( TYPE* ) ( TYPE IDENTIFIER )* STEXPR* )
 *
 * -----------------------------------------------------------------------------
 *
 * STEXPR := STATMENT | EXPR
 *
 * LET := ( let ( ( ( TYPE IDENTIFIER )+ EXPRIDENTLIT )+ ) STEXPR\* )
 *
 * COND := ( cond ( EXPRIDENTLIT STEXPR* )+ ( else STEXPR* )? )
 *
 * WHILE := ( while EXPRIDENTLIT STEXPR* )
 * BREAK := ( break )
 *
 * BLOCK := ( block STEXPR* )
 * LEAP  := ( leap )
 *
 * SELECT := ( select ( EXPRIDENT STEXPR*)* ( timeout EXPRIDENTLIT STEXPR* )? )
 *
 * RETURN := ( return EXPRIDENTLIT* )
 *
 * SCHEDULE := ( schedule )
 *
 * -----------------------------------------------------------------------------
 *
 * EXPR := CALLFUNC
 *
 * EXPRIDENT := EXPR | IDENTIFIER
 *
 * CALLFUNC := ( EXPRIDENT EXPRIDENTLIT* )
 *
 * -----------------------------------------------------------------------------
 *
 * LITERAL := STR32 | STR8 | CHAR32 | CHAR8 | INT | FLOAT | HEX | OCT | BIN | ATOM
 *
 * ATOM := `IDENTIFIER
 *
 * STR32  := " CHARS* "
 * STR8   := b " CHARS* "
 * ESCAPE := \a | \b | \f | \r | \n | \t | \v | \\ | \? | \' | \" | \0 | \UXXXXXXXX | \uXXXX
 * CHARS  := ESCAPE | ESCAPE以外の文字
 *
 * CHAR32 := ' CHARS '
 * CHAR8  := b ' CHARS '
 *
 * INT     := -? DIGIT
 * DIGIT   := NUM1to9 NUM0to9* | 0
 * NUM1to9 := 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
 * NUM0to9 := 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
 *
 * FLOAT := NUM . EXP? f?
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
 * BIN    := 0b BINNUM\* | 0B BINNUM\*
 * BINNUM := 0 | 1
 *
 * TRUE  := `true
 * FALSE := `false
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
    BT_ARRAY,
    BT_STRING,
    BT_BINARY,
    BT_LIST,
    BT_STRUCT,
    BT_DICT,
    BT_SET,
    BT_UNION,
    BT_CUNION,
    BT_FUNCTYPE,
    BT_RSTREAM,
    BT_WSTREAM,
    BT_RSOCKSTREAM,
    BT_WSOCKSTREAM,
    BT_RFILESTREAM,
    BT_WFILESTREAM,
    BT_RTHREADSTREAM,
    BT_WTHREADSTREAM,
    BT_RSIGSTREAM,
    BT_PTR,
    BT_PARSEC,
    BT_ID,
};

enum IR_TOP {
    IR_FUNC,
    IR_GLOBAL,
    IR_IMPORT,
    IR_EXPR,
    IR_STATEMENT,
};

class lunar_ir_base {
public:
    lunar_ir_base() : m_line(0), m_col(0) { }
    virtual ~lunar_ir_base() { }

    void set_col(uint64_t col) { m_col = col; }
    void set_line(uint64_t line) { m_line = line; }
    uint64_t get_col() { return m_col; }
    uint64_t get_line() { return m_line; }

    virtual void print(std::string &s, const std::string &from) { }

private:
    uint64_t m_line, m_col;
};

class lunar_ir_literal : public lunar_ir_base {
public:
    lunar_ir_literal() { }
    virtual ~lunar_ir_literal() { }
};

class lunar_ir_identifier : public lunar_ir_base
{
public:
    lunar_ir_identifier(std::unique_ptr<std::u32string> id) : m_id(std::move(id)) { }
    virtual ~lunar_ir_identifier() { }

    const std::u32string& get_id() { return *m_id; }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<std::u32string> m_id;
};

class lunar_ir_type : public lunar_ir_base {
public:
    lunar_ir_type(LANG_BASIC_TYPE type, LANG_OWNERSHIP owner_ship) : m_type(type), m_owner_ship(owner_ship) { }
    virtual ~lunar_ir_type() { }

    void print_ownership(std::string &s, const std::string &from);

protected:
    LANG_BASIC_TYPE m_type;
    LANG_OWNERSHIP  m_owner_ship;
};

class lunar_ir_top : public lunar_ir_base {
public:
    lunar_ir_top(IR_TOP type) : m_type(type) { }
    virtual ~lunar_ir_top() { }

private:
    IR_TOP m_type;
};

class lunar_ir_expr;

class lunar_ir_exprid : public lunar_ir_base {
public:
    enum EXPRID_TYPE {
        EXPRID_EXPR,
        EXPRID_ID,
    };

    lunar_ir_exprid(EXPRID_TYPE type, std::unique_ptr<lunar_ir_identifier> id) : m_type(type), m_id(std::move(id)) { }
    lunar_ir_exprid(EXPRID_TYPE type, std::unique_ptr<lunar_ir_expr> expr) : m_type(type), m_expr(std::move(expr)) { }
    virtual ~lunar_ir_exprid() { }

    virtual void print(std::string &s, const std::string &from);

private:
    EXPRID_TYPE m_type;
    std::unique_ptr<lunar_ir_identifier> m_id;
    std::unique_ptr<lunar_ir_expr>       m_expr;
};

class lunar_ir_expridlit : public lunar_ir_base {
public:
    enum EXPRIDLIT_TYPE {
        EXPRIDLIT_EXPR,
        EXPRIDLIT_ID,
        EXPRIDLIT_LITERAL,
    };

    lunar_ir_expridlit(EXPRIDLIT_TYPE type, std::unique_ptr<lunar_ir_identifier> id) : m_type(type), m_id(std::move(id)) { }
    lunar_ir_expridlit(EXPRIDLIT_TYPE type, std::unique_ptr<lunar_ir_literal> literal) : m_type(type), m_literal(std::move(literal)) { }
    lunar_ir_expridlit(EXPRIDLIT_TYPE type, std::unique_ptr<lunar_ir_expr> expr) : m_type(type), m_expr(std::move(expr)) { }
    virtual ~lunar_ir_expridlit() { }

    virtual void print(std::string &s, const std::string &from);

private:
    EXPRIDLIT_TYPE m_type;
    std::unique_ptr<lunar_ir_identifier> m_id;
    std::unique_ptr<lunar_ir_literal>    m_literal;
    std::unique_ptr<lunar_ir_expr>       m_expr;
};

class lunar_ir_expr : public lunar_ir_top {
public:
    lunar_ir_expr(std::unique_ptr<lunar_ir_exprid> func) : lunar_ir_top(IR_EXPR), m_func(std::move(func)) { }
    virtual ~lunar_ir_expr() { }

    void add_arg(std::unique_ptr<lunar_ir_expridlit> arg)
    {
        m_args.push_back(std::move(arg));
    }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_exprid> m_func;
    std::vector<std::unique_ptr<lunar_ir_expridlit>> m_args;
};

class lunar_ir_statement : public lunar_ir_top {
public:
    lunar_ir_statement() : lunar_ir_top(IR_STATEMENT) { }
    virtual ~lunar_ir_statement() { }
};

class lunar_ir_member {
public:
    lunar_ir_member() { }
    virtual ~lunar_ir_member() { }

    void add_member(std::unique_ptr<lunar_ir_type> type, std::unique_ptr<lunar_ir_identifier> name)
    {
        m_member_types.push_back(std::move(type));
        m_member_names.push_back(std::move(name));
    }

    void print_member(std::string &s, const std::string &from);

    void set_line_mem(uint64_t line) { m_line = line; }
    void set_col_mem(uint64_t col) { m_col = col; }

private:
    std::vector<std::unique_ptr<lunar_ir_type>>       m_member_types;
    std::vector<std::unique_ptr<lunar_ir_identifier>> m_member_names;
    uint64_t m_line;
    uint64_t m_col;
};

class lunar_ir_def_struct : public lunar_ir_statement, public lunar_ir_member {
public:
    lunar_ir_def_struct(std::unique_ptr<lunar_ir_identifier> name) : m_name(std::move(name)) { }
    virtual ~lunar_ir_def_struct() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_identifier> m_name;
};

class lunar_ir_def_cunion : public lunar_ir_statement, public lunar_ir_member {
public:
    lunar_ir_def_cunion(std::unique_ptr<lunar_ir_identifier> name) : m_name(std::move(name)) { }
    virtual ~lunar_ir_def_cunion() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_identifier> m_name;
};

class lunar_ir_def_union : public lunar_ir_statement, public lunar_ir_member {
public:
    lunar_ir_def_union(std::unique_ptr<lunar_ir_identifier> name) : m_name(std::move(name)) { }
    virtual ~lunar_ir_def_union() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_identifier> m_name;
};

class lunar_ir_import : public lunar_ir_top {
public:
    lunar_ir_import() : lunar_ir_top(IR_IMPORT) { }
    virtual ~lunar_ir_import() { }

    void add_module(std::u32string module)
    {
        m_modules.push_back(module);
    }

private:
    std::vector<std::u32string> m_modules;
};

class lunar_ir_stexpr : public lunar_ir_base {
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

class lunar_ir_type_id : public lunar_ir_type {
public:
    lunar_ir_type_id(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_identifier> id)
        : lunar_ir_type(BT_ID, owner_ship), m_id(std::move(id)) { }
    virtual ~lunar_ir_type_id() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_identifier> m_id;
};

class lunar_ir_lit_atom : public lunar_ir_literal {
public:
    lunar_ir_lit_atom(const std::u32string &str) : m_str(str) { }
    virtual ~lunar_ir_lit_atom() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::u32string m_str;
};

class lunar_ir_lit_str32 : public lunar_ir_literal {
public:
    lunar_ir_lit_str32(const std::u32string &str) : m_str(str) { }
    virtual ~lunar_ir_lit_str32() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::u32string m_str;
};

class lunar_ir_lit_str8 : public lunar_ir_literal {
public:
    lunar_ir_lit_str8(const std::u32string &str) : m_str(str) { }
    virtual ~lunar_ir_lit_str8() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::u32string m_str;
};

class lunar_ir_lit_char32 : public lunar_ir_literal {
public:
    lunar_ir_lit_char32(char32_t c) : m_char(c) { }
    virtual ~lunar_ir_lit_char32() { }

    virtual void print(std::string &s, const std::string &from);

private:
    char32_t m_char;
};

class lunar_ir_lit_char8 : public lunar_ir_literal {
public:
    lunar_ir_lit_char8(char c) : m_char(c) { }
    virtual ~lunar_ir_lit_char8() { }

    virtual void print(std::string &s, const std::string &from);

private:
    char m_char;
};

class lunar_ir_lit_int : public lunar_ir_literal {
public:
    lunar_ir_lit_int(int64_t num, const std::u32string &str) : m_num(num), m_str(str) { }
    virtual ~lunar_ir_lit_int() { }

    virtual void print(std::string &s, const std::string &from);

private:
    int64_t m_num;
    std::u32string m_str;
};

class lunar_ir_lit_uint : public lunar_ir_literal {
public:
    lunar_ir_lit_uint(uint64_t num, const std::u32string &str) : m_num(num), m_str(str) { }
    virtual ~lunar_ir_lit_uint() { }

    virtual void print(std::string &s, const std::string &from);

private:
    uint64_t m_num;
    std::u32string m_str;
};

class lunar_ir_lit_float : public lunar_ir_literal {
public:
    lunar_ir_lit_float(double num, bool is_float) : m_num(num), m_is_float(is_float) { }
    virtual ~lunar_ir_lit_float() { }

    virtual void print(std::string &s, const std::string &from);

private:
    double m_num;
    bool   m_is_float;
};

class lunar_ir_scalar : public lunar_ir_type {
public:
    lunar_ir_scalar(LANG_OWNERSHIP owner_ship, LANG_SCALAR scalar)
        : lunar_ir_type(BT_SCALAR, owner_ship), m_scalar(scalar) { }

    LANG_SCALAR get_type() { return m_scalar; }

    virtual void print(std::string &s, const std::string &from);

private:
    LANG_SCALAR m_scalar;
};

class lunar_ir_array : public lunar_ir_type {
public:
    lunar_ir_array(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> type, std::unique_ptr<lunar_ir_lit_uint> size)
        : lunar_ir_type(BT_ARRAY, owner_ship),
          m_type(std::move(type)),
          m_size(std::move(size)) { }
    virtual ~lunar_ir_array() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_type> m_type;
    std::unique_ptr<lunar_ir_lit_uint> m_size;
};

class lunar_ir_list : public lunar_ir_type {
public:
    lunar_ir_list(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_LIST, owner_ship),
          m_type(std::move(type)) { }

    virtual ~lunar_ir_list() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_struct : public lunar_ir_type, public lunar_ir_member {
public:
    lunar_ir_struct(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_identifier> name)
        : lunar_ir_type(BT_STRUCT, owner_ship), m_name(std::move(name)) { }
    virtual ~lunar_ir_struct() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_identifier> m_name;
};

class lunar_ir_cunion : public lunar_ir_type, public lunar_ir_member {
public:
    lunar_ir_cunion(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_identifier> name)
        : lunar_ir_type(BT_CUNION, owner_ship), m_name(std::move(name)) { }
    virtual ~lunar_ir_cunion() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_identifier> m_name;
};

class lunar_ir_dict : public lunar_ir_type {
public:
    lunar_ir_dict(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> key, std::unique_ptr<lunar_ir_type> val)
        : lunar_ir_type(BT_DICT, owner_ship), m_key(std::move(key)), m_val(std::move(val)) { }
    virtual ~lunar_ir_dict() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_type> m_key;
    std::unique_ptr<lunar_ir_type> m_val;
};

class lunar_ir_set : public lunar_ir_type {
public:
    lunar_ir_set(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_SET, owner_ship), m_type(std::move(type)) { }
    virtual ~lunar_ir_set() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_union : public lunar_ir_type, public lunar_ir_member {
public:
    lunar_ir_union(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_identifier> name)
        : lunar_ir_type(BT_UNION, owner_ship), m_name(std::move(name)) { }
    virtual ~lunar_ir_union() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_identifier> m_name;
};

class lunar_ir_func : public lunar_ir_type {
public:
    lunar_ir_func(LANG_OWNERSHIP owner_ship) : lunar_ir_type(BT_FUNCTYPE, owner_ship) { }
    virtual ~lunar_ir_func() { }

    void add_ret(std::unique_ptr<lunar_ir_type> ret)
    {
        m_ret.push_back(std::move(ret));
    }

    void add_arg(std::unique_ptr<lunar_ir_type> arg)
    {
        m_arg.push_back(std::move(arg));
    }

    virtual void print(std::string &s, const std::string &from);

private:
    std::vector<std::unique_ptr<lunar_ir_type>> m_ret;
    std::vector<std::unique_ptr<lunar_ir_type>> m_arg;
};

class lunar_ir_rstream : public lunar_ir_type {
public:
    lunar_ir_rstream(std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_RSTREAM, OWN_UNIQUE), m_type(std::move(type)) { }
    virtual ~lunar_ir_rstream() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_wstream : public lunar_ir_type {
public:
    lunar_ir_wstream(std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_WSTREAM, OWN_SHARED), m_type(std::move(type)) { }
    virtual ~lunar_ir_wstream() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_rsigstream : public lunar_ir_type {
public:
    lunar_ir_rsigstream() : lunar_ir_type(BT_RSIGSTREAM, OWN_UNIQUE) { }
    virtual ~lunar_ir_rsigstream() { }

    virtual void print(std::string &s, const std::string &from);
};

class lunar_ir_rsockstream : public lunar_ir_type {
public:
    lunar_ir_rsockstream() : lunar_ir_type(BT_RSOCKSTREAM, OWN_UNIQUE) { }
    virtual ~lunar_ir_rsockstream() { }

    virtual void print(std::string &s, const std::string &from);
};

class lunar_ir_wsockstream : public lunar_ir_type {
public:
    lunar_ir_wsockstream() : lunar_ir_type(BT_WSOCKSTREAM, OWN_SHARED) { }
    virtual ~lunar_ir_wsockstream() { }

    virtual void print(std::string &s, const std::string &from);
};

class lunar_ir_rfilestream : public lunar_ir_type {
public:
    lunar_ir_rfilestream() : lunar_ir_type(BT_RFILESTREAM, OWN_UNIQUE) { }
    virtual ~lunar_ir_rfilestream() { }

    virtual void print(std::string &s, const std::string &from);
};

class lunar_ir_wfilestream : public lunar_ir_type {
public:
    lunar_ir_wfilestream() : lunar_ir_type(BT_WFILESTREAM, OWN_SHARED) { }
    virtual ~lunar_ir_wfilestream() { }

    virtual void print(std::string &s, const std::string &from);
};

class lunar_ir_rthreadstream : public lunar_ir_type {
public:
    lunar_ir_rthreadstream(std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_RTHREADSTREAM, OWN_UNIQUE), m_type(std::move(type)) { }
    virtual ~lunar_ir_rthreadstream() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_wthreadstream : public lunar_ir_type {
public:
    lunar_ir_wthreadstream(std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_WTHREADSTREAM, OWN_SHARED), m_type(std::move(type)) { }
    virtual ~lunar_ir_wthreadstream() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_string : public lunar_ir_type {
public:
    lunar_ir_string(LANG_OWNERSHIP owner_ship) : lunar_ir_type(BT_STRING, owner_ship) { }
    virtual ~lunar_ir_string() { }

    virtual void print(std::string &s, const std::string &from);
};

class lunar_ir_binary : public lunar_ir_type {
public:
    lunar_ir_binary(LANG_OWNERSHIP owner_ship) : lunar_ir_type(BT_BINARY, owner_ship) { }
    virtual ~lunar_ir_binary() { }

    virtual void print(std::string &s, const std::string &from);
};

class lunar_ir_ptr : public lunar_ir_type {
public:
    lunar_ir_ptr(LANG_OWNERSHIP owner_ship, std::unique_ptr<lunar_ir_type> type)
        : lunar_ir_type(BT_PTR, owner_ship),
          m_type(std::move(type)) { }
    virtual ~lunar_ir_ptr() { }

    virtual void print(std::string &s, const std::string &from);

private:
    std::unique_ptr<lunar_ir_type> m_type;
};

class lunar_ir_parsec : public lunar_ir_type {
public:
    lunar_ir_parsec(bool is_binary) // binary or string
        : lunar_ir_type(BT_PARSEC, OWN_UNIQUE), m_is_binary(is_binary) { }
    virtual ~lunar_ir_parsec() { }

    bool is_binary() { return m_is_binary; }

    virtual void print(std::string &s, const std::string &from);

private:
    bool m_is_binary;
};

class lunar_ir_var : public lunar_ir_base {
public:
    lunar_ir_var(std::unique_ptr<lunar_ir_type> type, const std::u32string &name)
        : m_type(std::move(type)), m_name(name) { }

    std::u32string& get_name() { return m_name; }

private:
    std::unique_ptr<lunar_ir_type> m_type;
    std::u32string m_name;
};

class lunar_ir_defun : public lunar_ir_top {
public:
    lunar_ir_defun(const std::u32string &name) : lunar_ir_top(IR_FUNC), m_name(name) { }
    virtual ~lunar_ir_defun() { }

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
    std::unordered_map<std::u32string, lunar_ir_var*> m_argmap;
    std::u32string m_name;
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
        std::unordered_map<std::u32string, lunar_ir_var*> m_argmap;
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

}

#endif // LUNAR_IR_TREE