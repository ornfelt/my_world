use winit::window::CursorGrabMode;

pub struct Window {
	pub window: winit::window::Window,
	grabbed: bool,
}

impl Window {
	pub fn new(event_loop: &winit::event_loop::EventLoop<()>) -> Self {
		let window = winit::window::Window::new(&event_loop).unwrap();
		window.set_title("wow-rs");
		Self {
			window,
			grabbed: false,
		}
	}

	pub fn width(&self) -> u32 {
		self.window.inner_size().width
	}

	pub fn height(&self) -> u32 {
		self.window.inner_size().height
	}

	pub fn aspect_ratio(&self) -> f32 {
		self.width() as f32 / self.height() as f32
	}

	pub fn grab(&mut self) {
		self.window.set_cursor_grab(
			CursorGrabMode::Confined
		).or_else(|_e| {
			self.window.set_cursor_grab(CursorGrabMode::Locked)
		}).unwrap();
		self.window.set_cursor_visible(false);
		self.grabbed = true;
	}

	pub fn ungrab(&mut self) {
		self.window.set_cursor_grab(CursorGrabMode::None).unwrap();
		self.window.set_cursor_visible(true);
		self.grabbed = false;
	}

	pub fn grabbed(&self) -> bool {
		self.grabbed
	}
}
