#pragma once

#include "json.h"
#include <stack>
namespace json {
	class Builder {
	private:
		class ItemContext;
		class KeyItemContext;
		class DictItemContext;
		class ArrayItemContext;
	public:
		KeyItemContext Key(std::string key_);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Builder& Value(Node::Value value_);
		Node Build();

	private:
		Node root_;
		std::stack<Node*>nodes_stack_;
		void EmptyException();
		void Push(Node node);
		class ItemContext {
		public:
			ItemContext(Builder& builder) : builder_(builder) {};
			inline DictItemContext StartDict() {
				return builder_.StartDict();
			}
			inline ArrayItemContext StartArray() {
				return builder_.StartArray();
			}
			inline Builder& EndArray() {
				return builder_.EndArray();
			}
			inline KeyItemContext Key(std::string key) {
				return builder_.Key(key);
			}
			inline Builder& EndDict() {
				return builder_.EndDict();
			}
		protected:
			Builder& builder_;
		};

		class ArrayItemContext : public ItemContext {
		public:
			using ItemContext::ItemContext;
			inline ArrayItemContext Value(Node::Value value) {
				return builder_.Value(value);
			}

			KeyItemContext Key(std::string key) = delete;
			ItemContext EndDict() = delete;
			Node Build() = delete;
		};

		class DictItemContext : public ItemContext {
		public:
			using ItemContext::ItemContext;

			DictItemContext StartDict() = delete;
			ArrayItemContext StartArray() = delete;
			ItemContext EndArray() = delete;
			ItemContext Value(Node::Value value) = delete;
			Node Build() = delete;
		};

		class KeyItemContext : public ItemContext {
		public:
			using ItemContext::ItemContext;
			inline DictItemContext Value(Node::Value value) {
				return builder_.Value(value);
			}

			KeyItemContext Key(std::string key) = delete;
			ItemContext EndDict() = delete;
			ItemContext EndArray() = delete;
			Node Build() = delete;
		};

	};

}