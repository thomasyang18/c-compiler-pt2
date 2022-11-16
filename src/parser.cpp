#include "parser.hpp"
#include <algorithm>
#include <deque>
#include <unordered_set>
#include <iostream>
#include <stdexcept>
#include <stack>
#include <map>
// Custom TokenAccessor class to handle exceptions cleanly

// Tokens:
// Type tokens: Lookup in a global table
// Identifier tokens: just identify them bro
// Keyword tokens:  
// Number tokens:
// String tokens: 


std::shared_ptr<Node> parse_assignment();
std::shared_ptr<Node> parse_control_block();
std::shared_ptr<Node> parse_declarations();
std::shared_ptr<Node> parse_expressions();
std::shared_ptr<Node> parse_function();
std::shared_ptr<Node> parse_program();

#define INIT_NODE auto ret = std::shared_ptr<Node>(new Node());


class TokenAccessor
{
private:
    std::deque<std::string> tokens;
public:
    TokenAccessor(){};
    TokenAccessor(std::deque<std::string> p_tokens): tokens(p_tokens) {}

    std::string& operator [](int index) {
        if (index >= tokens.size()){
            throw std::invalid_argument("Bad Token Access");
        } 
        return tokens[index];
    }

    bool match(std::vector<std::string> input){
        for (int i = 0; i < input.size(); i++) if (input[i] != (*this)[i]) return false;
        return true; 
    }

    std::vector<std::string> consume(int len=1){
        std::vector<std::string> result;
        for (int i = 0; i < len; i++){
            result.push_back(tokens[0]);
            tokens.pop_front();
        }
        return result;
    }

    std::vector<std::string> consume(std::vector<std::string> input){
        if (!match(input)) throw std::invalid_argument("Bad consumption");
        std::vector<std::string> result;
        for (int i = 0; i < input.size(); i++){
            result.push_back(tokens[0]);
            tokens.pop_front();
        }
        return result;
    }

    bool empty(){
        return tokens.empty();
    }

} tokens;

void push_all_back(std::vector<std::string> &left, std::vector<std::string> right){
    for (auto x: right) left.push_back(x);
}

void push_all_back(std::vector<std::string> &left, std::stack<std::string> right){
    std::vector<std::string> temp;
    while (right.size()) {
        temp.push_back(right.top());
        right.pop();
    }
    std::reverse(temp.begin(), temp.end());
    for (auto x: temp) left.push_back(x);
}


bool is_identifier(std::string &str){
    for (int i = 0; i < str.size(); i++){
        if (i == 0 and isdigit(str[i])) return false;
        else if (isalnum(str[i]) or str[i] == '_' or isdigit(str[i]));
        else return false;
    }
    return true;
}

