//////////////////////////////////////
// ChocoLang 3.0.0 - Milky Way
// CoffeeShop Development
// Made by Camila "Mocha" Rose
//////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <map>
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

// Lexer
class Lexer {
    std::string source;
    size_t pos = 0;
    int line = 1;

public:
    Lexer(const std::string& src) : source(src) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < source.length()) {
            skipWhitespace();
            if (pos >= source.length()) break;

            if (source[pos] == '/' && pos + 1 < source.length() && source[pos + 1] == '/') {
                skipComment();
                continue;
            }

            Token tok = nextToken();
            tokens.push_back(tok);
        }
        tokens.push_back({TOKEN_EOF, "", line});
        return tokens;
    }

private:
    void skipWhitespace() {
        while (pos < source.length() && std::isspace(source[pos])) {
            if (source[pos] == '\n') line++;
            pos++;
        }
    }

    void skipComment() {
        while (pos < source.length() && source[pos] != '\n') pos++;
    }

    Token nextToken() {
        char c = source[pos];

        if (std::isdigit(c)) return number();
        if (std::isalpha(c) || c == '_') return identifier();
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
                break;
            case '|':
                if (pos < source.length() && source[pos] == '|') {
                    pos++;
                    return {TOKEN_OR, "||", line};
                }
                return {TOKEN_PIPE, "|", line};
        }
        return {TOKEN_EOF, "", line};
    }

    Token number() {
        std::string num;
        bool hasDot = false;
        
        while (pos < source.length()) {
            if (std::isdigit(source[pos])) {
                num += source[pos++];
            } else if (source[pos] == '.' && !hasDot) {
                if (pos + 1 < source.length() && source[pos + 1] == '.') {
                    break;
                }
                if (pos + 1 < source.length() && std::isdigit(source[pos + 1])) {
                    hasDot = true;
                    num += source[pos++];
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        
        return {TOKEN_NUMBER, num, line};
    }

    Token identifier() {
        std::string id;
        while (pos < source.length() && (std::isalnum(source[pos]) || source[pos] == '_')) {
            id += source[pos++];
        }

        if (id == "let") return {TOKEN_LET, id, line};
        if (id == "fn") return {TOKEN_FN, id, line};
        if (id == "if") return {TOKEN_IF, id, line};
        if (id == "else") return {TOKEN_ELSE, id, line};
        if (id == "while") return {TOKEN_WHILE, id, line};
        if (id == "for") return {TOKEN_FOR, id, line};
        if (id == "in") return {TOKEN_IN, id, line};
        if (id == "return") return {TOKEN_RETURN, id, line};
        if (id == "puts") return {TOKEN_PUTS, id, line};
        if (id == "true") return {TOKEN_TRUE, id, line};
        if (id == "false") return {TOKEN_FALSE, id, line};
        if (id == "struct") return {TOKEN_STRUCT, id, line};
        if (id == "impl") return {TOKEN_IMPL, id, line};
        if (id == "import") return {TOKEN_IMPORT, id, line};
        if (id == "from") return {TOKEN_FROM, id, line};
        if (id == "try") return {TOKEN_TRY, id, line};
        if (id == "catch") return {TOKEN_CATCH, id, line};
        if (id == "throw") return {TOKEN_THROW, id, line};
        if (id == "break") return {TOKEN_BREAK, id, line};
        if (id == "continue") return {TOKEN_CONTINUE, id, line};
        if (id == "match") return {TOKEN_MATCH, id, line};
        if (id == "case") return {TOKEN_CASE, id, line};
        if (id == "default") return {TOKEN_DEFAULT, id, line};
        if (id == "async") return {TOKEN_ASYNC, id, line};
        if (id == "await") return {TOKEN_AWAIT, id, line};

        return {TOKEN_IDENTIFIER, id, line};
    }

    Token string() {
        pos++; // skip opening "
        std::string str;
        while (pos < source.length() && source[pos] != '"') {
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
        pos++; // skip closing "
        return {TOKEN_STRING, str, line};
    }
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
    std::map<std::string, Value> structFields;
    std::string structType;
    
    // Lambda/closure support
    std::vector<std::string> lambdaParams;
    size_t lambdaBodyStart;
    size_t lambdaBodyEnd;
    std::map<std::string, Value> closureCaptures;

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

// Function definition
struct Function {
    std::vector<std::string> params;
    size_t bodyStart;
    size_t bodyEnd;
};

// Struct definition
struct StructDef {
    std::vector<std::string> fields;
};

// Exception for error handling
struct ChocoException {
    std::string message;
    ChocoException(const std::string& msg) : message(msg) {}
};

// Interpreter
class Interpreter {
public:
    std::map<std::string, Value> globalVars;
    std::vector<std::map<std::string, Value>> scopes;
    std::map<std::string, Function> functions;
    std::map<std::string, StructDef> structDefs;
    std::vector<Token> tokens;
    size_t current;
    
    bool inFunction;
    bool hasReturned;
    Value returnValue;
    bool shouldBreak;
    bool shouldContinue;
    bool inTryCatch;
    std::string currentException;

    Interpreter(const std::vector<Token>& toks) : tokens(toks), current(0), 
        inFunction(false), hasReturned(false), shouldBreak(false), 
        shouldContinue(false), inTryCatch(false) {
        scopes.push_back(std::map<std::string, Value>());
        srand(time(nullptr));
    }

    void execute() {
        while (!isAtEnd()) {
            statement();
        }
    }
    
    bool isAtEnd() { 
        return tokens.empty() || current >= tokens.size() || tokens[current].type == TOKEN_EOF; 
    }

    Token peek() { return tokens[current]; }
    Token advance() { return tokens[current++]; }
    bool match(TokenType type) {
        if (peek().type == type) {
            advance();
            return true;
        }
        return false;
    }

    bool isBuiltinFunction(const std::string& name) {
        return name == "len" || name == "push" || name == "pop" ||
               name == "sqrt" || name == "pow" || name == "abs" ||
               name == "floor" || name == "ceil" || name == "round" ||
               name == "min" || name == "max" || name == "random" || name == "random_int" ||
               name == "str" || name == "int" || name == "float" ||
               name == "uppercase" || name == "lowercase" || name == "substr" ||
               name == "split" || name == "join" ||
               name == "read_file" || name == "write_file" || name == "append_file" || name == "file_exists" ||
               name == "map" || name == "filter" || name == "reduce" || name == "typeof";
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
            if (scopes[i].find(name) != scopes[i].end()) {
                return scopes[i][name];
            }
        }
        if (globalVars.find(name) != globalVars.end()) {
            return globalVars[name];
        }
        return Value();
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
            shouldBreak = true;
            match(TOKEN_SEMICOLON);
        } else if (match(TOKEN_CONTINUE)) {
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
            returnStatement();
        } else if (peek().type == TOKEN_IDENTIFIER && current + 1 < tokens.size() && tokens[current + 1].type == TOKEN_EQUAL) {
            Token name = advance();
            match(TOKEN_EQUAL);
            Value val = expression();
            setVariable(name.value, val);
            match(TOKEN_SEMICOLON);
        } else {
            expression();
            match(TOKEN_SEMICOLON);
        }
    }

    void letStatement() {
        Token name = advance();
        match(TOKEN_EQUAL);
        Value val = expression();
        setVariable(name.value, val);
        match(TOKEN_SEMICOLON);
    }

    void functionDeclaration() {
        Token name = advance();
        match(TOKEN_LPAREN);
        
        std::vector<std::string> params;
        while (!match(TOKEN_RPAREN)) {
            Token param = advance();
            params.push_back(param.value);
            if (!match(TOKEN_COMMA)) {
                match(TOKEN_RPAREN);
                break;
            }
        }

        match(TOKEN_LBRACE);
        size_t bodyStart = current;
        
        int braceCount = 1;
        while (braceCount > 0 && !isAtEnd()) {
            if (tokens[current].type == TOKEN_LBRACE) braceCount++;
            if (tokens[current].type == TOKEN_RBRACE) braceCount--;
            current++;
        }
        size_t bodyEnd = current - 1;

        functions[name.value] = {params, bodyStart, bodyEnd};
    }

    void structDeclaration() {
        Token name = advance();
        match(TOKEN_LBRACE);
        
        std::vector<std::string> fields;
        while (!match(TOKEN_RBRACE)) {
            Token field = advance();
            fields.push_back(field.value);
            if (!match(TOKEN_COMMA)) {
                match(TOKEN_RBRACE);
                break;
            }
        }
        
        structDefs[name.value] = {fields};
    }

    void importStatement() {
        Token module = advance();
        match(TOKEN_SEMICOLON);
        
        std::string filename = module.value + ".choco";
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Error: Could not import module '" << module.value << "'" << std::endl;
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        Lexer lexer(source);
        std::vector<Token> moduleTokens = lexer.tokenize();
        
        size_t savedCurrent = current;
        std::vector<Token> savedTokens = tokens;
        
        tokens = moduleTokens;
        current = 0;
        
        while (!isAtEnd()) {
            statement();
        }
        
        tokens = savedTokens;
        current = savedCurrent;
    }

    void tryStatement() {
        match(TOKEN_LBRACE);
        size_t tryStart = current;
        
        int braceCount = 1;
        size_t tryEnd = current;
        while (braceCount > 0 && tryEnd < tokens.size()) {
            if (tokens[tryEnd].type == TOKEN_LBRACE) braceCount++;
            if (tokens[tryEnd].type == TOKEN_RBRACE) braceCount--;
            if (braceCount > 0) tryEnd++;
        }
        
        current = tryEnd + 1;
        
        if (!match(TOKEN_CATCH)) {
            std::cerr << "Error: Expected 'catch' after try block" << std::endl;
            return;
        }
        
        Token errorVar = advance();
        match(TOKEN_LBRACE);
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
        currentException = "";
        
        while (current < tryEnd && !hasReturned && currentException.empty()) {
            statement();
        }
        
        inTryCatch = false;
        
        if (!currentException.empty()) {
            scopes.push_back(std::map<std::string, Value>());
            setVariable(errorVar.value, Value(currentException));
            current = catchStart;
            
            while (current < catchEnd && !hasReturned) {
                statement();
            }
            
            scopes.pop_back();
            currentException = "";
        }
        
        current = catchEnd + 1;
    }

    void throwStatement() {
        Value msg = expression();
        match(TOKEN_SEMICOLON);
        
        if (inTryCatch) {
            currentException = msg.toString();
        } else {
            std::cerr << "Uncaught exception: " << msg.toString() << std::endl;
            exit(1);
        }
    }

    void matchStatement() {
        Value matchValue = expression();
        match(TOKEN_LBRACE);
        
        bool matched = false;
        
        // Parse and store all cases first
        std::vector<std::pair<Value, std::pair<size_t, size_t>>> cases;
        size_t defaultStart = 0, defaultEnd = 0;
        bool hasDefault = false;
        
        while (peek().type != TOKEN_RBRACE && !isAtEnd()) {
            if (match(TOKEN_CASE)) {
                Value caseValue = expression();
                match(TOKEN_ARROW_FAT);
                match(TOKEN_LBRACE);
                size_t caseBodyStart = current;
                
                int braceCount = 1;
                size_t caseBodyEnd = current;
                while (braceCount > 0 && caseBodyEnd < tokens.size()) {
                    if (tokens[caseBodyEnd].type == TOKEN_LBRACE) braceCount++;
                    if (tokens[caseBodyEnd].type == TOKEN_RBRACE) braceCount--;
                    if (braceCount > 0) caseBodyEnd++;
                }
                
                cases.push_back({caseValue, {caseBodyStart, caseBodyEnd}});
                current = caseBodyEnd + 1;
                
            } else if (match(TOKEN_DEFAULT)) {
                match(TOKEN_ARROW_FAT);
                match(TOKEN_LBRACE);
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
        
        // Save position after match block
        size_t afterMatch = current;
        match(TOKEN_RBRACE);
        
        // Now execute the matching case
        for (const auto& caseItem : cases) {
            Value caseValue = caseItem.first;
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
        
        // If no match, execute default
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
        match(TOKEN_SEMICOLON);
    }

    void putsStatement() {
        Value val = expression();
        std::cout << val.toString() << std::endl;
        match(TOKEN_SEMICOLON);
    }

    void ifStatement() {
        Value condition = expression();
        
        if (!match(TOKEN_LBRACE)) {
            std::cerr << "Error: Expected '{' after if condition" << std::endl;
            return;
        }
        
        size_t thenStart = current;
        
        int braceCount = 1;
        size_t thenEnd = current;
        
        while (braceCount > 0 && thenEnd < tokens.size()) {
            if (tokens[thenEnd].type == TOKEN_LBRACE) {
                braceCount++;
            } else if (tokens[thenEnd].type == TOKEN_RBRACE) {
                braceCount--;
            }
            if (braceCount > 0) {
                thenEnd++;
            }
        }

        size_t elseStart = 0, elseEnd = 0;
        bool hasElse = false;
        
        size_t afterThen = thenEnd + 1;
        if (afterThen < tokens.size() && tokens[afterThen].type == TOKEN_ELSE) {
            hasElse = true;
            current = afterThen;
            match(TOKEN_ELSE);
            match(TOKEN_LBRACE);
            elseStart = current;
            
            braceCount = 1;
            elseEnd = current;
            while (braceCount > 0 && elseEnd < tokens.size()) {
                if (tokens[elseEnd].type == TOKEN_LBRACE) {
                    braceCount++;
                } else if (tokens[elseEnd].type == TOKEN_RBRACE) {
                    braceCount--;
                }
                if (braceCount > 0) {
                    elseEnd++;
                }
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
        match(TOKEN_LBRACE);
        size_t bodyStart = current;
        
        int braceCount = 1;
        size_t bodyEnd = bodyStart;
        while (braceCount > 0 && bodyEnd < tokens.size()) {
            if (tokens[bodyEnd].type == TOKEN_LBRACE) braceCount++;
            if (tokens[bodyEnd].type == TOKEN_RBRACE) braceCount--;
            if (braceCount > 0) bodyEnd++;
        }
        
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
            match(TOKEN_LBRACE);
        }
        
        current = bodyEnd + 1;
    }

    void forStatement() {
        Token iterVar = advance();
        
        if (!match(TOKEN_IN)) {
            std::cerr << "Error: Expected 'in' in for loop" << std::endl;
            return;
        }
        
        Value start = expression();
        
        if (!match(TOKEN_DOTDOT)) {
            std::cerr << "Error: Expected '..' in for loop" << std::endl;
            return;
        }
        
        Value end = expression();
        
        if (!match(TOKEN_LBRACE)) {
            std::cerr << "Error: Expected '{' after for range" << std::endl;
            return;
        }
        
        size_t loopBodyStart = current;
        
        int depth = 1;
        size_t loopBodyEnd = current;
        
        while (depth > 0 && loopBodyEnd < tokens.size()) {
            if (tokens[loopBodyEnd].type == TOKEN_LBRACE) {
                depth++;
            } else if (tokens[loopBodyEnd].type == TOKEN_RBRACE) {
                depth--;
            }
            if (depth > 0) {
                loopBodyEnd++;
            }
        }
        
        if (start.type == Value::NUMBER && end.type == Value::NUMBER) {
            int iStart = static_cast<int>(start.num);
            int iEnd = static_cast<int>(end.num);
            
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
        }
        
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
            }
        }
        return left;
    }

    Value factor() {
        Value left = unary();
        
        while (match(TOKEN_STAR) || match(TOKEN_SLASH) || match(TOKEN_PERCENT)) {
            TokenType op = tokens[current - 1].type;
            Value right = unary();
            
            if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                if (op == TOKEN_STAR) left.num *= right.num;
                else if (op == TOKEN_SLASH && right.num != 0) left.num /= right.num;
                else if (op == TOKEN_PERCENT && right.num != 0) {
                    left.num = fmod(left.num, right.num);
                }
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
            Value val = unary();
            if (val.type == Value::NUMBER) {
                val.num = -val.num;
            }
            return val;
        }
        return call();
    }

    Value call() {
        Value val = primary();
        
        while (true) {
            if (match(TOKEN_LPAREN)) {
                std::vector<Value> args;
                while (!match(TOKEN_RPAREN)) {
                    args.push_back(expression());
                    if (!match(TOKEN_COMMA)) {
                        match(TOKEN_RPAREN);
                        break;
                    }
                }
                
                if (val.type == Value::STRING) {
                    val = callFunction(val.str, args);
                } else if (val.type == Value::LAMBDA) {
                    val = callLambda(val, args);
                } else {
                    val = Value();
                }
            } else if (match(TOKEN_LBRACKET)) {
                Value index = expression();
                match(TOKEN_RBRACKET);
                if (val.type == Value::ARRAY && index.type == Value::NUMBER) {
                    int idx = static_cast<int>(index.num);
                    if (idx >= 0 && idx < static_cast<int>(val.array.size())) {
                        val = val.array[idx];
                    } else {
                        val = Value();
                    }
                } else if (val.type == Value::STRING && index.type == Value::NUMBER) {
                    int idx = static_cast<int>(index.num);
                    if (idx >= 0 && idx < static_cast<int>(val.str.length())) {
                        val = Value(std::string(1, val.str[idx]));
                    } else {
                        val = Value();
                    }
                }
            } else if (match(TOKEN_DOT)) {
                Token field = advance();
                if (val.type == Value::STRUCT) {
                    if (val.structFields.find(field.value) != val.structFields.end()) {
                        val = val.structFields[field.value];
                    } else {
                        val = Value();
                    }
                }
            } else {
                break;
            }
        }
        
        return val;
    }

    Value callLambda(const Value& lambda, const std::vector<Value>& args) {
        scopes.push_back(lambda.closureCaptures);
        
        for (size_t i = 0; i < lambda.lambdaParams.size() && i < args.size(); i++) {
            scopes.back()[lambda.lambdaParams[i]] = args[i];
        }

        size_t savedCurrent = current;
        current = lambda.lambdaBodyStart;
        hasReturned = false;
        returnValue = Value();

        while (current < lambda.lambdaBodyEnd && !isAtEnd() && !hasReturned) {
            statement();
        }

        Value result = returnValue;
        hasReturned = false;
        
        scopes.pop_back();
        
        current = savedCurrent;
        return result;
    }

    Value callFunction(const std::string& name, const std::vector<Value>& args) {
        // Higher-order functions
        if (name == "map") {
            if (args.size() >= 2 && args[0].type == Value::ARRAY && args[1].type == Value::LAMBDA) {
                std::vector<Value> result;
                for (const auto& item : args[0].array) {
                    std::vector<Value> lambdaArgs = {item};
                    result.push_back(callLambda(args[1], lambdaArgs));
                }
                return Value(result);
            }
            return Value();
        }
        
        if (name == "filter") {
            if (args.size() >= 2 && args[0].type == Value::ARRAY && args[1].type == Value::LAMBDA) {
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
            return Value();
        }
        
        if (name == "reduce") {
            if (args.size() >= 3 && args[0].type == Value::ARRAY && args[2].type == Value::LAMBDA) {
                Value accumulator = args[1];
                for (const auto& item : args[0].array) {
                    std::vector<Value> lambdaArgs = {accumulator, item};
                    accumulator = callLambda(args[2], lambdaArgs);
                }
                return accumulator;
            }
            return Value();
        }
        
        if (name == "typeof") {
            if (args.size() > 0) {
                return Value(args[0].getType());
            }
            return Value("nil");
        }
        
        // Standard library functions
        if (name == "len") {
            if (args.size() > 0) {
                if (args[0].type == Value::ARRAY) {
                    return Value(static_cast<double>(args[0].array.size()));
                } else if (args[0].type == Value::STRING) {
                    return Value(static_cast<double>(args[0].str.length()));
                }
            }
            return Value(0.0);
        }
        
        if (name == "push") {
            if (args.size() >= 2 && args[0].type == Value::ARRAY) {
                Value arr = args[0];
                arr.array.push_back(args[1]);
                return arr;
            }
            return Value();
        }
        
        if (name == "pop") {
            if (args.size() > 0 && args[0].type == Value::ARRAY) {
                Value arr = args[0];
                if (!arr.array.empty()) {
                    Value last = arr.array.back();
                    arr.array.pop_back();
                    return last;
                }
            }
            return Value();
        }
        
        if (name == "sqrt") {
            if (args.size() > 0 && args[0].type == Value::NUMBER) {
                return Value(sqrt(args[0].num));
            }
            return Value();
        }
        
        if (name == "pow") {
            if (args.size() >= 2 && args[0].type == Value::NUMBER && args[1].type == Value::NUMBER) {
                return Value(pow(args[0].num, args[1].num));
            }
            return Value();
        }
        
        if (name == "abs") {
            if (args.size() > 0 && args[0].type == Value::NUMBER) {
                return Value(fabs(args[0].num));
            }
            return Value();
        }
        
        if (name == "floor") {
            if (args.size() > 0 && args[0].type == Value::NUMBER) {
                return Value(floor(args[0].num));
            }
            return Value();
        }
        
        if (name == "ceil") {
            if (args.size() > 0 && args[0].type == Value::NUMBER) {
                return Value(ceil(args[0].num));
            }
            return Value();
        }
        
        if (name == "round") {
            if (args.size() > 0 && args[0].type == Value::NUMBER) {
                return Value(round(args[0].num));
            }
            return Value();
        }
        
        if (name == "min") {
            if (args.size() >= 2 && args[0].type == Value::NUMBER && args[1].type == Value::NUMBER) {
                return Value(std::min(args[0].num, args[1].num));
            }
            return Value();
        }
        
        if (name == "max") {
            if (args.size() >= 2 && args[0].type == Value::NUMBER && args[1].type == Value::NUMBER) {
                return Value(std::max(args[0].num, args[1].num));
            }
            return Value();
        }
        
        if (name == "random") {
            return Value(static_cast<double>(rand()) / RAND_MAX);
        }
        
        if (name == "random_int") {
            if (args.size() >= 2 && args[0].type == Value::NUMBER && args[1].type == Value::NUMBER) {
                int min = static_cast<int>(args[0].num);
                int max = static_cast<int>(args[1].num);
                return Value(static_cast<double>(min + rand() % (max - min + 1)));
            }
            return Value();
        }
        
        if (name == "str") {
            if (args.size() > 0) {
                return Value(args[0].toString());
            }
            return Value("");
        }
        
        if (name == "int") {
            if (args.size() > 0 && args[0].type == Value::NUMBER) {
                return Value(static_cast<double>(static_cast<int>(args[0].num)));
            } else if (args.size() > 0 && args[0].type == Value::STRING) {
                try {
                    return Value(static_cast<double>(std::stoi(args[0].str)));
                } catch (...) {
                    return Value(0.0);
                }
            }
            return Value();
        }
        
        if (name == "float") {
            if (args.size() > 0 && args[0].type == Value::STRING) {
                try {
                    return Value(std::stod(args[0].str));
                } catch (...) {
                    return Value(0.0);
                }
            } else if (args.size() > 0 && args[0].type == Value::NUMBER) {
                return args[0];
            }
            return Value();
        }
        
        if (name == "uppercase") {
            if (args.size() > 0 && args[0].type == Value::STRING) {
                std::string result = args[0].str;
                std::transform(result.begin(), result.end(), result.begin(), ::toupper);
                return Value(result);
            }
            return Value();
        }
        
        if (name == "lowercase") {
            if (args.size() > 0 && args[0].type == Value::STRING) {
                std::string result = args[0].str;
                std::transform(result.begin(), result.end(), result.begin(), ::tolower);
                return Value(result);
            }
            return Value();
        }
        
        if (name == "substr") {
            if (args.size() >= 3 && args[0].type == Value::STRING && 
                args[1].type == Value::NUMBER && args[2].type == Value::NUMBER) {
                int start = static_cast<int>(args[1].num);
                int length = static_cast<int>(args[2].num);
                return Value(args[0].str.substr(start, length));
            }
            return Value();
        }
        
        if (name == "split") {
            if (args.size() >= 2 && args[0].type == Value::STRING && args[1].type == Value::STRING) {
                std::vector<Value> result;
                std::string str = args[0].str;
                std::string delim = args[1].str;
                size_t pos = 0;
                while ((pos = str.find(delim)) != std::string::npos) {
                    result.push_back(Value(str.substr(0, pos)));
                    str.erase(0, pos + delim.length());
                }
                result.push_back(Value(str));
                return Value(result);
            }
            return Value();
        }
        
        if (name == "join") {
            if (args.size() >= 2 && args[0].type == Value::ARRAY && args[1].type == Value::STRING) {
                std::string result;
                for (size_t i = 0; i < args[0].array.size(); i++) {
                    result += args[0].array[i].toString();
                    if (i < args[0].array.size() - 1) {
                        result += args[1].str;
                    }
                }
                return Value(result);
            }
            return Value();
        }
        
        if (name == "read_file") {
            if (args.size() > 0 && args[0].type == Value::STRING) {
                std::ifstream file(args[0].str);
                if (file) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    return Value(buffer.str());
                }
            }
            return Value();
        }
        
        if (name == "write_file") {
            if (args.size() >= 2 && args[0].type == Value::STRING && args[1].type == Value::STRING) {
                std::ofstream file(args[0].str);
                if (file) {
                    file << args[1].str;
                    return Value(true);
                }
                return Value(false);
            }
            return Value();
        }
        
        if (name == "append_file") {
            if (args.size() >= 2 && args[0].type == Value::STRING && args[1].type == Value::STRING) {
                std::ofstream file(args[0].str, std::ios::app);
                if (file) {
                    file << args[1].str;
                    return Value(true);
                }
                return Value(false);
            }
            return Value();
        }
        
        if (name == "file_exists") {
            if (args.size() > 0 && args[0].type == Value::STRING) {
                std::ifstream file(args[0].str);
                return Value(file.good());
            }
            return Value(false);
        }

        // User-defined functions
        if (functions.find(name) == functions.end()) {
            return Value();
        }

        Function& func = functions[name];
        
        scopes.push_back(std::map<std::string, Value>());
        
        for (size_t i = 0; i < func.params.size() && i < args.size(); i++) {
            scopes.back()[func.params[i]] = args[i];
        }

        size_t savedCurrent = current;
        current = func.bodyStart;
        hasReturned = false;
        returnValue = Value();

        while (current < func.bodyEnd && !isAtEnd() && !hasReturned) {
            statement();
        }

        Value result = returnValue;
        hasReturned = false;
        
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
        
        // Lambda expression: |params| => { body } or || => { body }
        if (match(TOKEN_PIPE)) {
            Value lambda;
            lambda.type = Value::LAMBDA;
            
            // Check if it's an empty parameter list ||
            if (peek().type == TOKEN_PIPE) {
                advance(); // consume second pipe
            } else {
                // Parse parameters
                while (peek().type != TOKEN_PIPE && !isAtEnd()) {
                    Token param = advance();
                    lambda.lambdaParams.push_back(param.value);
                    if (match(TOKEN_COMMA)) {
                        continue;
                    } else {
                        break;
                    }
                }
                match(TOKEN_PIPE);
            }
            
            match(TOKEN_ARROW_FAT);
            match(TOKEN_LBRACE);
            lambda.lambdaBodyStart = current;
            
            int braceCount = 1;
            size_t bodyEnd = current;
            while (braceCount > 0 && bodyEnd < tokens.size()) {
                if (tokens[bodyEnd].type == TOKEN_LBRACE) braceCount++;
                if (tokens[bodyEnd].type == TOKEN_RBRACE) braceCount--;
                if (braceCount > 0) bodyEnd++;
            }
            lambda.lambdaBodyEnd = bodyEnd;
            
            // Capture current scope variables (closure)
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
                    match(TOKEN_RBRACKET);
                    break;
                }
            }
            return Value(arr);
        }
        if (match(TOKEN_IDENTIFIER)) {
            std::string name = tokens[current - 1].value;
            
            // Check if it's a struct constructor
            if (structDefs.find(name) != structDefs.end() && peek().type == TOKEN_LBRACE) {
                match(TOKEN_LBRACE);
                Value structVal;
                structVal.type = Value::STRUCT;
                structVal.structType = name;
                
                while (!match(TOKEN_RBRACE)) {
                    Token fieldName = advance();
                    match(TOKEN_COLON);
                    Value fieldValue = expression();
                    structVal.structFields[fieldName.value] = fieldValue;
                    
                    if (!match(TOKEN_COMMA)) {
                        match(TOKEN_RBRACE);
                        break;
                    }
                }
                
                return structVal;
            }
            
            // Check if it's a function (user-defined or built-in)
            if (functions.find(name) != functions.end() || isBuiltinFunction(name)) {
                return Value(name);
            }
            
            // Otherwise it's a variable
            return getVariable(name);
        }
        if (match(TOKEN_LPAREN)) {
            Value val = expression();
            match(TOKEN_RPAREN);
            return val;
        }
        return Value();
    }
};

int main(int argc, char* argv[]) {
    // REPL mode (no arguments)
    if (argc == 1) {
        std::cout << "======================================" << std::endl;
        std::cout << "  ChocoLang 0.4.0 - Cocoa Dream" << std::endl;
        std::cout << "  Interactive REPL" << std::endl;
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
            
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\n\r"));
            line.erase(line.find_last_not_of(" \t\n\r") + 1);
            
            // Check for exit commands
            if (line == "exit" || line == "quit") {
                std::cout << "Goodbye!" << std::endl;
                break;
            }
            
            // Skip empty lines
            if (line.empty()) {
                continue;
            }
            
            // Check for special commands
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
                std::cout << "Variables: (feature coming soon)" << std::endl;
                lineNumber++;
                continue;
            }
            
            if (line == "funcs") {
                std::cout << "Functions: (feature coming soon)" << std::endl;
                lineNumber++;
                continue;
            }
            
            // Add semicolon if missing for single statements
            if (line.back() != ';' && line.back() != '}') {
                line += ";";
            }
            
            try {
                Lexer lexer(line);
                std::vector<Token> tokens = lexer.tokenize();
                
                // Save interpreter state
                size_t savedCurrent = repl.current;
                std::vector<Token> savedTokens = repl.tokens;
                
                // Execute the line
                repl.tokens = tokens;
                repl.current = 0;
                
                while (!repl.isAtEnd()) {
                    repl.statement();
                }
                
                // Restore for next iteration
                repl.tokens = savedTokens;
                repl.current = savedCurrent;
                
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown error occurred" << std::endl;
            }
            
            lineNumber++;
        }
        
        return 0;
    }
    
    // File execution mode
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [file.choco]" << std::endl;
        std::cerr << "       " << argv[0] << "              (for REPL mode)" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    Interpreter interpreter(tokens);
    interpreter.execute();

    return 0;
}