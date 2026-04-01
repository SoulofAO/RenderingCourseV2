#pragma once

#include "Abstracts/Components/UIRenderingComponent.h"

class SelectionGame;

class SelectionUIRenderingComponent : public UIRenderingComponent
{
public:
	SelectionUIRenderingComponent();
	~SelectionUIRenderingComponent() override;

protected:
	void RenderUI() override;

private:
	SelectionGame* GetOwningSelectionGame() const;
};
