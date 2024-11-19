#pragma once
#ifndef KABLUNK_EDITOR_PANELS_ASSET_DEFAULT_ASSET_VIEWER_PANELS_H
#define KABLUNK_EDITOR_PANELS_ASSET_DEFAULT_ASSET_VIEWER_PANELS_H

#include "Panels/AssetEditorPanel.h"

namespace kb
{

class TextureViewer : public AssetEditor
{
public:
	TextureViewer();

	virtual void set_asset(const arc<asset::IAsset>& asset) override { m_asset = asset.As<render::backend::texture_2d>(); }

private:
	virtual void on_open() override;
	virtual void on_close() override;
	virtual void render() override;

private:
	arc<render::backend::texture_2d> m_asset;
};

}

#endif
