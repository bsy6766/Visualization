#ifndef CUSTOM_NODE_H
#define CUSTOM_NODE_H

#include <cocos2d.h>
#include <ui/CocosGUI.h>
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
    friend class RectPackingScene;
    friend class EarClippingScene;
    
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
	friend class RectPackingScene;
    friend class EarClippingScene;

	enum class TYPE
	{
		CUSTOM,
		KEYBOARD,
		MOUSE_OVER_AND_KEY,
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
	cocos2d::Label* titleLabel;
	cocos2d::Label* backLabel;
	cocos2d::Label* fpsLabel;
	cocos2d::Label* timeTakenLabel;

	enum SHARED_LABEL_POS_TYPE
	{
		QUADTREE_SCENE,
		FLOCKING_SCENE,
		CIRCLE_PACKING_SCENE,
		RECT_PACKING_SCENE,
        EAR_CLIPPING_SCENE,
		FREE,
	};

	// Function for shared Labels
	void initTitleStr(const std::string& titleString, const cocos2d::Vec2& pos);
	void setSharedLabelPosition(SHARED_LABEL_POS_TYPE type);


    // Custom Labels
    std::vector<cocos2d::Label*> customLabels;
    cocos2d::Vec2 customLabelStartPos;
    
    // Input usage labels
    std::vector<cocos2d::Label*> keyboardUsageLabels;
    cocos2d::Vec2 keyboardUsageLabelStartPos;
    std::vector<cocos2d::Label*> mouseOverAndKeyUsageLabels;
    cocos2d::Vec2 mouseOverAndKeyLabelStartPos;
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
	void updateTimeTakenLabel(const std::string& timeTaken);
    void updateMouseHover(const cocos2d::Vec2& mousePos);
    const bool updateMouseDown(const cocos2d::Vec2& mousePos);

    // Add label
    void addLabel(TYPE type, const std::string& str, const int fontSize);
    void updateLabel(const int index, const std::string& str);
    void setColor(TYPE type, const int index, cocos2d::Color3B color, const bool playAnimation = true);
    void playAnimation(TYPE type, const int index);
};

/**
 *	@class ButtonModifierNode
 *	@brief Inherits cocos2d Node.
 *
 *	Purpose of this class is to create a label with two button modifier and value for flocking scene
 */

class ButtonModifierNode : public cocos2d::Node
{
private:
    friend class FlockingScene;
	friend class RectPackingScene;
    
    // font path
    static const std::string fontPath;
    
    //Default contructor
    ButtonModifierNode() = default;
    
    //cocos2d virtual
    virtual bool init() override;
    
    // Type
    enum class TYPE
    {
        LABEL,
        VALUE,
        LEFT_BUTTON,
        RIGHT_BUTTON
    };
    
    std::vector<cocos2d::Label*> buttonLabels;
    std::vector<cocos2d::Label*> valueLabels;
    std::vector<cocos2d::ui::Button*> leftButtons;
    std::vector<cocos2d::ui::Button*> rightButtons;
    
    cocos2d::Vec2 buttonLabelStartPos;
    float leftButtonXOffset;
    float rightButtonXOffset;
    float valueLabelXOffset;
    
    const bool isValidIndex(TYPE type, const int index);
public:
    //simple creator func
    static ButtonModifierNode* createNode();
    
    //Default destructor
    ~ButtonModifierNode() = default;
    
    //Cocos2d Macro
    CREATE_FUNC(ButtonModifierNode);
    
    // Add button
    void addButton(const std::string& labelStr, const int fontSize, const float value, const std::string& leftButtonSpriteNamePrefix, const std::string& rightButtonSpriteNamePrefix, const std::string& buttonSpriteNameSuffix, const std::string& format, const int leftActionTag, const int rightActionTag, const cocos2d::ui::AbstractCheckButton::ccWidgetClickCallback& callback);
    void updateValue(const int index, const float value);
};

/**
*	@class SliderLabelNode
*	@brief Inherits cocos2d Node.
*
*	Purpose of this class is to create node that has label and slider
*/

class SliderLabelNode : public cocos2d::Node
{
private:
	friend class QuadTreeScene;
	friend class FlockingScene;
	friend class CirclePackingScene;
	friend class RectPackingScene;
	friend class EarClippingScene;

	//Default contructor
	SliderLabelNode() = default;

	//cocos2d virtual
	virtual bool init() override;

	//Label and slider
	cocos2d::Vec2 sliderStartPos;

	struct SliderLabel
	{
		cocos2d::Label* label;
		cocos2d::ui::Slider* slider;
	};
	
	std::vector<SliderLabel> sliderLabels;
public:
	//simple creator func
	static SliderLabelNode* createNode();

	//Default destructor
	~SliderLabelNode() = default;

	//Cocos2d Macro
	CREATE_FUNC(SliderLabelNode);

	void addSlider(const std::string& labelStr, const std::string& sliderTextureName, const int percentage, const cocos2d::ui::Widget::ccWidgetClickCallback &callback);
};
#endif
