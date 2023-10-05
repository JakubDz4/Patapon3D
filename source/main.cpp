int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		stbi_set_flip_vertically_on_load(1);
		loadEntitiesData();
		loadItems();

		//Main loop flag
		bool quit = false;
		//Event handler
		SDL_Event e;

		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 600.0f);

		Shader planeShader("resources/shaders/plane.vert", "resources/shaders/plane.frag");
		planeShader.Bind();
		planeShader.SetUniform1i("u_Texture", 0);

		Shader shaderPatapon("resources/shaders/pataponModel.vert", "resources/shaders/pataponModel.frag");
		shaderPatapon.Bind();
		shaderPatapon.SetUniform1i("texture_diffuse1", 0);
		shaderPatapon.SetUniform1i("texture_specular1", 1);

		Shader shaderObstacle("resources/shaders/obstacle.vert", "resources/shaders/pataponModel.frag");
		shaderObstacle.Bind();
		shaderObstacle.SetUniform1i("texture_diffuse1", 0);

		Shader shaderInstancedProj("resources/shaders/instancedProjectile.vert", "resources/shaders/instancedProjectile.frag");
		shaderInstancedProj.Bind();
		shaderInstancedProj.SetUniform1i("texture_diffuse1", 0);

		Shader shaderHitNumber("resources/shaders/hitNumber.vert", "resources/shaders/hitNumber.frag");
		shaderHitNumber.Bind();
		shaderHitNumber.SetUniform1i("uTexture", 0);

		Shader shaderParticle("resources/shaders/Particle/particle.vert", "resources/shaders/Particle/particle.frag");
		shaderParticle.Bind();
		shaderParticle.SetUniform1i("uTexture", 0);

		Shader shaderSun("resources/shaders/Utility/sun.vert", "resources/shaders/Utility/sun.frag");

		Shader shaderDebugCircle("source/debug/debug.vert", "source/debug/debug.frag");

		Texture texture("resources/textures/texture.png");
		texture.Bind();

		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);

		//Renderer renderer;

		frameBuffer fbo;
		fbo.attachColorBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
		fbo.attachDepthBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
		fbo.unbind();

		Camera camera;
		LayerManager layerManager(camera, proj, "resources/textures/", SCREEN_WIDTH, SCREEN_HEIGHT);
		Time time;
		Rhytm rhytm;
		RhytmBar rhytmBar(&rhytm);
		//
		Sun sun(shaderSun, glm::vec3(-9.0f, 6.6f, -4.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f); //pos, color, intensity //1.85, 12, -9.55 // -9, 6.6, -4 // 15.8, 16.0, -15.6 // -12.2, 5.2, -15.6//-20.8   13   -15
		//Sun sun(shaderSun, glm::vec3(-3.4f, - 3.8f, - 4.0f), glm::vec3(1.0f, 0.5f, 0.0f), 1.0f);

		//Ubos
		UniformBlock matricesUbo;
		matricesUbo.push<glm::mat4, glm::mat4>();//view, proj
		matricesUbo.createUbo();

		layerManager.bindUbo(matricesUbo, 0, "uMatrices");
		matricesUbo.bindBlock(0, "uMatrices", shaderPatapon.getId());
		matricesUbo.bindBlock(0, "uMatrices", shaderObstacle.getId());
		matricesUbo.bindBlock(0, "uMatrices", shaderInstancedProj.getId());
		matricesUbo.bindBlock(0, "uMatrices", shaderDebugCircle.getId());
		matricesUbo.bindBlock(0, "uMatrices", shaderHitNumber.getId());
		matricesUbo.bindBlock(0, "uMatrices", shaderParticle.getId());
		matricesUbo.bindBlock(0, "uMatrices", shaderSun.getId());

		matricesUbo.update(proj, 1);

		UniformBlock lightUbo;
		lightUbo.push<glm::vec3, glm::vec3, glm::vec3>();//ligtDir, viewPos(camera position), color
		lightUbo.createUbo();

		layerManager.bindUbo(lightUbo, 1, "uLight");
		lightUbo.bindBlock(1, "uLight", shaderPatapon.getId());
		lightUbo.bindBlock(1, "uLight", shaderObstacle.getId());
		lightUbo.bindBlock(1, "uLight", shaderInstancedProj.getId());

		lightUbo.update(sun.getDirection(), 0);
		lightUbo.update(camera.m_pos, 1);
		lightUbo.update(sun.getColor(), 2);


		//ObstaclePool::GetInstance()->createObstacle(EntityClass::tmp, glm::vec3(3.0f, 0.0f, 0.0f), 1000, EntityAlignment::ENEMY);
		
		//2*attack, 2*defence
		std::vector<Equipment> equipment = {
			//Tate
			Equipment(1, 0, 1, 1),
			Equipment(1, 0, 1, 1),
			Equipment(1, 0, 1, 0),
			Equipment(0, 0, 1, 0),
			Equipment(0, 0, 1, 0),
			Equipment(0, 0, 0, 0),
			//Yari
			Equipment(1, 0, 1, 0),
			Equipment(0, 0, 1, 0),
			Equipment(1, 0, 0, 0),
			Equipment(0, 0, 0, 0),
			Equipment(0, 0, 0, 0),
			Equipment(1, 0, 0, 0),
			//Yumi
			Equipment(0, 0, 1, 0),
			Equipment(0, 0, 0, 0),
			Equipment(0, 0, 0, 0),
			Equipment(0, 0, 0, 0),
			Equipment(0, 0, 0, 0),
			Equipment(0, 0, 0, 0),
		};
		Formation<Tatepon, Yaripon, Yumipon> patapons(&equipment, 6, 6, 6);
		Boss dodonga(EntityClass::DODONGA_BOSS, glm::vec3(18.0f, 0.0f, 0.0f), 10000, 6.5f);

		float lastPlane[] = {
			-1.0f, -1.0f,  0.0f, 0.0f, 0.0f,
			 1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
			 1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
			-1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
		};
		unsigned int lastPlaneVBO, lastPlaneVAO;
		glGenVertexArrays(1, &lastPlaneVAO);
		glGenBuffers(1, &lastPlaneVBO);
		glBindVertexArray(lastPlaneVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lastPlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(lastPlane), &lastPlane, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)(3 * sizeof(float)));

		bool debugCol = false;
		while (!quit)
		{
//---------------------Update
			time.update();
			camera.updateView();
			rhytm.update();
			matricesUbo.bind();
			matricesUbo.update(camera.getView(), 0);
			lightUbo.bind();
			lightUbo.update(sun.getDirection(), 0);

//-----------------------Move & Actions

			ColliderPool::GetInstance()->manageCollisions();
			
			patapons.doActions();
			dodonga.doActions(patapons.getClosestPatapon(), rhytm.bossReadyToAttack());

			ProjectilePool::GetInstance()->onActions();

//--------------------------Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				...
			}
			

