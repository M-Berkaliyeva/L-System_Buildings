namespace octet {
	class font_helper
	{
		int lists;
		static font_helper *instance;
		font_helper() : lists(0)
		{
		}
	public:
		static font_helper *get_instance()
		{
			if(instance != NULL)
				return instance;
			instance = new font_helper();
			return instance;
		}


		~font_helper()
		{
			glDeleteLists(lists, 256);
		}

		void set_font(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, __in_opt LPCWSTR pszFaceName, HDC hdc)
		{
			HFONT font = CreateFont(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
			if(lists != 0)
				glDeleteLists(lists, 256);
			lists = glGenLists(256); 
			SelectObject(hdc,font);
			if(wglUseFontBitmaps(hdc, 0, 256, lists) == 0)
			{
				wglUseFontBitmaps(hdc, 0, 256, lists);
			}
			if(font != NULL)
			{
				DeleteObject(font);
			}
		}

		void draw_string(float r, float g, float b, int x, int y, int screen_width, int screen_height, const char* str, ...)
		{	
			va_list args;
			va_start(args, str);
			char buf[1024];
			vsprintf(buf, str, args);
			va_end(args);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, screen_width, 0, screen_height, 0, 1);
			glColor3f(r, g, b);
			glRasterPos2i(x, y);
			glListBase(lists);
			glUseProgram(0);
			glCallLists(strlen(buf), GL_UNSIGNED_BYTE, buf);
		}
	};
	font_helper* font_helper::instance = NULL;
}
