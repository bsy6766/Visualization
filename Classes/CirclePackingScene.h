#ifndef CIRCLEPACKINGSCENE_H
#define CIRCLEPACKINGSCENE_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "QuadTree.h"
#include "ECS.h"
#include "CustomNode.h"
#include "Component.h"
#include "System.h"
#include <memory>

class CirclePackingScene : public cocos2d::Scene
{
private:
	//default constructor
	CirclePackingScene() {};

	//Input Listeners
	cocos2d::EventListenerMouse* mouseInputListener;
	cocos2d::EventListenerKeyboard* keyInputListener;

	//Mouse events
	void onMouseMove(cocos2d::Event* event);
	void onMouseDown(cocos2d::Event* event);

	//keyboard events
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	//cocos2d virtuals
	virtual bool init() override;
	virtual void onEnter() override;
	virtual void update(float delta) override;
	virtual void onExit() override;

	void initInputListeners();
	void releaseInputListeners();

    // Labels
	LabelsNode* labelsNode;
	SliderLabelNode* sliderLabelNode;
	float simulationSpeedModifier;

	// Enums
	enum class CUSTOM_LABEL_INDEX
	{
		STATUS,
		IMAGE_SIZE,
		POSSIBLE_SPAWN_POINTS,
		SPAWNED_CIRCLES,
		GROWING_CIRCLES,
		MAX_CUSTOM_LABEL,
	};

	cocos2d::Label* imageNameLabel;
	cocos2d::Label* imageTestPurposeLabel;
	cocos2d::Label* imageSelectInstructionLabel;
	cocos2d::Node* imageSelectNode;
	cocos2d::Sprite* imageSelectPanelBg;
	cocos2d::Node* circleNode;

    // Images and sprites
	std::vector<cocos2d::Image*> images;
	std::vector<cocos2d::Sprite*> imageSprites;
	std::vector<cocos2d::ui::Button*> imageSpritesIconButtons;

	enum BUTTON_TAG
	{
		CPP,
		CAT,
		THE_SCREAM,
		GRADIENT
	};

	enum USAGE_KEY
	{
		NONE,
		PAUSE,
		SHOW_ORIGINAL_IMAGE,
		RESTART,
		SAVE,
		CLEAR,
	};

	// Show original image over circles
	bool showOriginalImage;

	enum class SPRITE_Z_ORDER
	{
		BEHIND_CIRCLES = 0,
		CIRCLES,
		ABOVE_CIRCLES
	};

	void initECS();

    // Initialize all images
	void initImages();
    
    // Initialize image and sprite
	void initImageAndSprite(const std::string& imageName, const BUTTON_TAG buttonTag);
    
    // Run circle packing algorithm with specific image
	void runCirclePacking(const ECS::CirclePackingSystem::IMAGE_INDEX imageIndex);
    
    // Set image name and sizelabel corresponding to image name
	void setImageNameAndSizeLabel();
    
	// Initialize entities and quad tree
	void initQuadTree();
    
	// Button press call back
	void onButtonPressed(cocos2d::Ref* sender);

	// On slider finishes click on slider
	void onSliderClick(cocos2d::Ref* sender);

public:
	//simple creator func
	static CirclePackingScene* createScene();

	//default destructor
	~CirclePackingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(CirclePackingScene);
};

#endif
