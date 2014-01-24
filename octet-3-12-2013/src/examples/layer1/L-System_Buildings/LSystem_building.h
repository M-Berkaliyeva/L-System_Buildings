namespace octet 
{
	const float PI = 3.1415926535898f;
	const float DEG2RAD = PI / 180;
	class lsystem
	{
		struct state
		{
			state() : pos(0, 0, 0),
			u(0)
			{
				m.loadIdentity();
				length = 0;
			}

			state(float l) : pos(0, 0, 0),
				length(l),
				u(0)
			{
				m.loadIdentity(); 
			}

			vec3 pos;
			float length;
			float u;
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
		dynarray<vec3> frame_mesh;
		dynarray<vec2> frame_uvs;
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
		GLuint frame_tex;
		float angle;
		float branch_length;
		float branch_length_decrement;
		int iteration;
		int floor_count;
		float winSize;
		float frameSize;
		vec3 vector;
		bool is_stochastic;
		

	public:
		lsystem() : initial_pos(0, 0, 0),
		floor_count(2)
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
			winSize = 1.2f;
			frameSize = 0.1f;
			vector = vec3(1, 0, 0);
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
			frame_tex = resources::get_texture_handle(GL_RGB, "#FFFFFFFF");
			floor_tex = resources::get_texture_handle(GL_RGB, "#7f7f7fff");
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
					else if(str == "floor count")
					{
						getline(f, str);
						sscanf(str.c_str(), "%d", &floor_count);
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
			circular_list<vec3> polygon_list;
			
			int size = polygon.size();
			for(int i = 0; i < size; i++)
			{
				polygon_list.push_back(polygon[i]);
			}
			polygon.reset();
			while(size != 3)
			{
				circular_list<vec3>::iterator iter = polygon_list.begin();
				circular_list<vec3>::iterator max_iter = polygon_list.begin();
				for(int i = 0; i < polygon_list.size(); i++)
				{
					if(max_iter->z() < iter->z())
						max_iter = iter;
					++iter;
				}
				polygon.push_back(*max_iter);
				circular_list<vec3>::iterator pre_iter = max_iter;
				circular_list<vec3>::iterator next_iter = max_iter;
				polygon.push_back(*--pre_iter);
				polygon.push_back(*++next_iter);
				polygon_list.remove(max_iter);
				size--;
			}
			circular_list<vec3>::iterator iter = polygon_list.begin();
			polygon.push_back(*iter);
			polygon.push_back(*++iter);
			polygon.push_back(*++iter);
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

			glBindTexture(GL_TEXTURE_2D, frame_tex);
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)&frame_mesh[0]);
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)&frame_uvs[0]);
			glDrawArrays(GL_QUADS, 0, frame_mesh.size());
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
					cutWindow();
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
					current_state.m = m_rotate_y_positive;
					break;
				case '-':
					current_state.m = m_rotate_y_negative;
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
			vec3 vector_h(1, 0, 0), vector_v(0, 1, 0), vector_2v(0.f, 2 * vector_v.y(), 0.f);
			vec3 p0 = current_state.pos - vector_v;
			vec3 p1 = current_state.pos + vector_v;
			vec3 p2 = initial_pos + vector_v;
			vec3 p3 = initial_pos - vector_v;
			float target_u = current_state.u + (initial_pos - current_state.pos).length();
			vec2 uv(current_state.u, 0), uv1(current_state.u, 2), uv2(target_u, 2), uv3(target_u, 0);
			for(int i = 0; i < floor_count; i++)
			{
				mesh.push_back(p0);
				mesh.push_back(p1);
				mesh.push_back(p2);
				mesh.push_back(p3);
				uvs.push_back(uv);
				uvs.push_back(uv1);
				uvs.push_back(uv2);
				uvs.push_back(uv3);
				p0 += vector_2v;
				p1 += vector_2v;
				p2 += vector_2v;
				p3 += vector_2v;
			}
		}

		// branch producing operation denoted by 'F'
		void extend()
		{
			vec3 vector_h, vector_v(0, 1, 0), vector_2v(0.f, 2 * vector_v.y(), 0.f), vector_floor_pos(current_state.pos);
			vec3 p0 = current_state.pos - vector_v;
			vec3 p1 = current_state.pos + vector_v;			
			vector = vector * current_state.m;
			vector_h = vector * current_state.length;
			vec3 p2 = p1 + vector_h;
			vec3 p3 = p0 + vector_h;
			float target_u = current_state.u + current_state.length;
			vec2 uv(current_state.u, 0), uv1(current_state.u, 2), uv2(target_u, 2), uv3(target_u, 0);
			for(int i = 0; i < floor_count; i++)
			{
				mesh.push_back(p0);
				mesh.push_back(p1);
				mesh.push_back(p2);
				mesh.push_back(p3);
				uvs.push_back(uv);
				uvs.push_back(uv1);
				uvs.push_back(uv2);
				uvs.push_back(uv3);
				vector_floor_pos += vector_v;
				/*
				floor_mesh.push_back(vector_floor_pos);
				floor_uvs.push_back(vec2(current_state.pos.x(), current_state.pos.z()));
				//*/
				p0 += vector_2v;
				p1 += vector_2v;
				p2 += vector_2v;
				p3 += vector_2v;
			}
			current_state.u = target_u;
			current_state.pos += vector_h;
			current_state.length -= branch_length_decrement;

		}

		void cutWindow()
		{
			int size = mesh.size();
			int uvSize = uvs.size();
			float halfWidth = winSize/2;
			//store last 4 points from floor_mesh as current wall 
			vec3 p0 = mesh[size-4];
			vec3 p1 = mesh[size-3];
			vec3 p2 = mesh[size-2];
			vec3 p3 = mesh[size-1];

			//find midpoint on the left edge of the wall
			float t = 0.5f;			
			vec3 left_midpoint = (1-t)*p0 + t*p1;
			//find distance to the middle of the wall
			float dist = branch_length/2;
			
			//find vector after rotation
			vec3 vector_h = vector * halfWidth;
			vec3 vector_v(.0f, halfWidth, .0f);
			
			//find  midpoint after rotation
			vec3 midpoint = left_midpoint + dist * vector;

			//find four points of the window
			vec3 wp0 = midpoint - vector_h - vector_v;//bottom left
			vector_v = vector_v * 2;
			vector_h = vector_h * 2;
			vec3 wp1 = wp0 + vector_v;//top left
			vec3 wp2 = wp1 + vector_h;//top right
			vec3 wp3 = wp0 + vector_h;//bottom right

			//find relative uv coordinates
			vec2 uv_p0(0, 0);
			vec2 uv_p1(0, 1);
			vec2 uv_p2(1, 1);
			vec2 uv_p3(1, 0);

			float denom_u = (p3.x() - p0.x());
			float denom_v = (p1.y() - p0.y());

			float u_wp0 = (wp0.x() - p0.x())/denom_u;
			float v_wp0 = (wp0.y() - p0.y())/denom_v;
			float u_wp1 = (wp1.x() - p0.x())/denom_u;
			float v_wp1 = (wp1.y() - p0.y())/denom_v;
			float u_wp2 = (wp2.x() - p0.x())/denom_u;
			float v_wp2 = (wp2.y() - p0.y())/denom_v;
			float u_wp3 = (wp3.x() - p0.x())/denom_u;
			float v_wp3 = (wp3.y() - p0.y())/denom_v;


			float uv_winSize = winSize /(p3.x() - p0.x());

			vec2 uv_wp0(u_wp0, v_wp0);
			vec2 uv_wp1(u_wp1, v_wp1);//u_wp0, v_wp0 + winSize);
			vec2 uv_wp2(u_wp2, v_wp2);//u_wp0 + winSize, v_wp0 + winSize);
			vec2 uv_wp3(u_wp3, v_wp3);//u_wp0 + winSize, v_wp0);

			////remove 4 old mesh points from the dynarray
			//mesh.erase(size-1);
			//mesh.erase(size-2);
			//mesh.erase(size-3);
			//mesh.erase(size-4);

			////remove 4 od uv coords
			//uvs.erase(uvSize-1);
			//uvs.erase(uvSize-2);
			//uvs.erase(uvSize-3);
			//uvs.erase(uvSize-4);
			
			//add 4 new quads to the dynarray
			//instead of erazing -> rewrite last four elements of mesh and uvs

			//mesh 1: p0, p1, wp1, wp0
			mesh[size-4] = p0;//mesh.push_back(p0);
			mesh[size-3] = p1;//mesh.push_back(p1);
			mesh[size-2] = wp1;//mesh.push_back(wp1);
			mesh[size-1] = wp0;//mesh.push_back(wp0);
			//mesh 1: uvs	
			uvs[uvSize-4] = uv_p0;//uvs.push_back(uv_p0);
			uvs[uvSize-3] = uv_p1;//uvs.push_back(uv_p1);
			uvs[uvSize-2] = uv_wp1;//uvs.push_back(uv_wp1);
			uvs[uvSize-1] = uv_wp0;//uvs.push_back(uv_wp0);

			//mesh 2: wp1, p1, p2, wp2
			mesh.push_back(wp1);
			mesh.push_back(p1);
			mesh.push_back(p2);
			mesh.push_back(wp2);
			//mesh 2: uvs
			uvs.push_back(uv_wp1);
			uvs.push_back(uv_p1);
			uvs.push_back(uv_p2);
			uvs.push_back(uv_wp2);

			//mesh 3: wp3, wp2, p2, p3
			mesh.push_back(wp3);
			mesh.push_back(wp2);
			mesh.push_back(p2);
			mesh.push_back(p3);
			//mesh 3: uvs
			uvs.push_back(uv_wp3);
			uvs.push_back(uv_wp2);
			uvs.push_back(uv_p2);
			uvs.push_back(uv_p3);

			//mesh 4: p0, wp0, wp3, p3
			mesh.push_back(p0);
			mesh.push_back(wp0);
			mesh.push_back(wp3);
			mesh.push_back(p3);
			//mesh 4: uvs
			uvs.push_back(uv_p0);
			uvs.push_back(uv_wp0);
			uvs.push_back(uv_wp3);
			uvs.push_back(uv_p3);

			makeWindowFrame(wp0, wp1, wp2, wp3); 
		}

		void makeWindowFrame(vec3 &wp0, vec3 &wp1, vec3 &wp2, vec3 &wp3)
		{
			float halfFrameSize = frameSize/2;
			float outerSize = winSize + frameSize;
			float innerSize = winSize - frameSize;

			//find vectors to get fist 4 frame points of wp0
			vec3 vector_x = vector * halfFrameSize;//(halfFrameSize, .0f, .0f);
			vec3 vector_y(.0f, halfFrameSize, .0f);			
			vec3 vector_z(.0f, .0f, halfFrameSize);

			//find vectors to get the rest of 3x4 frame point of other 3 window points
			vec3 inner_y(.0f, innerSize, .0f), outer_y(.0f, outerSize, .0f), inner_x, outer_x;
			inner_x = vector * innerSize;
			outer_x = vector * outerSize;

			//find 16 points of the window frame
			//wp0: wp0_fp0, wp0_fp1, wp0_fp2, wp0_fp3
			vec3 wp0_fp0 = wp0 + vector_y + vector_z;
			vector_y *= 2;
			vector_z *= 2;
			vec3 wp0_fp1 = wp0_fp0 - vector_y;
			vec3 wp0_fp2 = wp0_fp1 - vector_z;
			vec3 wp0_fp3 = wp0_fp0 - vector_z;

			wp0_fp0 += vector_x;
			wp0_fp1 -= vector_x;
			wp0_fp2 -= vector_x;
			wp0_fp3 += vector_x;

			//wp1: wp1_fp0, wp1_fp1, wp1_fp2, wp1_fp3		

			vec3 wp1_fp0 = wp0_fp0 + inner_y;
			vec3 wp1_fp1 = wp0_fp1 + outer_y;
			vec3 wp1_fp2 = wp0_fp2 + outer_y;
			vec3 wp1_fp3 = wp0_fp3 + inner_y;

			//wp2: wp2_fp0, wp2_fp1, wp2_fp2, wp2_fp3
			vec3 wp2_fp0 = wp1_fp0 + inner_x; 
			vec3 wp2_fp1 = wp1_fp1 + outer_x;
			vec3 wp2_fp2 = wp1_fp2 + outer_x;
			vec3 wp2_fp3 = wp1_fp3 + inner_x;

			//wp3: wp3_fp0, wp3_fp1, wp3_fp2, wp3_fp3
			vec3 wp3_fp0 = wp0_fp0 + inner_x;
			vec3 wp3_fp1 = wp0_fp1 + outer_x;
			vec3 wp3_fp2 = wp0_fp2 + outer_x;
			vec3 wp3_fp3 = wp0_fp3 + inner_x;


			//create 16 frame meshes (16 because other 4 are not visible)

			//edge1: left
			//mesh_front: 
			frame_mesh.push_back(wp0_fp1);
			frame_mesh.push_back(wp1_fp1);
			frame_mesh.push_back(wp1_fp0);
			frame_mesh.push_back(wp0_fp0);
			//uvs
			vec2 uv(0, 0), uv1(0, 1);
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_outer:
			frame_mesh.push_back(wp0_fp2);
			frame_mesh.push_back(wp1_fp2);
			frame_mesh.push_back(wp1_fp1);
			frame_mesh.push_back(wp0_fp1);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_back:
			frame_mesh.push_back(wp0_fp3);
			frame_mesh.push_back(wp1_fp3);
			frame_mesh.push_back(wp1_fp2);
			frame_mesh.push_back(wp0_fp2);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_inner:
			frame_mesh.push_back(wp0_fp0);
			frame_mesh.push_back(wp1_fp0);
			frame_mesh.push_back(wp1_fp3);
			frame_mesh.push_back(wp0_fp3);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//edge2: top
			//mesh_front: 
			frame_mesh.push_back(wp1_fp0);
			frame_mesh.push_back(wp1_fp1);
			frame_mesh.push_back(wp2_fp1);
			frame_mesh.push_back(wp2_fp0);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_outer:
			frame_mesh.push_back(wp1_fp1);
			frame_mesh.push_back(wp1_fp2);
			frame_mesh.push_back(wp2_fp2);
			frame_mesh.push_back(wp2_fp1);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_back:
			frame_mesh.push_back(wp1_fp2);
			frame_mesh.push_back(wp1_fp3);
			frame_mesh.push_back(wp2_fp3);
			frame_mesh.push_back(wp2_fp2);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_inner:
			frame_mesh.push_back(wp1_fp3);
			frame_mesh.push_back(wp1_fp0);
			frame_mesh.push_back(wp2_fp0);
			frame_mesh.push_back(wp2_fp3);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));


			//edge3: right
			//mesh_front: 
			frame_mesh.push_back(wp3_fp0);
			frame_mesh.push_back(wp2_fp0);
			frame_mesh.push_back(wp2_fp1);
			frame_mesh.push_back(wp3_fp1);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_outer:
			frame_mesh.push_back(wp3_fp1);
			frame_mesh.push_back(wp2_fp1);
			frame_mesh.push_back(wp2_fp2);
			frame_mesh.push_back(wp3_fp2);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_back:
			frame_mesh.push_back(wp3_fp2);
			frame_mesh.push_back(wp2_fp2);
			frame_mesh.push_back(wp2_fp3);
			frame_mesh.push_back(wp3_fp3);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_inner:
			frame_mesh.push_back(wp3_fp3);
			frame_mesh.push_back(wp2_fp3);
			frame_mesh.push_back(wp2_fp0);
			frame_mesh.push_back(wp3_fp0);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//edge4: bottom
			//mesh_front: 
			frame_mesh.push_back(wp0_fp1);
			frame_mesh.push_back(wp0_fp0);
			frame_mesh.push_back(wp3_fp0);
			frame_mesh.push_back(wp3_fp1);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_outer:
			frame_mesh.push_back(wp0_fp2);
			frame_mesh.push_back(wp0_fp1);
			frame_mesh.push_back(wp3_fp1);
			frame_mesh.push_back(wp3_fp2);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_back:
			frame_mesh.push_back(wp0_fp3);
			frame_mesh.push_back(wp0_fp2);
			frame_mesh.push_back(wp3_fp2);
			frame_mesh.push_back(wp3_fp3);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));

			//mesh_inner:
			frame_mesh.push_back(wp0_fp0);
			frame_mesh.push_back(wp0_fp3);
			frame_mesh.push_back(wp3_fp3);
			frame_mesh.push_back(wp3_fp0);
			//uvs
			frame_uvs.push_back(uv);
			frame_uvs.push_back(uv1);
			frame_uvs.push_back(uv1 + vec2(1, 0));
			frame_uvs.push_back(uv + vec2(1, 0));
		}
	};
}
