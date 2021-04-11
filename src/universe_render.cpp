#include "universe_render.h"
#include "universe_world.h"
#include "universe_tree.h"
#include "universe_constants.h"
#include <random>

extern tree_t	tree;


void	universe_render_tree(sf::RenderWindow* window, node_t node, sf::RectangleShape* shape, sf::Vector2f coeff, sf::Vector2f shift)
{
	sf::Vector2f	pos		= node.pos - shift;
					pos		= sf::Vector2f(pos.x * coeff.x, pos.y * coeff.y);
	sf::Vector2f	size	= sf::Vector2f(node.size * coeff.x, node.size * coeff.y);
	shape->setPosition(pos);
	shape->setSize(sf::Vector2f(size.x, size.y));
	window->draw(*shape);
	for (uint8_t idx = 0; idx < sizeof(node.childs) / sizeof(node.childs[0]); idx++)
	{
		if (node.childs[idx] != NULL)		universe_render_tree(window, *node.childs[idx], shape, coeff, shift);
	}
}


void	universe_render_func(sf::RenderWindow* window)
{
	window->setActive(true);
	
	sf::RenderTexture		renderTexture;
	renderTexture.create(world.params.width, world.params.height);
	renderTexture.clear(sf::Color(0, 0, 0, 32));
	sf::Sprite				worldSprite;
	worldSprite.setTexture(renderTexture.getTexture());
	worldSprite.setOrigin(world.params.width / 2, world.params.height / 2);
	worldSprite.setPosition(world.params.width / 2.f, world.params.height / 2.f);;
	sf::Texture				particleTexture;
	particleTexture.loadFromFile("Particle1.png");
	particleTexture.setSmooth(true);
	sf::Sprite				particleSprite(particleTexture);
	particleSprite.setScale(sf::Vector2f(1.0f, 1.0f));
	
	//Timer for measuring the time to draw a scene
	sf::Clock				drawTimer;
	uint64_t				drawTimeValue		= 0;
	uint16_t				drawTimeCounter		= 0;

	//Labels to show statistics
	sf::Font				textFont;			//Font
	sf::Text				drawTimeText;		//Draw scene time
	sf::Text				procTimeText;		//Worls processing time
	sf::Text				particlesCountText;	//Active particles count
	sf::Text				maxMassText;		//Particle max mass
	sf::Text				totalMassText;		//Total world mass
	textFont.loadFromFile("sansation.ttf");
	drawTimeText.setFont(textFont);
	drawTimeText.setPosition(4, 10);
	drawTimeText.setFillColor(sf::Color::Red);
	drawTimeText.setCharacterSize(12);
	procTimeText.setFont(textFont);
	procTimeText.setPosition(4, 22);
	procTimeText.setFillColor(sf::Color::Yellow);
	procTimeText.setCharacterSize(12);
	particlesCountText.setFont(textFont);
	particlesCountText.setPosition(4, 34);
	particlesCountText.setFillColor(sf::Color::Green);
	particlesCountText.setCharacterSize(12);
	maxMassText.setFont(textFont);
	maxMassText.setPosition(4, 46);
	maxMassText.setFillColor(sf::Color::Cyan);
	maxMassText.setCharacterSize(12);
	totalMassText.setFont(textFont);
	totalMassText.setPosition(4, 58);
	totalMassText.setFillColor(sf::Color::White);
	totalMassText.setCharacterSize(12);
	
	sf::RectangleShape		treeTect;
	treeTect.setFillColor(sf::Color::Transparent);
	treeTect.setOutlineThickness(0.5);
	treeTect.setOutlineColor(sf::Color::White);

	sf::Vector2f	coeff	= sf::Vector2f(0.6f, 0.6f);
	sf::Vector2f	shift	= sf::Vector2f(-500.0f, -400.0f);
	sf::Vector2f	pos		= sf::Vector2f(0.0f, 0.0f);

	uint32_t		elCount	= 0;

	while (1)
	{
		//Start measuring scene rendering time
		drawTimer.restart();
		drawTimeCounter++;

		//Clearing the scene
		window->clear();
		//renderTexture.draw(clearShape);
		renderTexture.clear();

		sf::Vector2f	scale	= 0.5f * world.statistic.boundBox.sizes;
		/*sf::Vector2f	scale	= 1.0f * sf::Vector2f(	2.0f * WORLD_SIZE_MULTIPLIER * world.params.width,
														2.0f * WORLD_SIZE_MULTIPLIER * world.params.height);*/
		coeff					= 0.99f * coeff + 0.01f * sf::Vector2f(world.params.width / scale.x, world.params.height / scale.y);
		sf::Vector2f	center	= tree.renderCenter;
		//sf::Vector2f	center	= world.statistic.boundBox.leftTop + 0.5f * world.statistic.boundBox.sizes;
		//shift					= 0.99f * shift + 0.01f * (center - sf::Vector2f((world.params.width / 2) / coeff.x, (world.params.height / 2) / coeff.y));
		shift		= 0.99f * shift + 0.01f * (world.statistic.maxMassPos - sf::Vector2f((world.params.width / 2) / coeff.x, (world.params.height / 2) / coeff.y));
		float			alphaMult	= 128 / PARTICLE_BASE_MASS;

		//Render scene elements
		for (uint32_t idx = 0; idx < PARTICLES_MAX_COUNT; idx++)
		{
			if (world.particles.state[idx] == PARTICLE_FREE)		continue;
			pos		= world.particles.pos[idx] - shift;
			pos		= sf::Vector2f(pos.x * coeff.x, pos.y * coeff.y);
			particleSprite.setPosition(pos);

			uint32_t	colA		= 64 + alphaMult * world.particles.mass[idx];
			if (colA > 255)	colA	= 255;
			//float		vel			= sqrtf(world.particles.vel[idx].x * world.particles.vel[idx].x + world.particles.vel[idx].y * world.particles.vel[idx].y);
			float		vel			= sqrtf(world.particles.temp[idx]);
			uint32_t	colR		= 30 + 4.0f * vel;
			uint32_t	colG		= 30 + 2.0f * vel;
			uint32_t	colB		= 150 + 1.0f * vel;
			if (colR > 255)	colR	= 255;
			if (colG > 255)	colG	= 255;
			if (colB > 255)	colB	= 255;
			sf::Color	elemColor(colR, colG, colB, colA);
			//sf::Color	elemColor;
			//if (world.particles.mass[idx] > (10 * PARTICLE_BASE_MASS))	elemColor	= sf::Color(255, 0, 0, 255);
			//else														elemColor	= sf::Color(colR, 255, 100, 150);
			particleSprite.setColor(elemColor);

			sf::RenderStates	states;
			states.blendMode	= sf::BlendAdd;
			renderTexture.draw(particleSprite, states);

			elCount++;
		}

		//Creating strings with text information
		std::string		labelText;
		labelText		= "Processing time, us:	" + std::to_string(world.statistic.procTime_us);
		procTimeText.setString(labelText);
		labelText		= "Particles count:	" + std::to_string(world.statistic.activeCount);
		particlesCountText.setString(labelText);
		labelText		= "Max mass:	" + std::to_string(world.statistic.maxMass);
		maxMassText.setString(labelText);
		labelText		= "Total mass:	" + std::to_string(world.statistic.totalMass);
		totalMassText.setString(labelText);

		renderTexture.display();
		window->draw(worldSprite);
		window->draw(drawTimeText);
		window->draw(procTimeText);
		window->draw(particlesCountText);
		window->draw(maxMassText);
		window->draw(totalMassText);
		//universe_render_tree(window, universeTree.topNode, &treeTect, coeff, shift);
		window->display();

		//Calculating the time to draw a scene
		drawTimeValue	+= drawTimer.getElapsedTime().asMicroseconds();
		if (drawTimeCounter == 16)
		{
			drawTimeValue				/= 16;
			std::string	elapsedInStr	= "Drawing time, us:	" + std::to_string(drawTimeValue);
			drawTimeText.setString(elapsedInStr);
			drawTimeCounter				= 0;
			drawTimeValue				= 0;
		}
		sf::sleep(sf::milliseconds(30));
	}
};

