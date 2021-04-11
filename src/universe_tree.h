#ifndef UNIVERSE_TREE_H_
#define UNIVERSE_TREE_H_

#include <SFML/Graphics.hpp>
#include "universe_constants.h"
#include "universe_world.h"


typedef struct node_t
{
	sf::Vector2f			pos;
	float					size;
	sf::Vector2f			centerOfMass;
	float					totalMass;
	uint32_t				particleIdx;
	node_t*					childs[4];
	bool					isLeaf;

	bool					isInside(sf::Vector2f* point);
};

typedef struct nodesPool_t
{
	std::vector<node_t>				nodes;
	uint32_t						next;

	void							reset()	{ next = 0; }
	node_t*							get(sf::Vector2f pos, float size);
};

typedef struct tree_t
{
	nodesPool_t							nodesPool;
	node_t								topNode;
	sf::Vector2f						renderCenter;
	void								init();
	void								construct(sf::Vector2f bbPos, sf::Vector2f bbSize);
	bool								_addParticle(uint32_t elIdx, node_t* node);
	void								getForce(const uint32_t elIdx, sf::Vector2f* force);
	void								_calcForce(sf::Vector2f* force, const uint32_t elIdx, const node_t* node);
};


extern tree_t	tree;



#endif /* UNIVERSE_TREE_H_ */
