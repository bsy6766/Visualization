#ifndef RECTPACKINGSCENE_H
#define RECTPACKINGSCENE_H

#include "cocos2d.h"
#include "CustomNode.h"
#include "ECS.h"
#include <queue>

class RectPackingScene : public cocos2d::Scene
{
private:
	//default constructor
	RectPackingScene() {};

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

	enum class Z_ORDER
	{
		BOX
	};

	enum class CUSTOM_LABEL_INDEX
	{
		STATUS,
		TOTAL_RECT_PACKED,
	};

	enum class USAGE_KEY
	{
		NONE,
		PAUSE,
		TOGGLE_LINE,
		RESTART,
		CLEAR,
		MAX_KEYBOARD_USAGE,
	};

	enum class ACTION_TAG
	{
		LEFT,
		RIGHT
	};

	LabelsNode* labelsNode;
	ButtonModifierNode* buttonModifierNode;
	DisplayBoundaryBoxNode* displayBoundaryBoxNode;
	SliderLabelNode* sliderLabelNode;
	float simulationSpeedModifier;
	cocos2d::Vec2 displayBoundaryShift;
	cocos2d::DrawNode* rectDrawNode;

	// Time needed to tick each step
	float stepDuration;
	float elapsedTime;

	// Random rectangles to insert
	std::queue<cocos2d::Size> randomSizes;
	int maxRects;
	int totalRectsPacked;

	float padding;

	bool pause;
	bool drawDivisionLine;
	bool finished;

	ECS::Entity* root;

	void drawRects(ECS::Entity* entity);

	const bool insert(ECS::Entity* entity, const cocos2d::Size& rectSize);
	ECS::Entity* createNewEntity();

	void restart();
	void initRects();
	void clear();
	void clearEntity();

	// Button click call back
	void onButtonPressed(cocos2d::Ref* sender);

	// On slider finishes click on slider
	void onSliderClick(cocos2d::Ref* sender);
public:
	//simple creator func
	static RectPackingScene* createScene();

	//default destructor
	~RectPackingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(RectPackingScene);
};

#endif
