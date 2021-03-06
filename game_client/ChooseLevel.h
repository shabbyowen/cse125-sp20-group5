#pragma once
#include "core.h"
#include "stb_image.h"
#include <nanogui/button.h>
#include <nanogui/window.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/textbox.h>
#include <nanogui/ImageView.h>

class ChooseLevel
{
public:
	ChooseLevel(GLFWwindow* window, nanogui::Screen* screen, uint numLevels);
	int getLevel();
	void removeWindow();
	void setPlayers(uint numPlayers);
	void readInImages(GLuint* imageID, const char* texturefile);

private:
	uint numPlayers;

	std::vector<nanogui::Button *> buttons;
	GLuint * images;
	nanogui::Screen * screen;
	nanogui::Window * wind;
	nanogui::TextBox * text;
};

