//////////////////////////////////////
// ChocoLang 0.6.0 - Almond Amour
// CoffeeShop Development
// Made by Camila "Mocha" Rose
//////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <functional>
#include "choco_gui.h"

// Token types
enum TokenType {
    TOKEN_EOF, TOKEN_NUMBER, TOKEN_STRING, TOKEN_IDENTIFIER,
    TOKEN_LET, TOKEN_FN, TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_FOR, TOKEN_IN,
    TOKEN_RETURN, TOKEN_PUTS, TOKEN_TRUE, TOKEN_FALSE, TOKEN_STRUCT, TOKEN_IMPL,
    TOKEN_IMPORT, TOKEN_FROM, TOKEN_TRY, TOKEN_CATCH, TOKEN_THROW, TOKEN_BREAK, TOKEN_CONTINUE,
    TOKEN_MATCH, TOKEN_CASE, TOKEN_DEFAULT, TOKEN_ARROW_FAT, TOKEN_ASYNC, TOKEN_AWAIT,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL, TOKEN_BANG_EQUAL,
    TOKEN_LESS, TOKEN_GREATER, TOKEN_LESS_EQUAL, TOKEN_GREATER_EQUAL,
    TOKEN_AND, TOKEN_OR, TOKEN_BANG,
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_LBRACKET, TOKEN_RBRACKET,
    TOKEN_COMMA, TOKEN_SEMICOLON, TOKEN_ARROW, TOKEN_DOT, TOKEN_DOTDOT, TOKEN_COLON,
    TOKEN_PIPE
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

// Runtime error exception
class RuntimeError : public std::runtime_error {
public:
    int line;
    RuntimeError(const std::string& msg, int line_num) 
        : std::runtime_error(msg), line(line_num) {}
};

// Parser error exception
class ParseError : public std::runtime_error {
public:
    int line;
    ParseError(const std::string& msg, int line_num) 
        : std::runtime_error(msg), line(line_num) {}
};

// Lexer error exception
class LexerError : public std::runtime_error {
public:
    int line;
    LexerError(const std::string& msg, int line_num) 
        : std::runtime_error(msg), line(line_num) {}
};

// Lexer
class Lexer {
    std::string source;
    size_t pos = 0;
    int line = 1;
    
    static const std::unordered_map<std::string, TokenType> keywords;

public:
    Lexer(const std::string& src) : source(src) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        tokens.reserve(source.length() / 4);
        
        try {
            while (pos < source.length()) {
                skipWhitespace();
                if (pos >= source.length()) break;

                if (source[pos] == '/' && pos + 1 < source.length() && source[pos + 1] == '/') {
                    skipComment();
                    continue;
                }

                Token tok = nextToken();
                tokens.push_back(std::move(tok));
            }
            tokens.push_back({TOKEN_EOF, "", line});
            tokens.shrink_to_fit();
        } catch (const LexerError& e) {
            std::cerr << "Lexer Error on line " << e.line << ": " << e.what() << std::endl;
            throw;
        }
        
        return tokens;
    }

private:
    void skipWhitespace() {
        while (pos < source.length() && std::isspace(static_cast<unsigned char>(source[pos]))) {
            if (source[pos] == '\n') line++;
            pos++;
        }
    }

    void skipComment() {
        while (pos < source.length() && source[pos] != '\n') pos++;
    }

    Token nextToken() {
        char c = source[pos];

        if (std::isdigit(static_cast<unsigned char>(c))) return number();
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') return identifier();
        if (c == '"') return string();

        pos++;
        switch (c) {
            case '+': return {TOKEN_PLUS, "+", line};
            case '*': return {TOKEN_STAR, "*", line};
            case '/': return {TOKEN_SLASH, "/", line};
            case '%': return {TOKEN_PERCENT, "%", line};
            case '(': return {TOKEN_LPAREN, "(", line};
            case ')': return {TOKEN_RPAREN, ")", line};
            case '{': return {TOKEN_LBRACE, "{", line};
            case '}': return {TOKEN_RBRACE, "}", line};
            case '[': return {TOKEN_LBRACKET, "[", line};
            case ']': return {TOKEN_RBRACKET, "]", line};
            case ',': return {TOKEN_COMMA, ",", line};
            case ';': return {TOKEN_SEMICOLON, ";", line};
            case ':': return {TOKEN_COLON, ":", line};
            case '.':
                if (pos < source.length() && source[pos] == '.') {
                    pos++;
                    return {TOKEN_DOTDOT, "..", line};
                }
                return {TOKEN_DOT, ".", line};
            case '-':
                if (pos < source.length() && source[pos] == '>') {
                    pos++;
                    return {TOKEN_ARROW, "->", line};
                }
                return {TOKEN_MINUS, "-", line};
            case '=':
                if (pos < source.length() && source[pos] == '=') {
                    pos++;
                    return {TOKEN_EQUAL_EQUAL, "==", line};
                } else if (pos < source.length() && source[pos] == '>') {
                    pos++;
                    return {TOKEN_ARROW_FAT, "=>", line};
                }
                return {TOKEN_EQUAL, "=", line};
            case '!':
                if (pos < source.length() && source[pos] == '=') {
                    pos++;
                    return {TOKEN_BANG_EQUAL, "!=", line};
                }
                return {TOKEN_BANG, "!", line};
            case '<':
                if (pos < source.length() && source[pos] == '=') {
                    pos++;
                    return {TOKEN_LESS_EQUAL, "<=", line};
                }
                return {TOKEN_LESS, "<", line};
            case '>':
                if (pos < source.length() && source[pos] == '=') {
                    pos++;
                    return {TOKEN_GREATER_EQUAL, ">=", line};
                }
                return {TOKEN_GREATER, ">", line};
            case '&':
                if (pos < source.length() && source[pos] == '&') {
                    pos++;
                    return {TOKEN_AND, "&&", line};
                }
                throw LexerError("Unexpected character '&'. Did you mean '&&'?", line);
            case '|':
                if (pos < source.length() && source[pos] == '|') {
                    pos++;
                    return {TOKEN_OR, "||", line};
                }
                return {TOKEN_PIPE, "|", line};
            default:
                throw LexerError("Unexpected character: '" + std::string(1, c) + "'", line);
        }
        return {TOKEN_EOF, "", line};
    }

    Token number() {
        std::string num;
        num.reserve(16);
        bool hasDot = false;
        int startLine = line;
        
        while (pos < source.length()) {
            if (std::isdigit(static_cast<unsigned char>(source[pos]))) {
                num += source[pos++];
            } else if (source[pos] == '.' && !hasDot) {
                if (pos + 1 < source.length() && source[pos + 1] == '.') {
                    break;
                }
                if (pos + 1 < source.length() && std::isdigit(static_cast<unsigned char>(source[pos + 1]))) {
                    hasDot = true;
                    num += source[pos++];
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        
        return {TOKEN_NUMBER, num, startLine};
    }

    Token identifier() {
        std::string id;
        id.reserve(32);
        int startLine = line;
        
        while (pos < source.length() && (std::isalnum(static_cast<unsigned char>(source[pos])) || source[pos] == '_')) {
            id += source[pos++];
        }

        auto it = keywords.find(id);
        if (it != keywords.end()) {
            return {it->second, id, startLine};
        }

        return {TOKEN_IDENTIFIER, id, startLine};
    }

    Token string() {
        int startLine = line;
        pos++; // skip opening "
        std::string str;
        str.reserve(64);
        
        while (pos < source.length() && source[pos] != '"') {
            if (source[pos] == '\n') {
                throw LexerError("Unterminated string literal", startLine);
            }
            if (source[pos] == '\\' && pos + 1 < source.length()) {
                pos++;
                switch (source[pos]) {
                    case 'n': str += '\n'; break;
                    case 't': str += '\t'; break;
                    case '\\': str += '\\'; break;
                    case '"': str += '"'; break;
                    default: str += source[pos];
                }
                pos++;
            } else if (source[pos] == '#' && pos + 1 < source.length() && source[pos + 1] == '{') {
                str += "#{";
                pos += 2;
            } else {
                str += source[pos++];
            }
        }
        
        if (pos >= source.length()) {
            throw LexerError("Unterminated string literal", startLine);
        }
        
        pos++; // skip closing "
        return {TOKEN_STRING, str, startLine};
    }
};

const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"let", TOKEN_LET}, {"fn", TOKEN_FN}, {"if", TOKEN_IF}, {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE}, {"for", TOKEN_FOR}, {"in", TOKEN_IN}, {"return", TOKEN_RETURN},
    {"puts", TOKEN_PUTS}, {"true", TOKEN_TRUE}, {"false", TOKEN_FALSE}, {"struct", TOKEN_STRUCT},
    {"impl", TOKEN_IMPL}, {"import", TOKEN_IMPORT}, {"from", TOKEN_FROM}, {"try", TOKEN_TRY},
    {"catch", TOKEN_CATCH}, {"throw", TOKEN_THROW}, {"break", TOKEN_BREAK}, {"continue", TOKEN_CONTINUE},
    {"match", TOKEN_MATCH}, {"case", TOKEN_CASE}, {"default", TOKEN_DEFAULT}, {"async", TOKEN_ASYNC},
    {"await", TOKEN_AWAIT}
};

// Forward declarations
class Interpreter;

// Value types
struct Value {
    enum Type { NUMBER, STRING, BOOL, ARRAY, STRUCT, LAMBDA, NIL } type;
    double num;
    std::string str;
    bool boolean;
    std::vector<Value> array;
    std::unordered_map<std::string, Value> structFields;
    std::string structType;
    
