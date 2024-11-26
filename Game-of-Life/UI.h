#include <string>
#include <vector>
#include <SDL.h>



namespace UI {
	class ScrollBar {
	public:
		ScrollBar(int width, int height, int min, int max, int value, bool vertical);
		void render(SDL_Renderer* renderer);
		void setValue(int value);

	private:
		bool isInBounds(int value);

		int width;
		int height;
		int min;
		int max;
		int value;
		bool is_vertical;
	};

	class Button {
	public:
		enum class ID {
			Play,
			Stop,
			Next,
			Recenter,
			Load,
			Export,
			Help,
			Clear
		};

		Button(int x, int y, int width, int height, std::string text, ID id);
		void render(SDL_Renderer* renderer);
		bool isHovered(int mouse_x, int mouse_y);
		void changeText(std::string text);
		void onResize(int width, int height);
		void setColor(SDL_Color color);
		void setID(ID id);
		ID getID();


	private:
		int x;
		int y;
		int width;
		int height;
		std::string text;
		SDL_Color color;
		ID id;
	};

	class Slider {
	public:
		Slider(int x, int y, int width, int height, int min = 1, int max = 50, int value = 5);
		void render(SDL_Renderer* renderer);
		bool isKnobHovered(int mouse_x, int mouse_y);
		bool isBodyHovered(int mouse_x, int mouse_y);
		void setValue(int value);
		int getValue();
		void setKnobPosition(int mouse_x);
		SDL_Rect getKnobRect();
		void setKnobColor(SDL_Color color);
	private:
		int x;
		int y;
		int width;
		int height;
		int min;
		int max;
		int value;
		SDL_Color knob_color;
	};
}