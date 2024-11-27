#include <string>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>


namespace UI {
	std::string getExecutableDirectory();

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
			Clear,
			Randomize,
			Confirm
		};

		Button(int x, int y, int width, int height, std::string text, ID id);
		void render(SDL_Renderer* renderer);
		bool isHovered(int mouse_x, int mouse_y);
		void setText(std::string text);
		void setColor(SDL_Color color);
		void setID(ID id);
		ID getID();
		void setLocked(bool locked);
		bool isLocked();

	private:
		void renderBody(SDL_Renderer* renderer);
		void renderText(SDL_Renderer* renderer);

		int x;
		int y;
		int width;
		int height;
		std::string text;
		SDL_Color color;
		SDL_Color font_color;
		ID id;
		bool locked = false;
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
		void renderBody(SDL_Renderer* renderer);
		void renderKnob(SDL_Renderer* renderer);

		int x;
		int y;
		int width;
		int height;
		int min;
		int max;
		int value;
		SDL_Color knob_color;
	};

	class NumericTextBox {
	public:
		enum class ID {
			Width,
			Height,
			Percent
		};

		NumericTextBox(int x, int y, int width, int height, int min, int max, int value, TTF_Font* font, std::string label, ID id);

		void render(SDL_Renderer* renderer);
		bool isHovered(int mouse_x, int mouse_y);
		bool isFocused();
		void setFocused(bool is_focused);
		int getValue();
		void setText(std::string text);
		void push_back(char ch);
		void pop_back();
		std::string getText();
		void setColor(SDL_Color color);
		ID getID();
		
	private:
		void renderLabel(SDL_Renderer* renderer);
		void renderBody(SDL_Renderer* renderer);
		void renderText(SDL_Renderer* renderer);

		int x;
		int y;
		int width;
		int height;
		int min;
		int max;
		int value;
		ID id;
		TTF_Font* font;
		std::string label;
		std::string text;
		bool is_focused;
		SDL_Color color;
	};
}