#ifndef ASTARSCENE_H
#define ASTARSCENE_H

#include <cocos2d.h>
#include "CustomNode.h"
#include <map>
#include <unordered_set>
#include <map>
#include "Component.h"

class AStarScene : public cocos2d::Scene
{
private:
	//default constructor
	AStarScene() {};

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
    
	enum class CUSTOM_LABEL_INDEX
	{
		STATUS,
		PATH_LENGTH,
	};

	enum class USAGE_KEY
	{
		NONE,
		PAUSE,
		AUTO,
		MANUAL,
		RESET,
		CLEAR_BLOCK,
		RANDOMIZE_BLOCK,
		ALLOW_DIAGONAL,
		F,
		G,
		H,
	};

	enum class USAGE_MOUSE
	{
		NONE,
		TOGGLE_BLOCK,
		DRAG_START,
		DRAG_END
	};

    enum Z_ORDER
    {
		CELL,
		GRID_LINE,
		HOVER,
		DRAG,
        BOX
    };
    
    LabelsNode* labelsNode;
    DisplayBoundaryBoxNode* displayBoundaryBoxNode;
	cocos2d::DrawNode* bgDrawNode;
	SliderLabelNode* sliderLabelNode;
	float simulationSpeedModifier;
	cocos2d::Sprite* hoveringCellSprite;
	std::vector<ECS::Entity*> cells;
	cocos2d::Sprite* draggingStartSprite;
	cocos2d::Sprite* draggingEndSprite;
	ECS::Entity* startingCell;
	ECS::Entity* endingCell;
	bool draggingStart;
	bool draggingEnd;
	bool pause;
	bool finished;
    bool cleared;
	bool allowDiagonal;
	float elapsedTime;
	float stepDuration;
	bool shiftPressing;
    bool stepMode;

	struct hashCCVec2
	{
		size_t operator()(const cocos2d::Vec2& vec2) const
		{
			return vec2.x;
		}
	};

	struct compareCell
	{
		bool operator()(const ECS::Cell* first, const ECS::Cell* second)
		{
			if (first->f == second->f)
			{
				return first->h < second->h;
			}
			else
			{
				return first->f < second->f;
			}
		}
	};


	//std::multimap<int/*f*/, ECS::Cell*> openSet;
	std::list<ECS::Cell*> openSet;
	std::list<ECS::Cell*> path;

	void initECS();
	void createNewCell(const cocos2d::Vec2& position);
	void findPath();
	cocos2d::Vec2 cursorPointToCellPos(const cocos2d::Vec2& point);
	unsigned int cellPosToIndex(const cocos2d::Vec2& cellPos);
	void cancelDragging();
    void updateScore();
	void resetPathFinding();
	std::vector<unsigned int> getNeightborIndicies(unsigned int currentIndex);
	void retracePath(ECS::Cell* currentCell);
	void revertPath();
	void insertCellToOpenSet(ECS::Cell* cell);
	void clearBlocks();
	// On slider finishes click on slider
	void onSliderClick(cocos2d::Ref* sender);

public:
	//simple creator func
	static AStarScene* createScene();

	//default destructor
	~AStarScene() {};

	//Cocos2d Macro
	CREATE_FUNC(AStarScene);
};

#endif
