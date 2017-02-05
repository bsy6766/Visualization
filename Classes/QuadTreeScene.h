#ifndef QTREESCENE_H
#define QTREESCENE_H

#include "cocos2d.h"
#include "Component.h"
#include "System.h"
#include "ECS.h"
#include "Component.h"
#include "CustomNode.h"
#include <vector>
#include <list>

class QuadTreeScene : public cocos2d::Scene
{
private:
	//default constructor
	QuadTreeScene() = default;

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

	// Initialize and release input listeners.
	void initInputListeners();
	void releaseInputListeners();

    // Enum for labels
    enum class CUSTOM_LABEL_INDEX
    {
        ENTITIES,
        COLLISION,
        COLLISION_WO_DUP_CHECK,
        BRUTE_FORCE,
        QUAD_TREE_MAX_LEVEL,
		MAX_CUSTOM_LABEL
    };
    
	enum class USAGE_KEY
	{
        NONE = 0,
		PAUSE,
		CLEAR,
		ADD_TEN,
		REMOVE_TEN,
		GRID,
		DUPL_CHECK,
		COL_RESOLVE,
		INC_QTREE_LEVEL,
		DEC_QTREE_LEVEL,
		MAX_KEYBOARD_USAGE,
	};
    
    enum class USAGE_MOUSE
    {
        NONE = 0,
        ADD_ONE,
        TRACK,
        REMOVE_ONE,
        MAX_MOUSE_USAGE
    };

	enum Z_ORDER
	{
		LINE,
		ENTITY,
		BOX
	};

	// flags
	bool pause;

	// Tracking entity
	int lastTrackingEntityID;

	// Node
	cocos2d::Node* areaNode;
	QuadTreeLineNode* quadTreeLineNode;
    DisplayBoundaryBoxNode* displayBoundaryBoxNode;
    LabelsNode* labelsNode;
	SliderLabelNode* sliderLabelNode;
	float simulationSpeedModifier;

	// Boundary holder
	cocos2d::Rect displayBoundary;
	
	// Initialize ECS
	void initECS();
    
	// Creates new entity with required components at position
	void createNewEntity(const cocos2d::Vec2& position = cocos2d::Vec2::ZERO);
    
    // Toggle color
    void toggleColor(const bool enabled, LabelsNode::TYPE type, const int index, const bool playAnimation = true);

	// On slider finishes click on slider
	void onSliderClick(cocos2d::Ref* sender);
public:
	//simple creator func
	static QuadTreeScene* createScene();

	//default destructor
	~QuadTreeScene() {};

	//Cocos2d Macro
	CREATE_FUNC(QuadTreeScene);
};

#endif
