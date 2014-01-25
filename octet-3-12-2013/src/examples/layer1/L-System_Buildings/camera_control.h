namespace octet 
{
	class camera_control
	{
		float degree_h;
		float degree_v;
		float view_distance;
		vec3 position;
		mat4t camera_to_world;

		void generate_matrix()
		{
			camera_to_world.loadIdentity();
			camera_to_world.translate(position.x(), position.y(), position.z());
			camera_to_world.rotateY(degree_h);
			camera_to_world.rotateX(degree_v);
			camera_to_world.translate(0, 0, view_distance);
		}

	public:
		camera_control() : degree_h(0),
		degree_v(0),
		view_distance(10),
		position(0, 0, 0)
		{
			generate_matrix();
		}

		void add_view_distance(float f)
		{
			view_distance += f;
			generate_matrix();
		}

		void set_view_distance(float distance)
		{
			view_distance = distance;
			generate_matrix();
		}

		void set_view_position(const vec3 &pos)
		{
			position = pos;
			generate_matrix();
		}

		const mat4t &get_matrix()
		{
			return camera_to_world;
		}

		void rotate_h(float f)
		{
			degree_h += f;
			generate_matrix();
		}
		
		void rotate_v(float f)
		{
			degree_v += f;
			if(degree_v > 90)
				degree_v = 90;
			else if(degree_v < -90)
				degree_v = -90;
			generate_matrix();
		}
	};
}