//---------------------------Draws
			fbo.bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			shaderPatapon.Bind();
			patapons.draw(shaderPatapon);
			dodonga.draw(shaderPatapon);

			shaderObstacle.Bind();
			ObstaclePool::GetInstance()->draw(shaderObstacle);

			layerManager.drawGround();

			glEnable(GL_BLEND);
			layerManager.drawBackgrounds();

			shaderInstancedProj.Bind();
			ProjectilePool::GetInstance()->instancedDraw(shaderInstancedProj);

			layerManager.drawOnGround();
			ParticleSystem::GetInstance()->draw(shaderParticle);

			layerManager.drawObjects();

			glDisable(GL_DEPTH_TEST);
			
			shaderHitNumber.Bind();
			HitNumberPool::GetInstance()->draw(shaderHitNumber);
			HitExplosionPool::GetInstance()->draw(shaderHitNumber);
			HitDrumPool::GetInstance()->draw();

			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			rhytmBar.draw();

			sun.draw(shaderSun);

			//debug
			if(debugCol){
				glDisable(GL_DEPTH_TEST);
				shaderDebugCircle.Bind();
				for (auto& col : ColliderPool::GetInstance()->getFriendlyColliders())
					Debug::drawCircle(col.point, col.radius, 12);
				for (auto& col : ColliderPool::GetInstance()->getEnemyColliders())
					Debug::drawCircle(col.point, col.radius, 12);

				for (auto& col : ColliderPool::GetInstance()->getFriendlyTriggerColliders())
					Debug::drawCircle(col.point, col.radius, 24);
				for (auto& col : ColliderPool::GetInstance()->getEnemyTriggerColliders())
					Debug::drawCircle(col.point, col.radius, 24);
				glEnable(GL_DEPTH_TEST);
			}

			fbo.unbind();

//----------------------PostProcessing

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			planeShader.Bind();
			planeShader.SetUniform1f("u_barIntensivity", rhytm.getRhytmBarIntesivity());
			planeShader.SetUniform1f("u_barStage", rhytm.getRhytmBarType());
			planeShader.SetUniform1f("u_typing", rhytm.getTypingModificator());
			patapons.setPataponShaderPos(planeShader, camera, proj);

			glBindTexture(GL_TEXTURE_2D, fbo.getColor());

			glBindVertexArray(lastPlaneVAO);
			glDrawArrays(GL_QUADS, 0, 4); 

			SDL_GL_SwapWindow(Glwindow);		
		}
		//tracker
		Debug::Timer::showResults();
	}

	//Free resources and close SDL
	close();

	return 0;
}