bool is_number(std::string &str){
    for (int i = 0; i < str.size(); i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

// parsing types sucks 

std::deque<std::string> lex_program(std::string& program){
    std::deque<std::string> test = {"int", "main", "(", ")", "{", "int", "x", "=", 
    "4", "+", "18", "/", "(", "9", "-", "3", ";",
    "}"};
    return test;
}

std::shared_ptr<Node> parse_program(std::string program){
    // Program is just a list of either functions or declaration statements
    
    // For now, no pointers, arrays, or structs are allowed. Advanced types come way later.

    tokens = TokenAccessor(lex_program(program));

    INIT_NODE

    while (!tokens.empty()){
        // First should always be a type, then it should be a name, then decide from there

        // ONLY TYPE FOR NOW IS INT
        if (tokens[0] != "int") throw std::invalid_argument("bad parse");
        
        // consume identifier
        if (!is_identifier(tokens[1])){
            throw std::invalid_argument("bad parse");
        }
        // If it's a parentheses, do function, otherwise declist
        if (tokens[2] == "(") ret->children.push_back(parse_function());
        else ret->children.push_back(parse_declarations());
    }

    return ret;
}

std::shared_ptr<Node> parse_function(){
    
    INIT_NODE

    push_all_back(ret->represent, tokens.consume(2));
    
    tokens.consume({"("});
    
    // parse paramlist

    tokens.consume({")"});
    
    // no function prototypes for now

    ret->children.push_back(parse_control_block()); // essentially treat the below as a control block    

    return ret;
}

std::shared_ptr<Node> parse_control_block(){
    // Control blocks are while, for, switch, as well as empty braces. Functions are a special 
    // control block that are the first level of all other control blocks.
    INIT_NODE

    if (tokens[0] == "while"){
        // parse while
        push_all_back(ret->represent, tokens.consume(1));
    }
    else if (tokens[0] == "for"){
        // parse for
        push_all_back(ret->represent, tokens.consume(1));
    }
    else if (tokens[0] == "if"){
        push_all_back(ret->represent, tokens.consume(1));
    }
    else if (tokens[0] == "else") {
        push_all_back(ret->represent, tokens.consume(1));
        // check if tokens[1] == else if
        if (tokens[0] == "if") 
        push_all_back(ret->represent, tokens.consume(1));
    }
    
    // should now only be {}. Techincally wrong since you can have no braces, but for now always assume
    if (tokens[0] == "{") {
        // empty control block
        tokens.consume({"{"});
        // Can be declist, can be control block, can be just <var> = <expr>, or just <expr>. For now, assume that <expr> is impossible
        while (tokens[0] != "}"){
            if (tokens[0] == "int") {
                ret->children.push_back(parse_declarations());
            }
            else if (tokens[0] == "while" || tokens[0] == "{" || tokens[0] == "for" || tokens[0] == "if" || tokens[0] == "else") {
                ret->children.push_back(parse_control_block());
            }
            else ret->children.push_back(parse_assignment());
        }

        tokens.consume({"}"});
        
    } 
    

    return nullptr;
}

std::shared_ptr<Node> parse_assignment(){
    INIT_NODE

    if (!is_identifier(tokens[0])) throw std::invalid_argument("assignemnt requires left ident");

    push_all_back(ret->represent, tokens.consume(1));

    if (tokens[0] != "=") throw std::invalid_argument("assignemnt requires '='");

    tokens.consume();

    ret->children.push_back(parse_expressions());

    return ret;
}

std::shared_ptr<Node> parse_declarations(){
    // <declist> := <type> <decl> [, <decl>]; 
    // <decl> := <var> '=' <expr>

    // consume declarations
    INIT_NODE
    
    push_all_back(ret->represent, tokens.consume(1));

    ret->children.push_back(parse_assignment());

    while (tokens[0] != ";"){
        std::cout << "Yo " << tokens[0] << std::endl;
        if (tokens[0] != ",") throw std::invalid_argument("fuck u");
        
        ret->children.push_back(parse_assignment());
    }
    tokens.consume(1);
     
    return ret;
}

std::shared_ptr<Node> parse_expressions(){
    // shunting yard expressions 
    INIT_NODE

    // does not handel negative numbers yet

    // 3 * 4 + 5 * var + 3 - 4
    std::stack<std::string> op_st;
    std::stack<std::string> output;

    std::map<std::string, int> prec; // lower precedence is better
    prec["+"] = 1;
    prec["-"] = 1;
    prec["*"] = 0;
    prec["/"] = 0; 
    prec["("] = prec[")"] = -1; // highest precedence technically

    while (is_identifier(tokens[0]) || is_number(tokens[0]) || 
        tokens[0] == "+" || 
        tokens[0] == "-" || 
        tokens[0] == "/" || 
        tokens[0] == "*" ||
        tokens[0] == "(" || 
        tokens[0] == ")" 
    ) {
        // while its a valid input for shutning yard; we assume comma or semicolon breaks
        auto head = tokens.consume()[0];
        if (is_identifier(head) || is_number(head)) output.push(head);
        else if (head == "(") op_st.push(head);
        else if (head == ")"){
            bool found = false;
            while (op_st.size()){
                auto top = op_st.top();
                op_st.pop();
                if (top == "(") {
                    found = true;
                    break;
                }
                output.push(top);
            }
            if (!found) throw std::invalid_argument("No matching left bracket");
        }
        else{
            while (op_st.size() and prec[op_st.top()] < prec[head]) {
                output.push(op_st.top());
                op_st.pop();
            }
            op_st.push(head);
            // has to be one of the operators
        }
    }
    while (op_st.size()) {
        output.push(op_st.top());
        op_st.pop();
    }

    push_all_back(ret->represent, op_st);

    return ret;
}
