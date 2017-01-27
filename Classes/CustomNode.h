#ifndef CUSTOM_NODE_H
#define CUSTOM_NODE_H

#include "cocos2d.h"
#include <string>
#include <vector>

/**
*	@class QuadTreeLineNode
*	@brief Inherits cocos2d Node.
*
*	Purpose of this class is to create node that has all the quadtree line
*/

class QuadTreeLineNode : public cocos2d::Node
{
private:
	friend class QuadTreeScene;

	//Default contructor
	QuadTreeLineNode() = default;

	//cocos2d virtual
	virtual bool init() override;

	// Draw node
	cocos2d::DrawNode* drawNode;
public:
	//simple creator func
	static QuadTreeLineNode* createNode();

	//Default destructor
	~QuadTreeLineNode() = default;

	//Cocos2d Macro
	CREATE_FUNC(QuadTreeLineNode);
};

/**
 *	@class DisplayBoundaryBoxNode
 *	@brief Inherits cocos2d Node.
 *
 *	Purpose of this class is to create node that draws display boundary box
 */

class DisplayBoundaryBoxNode : public cocos2d::Node
{
private:
    friend class QuadTreeScene;
    friend class FlockingScene;
    
    //Default contructor
    DisplayBoundaryBoxNode() = default;
    
    //cocos2d virtual
    virtual bool init() override;
    
    // Draw node
    cocos2d::DrawNode* drawNode;
    
    // Boundary
    cocos2d::Rect displayBoundary;
public:
    //simple creator func
    static DisplayBoundaryBoxNode* createNode();
    
    //Default destructor
    ~DisplayBoundaryBoxNode() = default;
    
    //Cocos2d Macro
    CREATE_FUNC(DisplayBoundaryBoxNode);
    
    // Draw box and modify display boundary to fit center left of screen
    void drawDisplayBoundaryBox();
};


/**
 *	@class LabelsNode
 *	@brief Inherits cocos2d Node.
 *
 *	Purpose of this class is to create node that holds multiple Labels
 */

class LabelsNode : public cocos2d::Node
{
private:
    friend class QuadTreeScene;
    friend class FlockingScene;
    friend class CirclePackingScene;
    
    enum class TYPE
    {
        CUSTOM,
        KEYBOARD,
        MOUSE
    };
    
    struct MouseUsageLabel
    {
        cocos2d::Label* label;
        int mouseButton;    //0 = left, 1 = right, 2 = middle
    };
    
    static const std::string fontPath;
    
    //Default contructor
    LabelsNode() = default;
    
    //cocos2d virtual
    virtual bool init() override;
    virtual void onExit() override;
    
    // Shared Labels
    cocos2d::Label* backLabel;
    cocos2d::Label* fpsLabel;
    
    // Custom Labels
    std::vector<cocos2d::Label*> customLabels;
    cocos2d::Vec2 customLabelStartPos;
    
    // Input usage labels
    std::vector<cocos2d::Label*> keyboardUsageLabels;
    cocos2d::Vec2 keyboardUsageLabelStartPos;
    std::vector<cocos2d::Label*> mouseUsageLabels;
    cocos2d::Vec2 mouseUsageLabelStartPos;
    
    // Animation
    cocos2d::ActionInterval* labelAnimation;
    
    // FPS
    int fps;
    float fpsElapsedTime;
    
    // check index
    const bool isValidIndex(TYPE type, const int index);
public:
    //simple creator func
    static LabelsNode* createNode();
    
    //Default destructor
    ~LabelsNode() = default;
    
    //Cocos2d Macro
    CREATE_FUNC(LabelsNode);
    
    // Updates
    void updateFPSLabel(const float delta);
    void updateMouseHover(const cocos2d::Vec2& mousePos);
    const bool updateMouseDown(const cocos2d::Vec2& mousePos);

    // Add label
    void addLabel(TYPE type, const std::string& str, const int fontSize);
    void updateLabel(const int index, const std::string& str);
    void setColor(TYPE type, const int index, cocos2d::Color3B color, const bool playAnimation = true);
    void playAnimation(TYPE type, const int index);
};

#endif
