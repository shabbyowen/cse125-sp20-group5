////////////////////////////////////////
// Client.cpp
////////////////////////////////////////

#include "Client.h"
#include "NetworkClient.h"
#include "Message.hpp"
#include "ClientParams.h"


// static
Client * Client::CLIENT;
int Client::winX;
int Client::winY;

Client::Client(GLFWwindow * window, nanogui::Screen  *screen, int argc, char **argv, ClientParams& clientParams)
 : config(clientParams) {
	windowHandle = window;

	// nangui stuff
	this->screen = screen;
	this->startPage = new ChooseLobby(window, screen);

	// initalize mouse state
	leftDown=middleDown=rightDown=false;
	mouseX=mouseY=0;

	// initialize keyboard state
	setupKeyboardPresses();

	// Initialize components
	glfwGetWindowSize(windowHandle, &winX, &winY);
	cam=new Camera(config);
	cam->SetAspect(float(winX)/float(winY));

	scene = new Scene(config); 
	scene->setupDirectionalLighting(cam->GetPosition());

	setupAudio();
	scene->aEngine = &aEngine;

	state = ClientState::GETIP;

}

////////////////////////////////////////////////////////////////////////////////

Client::~Client() {
	delete cam;
	delete scene;
	delete netClient;

	aEngine.Shutdown();

	glFinish();
	glfwDestroyWindow(windowHandle);
}

////////////////////////////////////////////////////////////////////////////////

void Client::loop() {
	while (!glfwWindowShouldClose(windowHandle)) {
		//std::cout << "main looop" << std::endl;

		// recieve the state from the server

		// Update the components in the world
		// calculate matrices for rendering
		if (state == ClientState::PLAYING) {
			sendKeyboardEvents();
			currentGameState = netClient->getCurrentState();
		}

		update();

		// Tell redraw the scene
		draw();

		// process new input events
		glfwPollEvents();
	}
}

void Client::update() {
	if (state == ClientState::PLAYING) {

		cam->Update();
		//std::cout << currentGameState << std::endl;
		if (currentGameState != nullptr) {
			scene->setState(currentGameState);
			scene->update();
		}
		else {
			aEngine.StopSounds(AUDIO_FILE_BGM_LOBBY);
			if (!aEngine.IsPlaying(AUDIO_FILE_BGM)) {
				aEngine.PlaySounds(AUDIO_FILE_BGM, glm::vec3(0), aEngine.VolumeTodB(config.BGMVolume));
			}
		}
	}
	else if (state == ClientState::GETIP) {
		// if the enter button is pressed go off
		if (startPage->getButtonStatus()) {
			//std::cout << "asdfasdf " << startPage->getIpAddress() << std::endl;
			setupNetwork(startPage->getIpAddress());
			state = ClientState::CHOOSELEVEL;
			startPage->removeWindow();
			// TODO decide on the number of levels
			levelPage = new ChooseLevel(windowHandle, screen, 5);

		}
	}
	else if (state == ClientState::CHOOSELEVEL) {
		if (netClient->getGameStarted()) {
			levelPage->removeWindow();
			state = ClientState::PLAYING;
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
		}
		else {
			int level = levelPage->getLevel();
			// this means we've selected a level
			levelPage->setPlayers(netClient->playerIdCount);
			if (level != -1) {
				// TODO we should set a message to the server with the level we choose
				netClient->sendMessage(OPCODE_LEVEL_SELECT, level);
				std::cout << "level: " << level << std::endl;
				levelPage->removeWindow();
				state = ClientState::PLAYING;
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
			}
		}

	}
	// Maybe show a loading screen or something if gameState is nullptr (not yet received)?
}

////////////////////////////////////////////////////////////////////////////////

void Client::reset() {
	cam->Reset();
	cam->SetAspect(float(winX)/float(winY));

}

void Client::draw() {
	// Begin drawing scene
	glViewport(0, 0, winX, winY);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//screen->drawContents();
	//screen->drawWidgets();
	if (state == ClientState::PLAYING) {
		scene->draw(cam->GetViewProjectMtx());
	}
	else if (state == ClientState::GETIP || state == ClientState::CHOOSELEVEL) {
		screen->drawContents();
		screen->drawWidgets();
	}

	// Finish drawing scene
	glFinish();
	glfwSwapBuffers(windowHandle);
}

/* Ideally there would be a single networking object that we would pass our 
 events to and have it serializae and send them
 */
