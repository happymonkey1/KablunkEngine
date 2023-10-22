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

		virtual void set_asset(const ref<asset::IAsset>& asset) override { m_asset = asset.As<Texture2D>(); }

	private:
		virtual void on_open() override;
		virtual void on_close() override;
		virtual void render() override;

	private:
		ref<Texture2D> m_asset;
	};

}

#endif
