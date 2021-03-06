#pragma once
#include "RenderController.hpp"
#include <Tool.hpp>
#include "Scene.h"
#include "HealthBar.h"
#include "TextUI.h"

#define SEED_BAG_SCALER 0.2
#define WATER_CAN_SCALER 0.35
#define SHOVEL_SCALER 0.35
#define SPRAY_SCALER 0.001
#define FERTILIZER_SCALER 0.05

class ToolController : public RenderController {
public:
	ToolController(Tool * tool, Scene * scene) {
		rootNode = new SceneNode(NULL, "ToolRootEmpty", tool->objectId);
		barNode = NULL;
		particleNode = NULL;
		pGroup = NULL;
		filledBar;
		type = tool->toolType;
		holderId = 0;

		switch (type) {
			case Tool::ToolType::WATER_CAN: {
				modelNode = scene->getModel(ModelType::WATERING_CAN)->createSceneNodes(tool->objectId);
				modelNode->scaler = WATER_CAN_SCALER;

				// init water filled bar 
				HealthBarSetting wBarSetting("texture/water_icon.png", CAN_BAR_TRANSLATE_Y, 1.0f, CAN_BAR_COLOR);
				std::tie(filledBar, barNode) = createHealthBar(wBarSetting, scene);
				break;
			}
			case Tool::ToolType::PLOW: {
				modelNode = scene->getModel(ModelType::SHOVEL)->createSceneNodes(tool->objectId);
				modelNode->scaler = SHOVEL_SCALER;
				break;
			}
			case Tool::ToolType::SEED: {
				modelNode = scene->getModel(ModelType::SEED_BAG)->createSceneNodes(tool->objectId);
				modelNode->scaler = SEED_BAG_SCALER;

				// init chat text
				std::tie(chatText, textNode) = createTextUI(
					FontType::CHUNK, CHAT_TEXT_COLOR,
					glm::translate(glm::vec3(0, CHAT_TEXT_TRANSLATE_Y, 0)), scene
				);
				chatText->shouldDisplay = false;
				chatText->setAlphaSetting(true, 0.0f, chatText->alphaStep);
				chatText->autoFadeOff = true;
				break;
			}
			case Tool::ToolType::PESTICIDE: {
				modelNode = scene->getModel(ModelType::SPRAY)->createSceneNodes(tool->objectId);
				modelNode->scaler = SPRAY_SCALER;
				break;
			}
			case Tool::ToolType::FERTILIZER: {
				modelNode = scene->getModel(ModelType::FERTILIZER)->createSceneNodes(tool->objectId);
				modelNode->scaler = FERTILIZER_SCALER;

				// init fertilizer cool down bar
				HealthBarSetting fBarSetting("texture/time_icon.png", CAN_BAR_TRANSLATE_Y, 1.0f, COOLDOWN_BAR_COLOR);
				std::tie(filledBar, barNode) = createHealthBar(fBarSetting, scene);
				filledBar->shouldDisplay = false;
				break;
			}
		}

		rootNode->addChild(modelNode);
		scene->getGroundNode()->addChild(rootNode);
	}

	~ToolController() {
		if (barNode) { barNode = deleteUiNode(barNode); }
		if (filledBar) delete filledBar;
		if (pGroup) delete pGroup;

		if (textNode) { textNode = deleteUiNode(textNode); }
		if (chatText) { delete chatText; }
	}

	void update(GameObject* gameObject, Scene* scene) override {
		Tool* tool = (Tool*)gameObject;

		// update transform on ground based on server if not held
		if (!tool->held) {
			rootNode->loadGameObject(tool);
			if (rootNode->parent != scene->getGroundNode()) {
				rootNode->scaler = 1.0;
				rootNode->pose = glm::vec3(0);
				scene->getGroundNode()->addChild(rootNode);
			}
		}

		updateBar(tool, scene);

		if (tool->toolType == Tool::ToolType::SEED && tool->seedType == Plant::PlantType::PLAYER) {
			updateChat(tool->playerPlant, chatText);
		}
	}

	void updateBar(Tool* tool, Scene* scene) {
		switch (tool->toolType) {
			case Tool::ToolType::WATER_CAN:
				if (tool->remainingWater == 0.0f) { 
					filledBar->resetBar(0.0f);
				}
				else {
					float oldFraction = filledBar->filledFraction;
					float newFraction = tool->remainingWater / tool->capacity;
					filledBar->updateBar(newFraction);
					if (oldFraction != newFraction
						&& !scene->aEngine->IsPlaying(AUDIO_FILE_WATER_REFILL)) {
						scene->aEngine->PlaySounds(AUDIO_FILE_WATER_REFILL, glm::vec3(rootNode->transform[3]),
							scene->aEngine->VolumeTodB(scene->volumeAdjust * 1.0f));
					}
				}
				break;
			case Tool::ToolType::PLOW: break;
			case Tool::ToolType::SEED: break;
			case Tool::ToolType::PESTICIDE: break;
			case Tool::ToolType::FERTILIZER: 
				if (tool->fertilizerCurrTime > tool->fertilizerCooldownTime) {
					filledBar->shouldDisplay = false;
					filledBar->resetBar(1.0f);
				}
				else {
					filledBar->shouldDisplay = true;
					filledBar->updateBar((tool->fertilizerCooldownTime - tool->fertilizerCurrTime) / tool->fertilizerCooldownTime);
				}
				break;
		}

		// move bar to the player if the tool with a bar is held
		if (barNode) {
			if (tool->held) {
				if (scene->objectIdMap.find(holderId) != scene->objectIdMap.end()) {
					barNode->pose[1] = -scene->objectIdMap[holderId]->pose[1];
				}
			}
			else { barNode->pose[1] = -rootNode->pose[1]; }
		}
	}

	void putInHand(SceneNode * handNode, float scaler, glm::vec3 holdVec, glm::vec3 holdPose, Scene * scene) {
		if (rootNode->parent != handNode) {
			holderId = handNode->objectId;
			handNode->addChild(rootNode);
		}
		rootNode->scaler = 1.0/scaler;
		rootNode->position = holdVec;
		rootNode->pose = holdPose;
	}

	float getScalerFromType(Tool::ToolType type) {
		switch (type) {
			case Tool::ToolType::PLOW:		return SHOVEL_SCALER;
			case Tool::ToolType::WATER_CAN:	return WATER_CAN_SCALER;
			case Tool::ToolType::SEED:		return SEED_BAG_SCALER;
		}
		return 1.0;
	}

	Tool::ToolType type;

private:
	uint holderId;
	HealthBar* filledBar;
	SceneNode* barNode;
	SceneNode* particleNode;
	ParticleGroup* pGroup;

	static constexpr glm::vec3 CAN_BAR_COLOR = glm::vec3(0.1, 0.9, 1.0);
	static constexpr float CAN_BAR_TRANSLATE_Y = 1.3;
	static constexpr glm::vec3 COOLDOWN_BAR_COLOR = glm::vec3(0.6, 0.6, 0.6); // grey

	TextUI* chatText;
	SceneNode* textNode;

	static constexpr glm::vec3 CHAT_TEXT_COLOR = glm::vec3(0.2); // grey
	static constexpr float CHAT_TEXT_TRANSLATE_Y = 1.5f;

};