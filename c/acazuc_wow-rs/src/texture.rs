use crate::{
	wow::Context,
};

use wgpu::util::DeviceExt;

#[derive(Debug)]
pub struct Texture {
	_texture: wgpu::Texture,
	texture_view: wgpu::TextureView,
}

impl Texture {
	pub fn new(context: &Context, blp: &libwow::Texture) -> Self {
		let mut data = Vec::<u8>::new();
		blp.mipmaps.iter().for_each(|mipmap| {
			data.extend(mipmap.data.iter());
		});
		let format = match blp.header.compression {
			2 => match blp.header.alpha_type {
				0 => wgpu::TextureFormat::Bc1RgbaUnorm,
				1 => wgpu::TextureFormat::Bc2RgbaUnorm,
				7 => wgpu::TextureFormat::Bc3RgbaUnorm,
				_ => wgpu::TextureFormat::Rgba8Unorm,
			},
			_ => wgpu::TextureFormat::Rgba8Unorm,
		};
		let texture = context.device.create_texture_with_data(&context.queue, &wgpu::TextureDescriptor {
			label: None,
			size: wgpu::Extent3d {
				width: blp.header.width,
				height: blp.header.height,
				depth_or_array_layers: 1,
			},
			mip_level_count: blp.mipmaps.len() as u32,
			sample_count: 1,
			dimension: wgpu::TextureDimension::D2,
			format: format,
			usage: wgpu::TextureUsages::TEXTURE_BINDING,
			view_formats: &[format],
		}, &data);
		let texture_view = texture.create_view(&wgpu::TextureViewDescriptor {
			label: None,
			format: Some(format),
			dimension: Some(wgpu::TextureViewDimension::D2),
			aspect: wgpu::TextureAspect::All,
			base_mip_level: 0,
			mip_level_count: Some(blp.mipmaps.len() as u32),
			base_array_layer: 0,
			array_layer_count: Some(1),
		});
		Self {
			_texture: texture,
			texture_view,
		}
	}

	pub fn texture_view(&self) -> &wgpu::TextureView {
		&self.texture_view
	}
}
