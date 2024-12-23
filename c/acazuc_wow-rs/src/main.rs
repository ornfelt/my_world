#![deny(elided_lifetimes_in_paths)]

mod assets;
mod liquid;
mod mapobj;
mod math;
mod terrain;
mod texture;
mod window;
mod world;
mod wow;

use winit::{
	event::{
		Event::{
			WindowEvent,
			DeviceEvent,
			AboutToWait,
		},
		WindowEvent::{
			CloseRequested,
			RedrawRequested,
			Resized,
			KeyboardInput,
			MouseInput,
		},
		ElementState::{
			Pressed,
			Released,
		},
		DeviceEvent::{
			MouseMotion,
		},
	},
};

#[pollster::main]
async fn main() {
	let event_loop = winit::event_loop::EventLoop::new().unwrap();
	event_loop.set_control_flow(winit::event_loop::ControlFlow::Poll);
	let mut wow = wow::Wow::new(&event_loop).await;
	wow.load_world(std::env::args().nth(1).or(Some(String::from("azeroth"))).unwrap());
	event_loop.run(move |event, elwt| match event {
		WindowEvent { event, window_id } if window_id == wow.window.window.id() => match event {
			CloseRequested => elwt.exit(),
			RedrawRequested => wow.render(),
			Resized(size) => {
				wow.context.resize(size.width, size.height);
			}
			KeyboardInput { event, .. } => {
				match (event.state, event.repeat) {
					(Pressed, false) => wow.on_key_down(event.physical_key),
					(Pressed, true)  => wow.on_key_press(event.physical_key),
					(Released, _)    => wow.on_key_up(event.physical_key),
				}
				match event.text {
					Some(text) => wow.on_char(text),
					None => {},
				}
			}
			MouseInput { state, button, .. } => {
				match state {
					Pressed => wow.on_mouse_down(button),
					Released => wow.on_mouse_up(button),
				}
			}
			_ => {} //println!{"WindowEvent: {event:?}"}
		}
		DeviceEvent { event, .. } => {
			match event {
				MouseMotion { delta } => {
					let (x, y) = delta;
					wow.on_mouse_move(x as f32, y as f32);
				}
				_ => {}
			}
		}
		AboutToWait => wow.window.window.request_redraw(),
		_ => {} //println!{"Event: {event:?}"}
	}).expect("event handling failed");
}