    std::vector<std::string> lambdaParams;
    size_t lambdaBodyStart;
    size_t lambdaBodyEnd;
    std::unordered_map<std::string, Value> closureCaptures;

    Value() : type(NIL), num(0), boolean(false), lambdaBodyStart(0), lambdaBodyEnd(0) {}
    Value(double n) : type(NUMBER), num(n), boolean(false), lambdaBodyStart(0), lambdaBodyEnd(0) {}
    Value(const std::string& s) : type(STRING), str(s), num(0), boolean(false), lambdaBodyStart(0), lambdaBodyEnd(0) {}
    Value(bool b) : type(BOOL), num(0), boolean(b), lambdaBodyStart(0), lambdaBodyEnd(0) {}
    Value(const std::vector<Value>& arr) : type(ARRAY), num(0), boolean(false), array(arr), lambdaBodyStart(0), lambdaBodyEnd(0) {}

    std::string toString() const {
        switch (type) {
            case NUMBER: {
                if (num == static_cast<int>(num)) {
                    return std::to_string(static_cast<int>(num));
                }
                std::string s = std::to_string(num);
                s.erase(s.find_last_not_of('0') + 1, std::string::npos);
                if (s.back() == '.') s.pop_back();
                return s;
            }
            case STRING: return str;
            case BOOL: return boolean ? "true" : "false";
            case ARRAY: {
                std::string result = "[";
                for (size_t i = 0; i < array.size(); i++) {
                    result += array[i].toString();
                    if (i < array.size() - 1) result += ", ";
                }
                result += "]";
                return result;
            }
            case STRUCT: {
                std::string result = structType + " { ";
                bool first = true;
                for (const auto& field : structFields) {
                    if (!first) result += ", ";
                    result += field.first + ": " + field.second.toString();
                    first = false;
                }
                result += " }";
                return result;
            }
            case LAMBDA: return "<lambda>";
            case NIL: return "nil";
        }
        return "";
    }
    
    std::string getType() const {
        switch (type) {
            case NUMBER: return "number";
            case STRING: return "string";
            case BOOL: return "bool";
            case ARRAY: return "array";
            case STRUCT: return structType.empty() ? "struct" : structType;
            case LAMBDA: return "lambda";
            case NIL: return "nil";
        }
        return "unknown";
    }
};

struct Function {
    std::vector<std::string> params;
    size_t bodyStart;
    size_t bodyEnd;
};

struct StructDef {
    std::vector<std::string> fields;
};

struct ChocoException {
    std::string message;
    ChocoException(const std::string& msg) : message(msg) {}
};

// Interpreter
class Interpreter {
public:
    std::unordered_map<std::string, Value> globalVars;
    std::vector<std::unordered_map<std::string, Value>> scopes;
    std::unordered_map<std::string, Function> functions;
    std::unordered_map<std::string, StructDef> structDefs;
    std::vector<Token> tokens;
    size_t current;
    bool inFunction;
    bool inLoop;
    bool hasReturned;
    Value returnValue;
    bool shouldBreak;
    bool shouldContinue;
    bool inTryCatch;
    std::string currentException;
    
    static const std::unordered_map<std::string, bool> builtinFunctions;

