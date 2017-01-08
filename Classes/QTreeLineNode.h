#ifndef LINE_NODE_H
#define LINE_NODE_H

#include "cocos2d.h"

/**
*	@class LineNode
*	@brief Inherits cocos2d Node.
*	
*	This class derives from Cocos2d's Node class.
*	Purpose of this class is to create node that has all the quadtree line
*/

class QTreeLineNode : public cocos2d::CCNode
{
private:
	//Default contructor
	QTreeLineNode() = default;

	//cocos2d virtual
	virtual bool init() override;

public:
	//simple creator func
	static QTreeLineNode* createNode();

	//Default destructor
	~QTreeLineNode() = default;

	//Cocos2d Macro
	CREATE_FUNC(QTreeLineNode);
};

#endif