#include <string>
#include <vector>
#include <SDL.h>

namespace UI {
	class ScrollBar {
	public:
		ScrollBar(int width, int height, int min, int max, int value, bool vertical);
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
		Button(int x, int y, int width, int height, std::string text);

	private:

	};

	class RadioButton {
	public:
		RadioButton(int x, int y, int radius, std::vector<RadioButton*> radios, std::string text);

	private:


	};

	class Slider {
	public:
		Slider(int x, int y, int width, int height, int min, int max, int value);	};

}