    Value callFunction(const std::string& name, const std::vector<Value>& args, int callLine) {
        // Higher-order functions
        if (name == "map") {
            if (args.size() < 2) {
                throw RuntimeError("map() expects 2 arguments (array, lambda), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::ARRAY) {
                throw RuntimeError("map() first argument must be an array, got " + args[0].getType(), callLine);
            }
            if (args[1].type != Value::LAMBDA) {
                throw RuntimeError("map() second argument must be a lambda, got " + args[1].getType(), callLine);
            }
            std::vector<Value> result;
            result.reserve(args[0].array.size());
            for (const auto& item : args[0].array) {
                std::vector<Value> lambdaArgs = {item};
                result.push_back(callLambda(args[1], lambdaArgs));
            }
            return Value(result);
        }
        
        if (name == "filter") {
            if (args.size() < 2) {
                throw RuntimeError("filter() expects 2 arguments (array, lambda), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::ARRAY) {
                throw RuntimeError("filter() first argument must be an array, got " + args[0].getType(), callLine);
            }
            if (args[1].type != Value::LAMBDA) {
                throw RuntimeError("filter() second argument must be a lambda, got " + args[1].getType(), callLine);
            }
            std::vector<Value> result;
            for (const auto& item : args[0].array) {
                std::vector<Value> lambdaArgs = {item};
                Value condition = callLambda(args[1], lambdaArgs);
                if (condition.type == Value::BOOL && condition.boolean) {
                    result.push_back(item);
                }
            }
            return Value(result);
        }
        
        if (name == "reduce") {
            if (args.size() < 3) {
                throw RuntimeError("reduce() expects 3 arguments (array, initial, lambda), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::ARRAY) {
                throw RuntimeError("reduce() first argument must be an array, got " + args[0].getType(), callLine);
            }
            if (args[2].type != Value::LAMBDA) {
                throw RuntimeError("reduce() third argument must be a lambda, got " + args[2].getType(), callLine);
            }
            Value accumulator = args[1];
            for (const auto& item : args[0].array) {
                std::vector<Value> lambdaArgs = {accumulator, item};
                accumulator = callLambda(args[2], lambdaArgs);
            }
            return accumulator;
        }
        
        if (name == "typeof") {
            if (args.size() == 0) {
                throw RuntimeError("typeof() expects 1 argument, got 0", callLine);
            }
            return Value(args[0].getType());
        }
        
        // Standard library functions
        if (name == "len") {
            if (args.size() == 0) {
                throw RuntimeError("len() expects 1 argument, got 0", callLine);
            }
            if (args[0].type == Value::ARRAY) {
                return Value(static_cast<double>(args[0].array.size()));
            } else if (args[0].type == Value::STRING) {
                return Value(static_cast<double>(args[0].str.length()));
            }
            throw RuntimeError("len() requires array or string, got " + args[0].getType(), callLine);
        }
        
        if (name == "push") {
            if (args.size() < 2) {
                throw RuntimeError("push() expects 2 arguments (array, value), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::ARRAY) {
                throw RuntimeError("push() first argument must be an array, got " + args[0].getType(), callLine);
            }
            Value arr = args[0];
            arr.array.push_back(args[1]);
            return arr;
        }
        
        if (name == "pop") {
            if (args.size() == 0) {
                throw RuntimeError("pop() expects 1 argument (array), got 0", callLine);
            }
            if (args[0].type != Value::ARRAY) {
                throw RuntimeError("pop() requires an array, got " + args[0].getType(), callLine);
            }
            Value arr = args[0];
            if (arr.array.empty()) {
                throw RuntimeError("Cannot pop from empty array", callLine);
            }
            Value last = arr.array.back();
            arr.array.pop_back();
            return last;
        }
        
        if (name == "sqrt") {
            if (args.size() == 0) {
                throw RuntimeError("sqrt() expects 1 argument, got 0", callLine);
            }
            if (args[0].type != Value::NUMBER) {
                throw RuntimeError("sqrt() requires a number, got " + args[0].getType(), callLine);
            }
            if (args[0].num < 0) {
                throw RuntimeError("sqrt() of negative number", callLine);
            }
            return Value(sqrt(args[0].num));
        }
        
        if (name == "pow") {
            if (args.size() < 2) {
                throw RuntimeError("pow() expects 2 arguments (base, exponent), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::NUMBER || args[1].type != Value::NUMBER) {
                throw RuntimeError("pow() requires two numbers", callLine);
            }
            return Value(pow(args[0].num, args[1].num));
        }
        
        if (name == "abs") {
            if (args.size() == 0) {
                throw RuntimeError("abs() expects 1 argument, got 0", callLine);
            }
            if (args[0].type != Value::NUMBER) {
                throw RuntimeError("abs() requires a number, got " + args[0].getType(), callLine);
            }
            return Value(fabs(args[0].num));
        }
        
        if (name == "floor") {
            if (args.size() == 0) {
                throw RuntimeError("floor() expects 1 argument, got 0", callLine);
            }
            if (args[0].type != Value::NUMBER) {
                throw RuntimeError("floor() requires a number, got " + args[0].getType(), callLine);
            }
            return Value(floor(args[0].num));
        }
        
        if (name == "ceil") {
            if (args.size() == 0) {
                throw RuntimeError("ceil() expects 1 argument, got 0", callLine);
            }
            if (args[0].type != Value::NUMBER) {
                throw RuntimeError("ceil() requires a number, got " + args[0].getType(), callLine);
            }
            return Value(ceil(args[0].num));
        }
        
        if (name == "round") {
            if (args.size() == 0) {
                throw RuntimeError("round() expects 1 argument, got 0", callLine);
            }
            if (args[0].type != Value::NUMBER) {
                throw RuntimeError("round() requires a number, got " + args[0].getType(), callLine);
            }
            return Value(round(args[0].num));
        }
        
        if (name == "min") {
            if (args.size() < 2) {
                throw RuntimeError("min() expects 2 arguments, got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::NUMBER || args[1].type != Value::NUMBER) {
                throw RuntimeError("min() requires two numbers", callLine);
            }
            return Value(std::min(args[0].num, args[1].num));
        }
        
        if (name == "max") {
            if (args.size() < 2) {
                throw RuntimeError("max() expects 2 arguments, got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::NUMBER || args[1].type != Value::NUMBER) {
                throw RuntimeError("max() requires two numbers", callLine);
            }
            return Value(std::max(args[0].num, args[1].num));
        }
        
        if (name == "random") {
            return Value(static_cast<double>(rand()) / RAND_MAX);
        }
        
        if (name == "random_int") {
            if (args.size() < 2) {
                throw RuntimeError("random_int() expects 2 arguments (min, max), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::NUMBER || args[1].type != Value::NUMBER) {
                throw RuntimeError("random_int() requires two numbers", callLine);
            }
            int min = static_cast<int>(args[0].num);
            int max = static_cast<int>(args[1].num);
            if (min > max) {
                throw RuntimeError("random_int(): min cannot be greater than max", callLine);
            }
            return Value(static_cast<double>(min + rand() % (max - min + 1)));
        }
        
        if (name == "str") {
            if (args.size() == 0) {
                return Value("");
            }
            return Value(args[0].toString());
        }
        
        if (name == "int") {
            if (args.size() == 0) {
                throw RuntimeError("int() expects 1 argument, got 0", callLine);
            }
            if (args[0].type == Value::NUMBER) {
                return Value(static_cast<double>(static_cast<int>(args[0].num)));
            } else if (args[0].type == Value::STRING) {
                try {
                    return Value(static_cast<double>(std::stoi(args[0].str)));
                } catch (...) {
                    throw RuntimeError("int(): cannot convert '" + args[0].str + "' to integer", callLine);
                }
            }
            throw RuntimeError("int() requires number or string, got " + args[0].getType(), callLine);
        }
        
        if (name == "float") {
            if (args.size() == 0) {
                throw RuntimeError("float() expects 1 argument, got 0", callLine);
            }
            if (args[0].type == Value::STRING) {
                try {
                    return Value(std::stod(args[0].str));
                } catch (...) {
                    throw RuntimeError("float(): cannot convert '" + args[0].str + "' to float", callLine);
                }
            } else if (args[0].type == Value::NUMBER) {
                return args[0];
            }
            throw RuntimeError("float() requires number or string, got " + args[0].getType(), callLine);
        }
        
        if (name == "uppercase") {
            if (args.size() == 0) {
                throw RuntimeError("uppercase() expects 1 argument, got 0", callLine);
            }
            if (args[0].type != Value::STRING) {
                throw RuntimeError("uppercase() requires a string, got " + args[0].getType(), callLine);
            }
            std::string result = args[0].str;
            std::transform(result.begin(), result.end(), result.begin(), ::toupper);
            return Value(result);
        }
        
        if (name == "lowercase") {
            if (args.size() == 0) {
                throw RuntimeError("lowercase() expects 1 argument, got 0", callLine);
            }
            if (args[0].type != Value::STRING) {
                throw RuntimeError("lowercase() requires a string, got " + args[0].getType(), callLine);
            }
            std::string result = args[0].str;
            std::transform(result.begin(), result.end(), result.begin(), ::tolower);
            return Value(result);
        }
        
        if (name == "substr") {
            if (args.size() < 3) {
                throw RuntimeError("substr() expects 3 arguments (string, start, length), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::STRING) {
                throw RuntimeError("substr() first argument must be a string, got " + args[0].getType(), callLine);
            }
            if (args[1].type != Value::NUMBER || args[2].type != Value::NUMBER) {
                throw RuntimeError("substr() start and length must be numbers", callLine);
            }
            int start = static_cast<int>(args[1].num);
            int length = static_cast<int>(args[2].num);
            if (start < 0 || start >= static_cast<int>(args[0].str.length())) {
                throw RuntimeError("substr(): start index out of bounds", callLine);
            }
            return Value(args[0].str.substr(start, length));
        }
        
        if (name == "split") {
            if (args.size() < 2) {
                throw RuntimeError("split() expects 2 arguments (string, delimiter), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::STRING || args[1].type != Value::STRING) {
                throw RuntimeError("split() requires two strings", callLine);
            }
            std::vector<Value> result;
            std::string str = args[0].str;
            std::string delim = args[1].str;
            if (delim.empty()) {
                throw RuntimeError("split(): delimiter cannot be empty", callLine);
            }
            size_t pos = 0;
            while ((pos = str.find(delim)) != std::string::npos) {
                result.push_back(Value(str.substr(0, pos)));
                str.erase(0, pos + delim.length());
            }
            result.push_back(Value(str));
            return Value(result);
        }
        
        if (name == "join") {
            if (args.size() < 2) {
                throw RuntimeError("join() expects 2 arguments (array, separator), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::ARRAY) {
                throw RuntimeError("join() first argument must be an array, got " + args[0].getType(), callLine);
            }
            if (args[1].type != Value::STRING) {
                throw RuntimeError("join() second argument must be a string, got " + args[1].getType(), callLine);
            }
            std::string result;
            for (size_t i = 0; i < args[0].array.size(); i++) {
                result += args[0].array[i].toString();
                if (i < args[0].array.size() - 1) {
                    result += args[1].str;
                }
            }
            return Value(result);
        }
        
        if (name == "read_file") {
            if (args.size() == 0) {
                throw RuntimeError("read_file() expects 1 argument (filename), got 0", callLine);
            }
            if (args[0].type != Value::STRING) {
                throw RuntimeError("read_file() requires a string filename, got " + args[0].getType(), callLine);
            }
            std::ifstream file(args[0].str);
            if (!file) {
                throw RuntimeError("read_file(): cannot open file '" + args[0].str + "'", callLine);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            return Value(buffer.str());
        }
        
        if (name == "write_file") {
            if (args.size() < 2) {
                throw RuntimeError("write_file() expects 2 arguments (filename, content), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::STRING || args[1].type != Value::STRING) {
                throw RuntimeError("write_file() requires two strings", callLine);
            }
            std::ofstream file(args[0].str);
            if (!file) {
                throw RuntimeError("write_file(): cannot open file '" + args[0].str + "' for writing", callLine);
            }
            file << args[1].str;
            return Value(true);
        }
        
        if (name == "append_file") {
            if (args.size() < 2) {
                throw RuntimeError("append_file() expects 2 arguments (filename, content), got " + std::to_string(args.size()), callLine);
            }
            if (args[0].type != Value::STRING || args[1].type != Value::STRING) {
                throw RuntimeError("append_file() requires two strings", callLine);
            }
            std::ofstream file(args[0].str, std::ios::app);
            if (!file) {
                throw RuntimeError("append_file(): cannot open file '" + args[0].str + "' for appending", callLine);
            }
            file << args[1].str;
            return Value(true);
        }
        
        if (name == "file_exists") {
            if (args.size() == 0) {
                throw RuntimeError("file_exists() expects 1 argument (filename), got 0", callLine);
            }
            if (args[0].type != Value::STRING) {
                throw RuntimeError("file_exists() requires a string filename, got " + args[0].getType(), callLine);
            }
            std::ifstream file(args[0].str);
            return Value(file.good());
        }
        
        if (name == "input") {
            // input() - read a line from stdin
            // input(prompt) - print prompt then read a line
            std::string prompt = "";
            if (args.size() > 0) {
                if (args[0].type != Value::STRING) {
                    throw RuntimeError("input() prompt must be a string, got " + args[0].getType(), callLine);
                }
                prompt = args[0].str;
            }
            
            if (!prompt.empty()) {
                std::cout << prompt;
                std::cout.flush();
            }
            
            std::string line;
            if (std::getline(std::cin, line)) {
                return Value(line);
            } else {
                return Value("");
            }
        }

        ChocoGUI* gui = ChocoGUI::getInstance(0, nullptr);
        gui->setInterpreter(this);

        if (name == "gui_init") return gui->gui_init(args, callLine);
        if (name == "gui_window") return gui->gui_window(args, callLine);
        if (name == "gui_button") return gui->gui_button(args, callLine);
        if (name == "gui_label") return gui->gui_label(args, callLine);
        if (name == "gui_entry") return gui->gui_entry(args, callLine);
        if (name == "gui_box") return gui->gui_box(args, callLine);
        if (name == "gui_add") return gui->gui_add(args, callLine);
        if (name == "gui_set_text") return gui->gui_set_text(args, callLine);
        if (name == "gui_get_text") return gui->gui_get_text(args, callLine);
        if (name == "gui_on") return gui->gui_on(args, callLine);
        if (name == "gui_show") return gui->gui_show(args, callLine);
        if (name == "gui_run") return gui->gui_run(args, callLine);
        if (name == "gui_quit") return gui->gui_quit(args, callLine);
        if (name == "gui_checkbox") return gui->gui_checkbox(args, callLine);
        if (name == "gui_textview") return gui->gui_textview(args, callLine);
        if (name == "gui_frame") return gui->gui_frame(args, callLine);
        if (name == "gui_separator") return gui->gui_separator(args, callLine);
        if (name == "gui_set_sensitive") return gui->gui_set_sensitive(args, callLine);
        if (name == "gui_get_checked") return gui->gui_get_checked(args, callLine);
        if (name == "gui_set_checked") return gui->gui_set_checked(args, callLine);
        // User-defined functions
        auto it = functions.find(name);
        if (it == functions.end()) {
            throw RuntimeError("Undefined function '" + name + "'", callLine);
        }

        Function& func = it->second;
        
        if (args.size() < func.params.size()) {
            throw RuntimeError("Function '" + name + "' expects " + std::to_string(func.params.size()) + 
                             " arguments, got " + std::to_string(args.size()), callLine);
        }
        
        scopes.push_back(std::unordered_map<std::string, Value>());
        
        for (size_t i = 0; i < func.params.size() && i < args.size(); i++) {
            scopes.back()[func.params[i]] = args[i];
        }

        size_t savedCurrent = current;
        current = func.bodyStart;
        bool wasInFunction = inFunction;
        inFunction = true;
        hasReturned = false;
        returnValue = Value();

        while (current < func.bodyEnd && !isAtEnd() && !hasReturned) {
            statement();
        }

        Value result = returnValue;
        hasReturned = false;
        inFunction = wasInFunction;
        
        scopes.pop_back();
        
        current = savedCurrent;
        return result;
    }

    Interpreter(const std::vector<Token>& toks) : tokens(toks), current(0), 
        inFunction(false), inLoop(false), hasReturned(false), shouldBreak(false), 
        shouldContinue(false), inTryCatch(false) {
        scopes.push_back(std::unordered_map<std::string, Value>());
        scopes.reserve(16);
        srand(time(nullptr));
    }

    void execute() {
        try {
            while (!isAtEnd()) {
                statement();
            }
        } catch (const RuntimeError& e) {
            std::cerr << "\n[Runtime Error] Line " << e.line << ": " << e.what() << std::endl;
            throw;
        } catch (const ParseError& e) {
            std::cerr << "\n[Parse Error] Line " << e.line << ": " << e.what() << std::endl;
            throw;
        }
    }
    
    inline bool isAtEnd() const { 
        return tokens.empty() || current >= tokens.size() || tokens[current].type == TOKEN_EOF; 
    }

    inline Token peek() const { 
        if (current >= tokens.size()) {
            return {TOKEN_EOF, "", tokens.empty() ? 1 : tokens.back().line};
        }
        return tokens[current]; 
    }
    
    inline Token advance() { 
        if (current >= tokens.size()) {
            throw ParseError("Unexpected end of file", tokens.empty() ? 1 : tokens.back().line);
        }
        return tokens[current++]; 
    }
    
    inline bool match(TokenType type) {
        if (peek().type == type) {
            advance();
            return true;
        }
        return false;
    }
    
    void expect(TokenType type, const std::string& message) {
        if (!match(type)) {
            throw ParseError(message, peek().line);
        }
    }

    inline bool isBuiltinFunction(const std::string& name) const {
        return builtinFunctions.find(name) != builtinFunctions.end();
    }

    void setVariable(const std::string& name, const Value& val) {
        for (int i = scopes.size() - 1; i >= 0; i--) {
            if (scopes[i].find(name) != scopes[i].end()) {
                scopes[i][name] = val;
                return;
            }
        }
        scopes.back()[name] = val;
    }

    Value getVariable(const std::string& name) {
        for (int i = scopes.size() - 1; i >= 0; i--) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) {
                return it->second;
            }
        }
        auto it = globalVars.find(name);
        if (it != globalVars.end()) {
            return it->second;
        }
        throw RuntimeError("Undefined variable '" + name + "'", peek().line);
    }

    void statement() {
        if (hasReturned || shouldBreak || shouldContinue) return;

        if (match(TOKEN_LET)) {
            letStatement();
        } else if (match(TOKEN_FN)) {
            functionDeclaration();
        } else if (match(TOKEN_STRUCT)) {
            structDeclaration();
        } else if (match(TOKEN_IMPORT)) {
            importStatement();
        } else if (match(TOKEN_TRY)) {
            tryStatement();
        } else if (match(TOKEN_THROW)) {
            throwStatement();
        } else if (match(TOKEN_BREAK)) {
            if (!inLoop) {
                throw RuntimeError("'break' can only be used inside loops", tokens[current - 1].line);
            }
            shouldBreak = true;
            match(TOKEN_SEMICOLON);
        } else if (match(TOKEN_CONTINUE)) {
            if (!inLoop) {
                throw RuntimeError("'continue' can only be used inside loops", tokens[current - 1].line);
            }
            shouldContinue = true;
            match(TOKEN_SEMICOLON);
        } else if (match(TOKEN_PUTS)) {
            putsStatement();
        } else if (match(TOKEN_IF)) {
            ifStatement();
        } else if (match(TOKEN_WHILE)) {
            whileStatement();
        } else if (match(TOKEN_FOR)) {
            forStatement();
        } else if (match(TOKEN_MATCH)) {
            matchStatement();
        } else if (match(TOKEN_RETURN)) {
            if (!inFunction) {
                throw RuntimeError("'return' can only be used inside functions", tokens[current - 1].line);
            }
            returnStatement();
        } else if (peek().type == TOKEN_IDENTIFIER && current + 1 < tokens.size() && tokens[current + 1].type == TOKEN_EQUAL) {
            Token name = advance();
            match(TOKEN_EQUAL);
            Value val = expression();
            setVariable(name.value, val);
            expect(TOKEN_SEMICOLON, "Expected ';' after assignment");
        } else {
            expression();
            expect(TOKEN_SEMICOLON, "Expected ';' after expression");
        }
    }

    void letStatement() {
        if (peek().type != TOKEN_IDENTIFIER) {
            throw ParseError("Expected variable name after 'let'", peek().line);
        }
        Token name = advance();
        expect(TOKEN_EQUAL, "Expected '=' after variable name");
        Value val = expression();
        setVariable(name.value, val);
        expect(TOKEN_SEMICOLON, "Expected ';' after variable declaration");
    }

    void functionDeclaration() {
        if (peek().type != TOKEN_IDENTIFIER) {
            throw ParseError("Expected function name after 'fn'", peek().line);
        }
        Token name = advance();
        expect(TOKEN_LPAREN, "Expected '(' after function name");
        
        std::vector<std::string> params;
        while (!match(TOKEN_RPAREN)) {
            if (peek().type != TOKEN_IDENTIFIER) {
                throw ParseError("Expected parameter name", peek().line);
            }
            Token param = advance();
            params.push_back(std::move(param.value));
            if (!match(TOKEN_COMMA)) {
                expect(TOKEN_RPAREN, "Expected ')' or ',' in parameter list");
                break;
            }
        }

        expect(TOKEN_LBRACE, "Expected '{' before function body");
        size_t bodyStart = current;
        
        int braceCount = 1;
        while (braceCount > 0 && !isAtEnd()) {
            if (tokens[current].type == TOKEN_LBRACE) braceCount++;
            if (tokens[current].type == TOKEN_RBRACE) braceCount--;
            current++;
        }
        
        if (braceCount != 0) {
            throw ParseError("Unclosed function body", name.line);
        }
        
        size_t bodyEnd = current - 1;
        functions[name.value] = {std::move(params), bodyStart, bodyEnd};
        
        // Store function name as a variable so it can be referenced
        setVariable(name.value, Value(name.value));
    }

    void structDeclaration() {
        if (peek().type != TOKEN_IDENTIFIER) {
            throw ParseError("Expected struct name after 'struct'", peek().line);
        }
        Token name = advance();
        expect(TOKEN_LBRACE, "Expected '{' after struct name");
        
        std::vector<std::string> fields;
        while (!match(TOKEN_RBRACE)) {
            if (peek().type != TOKEN_IDENTIFIER) {
                throw ParseError("Expected field name in struct", peek().line);
            }
            Token field = advance();
            fields.push_back(std::move(field.value));
            if (!match(TOKEN_COMMA)) {
                expect(TOKEN_RBRACE, "Expected '}' or ',' in struct definition");
                break;
            }
        }
        
        structDefs[name.value] = {std::move(fields)};
    }

    void importStatement() {
        if (peek().type != TOKEN_IDENTIFIER) {
            throw ParseError("Expected module name after 'import'", peek().line);
        }
        Token module = advance();
        expect(TOKEN_SEMICOLON, "Expected ';' after import statement");
        
        std::string filename = module.value + ".choco";
        std::ifstream file(filename);
        if (!file) {
            throw RuntimeError("Could not import module '" + module.value + "'. File '" + filename + "' not found", module.line);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        try {
            Lexer lexer(source);
            std::vector<Token> moduleTokens = lexer.tokenize();
            
            size_t savedCurrent = current;
            std::vector<Token> savedTokens = std::move(tokens);
            
            tokens = std::move(moduleTokens);
            current = 0;
            
            while (!isAtEnd()) {
                statement();
            }
            
            tokens = std::move(savedTokens);
            current = savedCurrent;
        } catch (...) {
            throw RuntimeError("Error while importing module '" + module.value + "'", module.line);
        }
    }

    void tryStatement() {
        expect(TOKEN_LBRACE, "Expected '{' after 'try'");
        size_t tryStart = current;
        
        int braceCount = 1;
        size_t tryEnd = current;
        while (braceCount > 0 && tryEnd < tokens.size()) {
            if (tokens[tryEnd].type == TOKEN_LBRACE) braceCount++;
            if (tokens[tryEnd].type == TOKEN_RBRACE) braceCount--;
            if (braceCount > 0) tryEnd++;
        }
        
        current = tryEnd + 1;
        expect(TOKEN_CATCH, "Expected 'catch' after try block");
        
        if (peek().type != TOKEN_IDENTIFIER) {
            throw ParseError("Expected error variable name after 'catch'", peek().line);
        }
        Token errorVar = advance();
        expect(TOKEN_LBRACE, "Expected '{' after catch variable");
        size_t catchStart = current;
        
        braceCount = 1;
        size_t catchEnd = current;
        while (braceCount > 0 && catchEnd < tokens.size()) {
            if (tokens[catchEnd].type == TOKEN_LBRACE) braceCount++;
            if (tokens[catchEnd].type == TOKEN_RBRACE) braceCount--;
            if (braceCount > 0) catchEnd++;
        }
        
        current = tryStart;
        inTryCatch = true;
        currentException.clear();
        
        while (current < tryEnd && !hasReturned && currentException.empty()) {
            statement();
        }
        
        inTryCatch = false;
        
        if (!currentException.empty()) {
            scopes.push_back(std::unordered_map<std::string, Value>());
            setVariable(errorVar.value, Value(currentException));
            current = catchStart;
            
            while (current < catchEnd && !hasReturned) {
                statement();
            }
            
            scopes.pop_back();
            currentException.clear();
        }
        
        current = catchEnd + 1;
    }

    void throwStatement() {
        Value msg = expression();
        expect(TOKEN_SEMICOLON, "Expected ';' after throw statement");
        
        if (inTryCatch) {
            currentException = msg.toString();
        } else {
            throw RuntimeError("Uncaught exception: " + msg.toString(), tokens[current - 2].line);
        }
    }

    void matchStatement() {
        Value matchValue = expression();
        expect(TOKEN_LBRACE, "Expected '{' after match value");
        
        bool matched = false;
        std::vector<std::pair<Value, std::pair<size_t, size_t>>> cases;
        cases.reserve(8);
        size_t defaultStart = 0, defaultEnd = 0;
        bool hasDefault = false;
        
        while (peek().type != TOKEN_RBRACE && !isAtEnd()) {
            if (match(TOKEN_CASE)) {
                Value caseValue = expression();
                expect(TOKEN_ARROW_FAT, "Expected '=>' after case value");
                expect(TOKEN_LBRACE, "Expected '{' after '=>'");
                size_t caseBodyStart = current;
                
                int braceCount = 1;
                size_t caseBodyEnd = current;
                while (braceCount > 0 && caseBodyEnd < tokens.size()) {
                    if (tokens[caseBodyEnd].type == TOKEN_LBRACE) braceCount++;
                    if (tokens[caseBodyEnd].type == TOKEN_RBRACE) braceCount--;
                    if (braceCount > 0) caseBodyEnd++;
                }
                
                cases.push_back({std::move(caseValue), {caseBodyStart, caseBodyEnd}});
                current = caseBodyEnd + 1;
                
            } else if (match(TOKEN_DEFAULT)) {
                if (hasDefault) {
                    throw ParseError("Match statement can only have one 'default' case", tokens[current - 1].line);
                }
                expect(TOKEN_ARROW_FAT, "Expected '=>' after 'default'");
                expect(TOKEN_LBRACE, "Expected '{' after '=>'");
                hasDefault = true;
                defaultStart = current;
                
                int braceCount = 1;
                defaultEnd = current;
                while (braceCount > 0 && defaultEnd < tokens.size()) {
                    if (tokens[defaultEnd].type == TOKEN_LBRACE) braceCount++;
                    if (tokens[defaultEnd].type == TOKEN_RBRACE) braceCount--;
                    if (braceCount > 0) defaultEnd++;
                }
                
                current = defaultEnd + 1;
            } else {
                advance();
            }
        }
        
        expect(TOKEN_RBRACE, "Expected '}' at end of match statement");
        size_t afterMatch = current;
        
        for (const auto& caseItem : cases) {
            const Value& caseValue = caseItem.first;
            size_t caseBodyStart = caseItem.second.first;
            size_t caseBodyEnd = caseItem.second.second;
            
            bool isMatch = false;
            if (matchValue.type == caseValue.type) {
                if (matchValue.type == Value::NUMBER) isMatch = matchValue.num == caseValue.num;
                else if (matchValue.type == Value::STRING) isMatch = matchValue.str == caseValue.str;
                else if (matchValue.type == Value::BOOL) isMatch = matchValue.boolean == caseValue.boolean;
            }
            
            if (isMatch) {
                matched = true;
                size_t savedCurrent = current;
                current = caseBodyStart;
                while (current < caseBodyEnd && !isAtEnd() && !hasReturned) {
                    statement();
                }
                current = savedCurrent;
                break;
            }
        }
        
        if (!matched && hasDefault) {
            size_t savedCurrent = current;
            current = defaultStart;
            while (current < defaultEnd && !isAtEnd() && !hasReturned) {
                statement();
            }
            current = savedCurrent;
        }
    }

    void returnStatement() {
        returnValue = expression();
        hasReturned = true;
        expect(TOKEN_SEMICOLON, "Expected ';' after return statement");
    }

    void putsStatement() {
        Value val = expression();
        std::cout << val.toString() << std::endl;
        expect(TOKEN_SEMICOLON, "Expected ';' after puts statement");
    }

    void ifStatement() {
        Value condition = expression();
        expect(TOKEN_LBRACE, "Expected '{' after if condition");
        
        size_t thenStart = current;
        int braceCount = 1;
        size_t thenEnd = current;
        
        while (braceCount > 0 && thenEnd < tokens.size()) {
            if (tokens[thenEnd].type == TOKEN_LBRACE) braceCount++;
            else if (tokens[thenEnd].type == TOKEN_RBRACE) braceCount--;
            if (braceCount > 0) thenEnd++;
        }

        size_t elseStart = 0, elseEnd = 0;
        bool hasElse = false;
        
        size_t afterThen = thenEnd + 1;
        if (afterThen < tokens.size() && tokens[afterThen].type == TOKEN_ELSE) {
            hasElse = true;
            current = afterThen;
            match(TOKEN_ELSE);
            expect(TOKEN_LBRACE, "Expected '{' after 'else'");
            elseStart = current;
            
            braceCount = 1;
            elseEnd = current;
            while (braceCount > 0 && elseEnd < tokens.size()) {
                if (tokens[elseEnd].type == TOKEN_LBRACE) braceCount++;
                else if (tokens[elseEnd].type == TOKEN_RBRACE) braceCount--;
                if (braceCount > 0) elseEnd++;
            }
        }

        bool shouldExecute = false;
        if (condition.type == Value::BOOL) {
            shouldExecute = condition.boolean;
        } else if (condition.type == Value::NUMBER) {
            shouldExecute = condition.num != 0;
        } else if (condition.type == Value::STRING) {
            shouldExecute = !condition.str.empty();
        }

        if (shouldExecute) {
            current = thenStart;
            while (current < thenEnd && !isAtEnd() && !hasReturned && !shouldBreak && !shouldContinue) {
                statement();
            }
        } else if (hasElse) {
            current = elseStart;
            while (current < elseEnd && !isAtEnd() && !hasReturned && !shouldBreak && !shouldContinue) {
                statement();
            }
        }
        
        current = hasElse ? (elseEnd + 1) : (thenEnd + 1);
    }

    void whileStatement() {
        size_t conditionStart = current;
        
        Value condition = expression();
        expect(TOKEN_LBRACE, "Expected '{' after while condition");
        size_t bodyStart = current;
        
        int braceCount = 1;
        size_t bodyEnd = bodyStart;
        while (braceCount > 0 && bodyEnd < tokens.size()) {
            if (tokens[bodyEnd].type == TOKEN_LBRACE) braceCount++;
            if (tokens[bodyEnd].type == TOKEN_RBRACE) braceCount--;
            if (braceCount > 0) bodyEnd++;
        }
        
        bool wasInLoop = inLoop;
        inLoop = true;
        
        while (condition.type == Value::BOOL && condition.boolean && !hasReturned) {
            current = bodyStart;
            shouldBreak = false;
            shouldContinue = false;
            
            while (current < bodyEnd && !isAtEnd() && !hasReturned && !shouldBreak) {
                statement();
                if (shouldContinue) {
                    shouldContinue = false;
                    break;
                }
            }
            
            if (shouldBreak) {
                shouldBreak = false;
                break;
            }
            
            current = conditionStart;
            condition = expression();
            expect(TOKEN_LBRACE, "Expected '{' after while condition");
        }
        
        inLoop = wasInLoop;
        current = bodyEnd + 1;
    }

    void forStatement() {
        if (peek().type != TOKEN_IDENTIFIER) {
            throw ParseError("Expected iterator variable name after 'for'", peek().line);
        }
        Token iterVar = advance();
        expect(TOKEN_IN, "Expected 'in' after iterator variable");
        
        Value start = expression();
        expect(TOKEN_DOTDOT, "Expected '..' in for loop range");
        Value end = expression();
        
        if (start.type != Value::NUMBER || end.type != Value::NUMBER) {
            throw RuntimeError("For loop range must be numbers", iterVar.line);
        }
        
        expect(TOKEN_LBRACE, "Expected '{' after for range");
        size_t loopBodyStart = current;
        
        int depth = 1;
        size_t loopBodyEnd = current;
        
        while (depth > 0 && loopBodyEnd < tokens.size()) {
            if (tokens[loopBodyEnd].type == TOKEN_LBRACE) depth++;
            else if (tokens[loopBodyEnd].type == TOKEN_RBRACE) depth--;
            if (depth > 0) loopBodyEnd++;
        }
        
        int iStart = static_cast<int>(start.num);
        int iEnd = static_cast<int>(end.num);
        
        bool wasInLoop = inLoop;
        inLoop = true;
        
        for (int i = iStart; i < iEnd; i++) {
            if (hasReturned || shouldBreak) break;
            
            setVariable(iterVar.value, Value(static_cast<double>(i)));
            
            size_t savedCurrent = current;
            current = loopBodyStart;
            shouldContinue = false;
            
            while (current < loopBodyEnd) {
                if (hasReturned || isAtEnd() || shouldBreak) break;
                statement();
                if (shouldContinue) {
                    shouldContinue = false;
                    break;
                }
            }
            
            if (shouldBreak) {
                shouldBreak = false;
                break;
            }
            
            current = savedCurrent;
        }
        
        inLoop = wasInLoop;
        current = loopBodyEnd + 1;
    }

    Value expression() {
        return logicalOr();
    }

    Value logicalOr() {
        Value left = logicalAnd();
        
        while (match(TOKEN_OR)) {
            Value right = logicalAnd();
            
            bool leftBool = false;
            if (left.type == Value::BOOL) leftBool = left.boolean;
            else if (left.type == Value::NUMBER) leftBool = left.num != 0;
            
            bool rightBool = false;
            if (right.type == Value::BOOL) rightBool = right.boolean;
            else if (right.type == Value::NUMBER) rightBool = right.num != 0;
            
            left = Value(leftBool || rightBool);
        }
        return left;
    }

    Value logicalAnd() {
        Value left = comparison();
        
        while (match(TOKEN_AND)) {
            Value right = comparison();
            
            bool leftBool = false;
            if (left.type == Value::BOOL) leftBool = left.boolean;
            else if (left.type == Value::NUMBER) leftBool = left.num != 0;
            
            bool rightBool = false;
            if (right.type == Value::BOOL) rightBool = right.boolean;
            else if (right.type == Value::NUMBER) rightBool = right.num != 0;
            
            left = Value(leftBool && rightBool);
        }
        return left;
    }

    Value comparison() {
        Value left = term();
        
        while (peek().type == TOKEN_EQUAL_EQUAL || peek().type == TOKEN_BANG_EQUAL ||
               peek().type == TOKEN_LESS || peek().type == TOKEN_GREATER ||
               peek().type == TOKEN_LESS_EQUAL || peek().type == TOKEN_GREATER_EQUAL) {
            TokenType op = advance().type;
            Value right = term();
            
            bool result = false;
            if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                if (op == TOKEN_EQUAL_EQUAL) result = left.num == right.num;
                else if (op == TOKEN_BANG_EQUAL) result = left.num != right.num;
                else if (op == TOKEN_LESS) result = left.num < right.num;
                else if (op == TOKEN_GREATER) result = left.num > right.num;
                else if (op == TOKEN_LESS_EQUAL) result = left.num <= right.num;
                else if (op == TOKEN_GREATER_EQUAL) result = left.num >= right.num;
            } else if (left.type == Value::BOOL && right.type == Value::BOOL) {
                if (op == TOKEN_EQUAL_EQUAL) result = left.boolean == right.boolean;
                else if (op == TOKEN_BANG_EQUAL) result = left.boolean != right.boolean;
            } else if (left.type == Value::STRING && right.type == Value::STRING) {
                if (op == TOKEN_EQUAL_EQUAL) result = left.str == right.str;
                else if (op == TOKEN_BANG_EQUAL) result = left.str != right.str;
            }
            left = Value(result);
        }
        return left;
    }

    Value term() {
        Value left = factor();
        
        while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) {
            TokenType op = tokens[current - 1].type;
            Value right = factor();
            
            if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                if (op == TOKEN_PLUS) left.num += right.num;
                else left.num -= right.num;
            } else if (left.type == Value::STRING && right.type == Value::STRING && op == TOKEN_PLUS) {
                left.str += right.str;
            } else if (op == TOKEN_PLUS) {
                throw RuntimeError("Cannot add " + left.getType() + " and " + right.getType(), tokens[current - 1].line);
            } else {
                throw RuntimeError("Cannot subtract " + right.getType() + " from " + left.getType(), tokens[current - 1].line);
            }
        }
        return left;
    }

    Value factor() {
        Value left = unary();
        
        while (match(TOKEN_STAR) || match(TOKEN_SLASH) || match(TOKEN_PERCENT)) {
            TokenType op = tokens[current - 1].type;
            int opLine = tokens[current - 1].line;
            Value right = unary();
            
            if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                if (op == TOKEN_STAR) {
                    left.num *= right.num;
                } else if (op == TOKEN_SLASH) {
                    if (right.num == 0) {
                        throw RuntimeError("Division by zero", opLine);
                    }
                    left.num /= right.num;
                } else if (op == TOKEN_PERCENT) {
                    if (right.num == 0) {
                        throw RuntimeError("Modulo by zero", opLine);
                    }
                    left.num = fmod(left.num, right.num);
                }
            } else {
                std::string opStr = (op == TOKEN_STAR) ? "multiply" : (op == TOKEN_SLASH) ? "divide" : "modulo";
                throw RuntimeError("Cannot " + opStr + " " + left.getType() + " and " + right.getType(), opLine);
            }
        }
        return left;
    }

    Value unary() {
        if (match(TOKEN_BANG)) {
            Value val = unary();
            if (val.type == Value::BOOL) {
                return Value(!val.boolean);
            }
            return Value(false);
        }
        if (match(TOKEN_MINUS)) {
            int opLine = tokens[current - 1].line;
            Value val = unary();
            if (val.type == Value::NUMBER) {
                val.num = -val.num;
                return val;
            }
            throw RuntimeError("Cannot negate " + val.getType(), opLine);
        }
        return call();
    }

    Value call() {
        Value val = primary();
        
        while (true) {
            if (match(TOKEN_LPAREN)) {
                int callLine = tokens[current - 1].line;
                std::vector<Value> args;
                while (!match(TOKEN_RPAREN)) {
                    args.push_back(expression());
                    if (!match(TOKEN_COMMA)) {
                        expect(TOKEN_RPAREN, "Expected ')' or ',' in function call");
                        break;
                    }
                }
                
                if (val.type == Value::STRING) {
                    val = callFunction(val.str, args, callLine);
                } else if (val.type == Value::LAMBDA) {
                    val = callLambda(val, args);
                } else {
                    throw RuntimeError("Cannot call " + val.getType(), callLine);
                }
            } else if (match(TOKEN_LBRACKET)) {
                int bracketLine = tokens[current - 1].line;
                Value index = expression();
                expect(TOKEN_RBRACKET, "Expected ']' after array index");
                
                if (val.type == Value::ARRAY) {
                    if (index.type != Value::NUMBER) {
                        throw RuntimeError("Array index must be a number, got " + index.getType(), bracketLine);
                    }
                    int idx = static_cast<int>(index.num);
                    if (idx < 0 || idx >= static_cast<int>(val.array.size())) {
                        throw RuntimeError("Array index " + std::to_string(idx) + " out of bounds (size: " + std::to_string(val.array.size()) + ")", bracketLine);
                    }
                    val = val.array[idx];
                } else if (val.type == Value::STRING) {
                    if (index.type != Value::NUMBER) {
                        throw RuntimeError("String index must be a number, got " + index.getType(), bracketLine);
                    }
                    int idx = static_cast<int>(index.num);
                    if (idx < 0 || idx >= static_cast<int>(val.str.length())) {
                        throw RuntimeError("String index " + std::to_string(idx) + " out of bounds (length: " + std::to_string(val.str.length()) + ")", bracketLine);
                    }
                    val = Value(std::string(1, val.str[idx]));
                } else {
                    throw RuntimeError("Cannot index " + val.getType(), bracketLine);
                }
            } else if (match(TOKEN_DOT)) {
                int dotLine = tokens[current - 1].line;
                if (peek().type != TOKEN_IDENTIFIER) {
                    throw ParseError("Expected field name after '.'", dotLine);
                }
                Token field = advance();
                if (val.type == Value::STRUCT) {
                    auto it = val.structFields.find(field.value);
                    if (it != val.structFields.end()) {
                        val = it->second;
                    } else {
                        throw RuntimeError("Struct '" + val.structType + "' has no field '" + field.value + "'", dotLine);
                    }
                } else {
                    throw RuntimeError("Cannot access field on " + val.getType(), dotLine);
                }
            } else {
                break;
            }
        }
        
        return val;
    }

    Value callLambda(const Value& lambda, const std::vector<Value>& args) {
        if (args.size() < lambda.lambdaParams.size()) {
            throw RuntimeError("Lambda expects " + std::to_string(lambda.lambdaParams.size()) + 
                             " arguments, got " + std::to_string(args.size()), peek().line);
        }
        
        scopes.push_back(lambda.closureCaptures);
        
        for (size_t i = 0; i < lambda.lambdaParams.size() && i < args.size(); i++) {
            scopes.back()[lambda.lambdaParams[i]] = args[i];
        }

        size_t savedCurrent = current;
        current = lambda.lambdaBodyStart;
        bool wasInFunction = inFunction;
        inFunction = true;
        hasReturned = false;
        returnValue = Value();

        while (current < lambda.lambdaBodyEnd && !isAtEnd() && !hasReturned) {
            statement();
        }

        Value result = returnValue;
        hasReturned = false;
        inFunction = wasInFunction;
        
        scopes.pop_back();
        
        current = savedCurrent;
        return result;
    }

    Value primary() {
        if (match(TOKEN_NUMBER)) {
            return Value(std::stod(tokens[current - 1].value));
        }
        if (match(TOKEN_STRING)) {
            std::string str = tokens[current - 1].value;
            
            size_t pos = 0;
            while ((pos = str.find("#{", pos)) != std::string::npos) {
                size_t end = str.find("}", pos);
                if (end != std::string::npos) {
                    std::string varName = str.substr(pos + 2, end - pos - 2);
                    Value val = getVariable(varName);
                    str.replace(pos, end - pos + 1, val.toString());
                }
                pos++;
            }
            
            return Value(str);
        }
        if (match(TOKEN_TRUE)) return Value(true);
        if (match(TOKEN_FALSE)) return Value(false);
        
        // Lambda expression
        if (match(TOKEN_PIPE)) {
            Value lambda;
            lambda.type = Value::LAMBDA;
            
            if (peek().type == TOKEN_PIPE) {
                advance();
            } else {
                while (peek().type != TOKEN_PIPE && !isAtEnd()) {
                    if (peek().type != TOKEN_IDENTIFIER) {
                        throw ParseError("Expected parameter name in lambda", peek().line);
                    }
                    Token param = advance();
                    lambda.lambdaParams.push_back(std::move(param.value));
                    if (match(TOKEN_COMMA)) {
                        continue;
                    } else {
                        break;
                    }
                }
                expect(TOKEN_PIPE, "Expected '|' after lambda parameters");
            }
            
            expect(TOKEN_ARROW_FAT, "Expected '=>' after lambda parameters");
            expect(TOKEN_LBRACE, "Expected '{' after '=>'");
            lambda.lambdaBodyStart = current;
            
            int braceCount = 1;
            size_t bodyEnd = current;
            while (braceCount > 0 && bodyEnd < tokens.size()) {
                if (tokens[bodyEnd].type == TOKEN_LBRACE) braceCount++;
                if (tokens[bodyEnd].type == TOKEN_RBRACE) braceCount--;
                if (braceCount > 0) bodyEnd++;
            }
            lambda.lambdaBodyEnd = bodyEnd;
            
            for (int i = scopes.size() - 1; i >= 0; i--) {
                for (const auto& var : scopes[i]) {
                    if (lambda.closureCaptures.find(var.first) == lambda.closureCaptures.end()) {
                        lambda.closureCaptures[var.first] = var.second;
                    }
                }
            }
            
            current = bodyEnd + 1;
            return lambda;
        }
        
        if (match(TOKEN_LBRACKET)) {
            std::vector<Value> arr;
            while (!match(TOKEN_RBRACKET)) {
                arr.push_back(expression());
                if (!match(TOKEN_COMMA)) {
                    expect(TOKEN_RBRACKET, "Expected ']' or ',' in array literal");
                    break;
                }
            }
            return Value(arr);
        }
        
        if (match(TOKEN_IDENTIFIER)) {
            std::string name = tokens[current - 1].value;
            
            auto structIt = structDefs.find(name);
            if (structIt != structDefs.end() && peek().type == TOKEN_LBRACE) {
                match(TOKEN_LBRACE);
                Value structVal;
                structVal.type = Value::STRUCT;
                structVal.structType = name;
                
                while (!match(TOKEN_RBRACE)) {
                    if (peek().type != TOKEN_IDENTIFIER) {
                        throw ParseError("Expected field name in struct literal", peek().line);
                    }
                    Token fieldName = advance();
                    expect(TOKEN_COLON, "Expected ':' after field name");
                    Value fieldValue = expression();
                    structVal.structFields[fieldName.value] = std::move(fieldValue);
                    
                    if (!match(TOKEN_COMMA)) {
                        expect(TOKEN_RBRACE, "Expected '}' or ',' in struct literal");
                        break;
                    }
                }
                
                return structVal;
            }
            
            if (functions.find(name) != functions.end() || isBuiltinFunction(name)) {
                return Value(name);
            }
            
            return getVariable(name);
        }
        
        if (match(TOKEN_LPAREN)) {
            Value val = expression();
            expect(TOKEN_RPAREN, "Expected ')' after expression");
            return val;
        }
        
        throw ParseError("Unexpected token: '" + peek().value + "'", peek().line);
    }
};

const std::unordered_map<std::string, bool> Interpreter::builtinFunctions = {
    {"len", true}, {"push", true}, {"pop", true},
    {"sqrt", true}, {"pow", true}, {"abs", true},
    {"floor", true}, {"ceil", true}, {"round", true},
    {"min", true}, {"max", true}, {"random", true}, {"random_int", true},
    {"str", true}, {"int", true}, {"float", true},
    {"uppercase", true}, {"lowercase", true}, {"substr", true},
    {"split", true}, {"join", true},
    {"read_file", true}, {"write_file", true}, {"append_file", true}, {"file_exists", true},
    {"map", true}, {"filter", true}, {"reduce", true}, {"typeof", true},
    {"input", true}, {"gui_init", true}, {"gui_window", true}, {"gui_button", true},
    {"gui_label", true}, {"gui_entry", true}, {"gui_box", true},
    {"gui_add", true}, {"gui_set_text", true}, {"gui_get_text", true},
    {"gui_on", true}, {"gui_show", true}, {"gui_run", true},
    {"gui_quit", true}, {"gui_checkbox", true}, {"gui_textview", true},
    {"gui_frame", true}, {"gui_separator", true}, {"gui_set_sensitive", true},
    {"gui_get_checked", true}, {"gui_set_checked", true}
};

static Value interpreterCallbackWrapper(Interpreter* interp, const std::string& funcName, 
                                       const std::vector<Value>& args, int line) {
    return interp->callFunction(funcName, args, line);
}

int main(int argc, char* argv[]) {
    ChocoGUI* gui = ChocoGUI::getInstance(argc, argv);
    gui->setCallbackFunction(interpreterCallbackWrapper);
    if (argc == 1) {
        std::cout << "======================================" << std::endl;
        std::cout << "  ChocoLang 0.5.0 - Nutty Nougat" << std::endl;
        std::cout << "  REPL (CocoaInterpreter v0.1.1)" << std::endl;
        std::cout << "  Type 'exit' or 'quit' to leave" << std::endl;
        std::cout << "======================================" << std::endl;
        std::cout << std::endl;
        
        std::vector<Token> emptyTokens;
        Interpreter repl(emptyTokens);
        std::string line;
        int lineNumber = 1;
        
        while (true) {
            std::cout << "choco:" << lineNumber << "> ";
            
            if (!std::getline(std::cin, line)) {
                std::cout << std::endl;
                break;
            }
            
            line.erase(0, line.find_first_not_of(" \t\n\r"));
            line.erase(line.find_last_not_of(" \t\n\r") + 1);
            
            if (line == "exit" || line == "quit") {
                std::cout << "Goodbye!" << std::endl;
                break;
            }
            
            if (line.empty()) {
                continue;
            }
            
            if (line == "help") {
                std::cout << "ChocoLang REPL Commands:" << std::endl;
                std::cout << "  exit, quit     - Exit the REPL" << std::endl;
                std::cout << "  help           - Show this help message" << std::endl;
                std::cout << "  clear          - Clear all variables and functions" << std::endl;
                std::cout << "  vars           - Show all defined variables" << std::endl;
                std::cout << "  funcs          - Show all defined functions" << std::endl;
                std::cout << std::endl;
                std::cout << "Examples:" << std::endl;
                std::cout << "  let x = 10;" << std::endl;
                std::cout << "  puts x + 5;" << std::endl;
                std::cout << "  fn greet(name) { return \"Hello, \" + name; }" << std::endl;
                std::cout << "  puts greet(\"World\");" << std::endl;
                lineNumber++;
                continue;
            }
            
            if (line == "clear") {
                std::vector<Token> emptyTokens2;
                repl = Interpreter(emptyTokens2);
                std::cout << "Environment cleared." << std::endl;
                lineNumber = 1;
                continue;
            }
            
            if (line == "vars") {
                std::cout << "Defined variables:" << std::endl;
                if (repl.scopes.empty() || repl.scopes[0].empty()) {
                    std::cout << "  (none)" << std::endl;
                } else {
                    for (const auto& var : repl.scopes[0]) {
                        std::cout << "  " << var.first << " = " << var.second.toString() << std::endl;
                    }
                }
                lineNumber++;
                continue;
            }
            
            if (line == "funcs") {
                std::cout << "Defined functions:" << std::endl;
                if (repl.functions.empty()) {
                    std::cout << "  (none)" << std::endl;
                } else {
                    for (const auto& func : repl.functions) {
                        std::cout << "  " << func.first << "(";
                        for (size_t i = 0; i < func.second.params.size(); i++) {
                            std::cout << func.second.params[i];
                            if (i < func.second.params.size() - 1) std::cout << ", ";
                        }
                        std::cout << ")" << std::endl;
                    }
                }
                lineNumber++;
                continue;
            }
            
            if (line.back() != ';' && line.back() != '}') {
                line += ";";
            }
            
            try {
                Lexer lexer(line);
                std::vector<Token> tokens = lexer.tokenize();
                
                size_t savedCurrent = repl.current;
                std::vector<Token> savedTokens = std::move(repl.tokens);
                
                repl.tokens = std::move(tokens);
                repl.current = 0;
                
                while (!repl.isAtEnd()) {
                    repl.statement();
                }
                
                repl.tokens = std::move(savedTokens);
                repl.current = savedCurrent;
                
            } catch (const LexerError& e) {
                std::cerr << "Lexer Error: " << e.what() << std::endl;
            } catch (const ParseError& e) {
                std::cerr << "Parse Error: " << e.what() << std::endl;
            } catch (const RuntimeError& e) {
                std::cerr << "Runtime Error: " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown error occurred" << std::endl;
            }
            
            lineNumber++;
        }
        
        return 0;
    }
    
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [file.choco]" << std::endl;
        std::cerr << "       " << argv[0] << "              (for REPL mode)" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file '" << argv[1] << "'" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    try {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        Interpreter interpreter(tokens);

        ChocoGUI* gui = ChocoGUI::getInstance(argc, argv);
        gui->setCallbackFunction(interpreterCallbackWrapper);
        gui->setInterpreter(&interpreter);

        interpreter.execute();
        
        return 0;
    } catch (const LexerError& e) {
        return 1;
    } catch (const ParseError& e) {
        return 1;
    } catch (const RuntimeError& e) {
        return 1;
    } catch (...) {
        std::cerr << "Fatal error occurred" << std::endl;
        return 1;
    }
}