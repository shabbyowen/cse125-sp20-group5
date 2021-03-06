#include "DrawableUI.h"

bool DrawableUI::isDrawUiMode = false;

DrawableUI::DrawableUI() {
    maxAlphaStartTime = std::chrono::system_clock::now() - std::chrono::milliseconds(RENDER_TIME_MILLISEC);
}

bool DrawableUI::canDraw() {
    if (!DrawableUI::isDrawUiMode) { return false; } // isDrawUiMode is toggled when drawing all UI at the end of render pass for the sake of alpha blending

    if (this->alphaEffectOn) {
        if (this->shouldDisplay) {
            alphaValue = std::min(maxAlpha, alphaValue + alphaStep);
        }
        else {
            if (alphaValue == 0.0f) { return false; } // do not display
            alphaValue = std::max(0.0f, alphaValue - alphaStep);
        }
    }
    else if (!this->shouldDisplay) { return false; } // do not display

    return true;
}

void DrawableUI::update(SceneNode* node) {
    // update autoFadeOff effect, here might not be the best place to do this, need refactoring TODO
	if (autoFadeOff && alphaEffectOn
        && shouldDisplay) {
        if (alphaValue != maxAlpha) {
            maxAlphaStartTime = std::chrono::system_clock::now();
        }
        else {
            auto durationMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - maxAlphaStartTime);
            if (durationMilliSec.count() > RENDER_TIME_MILLISEC) {
                shouldDisplay = false;
            }
        }
	}
}

SceneNode* DrawableUI::createSceneNodes(uint objectId) {
    return new SceneNode(this, std::string("DrawableUI"), objectId);
}

void DrawableUI::setAlphaSetting(bool alphaEffectOn, float initialAlpha, float alphaStep) {
    this->alphaEffectOn = alphaEffectOn;
    this->alphaValue = initialAlpha;
    this->alphaStep = alphaStep;
}

// borrowed from HealthBar
void DrawableUI::loadTexture(const char* textureFile, GLuint* textureID)
{
	int width, height, nrComponents;
	unsigned char* tdata;  // texture pixel data

	tdata = stbi_load(textureFile, &width, &height, &nrComponents, STBI_rgb_alpha);
	if (tdata == NULL) return;

	// Create ID for texture
	glGenTextures(1, textureID);

	// Set this texture to be the one we are working with
	glBindTexture(GL_TEXTURE_2D, *textureID);

	// Generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Set bi-linear filtering for both minification and magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}