void Client::sendKeyboardEvents()
{
	if ((*keyPresses)[GLFW_KEY_W] && (*keyPresses)[GLFW_KEY_A]) {
		netClient->sendMessage(OPCODE_PLAYER_MOVE_UPPER_LEFT);
	}
	else if ((*keyPresses)[GLFW_KEY_W] && (*keyPresses)[GLFW_KEY_D]) {
		netClient->sendMessage(OPCODE_PLAYER_MOVE_UPPER_RIGHT);
	}
	else if ((*keyPresses)[GLFW_KEY_S] && (*keyPresses)[GLFW_KEY_A]) {
		netClient->sendMessage(OPCODE_PLAYER_MOVE_LOWER_LEFT);
	}
	else if ((*keyPresses)[GLFW_KEY_S] && (*keyPresses)[GLFW_KEY_D]) {
		netClient->sendMessage(OPCODE_PLAYER_MOVE_LOWER_RIGHT);
	}
	else if ((*keyPresses)[GLFW_KEY_W]) {
        netClient->sendMessage(OPCODE_PLAYER_MOVE_UP);
	}
	else if ((*keyPresses)[GLFW_KEY_A]) {
        netClient->sendMessage(OPCODE_PLAYER_MOVE_LEFT);
	}
	else if ((*keyPresses)[GLFW_KEY_S]) {
        netClient->sendMessage(OPCODE_PLAYER_MOVE_DOWN);
	}
	else if ((*keyPresses)[GLFW_KEY_D]) {
		netClient->sendMessage(OPCODE_PLAYER_MOVE_RIGHT);
	}
	else {
		netClient->sendMessage(OPCODE_PLAYER_MOVE_FREEZE);
	}

	if ((*keyPresses)[GLFW_KEY_E]) {
		netClient->sendMessage(OPCODE_PLAYER_START_ACTION);
	} else {
		netClient->sendMessage(OPCODE_PLAYER_END_ACTION);
	}

	if ((*keyPresses)[GLFW_KEY_0]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_0); }
	else if ((*keyPresses)[GLFW_KEY_1]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_1); }
	else if ((*keyPresses)[GLFW_KEY_2]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_2); }
	else if ((*keyPresses)[GLFW_KEY_3]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_3); }
	else if ((*keyPresses)[GLFW_KEY_4]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_4); }
	else if ((*keyPresses)[GLFW_KEY_5]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_5); }
	else if ((*keyPresses)[GLFW_KEY_6]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_6); }
	else if ((*keyPresses)[GLFW_KEY_7]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_7); }
	else if ((*keyPresses)[GLFW_KEY_8]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_8); }
	else if ((*keyPresses)[GLFW_KEY_9]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_9); }
	else if ((*keyPresses)[GLFW_KEY_P]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_SECRET); }
	else if ((*keyPresses)[GLFW_KEY_Z]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_ZHU); }
	else if ((*keyPresses)[GLFW_KEY_U]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_SUGUMAR); }
	else if ((*keyPresses)[GLFW_KEY_J]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_JOYAAN); }
	else if ((*keyPresses)[GLFW_KEY_M]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_MINGQI); }
	else if ((*keyPresses)[GLFW_KEY_Y]) { netClient->sendMessage(OPCODE_PLAYER_CHAT_YANG); }
}

////////////////////////////////////////////////////////////////////////////////

void Client::quit() {
	glFinish();
	glfwDestroyWindow(windowHandle);
}

////////////////////////////////////////////////////////////////////////////////

void Client::resize(GLFWwindow* window, int width, int height) {
	winX = width;
	winY = height;
	cam->SetAspect(float(winX)/float(winY));
}

////////////////////////////////////////////////////////////////////////////////

void Client::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key) {
		case GLFW_KEY_ESCAPE:		// Escape
			quit();
			break;
		case GLFW_KEY_R:
			reset();
			break;
		default:
			break;	
	}

	if (action == GLFW_PRESS && keyPresses->count((char)key) > 0) {
		(*keyPresses)[key] = true;
	}

	if (action == GLFW_RELEASE && keyPresses->count((char)key) > 0) {
		(*keyPresses)[key] = false;
		if (key == GLFW_KEY_SPACE) {
			netClient->sendMessage(OPCODE_PLAYER_INTERACT);
		}
	}

	
}

////////////////////////////////////////////////////////////////////////////////

