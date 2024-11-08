#include "json_builder.h"

namespace json {
    void Builder::EmptyException() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("");
        }
    }
    Builder::KeyItemContext Builder::Key(std::string key_) {
        EmptyException();
        if (nodes_stack_.top()->IsMap()) {
            nodes_stack_.push(new Node(key_));
        }
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        nodes_stack_.push(new Node(Dict()));
        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {
        nodes_stack_.push(new Node(Array()));
        return *this;
    }

    Builder& Builder::EndDict() {
        EmptyException();
        if (Node node = *nodes_stack_.top(); !node.IsMap()) {
            throw std::logic_error("");
        }
        else {
            nodes_stack_.pop();
            Push(node);
        }
        return *this;
    }

    Builder& Builder::EndArray() {
        EmptyException();
        if (Node node = *nodes_stack_.top(); !node.IsArray()) {
            throw std::logic_error("");
        }
        else {
            nodes_stack_.pop();
            Push(node);
        }
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        if (std::holds_alternative<Array>(value)) {
            Push(Node(std::get<Array>(value)));
        }
        else if (std::holds_alternative<Dict>(value)) {
            Push(Node(std::get<Dict>(value)));
        }
        else if (std::holds_alternative<bool>(value)) {
            Push(Node(std::get<bool>(value)));
        }
        else if (std::holds_alternative<int>(value)) {
            Push(Node(std::get<int>(value)));
        }
        else if (std::holds_alternative<double>(value)) {
            Push(Node(std::get<double>(value)));
        }
        else if (std::holds_alternative<std::string>(value)) {
            Push(Node(std::get<std::string>(value)));
        }
        else {
            Push(Node());
        }
        return *this;
    }

    Node Builder::Build() {
        if (root_.IsNull() || !nodes_stack_.empty()) {
            throw std::logic_error("");
        }
        return root_;
    }

    void Builder::Push(Node node) {
        if (nodes_stack_.empty() && root_.IsNull()) {
            root_ = node;
        }
        else if (nodes_stack_.top()->IsString()) {
            Node& top = *nodes_stack_.top();
            nodes_stack_.pop();
            const_cast<Dict&>(nodes_stack_.top()->AsMap()).emplace(top.AsString(), node);
        }
        else if (nodes_stack_.top()->IsArray()) {
            const_cast<Array&>(nodes_stack_.top()->AsArray()).push_back(node);
        }
        else {
            throw std::logic_error("");
        }
    }
}