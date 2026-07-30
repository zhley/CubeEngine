#pragma once

#include <memory>
#include <vector>

#include "glm/glm.hpp"

#include "Cube/Event/Event.h"
#include "Cube/Renderer/Color.h"
#include "Cube/Renderer/Texture.h"
#include "Cube/Renderer/TextureRegion.h"
#include "Cube/Renderer/Font.h"
#include "Cube/Resource/ResourceManager.h"

namespace Cube {
    struct UIStyle {
        Color bgColor;
        glm::vec4 border;
        Color borderColor;
    };

    // base class for all UI elements
    class UIWidget {
    public:
        UIWidget(){ style = globalStyle; }
        UIWidget(const glm::vec2& pos, const glm::vec2& size) : pos(pos), size(size) { style = globalStyle; }
        virtual ~UIWidget() = default;

        virtual void render();
        glm::vec2 getWorldPosition() const;
        bool isHovered() const;

        void setParent(UIWidget* p) { parent = p; }
        UIWidget* getParent() const { return parent; }

        void setPosition(const glm::vec2& p) { pos = p; }
        glm::vec2 getPosition() const { return pos; }

        void setSize(const glm::vec2& s) { size = s; }
        glm::vec2 getSize() const { return size; }

        UIStyle& getStyle() { return style; }

        void setVisible(bool v) { visible = v; }
        bool isVisible() const { return visible; }

        static UIStyle globalStyle;

    protected:
        glm::vec2 pos = {0.0f, 0.0f};     // relative position to parent
        glm::vec2 size = {100.0f, 30.0f};
        UIStyle style = {};
        bool visible = true;

        UIWidget* parent = nullptr; // upper container

        bool contain(const glm::vec2& point) const;
    };

    // UIPanel
    class UIPanel : public UIWidget {
    public:
        UIPanel() = default;
        UIPanel(std::initializer_list<std::shared_ptr<UIWidget>> children) : children(children) {}
        ~UIPanel() override = default;

        void render() override;

        void addChild(const std::shared_ptr<UIWidget>& child);
        void removeChild(const std::shared_ptr<UIWidget>& child);

    protected:
        std::vector<std::shared_ptr<UIWidget>> children;
    };

    // UIImage
    class UIImage : public UIWidget {
    public:
        UIImage(const std::string& textureId, const glm::vec2& pos = {0.0f, 0.0f}, const glm::vec2& size = {100.0f, 30.0f}, const TextureRegion& texRegion = {{0.0f, 1.0f}, {1.0f, 0.0f}}) : UIWidget(pos, size), texRegion(texRegion) {
            texture = ResourceManager::get().load<Texture2D>(textureId);
        }
        ~UIImage() override {
            ResourceManager::get().release(texture);
        }

        void render() override;
    protected:
        Texture2D* texture;
        TextureRegion texRegion;
    };

    // UILabel
    class UILabel : public UIWidget {
    public:
        UILabel(const std::string& text, const std::string& fontId, const Color& color, const glm::vec2& pos) : text(text), color(color) {
            this->pos = pos;
            font = ResourceManager::get().load<Font>(fontId);
            size = font->calcTextSize(text);
        }
        UILabel(const std::string& text, const Color& color, const glm::vec2& pos = {0.0f, 0.0f}) : UILabel(text, globalFont, color, pos){}
        ~UILabel() override {
            ResourceManager::get().release(font);
        }

        void render() override;

        static std::string globalFont;
    protected:
        std::string text;
        Color color;
        Font* font;
    };

    // UIButton
    class UIButton : public UIWidget{
    public:
        UIButton(const std::function<void()>& onClicked, const glm::vec2& pos = {0.0f, 0.0f}, const glm::vec2& size = {100.0f, 30.0f}) : UIWidget(pos, size), onClicked(onClicked) {
            // EventDispatcher::get().subscribe<MousePressedEvent>(std::bind(&UIButton::onMousePressed, this, std::placeholders::_1));
        }
        ~UIButton() = default;
    private:
        bool onMousePressed(const Event& e);

        std::function<void()> onClicked;
    };
}