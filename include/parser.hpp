#ifndef PARSER_HPP_
#define PARSER_HPP_
#include <string>
#include <vector>
#include <memory>
class Node {
public:
    std::vector<std::string> represent;
    std::vector<std::shared_ptr<Node>> children;

};

std::shared_ptr<Node> parse_program(std::string program);


#endif