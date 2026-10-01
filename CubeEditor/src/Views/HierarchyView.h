#pragma once

#include "View.h"

class HierarchyView : public View {
public:
    HierarchyView(EditorPage& editorPage) : View(editorPage) {}
    ~HierarchyView() override = default;

    void render(float deltaTime) override;

private:
};
