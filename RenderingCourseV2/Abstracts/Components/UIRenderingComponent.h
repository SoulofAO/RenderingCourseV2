#pragma once

#include "Abstracts/Components/RenderingComponent.h"
#include <windows.h>

class UIRenderingComponent : public RenderingComponent
{
public:
	UIRenderingComponent();
	~UIRenderingComponent() override;

	void Initialize() override;
	void Update(float DeltaTime) override;
	void Shutdown() override;

	bool HandleMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

protected:
	virtual void RenderUI() = 0;

	friend class UIRenderingForwardRendererProxyObject;
	friend class UIRenderingDeferredRendererProxyObject;
};