void Client::mouseButton(GLFWwindow* window,int btn,int action,int mods) {
	if(btn== GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			leftDown = true;
		}
		else if (action == GLFW_RELEASE) {
			leftDown = false;
		}
	}
	else if(btn== GLFW_MOUSE_BUTTON_MIDDLE) {
		if (action == GLFW_PRESS) {
			middleDown = true;
		}
		else if (action == GLFW_RELEASE) {
			middleDown = false;
		}
	}
	else if(btn== GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			rightDown = true;
		}
		else if (action == GLFW_RELEASE) {
			rightDown = false;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void Client::mouseMotion(GLFWwindow* window, double nx, double ny) {
	int maxDelta=100;
	int dx = glm::clamp((int)nx - mouseX,-maxDelta,maxDelta);
	int dy = glm::clamp(-((int)ny - mouseY),-maxDelta,maxDelta);

	mouseX = nx;
	mouseY = ny;

	// Move camera
	// NOTE: this should really be part of Camera::Update()
	if(leftDown) {
		const float rate=1.0f;
		cam->SetAzimuth(cam->GetAzimuth()+dx*rate);
		cam->SetIncline(glm::clamp(cam->GetIncline()-dy*rate,-90.0f,90.0f));
	}
	if(rightDown) {
		const float rate=0.005f;
		float dist=glm::clamp(cam->GetDistance()*(1.0f-dx*rate),0.01f,1000.0f);
		cam->SetDistance(dist);
	}
}

////////////////////////////////////////////////////////////////////////////////

void Client::zoomScreen(GLFWwindow* window, double xoffset, double yoffset) {
	float lowerBound = 0.10f;
	float upperBound = 75.0f;
	if (cam->GetFOV() >= lowerBound && cam->GetFOV() <= upperBound)
		cam->SetFOV(cam->GetFOV() - yoffset);
	if (cam->GetFOV() <= lowerBound)
		cam->SetFOV(lowerBound);
	if (cam->GetFOV() >= upperBound)
		cam->SetFOV(upperBound);
}

////////////////////////////////////////////////////////////////////////////////

void Client::setupKeyboardPresses()
{
	keyPresses = new std::unordered_map<int, bool>;
	for (int c : keys) {
		(*keyPresses)[c] = false;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Client::setupAudio() {
	aEngine.Init();

	aEngine.LoadSound(AUDIO_FILE_BGM, false, true); 
	aEngine.LoadSound(AUDIO_FILE_BGM_LOBBY, false, true);
	aEngine.LoadSound(AUDIO_FILE_ZOMBIE_DAMAGED, true, false);
	aEngine.LoadSound(AUDIO_FILE_ZOMBIE_DIE, true, false);
	aEngine.LoadSound(AUDIO_FILE_PLAYER_PICK, true, false);
	aEngine.LoadSound(AUDIO_FILE_HOMEBASE_DAMAGED, true, false);
	aEngine.LoadSound(AUDIO_FILE_HOMEBASE_DIE, true, false);

	aEngine.LoadSound(AUDIO_FILE_PESTICIDE, true, false);
	aEngine.LoadSound(AUDIO_FILE_FERTILIZER, true, false);
	aEngine.LoadSound(AUDIO_FILE_WATERING, true, false);
	aEngine.LoadSound(AUDIO_FILE_WATER_REFILL, true, false);

	aEngine.PlaySounds(AUDIO_FILE_BGM_LOBBY, glm::vec3(0), aEngine.VolumeTodB(config.BGMVolume * 0.7f));

	glm::vec3 listenerPos = glm::normalize(
		glm::eulerAngleY(glm::radians(-cam->GetAzimuth())) 
		* glm::eulerAngleX(glm::radians(-cam->GetIncline()))
		* glm::vec4(0, 0, cam->GetDistance(), 1)
	);
	glm::vec3 listenerDir = glm::normalize(
		glm::eulerAngleY(glm::radians(-cam->GetAzimuth()))
		* glm::eulerAngleX(glm::radians(-cam->GetIncline()))
		* glm::vec4(0, 0, -cam->GetDistance(), 0)
	);
	aEngine.Set3dListenerAndOrientation(listenerPos, listenerDir, glm::vec3(0, 1, 0));
}

////////////////////////////////////////////////////////////////////////////////
void Client::setupNetwork(std::string ipAddress) {
	netClient = new NetworkClient(ipAddress.c_str(), "10032");
	netClient->start();
}

////////////////////////////////////////////////////////////////////////////////
