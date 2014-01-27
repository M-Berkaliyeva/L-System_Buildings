namespace octet {
class LSystem_building_app : public app {
	// Matrix to transform points on our triangle to the world space
	// This allows us to move and rotate our triangle
	mat4t modelToWorld;

	unsigned int key_cool_down;

	// Matrix to transform points in our camera space to the world.

	// texture shader
	texture_shader texture_shader_;

	float rotate_h;
	float rotate_v;

	int mouse_x;
	int mouse_y;
	int mouse_wheel;

	lsystem l;

	camera_control cc;

	bool is_left_button_down;

	public:

	// this is called when we construct the class
	LSystem_building_app(int argc, char **argv) : app(argc, argv),
	key_cool_down(0),
	rotate_h(0),
	rotate_v(0),
	mouse_x(0),
	mouse_y(0),
	mouse_wheel(0),
	is_left_button_down(false),
	l(texture_shader_)
	{
	}

	// this is called once OpenGL is initialized
	void app_init() {

		l.load("..\\..\\assets\\1.txt");
		// initialize the shader
		texture_shader_.init();

		// set camera control info
		cc.set_view_distance(20);
		cc.set_view_position(vec3(0.f, l.get_building_height() * .5f, 0.f));

		// put the camera a short distance from the center, looking towards the triangle
		///*
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		glDisable(GL_CULL_FACE);
		mouse_wheel = get_mouse_wheel();
		//*/
	}

	// this is called to draw the world
	void draw_world(int x, int y, int w, int h) {
		unsigned int current_time = GetTickCount();
		// set a viewport - includes whole window area
		glViewport(x, y, w, h);

		// clear the background to black
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// allow Z buffer depth testing (closer objects are always drawn in front of far ones)
		glEnable(GL_DEPTH_TEST);

		vec4 color(0, 1, 0, 1);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


		// attribute_pos (=0) is position of each corner
		// each corner has 3 floats (x, y, z)
		// there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
		l.render(cc.get_matrix());
		if(is_key_down(key_lmb) && !is_left_button_down)
		{
			is_left_button_down = true;
			get_mouse_pos(mouse_x, mouse_y);
			SetCapture(get_hwnd());
		}
		else if(is_left_button_down && !is_key_down(key_lmb))
		{
			is_left_button_down = false;
			ReleaseCapture();
		}
		int mouse_wheel_delta = get_mouse_wheel() - mouse_wheel;
		if(mouse_wheel_delta != 0)
		{
			static float factor = 1.f;
			cc.add_view_distance(mouse_wheel_delta / WHEEL_DELTA * factor);
			mouse_wheel = get_mouse_wheel();
		}
		if(is_left_button_down)
		{
			static float factor = .2f;
			is_left_button_down = true;
			int x, y;
			get_mouse_pos(x, y);
			short sx = x, sy = y;
			int delta_x = mouse_x - sx, delta_y = mouse_y - sy;
			if(delta_x != 0)
				cc.rotate_h((float)delta_x * factor);
			if(delta_y != 0)
				cc.rotate_v((float)delta_y * factor);
			mouse_x = sx;
			mouse_y = sy;
		}
		static float key_rotation_delta = 1.f;
		if(is_key_down('W'))
		{
			cc.rotate_v(key_rotation_delta);
		}
		if(is_key_down('S'))
		{
			cc.rotate_v(-key_rotation_delta);
		}
		if(is_key_down('A'))
		{
			cc.rotate_h(key_rotation_delta);
		}
		if(is_key_down('D'))
		{
			cc.rotate_h(-key_rotation_delta);
		}
		static float key_translation_delta = .2f;
		if(is_key_down('Q'))
		{
			cc.add_view_distance(-key_translation_delta);
		}
		if(is_key_down('E'))
		{
			cc.add_view_distance(key_translation_delta);
		}
		if(current_time - key_cool_down > 100)
		{
			char buf[256];
			for(int i = 0; i < 10; i++)
			{
				char c = '0' + i;
				if(is_key_down(c))
				{
					sprintf(buf, "..\\..\\assets\\%c.txt", c);
					l.load(buf);
					key_cool_down = current_time;
				}
			}
		}
	}
};
}
