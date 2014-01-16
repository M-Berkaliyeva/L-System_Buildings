namespace octet 
{
	const float PI = 3.1415926535898f;
	const float DEG2RAD = PI / 180;
	class lsystem
	{
		struct state
		{
			state() : pos(0, 0, 0)
			{
				m.loadIdentity();
				length = 0;
			}

			state(float l) : pos(0, 0, 0),
				length(l)
			{
				m.loadIdentity(); 
			}

			vec3 pos;
			float length;
			mat4t m;
		};

		class stochastic_rule
		{
			char letter;
			dynarray<std::string> rules;
		public:

			stochastic_rule()
			{
			}

			stochastic_rule(const stochastic_rule &)
			{
			}

			char get_letter()
			{
				return letter;
			}

			void set_letter(char c)
			{
				letter = c;
			}

			void add_rule(std::string &str)
			{
				rules.push_back(str);
			}

			std::string &get_rule()
			{
				int a = rand() % rules.size();
				return rules[a];
			}
		};

		std::string axiom;
		std::string output_str;
		dynarray<vec3> mesh;
		dynarray<vec2> uvs;
		dynarray<vec3> floor_mesh;
		dynarray<vec2> floor_uvs;
		std::string letters;
		dynarray<stochastic_rule> rules;
		state current_state;
		dynarray<state> state_stack;
		mat4t m_rotate_x_positive;
		mat4t m_rotate_x_negative;
		mat4t m_rotate_y_positive;
		mat4t m_rotate_y_negative;
		mat4t m_rotate_z_positive;
		mat4t m_rotate_z_negative;
		mat4t m_rotate_x_180;
		mat4t m_rotate_y_180;
		mat4t m_rotate_z_180;
		vec3 initial_pos;
		GLuint wall_tex;
		GLuint floor_tex;
		float angle;
		float branch_length;
		float branch_length_decrement;
		int iteration;
		bool is_stochastic;
	public:
		lsystem() : initial_pos(0, 0, 0)
		{
			srand((DWORD)time(NULL));
			m_rotate_x_180.loadIdentity();
			m_rotate_x_180.rotateX(180);
			m_rotate_y_180.loadIdentity();
			m_rotate_y_180.rotateY(180);
			m_rotate_z_180.loadIdentity();
			m_rotate_z_180.rotateZ(180);
			branch_length_decrement = 0;
			is_stochastic = false;
		}

		//get the current iteration count
		int get_iteration()
		{
			return iteration;
		}

		//get the branch length decrement
		float get_branch_length_decrement()
		{
			return branch_length_decrement;
		}

		//get the inital branch 
		float get_inital_branch_length()
		{
			return branch_length;
		}

		//get the angle for each turn
		float get_angle()
		{
			return angle;
		}

		//set the angle for each turn, and update rotation matrix accordingly
		void set_angle(float angle)
		{
			this->angle = angle;
			m_rotate_x_positive.loadIdentity();
			m_rotate_x_positive.rotateZ(angle);
			m_rotate_x_negative.loadIdentity();
			m_rotate_x_negative.rotateZ(-angle);
			m_rotate_y_positive.loadIdentity();
			m_rotate_y_positive.rotateY(angle);
			m_rotate_y_negative.loadIdentity();
			m_rotate_y_negative.rotateY(-angle);
			m_rotate_z_positive.loadIdentity();
			m_rotate_z_positive.rotateX(angle);
			m_rotate_z_negative.loadIdentity();
			m_rotate_z_negative.rotateX(-angle);
		}

		//load rule configuration file from a given path
		void load(char *path) 	
		{
			wall_tex = resources::get_texture_handle(GL_RGB, "!bricks");
			floor_tex = resources::get_texture_handle(GL_RGB, "!bricks");
			std::ifstream f(path, std::ios::in);
			dynarray<std::string> strs;
			rules.reset();
			branch_length_decrement = 0;
			is_stochastic = false;
			if(f)
			{
				std::string str;
				while(!f.eof())
				{
					getline(f, str);
					if(f.eof())
						break;
					if(str == "angle")
					{
						getline(f, str);
						sscanf(str.c_str(), "%f", &angle);
						set_angle(angle);
					}
					else if(str == "branch length")
					{
						getline(f, str);
						sscanf(str.c_str(), "%f", &branch_length);
					}
					else if(str == "branch length decrement")
					{
						getline(f, str);
						sscanf(str.c_str(), "%f", &branch_length_decrement);
					}
					else if(str == "iteration")
					{
						getline(f, str);
						sscanf(str.c_str(), "%d", &iteration);
					}
					else if(str == "axiom")
					{
						getline(f, str);
						axiom = str;
					}
					else if(str == "letters")
					{
						getline(f, str);
						letters = str;
					}
					else if(str == "rules")
					{
						for(unsigned int j = 0; j < letters.size(); j++)
						{
							getline(f, str);
							unsigned int m = 0;
							for(; m < rules.size(); m++)
							{
								if(letters[j] == rules[m].get_letter())
								{
									is_stochastic = true;
									rules[m].add_rule(str);
									break;
								}
							}
							if(m == rules.size())
							{
								rules.resize(rules.size() + 1);
								rules[m].set_letter(letters[j]);
								rules[m].add_rule(str);
							}
							if(f.eof())
								break;
						}
					}
				}
			}
			generate_output_str();
			generate_mesh();
		}

		void ear_clipping_triangulation(dynarray<vec3> &polygon)
		{
			if(polygon.size() < 3)
				return;
			dynarray<vec3> temp_polygon;
			temp_polygon.resize(polygon.size());
			for(unsigned int i = 0; i < temp_polygon.size(); i++)
			{
				temp_polygon[i] = polygon[i];
			}
			polygon.reset();
			int index;
			while(temp_polygon.size() != 3)
			{
				index = 0;
				for(unsigned int i = 1; i < temp_polygon.size(); i++)
				{
					if(temp_polygon[index].z() < temp_polygon[i].z())
						index = i;
				}
				polygon.push_back(temp_polygon[index]);
				polygon.push_back(temp_polygon[(temp_polygon.size() + index - 1) % temp_polygon.size()]);
				polygon.push_back(temp_polygon[(index + 1) % temp_polygon.size()]);
				temp_polygon.erase(index);
			}
			polygon.push_back(temp_polygon[0]);
			polygon.push_back(temp_polygon[1]);
			polygon.push_back(temp_polygon[2]);
		}

		//render both branch mesh
		void render()
		{
			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_uv);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, wall_tex);
			
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)&mesh[0]);
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)&uvs[0]);
			glDrawArrays(GL_QUADS, 0, mesh.size());

			glBindTexture(GL_TEXTURE_2D, floor_tex);
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)&floor_mesh[0]);
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)&floor_uvs[0]);
			glDrawArrays(GL_TRIANGLES, 0, floor_mesh.size());
		}

		//generate the final string with current rule
		void generate_output_str()
		{
			std::string str;
			output_str = axiom;
			if(rules.size() == 0)
				return;
			for(int i = 0; i < iteration; i++)
			{
				str = "";
				for(unsigned int k = 0; k < output_str.size(); k++)
				{
					unsigned m = 0;
					for(; m < rules.size(); m++)
					{
						if(rules[m].get_letter() == output_str[k])
						{
							str += rules[m].get_rule();
							break;
						}

					}
					if(m == rules.size())
					{
						str += output_str[k];
					}
				}
				output_str = str;
			}
		}

		//generate one mesh for branchs and the other mesh for leaves according to the output string
		void generate_mesh()
		{
			current_state.state::state(branch_length);
			floor_mesh.reset();
			floor_mesh.push_back(vec3(0, 1, 0));
			state_stack.reset();
			mesh.reset();
			uvs.reset();
			mat4t m;
			for(unsigned int i = 0; i < output_str.size(); i++)
			{
				switch(output_str[i])
				{
				case 'K':
					break;
				case 'F':
					extend();
					break;
				case '[':
					push();
					break;
				case ']':
					pop();
					break;
				case '+':
					current_state.m = current_state.m * m_rotate_y_positive;
					break;
				case '-':
					current_state.m = current_state.m * m_rotate_y_negative;
					break;
				}
			}
			enclose();
			ear_clipping_triangulation(floor_mesh);
			floor_uvs.reset();
			floor_uvs.resize(floor_mesh.size());
			for(unsigned int i = 0; i < floor_mesh.size(); i++)
			{
				floor_uvs[i] = vec2(floor_mesh[i].x(), floor_mesh[i].z());
			}
		}

		//decrease iteration count
		void decrease_iteration()
		{
			if(iteration != 0)
			{
				iteration--;
			}
		}

		//increase iteration count
		void increase_iteration()
		{
			iteration++;
		}

		//adjust branch length decrement
		void adjust_branch_length_decrement(float f)
		{
			branch_length_decrement += f;
		}

		//adjust inital branch length
		void adjust_inital_branch_length(float f)
		{
			branch_length += f;
		}

		//adjust angle for each turn
		void adjust_angle(float f)
		{
			angle += f;
			set_angle(angle);
		}

		~lsystem()
		{
		}
	protected:

		//push operation denoted by '['
		void push()
		{
			state_stack.push_back(current_state);
		}

		//pop operation denoted by ']'
		void pop()
		{
			unsigned int last_item = state_stack.size();
			if(last_item != 0)
			{
				current_state = state_stack[last_item - 1];
				state_stack.pop_back();
			}
		}

		//enclose the wall
		void enclose()
		{
			vec3 vector_h(1, 0, 0), vector_v(0, 1, 0);
			vec3 p0 = current_state.pos - vector_v;
			vec3 p1 = current_state.pos + vector_v;
			vec3 p2 = initial_pos + vector_v;
			vec3 p3 = initial_pos - vector_v;
			mesh.push_back(p0);
			mesh.push_back(p1);
			mesh.push_back(p2);
			mesh.push_back(p3);
			vec2 uv(0, 0), uv1(0, 1);
			uvs.push_back(uv);
			uvs.push_back(uv1);
			uvs.push_back(uv1 + vec2(1, 0));
			uvs.push_back(uv + vec2(1, 0));
		}

		// branch producing operation denoted by 'F'
		void extend()
		{
			vec3 vector_h((float)current_state.length, 0.f, 0.f), vector_v(0, 1, 0);
			vec3 p0 = current_state.pos - vector_v;
			vec3 p1 = current_state.pos + vector_v;
			vector_h = vector_h * current_state.m;
			vec3 p2 = p1 + vector_h;
			vec3 p3 = p0 + vector_h;
			mesh.push_back(p0);
			mesh.push_back(p1);
			mesh.push_back(p2);
			mesh.push_back(p3);
			vec2 uv(0, 0), uv1(0, 2);
			uvs.push_back(uv);
			uvs.push_back(uv1);
			uvs.push_back(uv1 + vec2(2, 0));
			uvs.push_back(uv + vec2(2, 0));
			current_state.pos += vector_h;
			floor_mesh.push_back(current_state.pos + vector_v);
			floor_uvs.push_back(vec2(current_state.pos.x(), current_state.pos.z()));
			current_state.length -= branch_length_decrement;

		}
	};
}
