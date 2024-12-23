pub type Vec3 = glam::Vec3;
pub type Vec4 = glam::Vec4;
pub type Mat2 = glam::Mat2;
pub type Mat3 = glam::Mat3;
pub type Mat4 = glam::Mat4;

#[derive(Debug)]
pub struct Aabb {
	pub p0: Vec3,
	pub p1: Vec3,
}

#[derive(Debug, Clone)]
pub struct Frustum {
	planes: Vec<Vec4>,
}

impl Aabb {
	pub fn new(p0: Vec3, p1: Vec3) -> Self {
		Self {
			p0,
			p1,
		}
	}

	pub fn transform(&self, mat: &Mat4) -> Self {
		let center = (self.p0 + self.p1) / 2.0;
		let extents = self.p1 - center;
		let new_center = mat.transform_point3(center);
		let abs_mat = Mat3::from_cols(
			mat.row(0).truncate().abs(),
			mat.row(1).truncate().abs(),
			mat.row(2).truncate().abs(),
		);
		let new_extents = abs_mat.mul_vec3(extents);
		let p0 = new_center + new_extents;
		let p1 = new_center - new_extents;
		Self {
			p0: p0.min(p1),
			p1: p0.max(p1),
		}
	}
}

impl Frustum {
	pub fn new() -> Self {
		Self {
			planes: Vec::new(),
		}
	}

	pub fn clear(&mut self) {
		self.planes.clear();
	}

	pub fn push_plane(&mut self, plane: Vec4) {
		self.planes.push(plane);
	}

	pub fn push_mat4(&mut self, mat: &Mat4) {
		self.push_plane(mat.row(3) - mat.row(0));
		self.push_plane(mat.row(3) + mat.row(0));
		self.push_plane(mat.row(3) - mat.row(1));
		self.push_plane(mat.row(3) + mat.row(1));
		self.push_plane(mat.row(3) - mat.row(2));
		self.push_plane(mat.row(3) + mat.row(2));
	}

	pub fn test_aabb(&self, aabb: &Aabb) -> bool {
		let points = [
			Vec4::new(aabb.p0.x, aabb.p0.y, aabb.p0.z, 1.0),
			Vec4::new(aabb.p1.x, aabb.p0.y, aabb.p0.z, 1.0),
			Vec4::new(aabb.p0.x, aabb.p1.y, aabb.p0.z, 1.0),
			Vec4::new(aabb.p1.x, aabb.p1.y, aabb.p0.z, 1.0),
			Vec4::new(aabb.p0.x, aabb.p0.y, aabb.p1.z, 1.0),
			Vec4::new(aabb.p1.x, aabb.p0.y, aabb.p1.z, 1.0),
			Vec4::new(aabb.p0.x, aabb.p1.y, aabb.p1.z, 1.0),
			Vec4::new(aabb.p1.x, aabb.p1.y, aabb.p1.z, 1.0),
		];
		for plane in &self.planes {
			if plane.dot(points[0]) < 0.0
			&& plane.dot(points[1]) < 0.0
			&& plane.dot(points[2]) < 0.0
			&& plane.dot(points[3]) < 0.0
			&& plane.dot(points[4]) < 0.0
			&& plane.dot(points[5]) < 0.0
			&& plane.dot(points[6]) < 0.0
			&& plane.dot(points[7]) < 0.0 {
				return false;
			}
		}
		true
	}
}
