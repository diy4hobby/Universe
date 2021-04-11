#include "universe_tree.h"
#include <iostream>

tree_t			tree;


bool	node_t::isInside(sf::Vector2f* point)
{
	if (point->x < pos.x)			return false;
	if (point->x > (pos.x + size))	return false;
	if (point->y < pos.y)			return false;
	if (point->y > (pos.y + size))	return false;
	return true;
}




node_t*	nodesPool_t::get(sf::Vector2f pos, float size)
{
	if (next == nodes.size())
	{
		return NULL;
	}
	node_t*  result	= &nodes[next];
	next++;
	result->pos				= pos;
	result->size			= size;
	result->centerOfMass	= sf::Vector2f(0.0f, 0.0f);
	result->totalMass		= 0.0f;
	result->particleIdx		= 0xFFFFFFFF;
	memset(result->childs, 0, sizeof(result->childs));
	result->isLeaf			= false;
	return result;
}




void	tree_t::init()
{
	uint32_t	powOf4		= 1;
	uint32_t	nodesCount	= 0;
	while (powOf4 < 4 * PARTICLES_MAX_COUNT)
	{
		powOf4		*= 4;
		nodesCount	+= powOf4;
	}
	this->nodesPool.nodes.resize(nodesCount);
}




void	tree_t::construct(sf::Vector2f bbPos, sf::Vector2f bbSize)
{
	//Reset pool of nodes
	this->nodesPool.reset();
	//Reset top node
	this->topNode.centerOfMass	= sf::Vector2f(0.0f, 0.0f);
	this->topNode.totalMass		= 0;
	this->topNode.pos			= bbPos;
	this->topNode.size			= (bbSize.x > bbSize.y) ? bbSize.x : bbSize.y;
	this->topNode.isLeaf		= false;
	this->topNode.particleIdx	= 0xFFFFFFFF;
	memset(this->topNode.childs, 0, sizeof(this->topNode.childs));

	//Go through all particles and add them to the tree
	for (uint32_t idx = 0; idx < PARTICLES_MAX_COUNT; idx++)
	{
		if (world.particles.state[idx] == PARTICLE_FREE)		continue;
		this->_addParticle(idx, &this->topNode);
	}

	//Calculate the center of mass of each node
	this->topNode.centerOfMass	/= this->topNode.totalMass;
	float	coeff				= 0;
	float	maxCoeff			= 0;
	for (uint32_t idx = 0; idx < tree.nodesPool.next; idx++)
	{
		if (tree.nodesPool.nodes[idx].totalMass != 0)
		{
			tree.nodesPool.nodes[idx].centerOfMass	/= tree.nodesPool.nodes[idx].totalMass;
			coeff				= tree.nodesPool.nodes[idx].size / tree.nodesPool.nodes[idx].totalMass;
			if (maxCoeff < coeff)
			{	maxCoeff			= coeff;
				tree.renderCenter	= tree.nodesPool.nodes[idx].pos;
			}
		}
	}
};

