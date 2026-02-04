#pragma once

enum class ShaderType {
	Vertex = 0,
	Pixel,
	Geometry,
	Hull,
	Compute,
	Domain,
	Amplification,
	Mesh,
	Library,
	_Count
};

struct IMainFrame {
	virtual BOOL UIAddToolBar(HWND tb) = 0;
	virtual BOOL UIEnable(UINT id, bool enable) = 0;
	virtual void SetTabModified(HWND hTab, bool modified) = 0;
	virtual void SetTabTitle(HWND hTab, PCWSTR title) = 0;
};

