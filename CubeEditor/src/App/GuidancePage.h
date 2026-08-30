#pragma once

#include "Cube/Core/Path.h"
#include "Cube/Renderer/Texture.h"
#include "Page.h"

#include <memory>
#include <vector>

class GuidancePage : public Page {
public:
	GuidancePage();
	~GuidancePage() override;

	void render(float deltaTime) override;
    Page::Type getType() const override { return Page::Type::Guidance; }

private:
	std::vector<Cube::Path> projectsPathCache;
	std::unique_ptr<Cube::Texture2D> new_project_png = std::make_unique<Cube::Texture2D>("assets/icons/new_project.png");
	std::unique_ptr<Cube::Texture2D> open_project_png = std::make_unique<Cube::Texture2D>("assets/icons/open_project.png");
};