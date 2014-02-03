namespace octet {
class LSystem_building_app : public app {
	// Matrix to transform points on our triangle to the world space
	// This allows us to move and rotate our triangle
	mat4t modelToWorld;

	unsigned int key_cool_down;
	unsigned int current_time;
	// Matrix to transform points in our camera space to the world.

	// texture shader
	texture_shader texture_shader_;

	dynarray <GLuint> wall_tex_list;
	dynarray <GLuint> floor_tex_list;
	dynarray <GLuint> frame_tex_list;
	dynarray <GLuint> balcony_tex_list;

	float rotate_h;
	float rotate_v;

	int mouse_x;
	int mouse_y;
	int mouse_wheel;
	int iterations;
	float angle;
	float view_distance_delta;

	enum
	{
		BUILDING_COUNT = 9		
	};

	enum
	{
		BUILDING_INDEX_TO_MODIFY = 4
	};

	lsystem l[BUILDING_COUNT];

	font_helper *font;

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
	view_distance_delta(.8f)
	{
	}

	void reload()
	{
		const float DELTA = 60.f;
		vec3 initial_pos(-DELTA, 0.f, -DELTA);
		for(int i = 0; i < BUILDING_COUNT; i++)
		{
			char buf[256];
			sprintf(buf, "..\\..\\assets\\%d.txt", i);
			l[i].load_from_file(buf, &texture_shader_);
			l[i].set_world_position(initial_pos + vec3(i % 3 * DELTA, 0.f, i / 3 * DELTA));
		}
	}

	// this is called once OpenGL is initialized
	void app_init() {
		font = font_helper::get_instance();
		font->set_font(15,0,0,0,FW_BOLD,0,0,0,GB2312_CHARSET,OUT_STROKE_PRECIS,CLIP_STROKE_PRECIS,DRAFT_QUALITY,VARIABLE_PITCH+FF_MODERN,L"Arial", hdc);

		reload();
		// initialize the shader
		texture_shader_.init();
		loadTextures();
		// set camera control info
		cc.set_view_distance(40);
		for(int i = 0; i < BUILDING_COUNT; i++)
		{
			if(l[i].is_loaded())
			{
				cc.set_view_position(vec3(0.f, l[i].get_building_height() * .5f, 0.f));
				break;
			}
		}

		// put the camera a short distance from the center, looking towards the triangle
		///*
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		glDisable(GL_CULL_FACE);
		mouse_wheel = get_mouse_wheel();
		//*/
	}

	/**********************************************************************
	  Method that reads the keyboard and mouse inputs		
	 ***********************************************************************/
	void hotkeys()
	{
		if(is_key_down('B'))
		{
			reload();
			key_cool_down = current_time;
		}
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
		if(is_key_down('K'))
		{
			if(is_key_down(key_shift))
			{
				view_distance_delta -= .1f;
				if(view_distance_delta < 0)
					view_distance_delta = 0;
			}
			else
			{
				view_distance_delta += .1f;
			}
		}
		if(is_key_down('Q'))
		{
			cc.add_view_distance(-view_distance_delta);
		}
		if(is_key_down('E'))
		{
			cc.add_view_distance(view_distance_delta);
		}
		//timer count for keyboard 
		if (current_time - key_cool_down > 100)
		{
			//control iterations 
			if (is_key_down(key_up))//increase
			{
				if(l[BUILDING_INDEX_TO_MODIFY].get_iteration()<5)//do not make more than 3 iterations -> run out of memory
					l[BUILDING_INDEX_TO_MODIFY].increase_iteration();

				key_cool_down = current_time;
			} 
			if (is_key_down(key_down)) //decrease
			{
				l[BUILDING_INDEX_TO_MODIFY].decrease_iteration();
				key_cool_down = current_time;
			}
			//control building angle
			if(is_key_down(key_right))//increase
			{
				float angle_b = 5.0f;
				l[BUILDING_INDEX_TO_MODIFY].adjust_angle(angle_b);
				key_cool_down = current_time;
			}
			if(is_key_down(key_left))//decrease
			{
				float angle_b = -5.0f;
				l[BUILDING_INDEX_TO_MODIFY].adjust_angle(angle_b);
				key_cool_down = current_time;
			}
			//control branch length
			if (is_key_down('P'))//increase
			{
				if (!is_key_down(key_shift))
				{
					float branch_length = 0.5f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_inital_branch_length(branch_length);
					key_cool_down = current_time;
				}
				else//decrease
				{
					float branch_length = -0.5f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_inital_branch_length(branch_length);
					key_cool_down = current_time;
				}
			} 				
			//control wall height
			if(is_key_down('H'))//increase
			{
				if(!is_key_down(key_shift))
				{
					float wall_height = 0.5f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_wall_height(wall_height);
					key_cool_down = current_time;
				}
				else//decrease
				{
					float wall_height = -0.5f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_wall_height(wall_height);
					key_cool_down = current_time;
				}	
			}

			//control window size
			if(is_key_down('M'))//increase
			{
				if(!is_key_down(key_shift))//decrease
				{
					float winS = 0.1f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_winSize(winS);
					key_cool_down = current_time;
				}
				else//decrease
				{
					float winS = -0.1f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_winSize(winS);
					key_cool_down = current_time;
				}	
			}

			//control floor count
			if(is_key_down('L'))//increase
			{
				if(!is_key_down(key_shift))
				{
					l[BUILDING_INDEX_TO_MODIFY].increase_floor_count();
					key_cool_down = current_time;
				}
				else//decrease
				{
					l[BUILDING_INDEX_TO_MODIFY].decrease_floor_count();
					key_cool_down = current_time;
				}
			}

			//extension_length
			if(is_key_down('X'))//increase
			{
				if(!is_key_down(key_shift))
				{
					float ext = 0.05f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_extension_length(ext);
					key_cool_down = current_time;
				}
				else//decrease
				{
					float ext = -0.05f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_extension_length(ext);
					key_cool_down = current_time;
				}
			}

			//floor_board_thickness
			if(is_key_down('I'))//increase
			{
				if(!is_key_down(key_shift))
				{
					float b_th = 0.05f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_floor_board_thickness(b_th);
					key_cool_down = current_time;
				}
				else//decrease
				{
					float b_th = -0.05f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_floor_board_thickness(b_th);
					key_cool_down = current_time;
				}
			}
			//balcony_extention
			if(is_key_down('O'))//increase
			{
				if(!is_key_down(key_shift))
				{
					float b = 0.05f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_balcony_extention(b);
					key_cool_down = current_time;
				}
				else//decrease
				{
					float b = -0.05f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_balcony_extention(b);
					key_cool_down = current_time;
				}
			}
			//balcony_heigh
			if(is_key_down('J'))//increase
			{
				if(!is_key_down(key_shift))
				{
					float b = 0.05f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_balcony_height(b);
					key_cool_down = current_time;
				}
				else//decrease
				{
					float b = -0.05f;
					l[BUILDING_INDEX_TO_MODIFY].adjust_balcony_height(b);
					key_cool_down = current_time;
				}
			}

			//next texture
			if(is_key_down('T'))
			{
				l[BUILDING_INDEX_TO_MODIFY].next_texture();
				key_cool_down = current_time;
			}

			//random building
			if(is_key_down('R'))
			{
				l[BUILDING_INDEX_TO_MODIFY].randomize();
				key_cool_down = current_time;
			}
		}
	}

	//set texture
	void loadTextures()
	{
		//texture_num = 1
		wall_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/brick_wall.gif"));//"!bricks");
		frame_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/frame.gif"));//"#FFFFFFFF");
		balcony_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/balcony.gif"));
		floor_tex_list.push_back(resources::get_texture_handle(GL_RGBA, "assets/floor.gif"));
		//texture_num = 2
		wall_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/metal_base.gif"));
		frame_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/frame.gif"));
		balcony_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/balcony.gif"));
		floor_tex_list.push_back(resources::get_texture_handle(GL_RGBA, "assets/parquet_floor.gif"));
		//texture_num = 3
		wall_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/brick_wall1.gif"));
		frame_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/frame.gif"));
		balcony_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/balcony.gif"));
		floor_tex_list.push_back(resources::get_texture_handle(GL_RGBA, "assets/floor1.gif"));
		//texture_num = 4
		wall_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/floor_slab.gif"));
		frame_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/frame.gif"));
		balcony_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/balcony.gif"));
		floor_tex_list.push_back(resources::get_texture_handle(GL_RGBA, "assets/fl_texture.gif"));
		//texture_num = 5
		wall_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/stone_texture.gif"));
		frame_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/frame.gif"));
		balcony_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/balcony.gif"));
		floor_tex_list.push_back(resources::get_texture_handle(GL_RGBA, "assets/floor1.gif"));
		//texture_num = 6
		wall_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/wood_wall.gif"));
		frame_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/frame.gif"));
		balcony_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/balcony.gif"));
		floor_tex_list.push_back(resources::get_texture_handle(GL_RGBA, "assets/floor_slab.gif"));
		//texture_num = 7
		wall_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/metal_wall.gif"));
		frame_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/frame.gif"));
		balcony_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/balcony.gif"));
		floor_tex_list.push_back(resources::get_texture_handle(GL_RGBA, "assets/fl_texture.gif"));
		//texture_num = 8
		wall_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/metallic.gif"));
		frame_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/frame.gif"));
		balcony_tex_list.push_back(resources::get_texture_handle(GL_RGB, "assets/balcony.gif"));
		floor_tex_list.push_back(resources::get_texture_handle(GL_RGBA, "assets/metal_base.gif"));
	}

	// this is called to draw the world
	void draw_world(int x, int y, int w, int h) {
		current_time = GetTickCount();
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
		for(int i = 0; i < BUILDING_COUNT; i++)
		{
			if(l[i].is_loaded())
			{	
				int index = l[i].get_texture_index();
				l[i].render(cc.get_matrix(), wall_tex_list[index-1],floor_tex_list[index-1], frame_tex_list[index-1], balcony_tex_list[index-1]);
			}
		}
		hotkeys();
		if(current_time - key_cool_down > 100)
		{
			/*
			char buf[256];
			for(int i = 0; i < 10; i++)
			{
				char c = '0' + i;
				if(is_key_down(c))
				{
					sprintf(buf, "..\\..\\assets\\%c.txt", c);
					l.load(buf, &texture_shader_);
					key_cool_down = current_time;
				}
			}
			*/
		}
		const int DELTA = 20;
		static int w1 = w - 320, h1 = h - 500, w2 = w1 + 215;
		int i = 0;
		font->draw_string(.0f, 1.f, 1.f, 0, 720, w, h, l[BUILDING_INDEX_TO_MODIFY].get_string().c_str());
		font->draw_string(.0f, 1.f, 1.f, w1 + 225, h1 - i, w, h, "hotkey");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "iteration: %d", l[BUILDING_INDEX_TO_MODIFY].get_iteration());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(down/up)");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "angle: %.3f", l[BUILDING_INDEX_TO_MODIFY].get_angle());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(left/right)");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "floor_count: %d", l[BUILDING_INDEX_TO_MODIFY].get_floor_count());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(l/L)");		
		i += DELTA;		
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "wall_height: %.3f", l[BUILDING_INDEX_TO_MODIFY].get_wall_height());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(h/H)");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "wall_length: %.3f", l[BUILDING_INDEX_TO_MODIFY].get_inital_branch_length());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(p/P)");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "balcony_height: %.3f", l[BUILDING_INDEX_TO_MODIFY].get_balcony_height());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(j/J)");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "balcony_extention: %.3f", l[BUILDING_INDEX_TO_MODIFY].get_balcony_extension());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(o/O)");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "floor_thickness: %.3f", l[BUILDING_INDEX_TO_MODIFY].get_floor_board_thickness());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(i/I)");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "extension_length: %.3f", l[BUILDING_INDEX_TO_MODIFY].get_extension_length());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(x/X)");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "texture: %d", l[BUILDING_INDEX_TO_MODIFY].get_texture_index());
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(t)");
		i += DELTA;
		font->draw_string(.0f, 1.f, 1.f, w1, h1 - i, w, h, "randomize");
		font->draw_string(.0f, 1.f, 1.f, w2, h1 - i, w, h, "(r)");
	}
};
}
