#pragma once
#include "View.h"
#include "Views/ResourcePickerDialog.h"

class EntityPropertyPanel : public View{
public:
    EntityPropertyPanel(EditorPage& editorPage) : View(editorPage) {}
	~EntityPropertyPanel() override = default;

	void render(float deltaTime) override;

private:
	ResourcePickerDialog resourcePickerDialog;
};