#ifndef FLOCKINGSCENE_H
#define FLOCKINGSCENE_H

#include <cocos2d.h>
#include <ui/CocosGUI.h>
#include "ECS.h"
#include "CustomNode.h"
#include "Component.h"
#include "System.h"

class FlockingScene : public cocos2d::Scene
{
private:
	//default constructor
	FlockingScene() {};

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

	// cocos2d
	cocos2d::Rect displayBoundary;
	cocos2d::Node* areaNode;
    cocos2d::Vec2 curMousePosition;
    DisplayBoundaryBoxNode* displayBoundaryBoxNode;
    LabelsNode* labelsNode;
    ButtonModifierNode* buttonModifierNode;
	SliderLabelNode* sliderLabelNode;
	float simulationSpeedModifier;
    
    // Enum for labels
    enum class CUSTOM_LABEL_INDEX
    {
        BOIDS,
		OBSTACLES,
        WEIGHTS,
		MAX_CUSTOM_LABEL,
    };
    
	enum class USAGE_KEY
	{
        NONE,
		PAUSE = 1,
		CLEAR,
		ADD_TEN,
		REMOVE_TEN,
		SMOOTH_STEERING,
        MAX_KEYBOARD_USAGE,
	};
    
    enum class USAGE_MOUSE_OVER_AND_KEY
    {
        NONE,
        ADD_OBSTACLE,
        REMOVE_OBSTACLE,
        MAX_MOUSE_HOVER_AND_KEY_USAGE,
    };
    
    enum class USAGE_MOUSE
    {
        NONE,
        ADD_ONE,
        TRACK,
        REMOVE_ONE,
        ADD_OBSTACLE,
        REMOVE_OBSTACLE,
        MAX_MOUSE_USAGE
    };
    
    enum class WEIGHT_INDEX
    {
        ALIGNMENT,
        COHESION,
        SEPARATION,
        AVOID,
		MAX_WEIGHT,
    };

	enum class ACTION_TAG
	{
		ALIGNMENT_LEFT,
		ALIGNMENT_RIGHT,
		COHESION_LEFT,
		COHESION_RIGHT,
		SEPARATION_LEFT,
		SEPARATION_RIGHT,
		AVOID_LEFT,
		AVOID_RIGHT
	};
    
    enum Z_ORDER
    {
        BOID,
        OBSTACLE,
        BOX
    };

	// RangeChecker
	cocos2d::Sprite* rangeChecker;
	
	// Initialize ECS
	void initECS();

	// Creates new entity with required components
	void createNewBoid(const cocos2d::Vec2& pos = cocos2d::Vec2::ZERO);
	void createNewObstacle(const cocos2d::Vec2& pos);

    // Button click call back
    void onButtonPressed(cocos2d::Ref* sender);
	// On slider finishes click on slider
	void onSliderClick(cocos2d::Ref* sender);
public:
	//simple creator func
	static FlockingScene* createScene();

	//default destructor
	~FlockingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(FlockingScene);
};

#endif
