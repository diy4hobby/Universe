#include "universe_worker.h"
#include "universe_constants.h"
#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <ppl.h>


void	universe_worker_func()
{
	//Initializes processing tree
	tree.init();
	
	//Initializing the random number generator
	std::mt19937							rngGen(std::random_device{}());
	std::uniform_real_distribution<float>	rndAngle(0.0f, 2.0f * 3.14159f);
	std::uniform_real_distribution<float>	rndDist(0, WORLD_SIZE_MULTIPLIER * world.params.height);
	std::uniform_real_distribution<float>	rndMass(0.7f * PARTICLE_BASE_MASS, PARTICLE_BASE_MASS);
	std::uniform_real_distribution<float>	rndVel(-1.1f, 1.1f);
	std::uniform_real_distribution<float>	rndTemp(0.01f, 0.05f);

	//Initializing the initial set of particles with random values
	world.statistic.activeCount			= 0;
	world.statistic.totalMass			= 0;
	world.params.partPos				= sf::Vector2f(0.0f, 0.0f);
	for (uint32_t idx = 0; idx < PARTICLES_MAX_COUNT; idx++)
	{
		world.particles.state[idx]		= PARTICLE_ACTIVE;
		world.particles.pos[idx].x		= rndDist(rngGen) * cosf(rndAngle(rngGen));
		world.particles.pos[idx].y		= rndDist(rngGen) * sinf(rndAngle(rngGen));
		world.particles.mass[idx]		= rndMass(rngGen);
		world.particles.vel[idx]		= sf::Vector2f(rndVel(rngGen), rndVel(rngGen));
		world.particles.acc[idx]		= sf::Vector2f(0, 0);
		world.particles.accExpld[idx]	= sf::Vector2f(0, 0);
		world.particles.temp[idx]		= rndTemp(rngGen);
		world.statistic.activeCount++;
		world.statistic.totalMass		+= world.particles.mass[world.statistic.activeCount];
	}

	//Initializing the world calculation time measurement
	sf::Clock				procTimer;
	uint64_t				procTimeValue	= 0;

	float					maxMass			= 0;
	sf::Vector2f			maxMassPos		= {0, 0};
	float					summMass		= 0;


	while (1)
	{
		//Start of the world calculation time measurement
		procTimer.restart();
		
		bool			addParticle		= true;
		float			maxMass			= 0;
		sf::Vector2f	maxMassPos		= {0, 0};
		float			summMass		= 0;
		sf::Vector2f	leftTop			= {10000000.f, 10000000.f};
		sf::Vector2f	rightBot		= {-10000000.f, -10000000.f};
		uint32_t		activeCount		= 0;

		for (uint32_t idx = 0; idx < PARTICLES_MAX_COUNT; idx++)
		{
			//For "active" particles, update their position, while checking the exit from the world and
			//updating the world's bounding box
			if (world.particles.state[idx] == PARTICLE_ACTIVE)
			{
				//Check the maximum mass of the particle, if it is greater than the threshold,
				//then we make the particle "exploding" - we enter it in a special vector and change its state
				if (world.particles.mass[idx] > PARTICLE_EXPLODE_MASS)
				{
					world.particles.state[idx] = PARTICLE_EXPLODE;
					//world.particles.mass[idx]		= 0;
					world.explodeIndexes.push_back(idx);
					continue;
				}
				
				
				sf::Vector2f	newPos			= world.particles.pos[idx] + (SIMULATION_DELTA * world.particles.vel[idx]
													+ 0.5f * SIMULATION_DELTA * SIMULATION_DELTA * world.particles.acc[idx]);
				//Check that the particle goes out of the world, and if it goes out, we return it from the other side
				if (newPos.x < -WORLD_SIZE_MULTIPLIER * world.params.width)		newPos.x	= WORLD_SIZE_MULTIPLIER * world.params.width;
				if (newPos.y < -WORLD_SIZE_MULTIPLIER * world.params.height)	newPos.y	= WORLD_SIZE_MULTIPLIER * world.params.height;
				if (newPos.x > WORLD_SIZE_MULTIPLIER* world.params.width)		newPos.x	= -WORLD_SIZE_MULTIPLIER * world.params.width;
				if (newPos.y > WORLD_SIZE_MULTIPLIER* world.params.height)		newPos.y	= -WORLD_SIZE_MULTIPLIER * world.params.height;
				//Update particles bound box
				if (leftTop.x > newPos.x)		leftTop.x	= newPos.x;
				if (leftTop.y > newPos.y)		leftTop.y	= newPos.y;
				if (rightBot.x < newPos.x)		rightBot.x	= newPos.x;
				if (rightBot.y < newPos.y)		rightBot.y	= newPos.y;
				//Update particle position and half of velocity
				world.particles.pos[idx]					= newPos;
				world.particles.vel[idx]					+= 0.5f * SIMULATION_DELTA * world.particles.acc[idx];
				//Search max mass value and pos
				if (maxMass < world.particles.mass[idx])
				{	maxMass		= world.particles.mass[idx];
					maxMassPos	= world.particles.pos[idx];
				}
				//Update summ of mass
				summMass		+= world.particles.mass[idx];
				//Update count of active particles
				activeCount++;
				continue;
			}
			//If the particle is free - create a new one
			if ((world.particles.state[idx] == PARTICLE_FREE) && (world.statistic.totalMass < WORLD_TOTAL_MASS))
			{	world.particles.state[idx]		= PARTICLE_ACTIVE;
				world.particles.pos[idx].x		= world.params.partPos.x + 0.9f * rndDist(rngGen) * cosf(rndAngle(rngGen));
				world.particles.pos[idx].y		= world.params.partPos.y + 0.9f * rndDist(rngGen) * sinf(rndAngle(rngGen));
				world.particles.mass[idx]		= rndMass(rngGen);
				world.particles.vel[idx]		= sf::Vector2f(rndVel(rngGen), rndVel(rngGen));
				world.particles.acc[idx]		= sf::Vector2f(0, 0);
				world.particles.accExpld[idx]	= sf::Vector2f(0, 0);
				world.particles.temp[idx]		= rndTemp(rngGen);
				world.statistic.totalMass		+= world.particles.mass[idx];
			}
		}
		
		//Update world statistics
		world.statistic.activeCount			= activeCount;
		world.statistic.maxMass				= maxMass;
		world.statistic.maxMassPos			= maxMassPos;
		world.statistic.totalMass			= summMass;
		world.statistic.boundBox.leftTop	= leftTop - sf::Vector2f(1.0f, 1.0f);
		world.statistic.boundBox.sizes		= rightBot - leftTop + sf::Vector2f(2.0f, 2.0f);

		//Update processing tree
		tree.construct(world.statistic.boundBox.leftTop, world.statistic.boundBox.sizes);


#ifdef USE_PARALLEL_FOR
		concurrency::parallel_for (0, PARTICLES_MAX_COUNT, [&](uint32_t idx)
#else
		for (uint32_t idx = 0; idx < PARTICLES_MAX_COUNT; idx++)
#endif // !USE_PARALLEL_FOR
		{
			if (world.particles.state[idx] == PARTICLE_ACTIVE)
			{
				sf::Vector2f	force		= { 0, 0 };
				tree.getForce(idx, &force);
				force		/= world.particles.mass[idx];
				world.particles.vel[idx]	+= 0.5f * force * SIMULATION_DELTA + world.particles.accExpld[idx];
				world.particles.vel[idx]	*= (1.0f - DECELERATION_VALUE);
				world.particles.acc[idx]	= force;
				//world.particles.acc[idx]	+= force;
				//world.particles.acc[idx]	*= 0.2f;

				world.particles.accExpld[idx]	*= 0.99f;

				world.particles.mass[idx]	*= (1.0f - MASS_LEAKAGE_VALUE);
				if (world.particles.mass[idx] < 0)						world.particles.state[idx]	= PARTICLE_FREE;

				if (world.particles.temp[idx] > TEMP_RADIATION_VALUE)	world.particles.temp[idx]	-= TEMP_RADIATION_VALUE;
				else													world.particles.temp[idx]	= 0;
			}
#ifdef USE_PARALLEL_FOR
		});
#else
		}
#endif // !USE_PARALLEL_FOR


		std::list<uint32_t>::iterator	explodeIdx	= world.explodeIndexes.begin();
		while (explodeIdx != world.explodeIndexes.end())
		{
			uint32_t						elIdx	= *explodeIdx;
			sf::Vector2f					elPos	= world.particles.pos[elIdx];
			float							elMass	= world.particles.mass[elIdx];
			world.particles.state[elIdx]			= PARTICLE_FREE;
			explodeIdx		= world.explodeIndexes.erase(explodeIdx);
			for (uint32_t idx = 0; idx < PARTICLES_MAX_COUNT; idx++)
			{
				if ((world.particles.state[idx] == PARTICLE_FREE) || (idx == elIdx))	continue;
				/*if (world.particles.mass[idx] > 0.1f * PARTICLE_EXPLODE_MASS)
				{	world.particles.state[idx]		= PARTICLE_EXPLODE;
					world.explodeIndexes.push_back(idx);
					continue;
				}*/
				
				sf::Vector2f	distVect		= world.particles.pos[idx] - elPos;
				float			dist			= sqrtf(distVect.x * distVect.x + distVect.y * distVect.y);
				distVect						/= dist;
				float	velValue				= elMass / dist;
				velValue						/= world.particles.mass[idx];
				velValue						*= (1.0f - velValue / (0.1f + velValue));
				world.particles.accExpld[idx]	= 50.5f * distVect * velValue;
			}
		}


		world.statistic.procTime_us = procTimer.getElapsedTime().asMicroseconds();

		sf::sleep(sf::microseconds(40000));
		//sf::sleep(sf::microseconds(2000));
	}
};

void	universe_control_func()
{
	//Initializing the random number generator
	std::mt19937							rngGen(std::random_device{}());
	std::uniform_real_distribution<float>	rndPosX(-0.7f * WORLD_SIZE_MULTIPLIER * world.params.width, 0.7f * WORLD_SIZE_MULTIPLIER * world.params.width);
	std::uniform_real_distribution<float>	rndPosY(-0.7f * WORLD_SIZE_MULTIPLIER * world.params.height, 0.7f * WORLD_SIZE_MULTIPLIER * world.params.height);

	while (1)
	{
		world.params.partPos		= sf::Vector2f(rndPosX(rngGen), rndPosY(rngGen));
		sf::sleep(sf::milliseconds(60000));
	}
};