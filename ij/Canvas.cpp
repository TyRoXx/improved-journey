#include "Canvas.h"

ij::Text::Text()
{
}

ij::Text::Text(Text &&other) noexcept
    : _canvas(other._canvas)
    , _id(other._id)
{
    other._canvas = nullptr;
}

ij::Text::Text(Canvas &canvas, TextId id)
    : _canvas(&canvas)
    , _id(id)
{
}

ij::Text::~Text()
{
    if (_canvas == nullptr)
    {
        return;
    }
    _canvas->DeleteText(_id);
}

ij::Text &ij::Text::operator=(Text &&other) noexcept
{
    using std::swap;
    swap(_canvas, other._canvas);
    swap(_id, other._id);
    return *this;
}

void ij::Text::SetPosition(const Vector2f &position)
{
    assert(_canvas);
    _canvas->SetTextPosition(_id, position);
}

ij::Vector2f ij::Text::GetPosition()
{
    assert(_canvas);
    return _canvas->GetTextPosition(_id);
}

void ij::Text::Draw()
{
    assert(_canvas);
    _canvas->DrawText(_id);
}

ij::Canvas::~Canvas()
{
}
