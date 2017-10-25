#ifndef SORTSCENE_H
#define SORTSCENE_H

#include "cocos2d.h"
#include "CustomNode.h"
#include <vector>

class SortScene : public cocos2d::Scene
{
private:
	//default constructor
	SortScene() {};

	enum Z_ORDER
	{
		DEBUG,
		BAR,
		BOUNDARY_WALL,
	};

	enum class CUSTOM_LABEL_INDEX
	{
		STATUS,
		CURRENT_SORT
	};

	enum class SORT_MODE
	{
		NONE,
		SELECTION,
		INSERTION,
		MERGE,
		BUBBLE,
		QUICK
	};

	enum class USAGE_KEY
	{
		NONE,
		PAUSE,
		RESET,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5
	};

	const int MAX_VALUE_SIZE = 65;

	SORT_MODE sortMode;

	LabelsNode* labelsNode;
	DisplayBoundaryBoxNode* displayBoundaryBoxNode;
	SliderLabelNode* sliderLabelNode;
	float simulationSpeedModifier;

	// Selection sort
	int minSearchIndex;
	int minSelectedIndex;
	int sortedIndex;
	float searchSpeed;
	float searchElapsedTime;
	enum class SELECTION_SORT_STATE
	{
		NONE,
		SEARCHING_MIN_VALUE,
		SWAP,
		FINISHED,
	};
	SELECTION_SORT_STATE selectionSortState;

	// total 65
	std::vector<int> values;
	std::vector<cocos2d::Sprite*> bars;
	
	//Input Listeners
	cocos2d::EventListenerMouse* mouseInputListener;
	cocos2d::EventListenerKeyboard* keyInputListener;

	//Mouse events
	void onMouseMove(cocos2d::Event* event);
	void onMouseDown(cocos2d::Event* event);
	void onMouseUp(cocos2d::Event* event);
	void onMouseScroll(cocos2d::Event* event);

	//keyboard events
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	//cocos2d virtuals
	virtual bool init() override;
	virtual void onEnter() override;
	virtual void update(float delta) override;
	virtual void onExit() override;

	void initInputListeners();
	void releaseInputListeners();

	// On slider finishes click on slider
	void onSliderClick(cocos2d::Ref* sender);

	void resetValues();
	void randomizeValues();
	void reset();
	void resetBarColor();

	void initSelectionSort();
	void updateSelectionSort(const float delta);
public:
	//simple creator func
	static SortScene* createScene();

	//default destructor
	~SortScene() {};

	//Cocos2d Macro
	CREATE_FUNC(SortScene);
};

#endif