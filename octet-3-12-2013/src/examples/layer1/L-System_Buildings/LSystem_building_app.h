namespace octet {
class LSystem_building_app : public app {
	// Matrix to transform points on our triangle to the world space
	// This allows us to move and rotate our triangle
	mat4t modelToWorld;

	unsigned int key_cool_down;

	// Matrix to transform points in our camera space to the world.
	// This lets us move our camera
	mat4t cameraToWorld;

	// texture shader
	texture_shader texture_shader_;

	lsystem l;

	public:

	// this is called when we construct the class
	LSystem_building_app(int argc, char **argv) : app(argc, argv),
	key_cool_down(0)
	{
	}

	// this is called once OpenGL is initialized
	void app_init() {

		l.load("..\\..\\assets\\1.txt");
		// initialize the shader
		texture_shader_.init();

		// put the triangle at the center of the world
		modelToWorld.loadIdentity();

		// put the camera a short distance from the center, looking towards the triangle
		cameraToWorld.loadIdentity();
		cameraToWorld.translate(0, 2.5, 5);
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

		// build a projection matrix: model -> world -> camera -> projection
		// the projection space is the cube -1 <= x/w, y/w, z/w <= 1
		mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
		//modelToWorld.rotateX(1);
		modelToWorld.rotateY(2);

		vec4 color(0, 1, 0, 1);
		texture_shader_.render(modelToProjection, 0);
		///*
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		//*/

		//glPolygonMode(GL_BACK, GL_LINE);


		// attribute_pos (=0) is position of each corner
		// each corner has 3 floats (x, y, z)
		// there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
		l.render();
		if(current_time - key_cool_down > 100)
		{
			char buf[256];
			for(int i = 0; i < 10; i++)
			{
				char c = '0' + i;
				if(is_key_down(c))
				{
					sprintf(buf, "%c.txt", c);
					l.load(buf);
					key_cool_down = current_time;
				}
			}
		}
	}
};
}