bool	tree_t::_addParticle(uint32_t particleIdx, node_t* node)
{
	if ((node->isInside(&world.particles.pos[particleIdx]) == false)
		|| (node->size == 0))											return false;

	if (node->isLeaf == true)
	{
		//Add 4 child nodes, 1/2 size along each axis from this node, clockwise
		float	halfSize	= node->size / 2;
		node->childs[0]		= this->nodesPool.get(node->pos, halfSize);
		node->childs[1]		= this->nodesPool.get(node->pos + sf::Vector2f(halfSize, 0), halfSize);
		node->childs[2]		= this->nodesPool.get(node->pos + sf::Vector2f(halfSize, halfSize), halfSize);
		node->childs[3]		= this->nodesPool.get(node->pos + sf::Vector2f(0, halfSize), halfSize);

		//For particle in this node calculate which child node it falls into and,
		//by recursively calling _addParticle, we place the particle in the node
		sf::Vector2f	particlePos		= world.particles.pos[node->particleIdx];
		uint8_t			nodeHalfIdx		= (particlePos.x >= (node->pos.x + halfSize)) ? 1 : 0;
		nodeHalfIdx						+= (particlePos.y >= (node->pos.y + halfSize)) ? 2 : 0;
		switch (nodeHalfIdx)
		{
			case 0:		_addParticle(node->particleIdx, node->childs[0]);	break;
			case 1:		_addParticle(node->particleIdx, node->childs[1]);	break;
			case 2:		_addParticle(node->particleIdx, node->childs[3]);	break;
			case 3:		_addParticle(node->particleIdx, node->childs[2]);	break;
		}
		
		node->isLeaf						= false;
		node->particleIdx					= 0xFFFFFFFF;
	}

	node->centerOfMass	+= world.particles.pos[particleIdx] * world.particles.mass[particleIdx];
	node->totalMass		+= world.particles.mass[particleIdx];
	
	if (node->childs[0] == NULL)
	{
		node->particleIdx					= particleIdx;
		node->isLeaf						= true;
		return true;
	}

	//Calculate which child node gets the particle passed to this method,
	//and recursively call _addParticle to place the particle in the node
	sf::Vector2f	particlePos		= world.particles.pos[particleIdx];
	float			halfSize		= node->size / 2;
	uint8_t			nodeHalfIdx		= (particlePos.x >= (node->pos.x + halfSize)) ? 1 : 0;
	nodeHalfIdx						+= (particlePos.y >= (node->pos.y + halfSize)) ? 2 : 0;
	switch (nodeHalfIdx)
	{
		case 0:		_addParticle(particleIdx, node->childs[0]);	break;
		case 1:		_addParticle(particleIdx, node->childs[1]);	break;
		case 2:		_addParticle(particleIdx, node->childs[3]);	break;
		case 3:		_addParticle(particleIdx, node->childs[2]);	break;
	}

	return true;
};




void	tree_t::getForce(const uint32_t particleIdx, sf::Vector2f* force)
{
	_calcForce(force, particleIdx, &this->topNode);
};

void	tree_t::_calcForce(sf::Vector2f* force, const uint32_t particleIdx, const node_t* node)
{
	if (node->totalMass == 0)									return;
	if (world.particles.state[particleIdx] != PARTICLE_ACTIVE)	return;
	if (node->particleIdx == particleIdx)						return;
	//if ((node->isLeaf == true) && (world.particles.state[node->particleIdx] == PARTICLE_FREE))	return;

	sf::Vector2f	particlePos		= world.particles.pos[particleIdx];
	float			particleMass	= world.particles.mass[particleIdx];
	sf::Vector2f	distVect		= node->centerOfMass - particlePos;
	float			dist2			= distVect.x * distVect.x + distVect.y * distVect.y;
	float			dist			= sqrtf(dist2);
	float			size			= node->size;

	if (node->isLeaf == true)
	{
		if (world.particles.state[node->particleIdx] != PARTICLE_ACTIVE)	return;

		float		massSum			= world.particles.mass[particleIdx] + world.particles.mass[node->particleIdx];
		if (dist < 0.0002f * massSum)
		{
			if (world.particles.mass[particleIdx] > node->totalMass)
			{
				world.particles.mass[particleIdx]			+= node->totalMass;
				if (world.particles.temp[particleIdx] < TEMP_MAX_VALUE)
							world.particles.temp[particleIdx]			+= 1.1f * node->totalMass;
				//world.particles.vel[particleIdx]		+= world.particles.vel[this->particles[idx]];
				world.particles.state[node->particleIdx]	= PARTICLE_FREE;
			}
			else
			{
				world.particles.mass[node->particleIdx]		+= world.particles.mass[particleIdx];
				if (world.particles.temp[particleIdx] < TEMP_MAX_VALUE)
							world.particles.temp[node->particleIdx]		+= 1.1f * world.particles.mass[particleIdx];
				//world.particles.vel[this->particles[idx]]	+= world.particles.vel[particleIdx];
				world.particles.state[particleIdx]			= PARTICLE_FREE;
				return;
			}
		}

	}else
	{
		if ((size / dist) > ACCURACY_THETA)
		{
			for (uint8_t idx = 0; idx < sizeof(node->childs) / sizeof(node->childs[0]); idx++)
			{
				_calcForce(force, particleIdx, node->childs[idx]);
			}
			return;
		}
	}

	float	dist3		= dist2 * dist2;
	float	forceValue	= GRAVITY_VALUE * node->totalMass / (0.001f + dist3);
	//forceValue			*= (1.0f - forceValue / (0.1f + forceValue));
	forceValue			= sqrtf(forceValue);
	*force				+= distVect * forceValue;

};
