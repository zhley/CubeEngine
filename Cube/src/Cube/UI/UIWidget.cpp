#include "UIWidget.h"

#include "Cube/Core/Input.h"
#include "Cube/Renderer/Renderer.h"
#include "Cube/Event/MouseEvent.h"
#include "Cube/Utils/Utils.h"

namespace Cube {

    // UIWidget
    UIStyle UIWidget::globalStyle = {
        {0.0f, 0.0f, 0.0f, 1.0f},  // bgColor
        {0.0f, 0.0f, 0.0f, 0.0f},  // border: top right bottom left
        {0.0f, 0.0f, 0.0f, 1.0f}   // borderColor
    };

    void UIWidget::render() {
        if(!visible) return;
        glm::vec2 renderPos = getWorldPosition();
        Renderer2D::drawQuad(renderPos + size / 2.0f, size, nullptr, style.bgColor);

        Renderer2D::drawLine(renderPos + glm::vec2(0, size.y), renderPos + size, style.borderColor, style.border.x);
        Renderer2D::drawLine(renderPos + size, renderPos + glm::vec2(size.x, 0), style.borderColor, style.border.y);
        Renderer2D::drawLine(renderPos + glm::vec2(size.x, 0), renderPos, style.borderColor, style.border.z);
        Renderer2D::drawLine(renderPos, renderPos + glm::vec2(0, size.y), style.borderColor, style.border.w);
    }

    glm::vec2 UIWidget::getWorldPosition() const {
        if(parent) {
            return parent->getWorldPosition() + pos;
        }
        return pos;
    }

    bool UIWidget::isHovered() const {
        return contain(Input::getMousePos());
    }

    bool UIWidget::contain(const glm::vec2& point) const {
        glm::vec2 pos = getWorldPosition();
        return point.x >= pos.x && point.x <= pos.x + size.x && point.y >= pos.y && point.y <= pos.y + size.y;
    }

    // UIPanel
    void UIPanel::render() {
        if(!isVisible()) return;
        UIWidget::render();
        for(auto& child : children) {
            child->render();
        }
    }

    void UIPanel::addChild(const std::shared_ptr<UIWidget>& child) {
        child->setParent(this);
        children.push_back(child);
    }

    void UIPanel::removeChild(const std::shared_ptr<UIWidget>& child) { children.erase(std::remove(children.begin(), children.end(), child), children.end()); }

    // UIImage
    void UIImage::render() {
        UIWidget::render();
        Renderer2D::drawQuad(getWorldPosition() + size / 2.0f, size, texture, WHITE, 0, texRegion.getUVCoord());
    }

    // UILabel
    void UILabel::render() {
        UIWidget::render();
        glm::vec2 renderPos = getWorldPosition();
        renderPos.y = renderPos.y + size.y + font->getDescender();
        for(uint32_t c : Utils::utf8To32(text)) {
            Glyph* g = font->getGlyph(c);
            Renderer2D::drawQuad(glm::vec2(renderPos.x + g->bearing.x, renderPos.y - g->bearing.y) + g->size / 2.0f, g->size, g->texture, color, 0, g->texRegion.getUVCoord());
            renderPos.x += (float)g->advance;
        }
    }

    bool UIButton::onMousePressed(const Event& e) {
        if(isHovered() && static_cast<const MousePressedEvent&>(e).mouseCode == Mouse::ButtonLeft) {
            onClicked();
            return true;
        }
        return false;
    }

}  // namespace Cube