#include "DECL.h"
#include <stack>
#include "vendor/jsonxx/jsonxx.h"

namespace HAYDEN {


    class DeclParser {
        enum class TokenType {
            String,
            QString,
            Number,
            ObjStart,
            ObjEnd,
            Equals,
            Semicolon,
            Eof,
            Null,
        };
        struct Token {
            TokenType m_type;
            std::string m_value;

        };
    public:
        explicit DeclParser(const std::vector<char> &data) : m_data(data), offset(0), m_failed(false) {};

        bool eof() {
            return offset >= m_data.size();
        }

        Token nextToken() {
            if (m_peek.m_type != TokenType::Null) {
                Token res = m_peek;
                m_peek = {TokenType::Null, ""};
                return res;
            }

            while (!eof()) {
                char curChar = currentChar();
                switch (curChar) {
                    case ('{'): {
                        advance();
                        return Token{TokenType::ObjStart, "{"};
                    }
                    case ('}'): {
                        advance();
                        return Token{TokenType::ObjEnd, "}"};
                    }
                    case ('='): {
                        advance();
                        return Token{TokenType::Equals, "="};
                    }
                    case (';'): {
                        advance();
                        return Token{TokenType::Semicolon, ";"};
                    }
                    case ('"'): {
                        advance();
                        std::string buffer = parseUntilTerminator("\"");
                        advance();
                        return Token{TokenType::QString, std::move(buffer)};
                    }
                    case ('\n'):
                    case ('\r'):
                    case ('\t'):
                    case (','):
                    case (' '): {
                        advance();
                        continue;
                    }
                    default: {
                        if (std::isalpha(curChar)) {
                            std::string buffer = parseUntilTerminator(" \t\n\r;");
                            return Token{TokenType::String, std::move(buffer)};

                        } else if (isNumeric(curChar)) {
                            std::string buffer = parseUntilTerminator(" \t\n\r;");
                            return Token{TokenType::Number, std::move(buffer)};

                        }
                    }
                }
                std::cout << "Unhandled \"" << advance() << "\"" << std::endl;
            }
            return Token{TokenType::Eof, ""};
        }

        Token peekToken() {
            if (m_peek.m_type != TokenType::Null) {
                return m_peek;
            }
            m_peek = nextToken();
            return m_peek;
        }

        bool hasNextOfType(TokenType type) {
            Token peek = peekToken();
            return peek.m_type == type;
        }

        bool expect(TokenType type) {
            return expect(type, false);
        }

        bool expect(TokenType type, bool consume) {
            Token peek = peekToken();
            if (peek.m_type != type) {
                return false;
            }
            if (consume)
                nextToken();
            return true;
        }

        jsonxx::Object parse() {
            if (!expect(TokenType::ObjStart))
                return {};
            jsonxx::Value v = parseObjectOrArray();
            if (v.is<jsonxx::Object>()) {
                return v.get<jsonxx::Object>();
            }
            return {};
        }

    private:

        jsonxx::Value parseValue() {
            if (expect(TokenType::ObjStart)) {
                return parseObjectOrArray();
            } else if (expect(TokenType::Number)) {
                Token numberToken = nextToken();
                expect(TokenType::Semicolon, true);
                std::string number = numberToken.m_value;
                if (number.find('.') != std::string::npos) {
                    return jsonxx::Number(std::stod(number));
                } else {
                    return jsonxx::Number(std::stol(number));
                }
            } else if (expect(TokenType::QString)) {
                Token stringToken = nextToken();
                expect(TokenType::Semicolon, true);
                return jsonxx::String(stringToken.m_value);
            } else if (expect(TokenType::String)) {
                Token stringToken = nextToken();
                expect(TokenType::Semicolon, true);
                if(stringToken.m_value=="true" || stringToken.m_value=="false"){
                    return jsonxx::Boolean(stringToken.m_value=="true");
                }
                std::cout << "Unexpected keyword \"" << stringToken.m_value << "\""
                          << std::endl;
                return jsonxx::Null();
            } else {
                m_failed = true;
                std::cout << "Unexpected token " << std::to_string((uint32_t)peekToken().m_type) << " \"" << peekToken().m_value << "\""
                          << std::endl;
                return jsonxx::Null();
            }
        }

        jsonxx::Value parseObjectOrArray() {
            if (expect(TokenType::ObjStart, true)) {
                Token peek = peekToken();
                if (peek.m_value.find('[') != std::string::npos &&
                    peek.m_value.find(']') != std::string::npos) {
                    jsonxx::Array arr;
                    while (!expect(TokenType::ObjEnd)) {
                        nextToken();//Skip name;
                        if (!expect(TokenType::Equals, true)) {
                            m_failed = true;
                            break;
                        }
                        arr << parseValue();
                    }
                    return arr;
                } else {
                    jsonxx::Object o;

                    while (true) {
                        Token keyToken = nextToken();
                        if (keyToken.m_type == TokenType::ObjEnd) {
                            break;
                        }
                        if (keyToken.m_type != TokenType::String || !expect(TokenType::Equals, true)) {
                            m_failed = true;
                            break;
                        }

                        o << keyToken.m_value;
                        o << parseValue();
                        if (m_failed)
                            break;
                    }
                    return o;
                }
            }
            return {};
        };

        [[nodiscard]] char currentChar() const { return m_data[offset]; }

        char advance() {
            if (offset >= m_data.size()) {
                return 0;
            }
            return m_data[offset++];
        }

        std::string parseUntilTerminator(const std::string &terminators) {
            std::string result;

            // While the current character is not a terminator...
            while (terminators.find(currentChar()) == std::string::npos) {
                // Add the current character to the result
                result += currentChar();

                // Move to the next character
                advance();
            }

            // Return the accumulated string
            return result;
        }

        static bool isNumeric(char c) {
            return (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.';
        }

        const std::vector<char> &m_data;
        bool m_failed;
        size_t offset;
        Token m_peek{TokenType::Null, ""};
    };


    DeclFile::DeclFile(const ResourceManager &resourceManager, const std::string &resourcePath) {
        m_loaded = false;
        auto optHeaderData = resourceManager.queryFileByName(resourcePath);
        if (!optHeaderData.has_value()) {
            return;
        }
        m_data.resize(optHeaderData->size());
        std::memcpy(m_data.data(), optHeaderData->data(), m_data.size());
    }

    void DeclFile::parse() {
        DeclParser parser(m_data);
        m_parsed = parser.parse();
    }
}