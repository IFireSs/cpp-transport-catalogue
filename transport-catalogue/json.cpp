#include "json.h"
#include <algorithm>
using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw ParsingError("");
            }
            return Node(move(result));
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
                };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
                };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }
        Node LoadDict(std::istream& input) {
            Dict dict;
            for (char ch; input >> ch && ch != '}';) {
                if (ch == '"') {
                    std::string key = LoadString(input).AsString(); 
                    if (input >> ch && ch == ':') {
                        if (dict.find(key) != dict.end()) {
                            throw ParsingError("");
                        }
                        dict.emplace(std::move(key), LoadNode(input));
                    }
                    else {
                        throw ParsingError("");
                    }
                }
                else if (ch != ',') {
                    throw ParsingError("");
                }
            }
            if (!input) {
                throw ParsingError("");
            }
            return Node(dict);

        }
        
        void LoadLiteral(std::istream& input, std::string& str) {
            while (std::isalpha(input.peek())) {
                str.push_back(static_cast<char>(input.get()));
            }
        }
        Node LoadNull(std::istream& input) {
            std::string str;
            if (LoadLiteral(input, str); str != "null"sv) {
                throw ParsingError("");
            }
            return Node(nullptr);
        }
        Node LoadBool(std::istream& input) {
            std::string str;
            LoadLiteral(input, str);
            if (str == "true"sv) {
                return Node(true);
            }
            else if (str == "false"sv) {
                return Node(false);
            }
            else {
                throw ParsingError("unable to parse '"s + str + "' as bool"s);
            }
        }
        Node LoadNode(istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError(""s);
            }
            else if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }
    }  // namespace

    const Array& Node::AsArray() const { 
        if (!IsArray()) {
            throw std::logic_error("");
        }
        return std::get<Array>(*this);
    }
    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("");
        }
        return std::get<Dict>(*this);
    }
    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("");
        }
        return std::get<bool>(*this);
    }
    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("");
        }
        return std::get<int>(*this);
    }
    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("");
        }
        return IsPureDouble() ? std::get<double>(*this) : AsInt();
    }
    const string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("");
        }
        return std::get<std::string>(*this);
    } 

    bool Node::IsNull() const {
        return (*this).index() == 0;
    };
    bool Node::IsArray() const {
        return (*this).index() == 1;
    };
    bool Node::IsMap() const {
        return (*this).index() == 2;
    };
    bool Node::IsBool() const {
        return (*this).index() == 3;
    };
    bool Node::IsInt() const {
        return (*this).index() == 4;
    };
    bool Node::IsPureDouble() const {
        return (*this).index() == 5;
    };
    bool Node::IsDouble() const {
        return (*this).index() == 5 || (*this).index() == 4;
    };
    bool Node::IsString() const {
        return (*this).index() == 6;
    };

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }
    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }
    void PrintValue(const bool& value, const PrintContext& ctx) {
        ctx.out << (value ? "true"sv : "false"sv);
    }
    void PrintNode(const Node& node, const PrintContext& ctx);

    void PrintValue(const string& str, const PrintContext& ctx) {
        ctx.out.put('"');
        for (const char& ch : str) {
            if (ch == '\r') {
                ctx.out << "\\r"s;
            }
            else if (ch == '\n') {
                ctx.out << "\\n"s;
            }
            else if (ch == '\t') {
                ctx.out << "\\t"s;
            }
            else if (ch == '"') {
                ctx.out << "\\\""s;
            }
            else if (ch == '\\') {
                ctx.out << "\\\\"s;
            }
            else {
                ctx.out.put(ch);
            }
        }
        ctx.out.put('"');
    }
    void PrintValue(const Array& arr, const PrintContext& ctx) {
        ctx.out << "[\n"s;
        for (auto it = arr.begin(); it != arr.end(); it++) {
            if (it != arr.begin()) {
                ctx.out << ",\n"s;
            }
            ctx.Indented().PrintIndent();
            PrintNode(*it, ctx.Indented());
        }
        ctx.out.put('\n');
        ctx.PrintIndent();
        ctx.out.put(']');
    }
    void PrintValue(const Dict& dict, const PrintContext& ctx) {
        ctx.out << "{\n"s;
        for (auto it = dict.begin(); it != dict.end(); it++) {
            if (it != dict.begin()) {
                ctx.out << ",\n"s;
            }
            ctx.Indented().PrintIndent();
            PrintValue((*it).first, ctx);
            ctx.out << ": "s;
            PrintNode((*it).second, ctx.Indented());
        }
        ctx.out.put('\n');
        ctx.PrintIndent();
        ctx.out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) { PrintValue(value, ctx); },
            node.GetValue());
    }
    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json