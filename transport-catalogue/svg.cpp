#include "svg.h"

namespace svg {
    using namespace std::literals;

    std::ostream& operator<<(std::ostream& os, svg::StrokeLineCap slc) {
        switch (slc) {
            case svg::StrokeLineCap::BUTT:
                os << "butt"sv;
                break;
            case svg::StrokeLineCap::ROUND:
                os << "round"sv;
                break;
            case svg::StrokeLineCap::SQUARE:
                os << "square"sv;
                break;
            default:
                break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, StrokeLineJoin slj) {
        switch (slj) {
            case StrokeLineJoin::ARCS:
                os << "arcs"sv;
                break;
            case StrokeLineJoin::BEVEL:
                os << "bevel"sv;
                break;
            case StrokeLineJoin::MITER:
                os << "miter"sv;
                break;
            case StrokeLineJoin::MITER_CLIP:
                os << "miter-clip"sv;
                break;
            case StrokeLineJoin::ROUND:
                os << "round"sv;
                break;
            default:
                break;
        }

        return os;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (auto it = points_.begin(); it != points_.end(); it++) {
            if (it != points_.begin()) {
                out << " ";
            }
            out << (*it).x << ","sv << (*it).y;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << " />"sv;
    }

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << "<text";
        RenderAttrs(context.out);
        out <<" x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv
            << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv
            << "font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv << data_ << "</text>"sv;
    }


    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        RenderContext context(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl
            << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (auto& object : objects_) {
            (*object).Render(context);
        }
        out << "</svg>"sv << std::endl;
    }
}  // namespace svg