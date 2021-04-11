#ifndef UNIVERSE_WORLD_H_
#define UNIVERSE_WORLD_H_

#include <vector>
#include <list>
#include <SFML/Graphics.hpp>
#include "universe_constants.h"


typedef	enum
{
	PARTICLE_FREE,
	PARTICLE_ACTIVE,
	PARTICLE_EXPLODE
}particleState_e;

typedef struct
{
	particleState_e		state[PARTICLES_MAX_COUNT];
	sf::Vector2f		pos[PARTICLES_MAX_COUNT];
	float				mass[PARTICLES_MAX_COUNT];
	sf::Vector2f		vel[PARTICLES_MAX_COUNT];
	sf::Vector2f		acc[PARTICLES_MAX_COUNT];
	sf::Vector2f		accExpld[PARTICLES_MAX_COUNT];
	float				temp[PARTICLES_MAX_COUNT];
}particles_t;

typedef struct
{
	particles_t				particles;
	std::list<uint32_t>		explodeIndexes;

	struct {
		uint32_t			activeCount;
		float				maxMass;
		sf::Vector2f		maxMassPos;
		float				totalMass;
		struct{
			sf::Vector2f	leftTop;
			sf::Vector2f	sizes;
		}boundBox;
		uint64_t			procTime_us;
	}statistic;
	struct {
		int32_t				width;
		int32_t				height;
		sf::Vector2f		partPos;	//Center of the area where the particles are created
	}params;
}world_t;


//Мир для просчета
extern world_t			world;




#endif /* UNIVERSE_WORLD_H_ */
