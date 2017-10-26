#ifndef SORTSCENE_H
#define SORTSCENE_H

#include "cocos2d.h"
#include "CustomNode.h"
#include <vector>

struct MergeElem
{
	std::vector<int> values;
	MergeElem* parent;
	MergeElem* left;
	MergeElem* right;
	bool leaf;

	int startIndex;
	int endIndex;

	MergeElem() :left(nullptr), right(nullptr), parent(nullptr), leaf(false), startIndex(-1), endIndex(-1) {}
	~MergeElem() 
	{
		if (left) delete left;
		if (right) delete right;
	}
};

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
		STEP,
		RESET,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5
	};

	int MAX_VALUE_SIZE = 65;
	float barScaleX;
	float barScaleYMul;

	SORT_MODE sortMode;

	LabelsNode* labelsNode;
	DisplayBoundaryBoxNode* displayBoundaryBoxNode;
	SliderLabelNode* barCountSliderNode;
	SliderLabelNode* sliderLabelNode;
	float simulationSpeedModifier;
	cocos2d::Label* barInfoLabel;
	int hoveringBarIndex;

	bool paused;

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
		CHECK,
		FINISHED,
	};
	SELECTION_SORT_STATE selectionSortState;

	// Insertion sort
	int curInsertionSortIndex;
	int swappingIndex;
	float animSpeed;
	enum class INSERTION_SORT_STATE
	{
		NONE,
		SELECTING_NEXT,
		SWAP,
		SWAPPING,
		CHECK,
		FINISHED
	};
	INSERTION_SORT_STATE insertionSortState;

	// Merge sort
	MergeElem* root;
	MergeElem* curElem;
	enum class MERGE_SORT_STATE
	{
		NONE,
		SEARCH,
		SORT,
		CHECK,
		FINISHED
	};
	MERGE_SORT_STATE mergeSortState;

	int checkIndex;
	float checkSpeed;
	float checkElapsedTime;

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
	void onArraySizeSliderClick(cocos2d::Ref* sender);
	void onSliderClick(cocos2d::Ref* sender);

	void resetValues();
	void randomizeValues();
	void reset();
	void resetBar();
	void clearPrevSortModeLabelColor();

	void initSelectionSort();
	void updateSelectionSort(const float delta);
	void stepSelectionSort();

	void initInsertionSort();
	void updateInsertionSort(const float delta);
	void stepInsertionSort();

	void initMergeSort();
	void updateMergeSort(const float delta);
	void buildMergeElemTree(MergeElem* me, const int start, const int end, const int size);
	MergeElem* searchUnsortedElem(MergeElem* elem);
	void sortCurElem();

	void checkSort(const float delta);

	void checkMouseOver(const cocos2d::Vec2& mousePos);

	void replaceBars();
public:
	//simple creator func
	static SortScene* createScene();

	//default destructor
	~SortScene() {};

	//Cocos2d Macro
	CREATE_FUNC(SortScene);
};

#endif