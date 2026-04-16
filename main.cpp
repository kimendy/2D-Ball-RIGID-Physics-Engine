#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>

#include<string>
#include<vector>
#include<SDL3_ttf/SDL_ttf.h>

// basic stuff for window + renderer
namespace BaseDec {

	int width{ 1600 };
	int height{ 900 };

	SDL_Window* window{ nullptr };
	SDL_Texture* texture{ nullptr };
	SDL_Renderer* renderer{ nullptr };

}

// start SDL + stuff
void init() {

	TTF_Init();

	if (SDL_Init(SDL_INIT_VIDEO) == false) {
		SDL_Log("SDL_init failed: %s", SDL_GetError());
	}

	if (SDL_CreateWindowAndRenderer("2D Rigid Body Physics Engine",
		BaseDec::width, BaseDec::height, 0,
		&BaseDec::window, &BaseDec::renderer) == false) {

		SDL_Log("Window/Renderer failed: %s", SDL_GetError());
	}
}

// clean everything before exit
void quit() {
	SDL_DestroyTexture(BaseDec::texture);
	SDL_DestroyRenderer(BaseDec::renderer);
	SDL_DestroyWindow(BaseDec::window);
	SDL_Quit();
}

// draw circle using lines
void DrawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius) {
	for (float y = -radius; y <= radius; y++) {
		float width = sqrt(radius * radius - y * y);
		SDL_RenderLine(renderer, cx - width, cy + y, cx + width, cy + y);
	}
}

// ball data
struct Ball {
	float x, y;
	float vx, vy;
	float mass;
	float radius;
	SDL_Color color;
};

std::vector<Ball> balls;

// make a new random ball
void SpawnBall(float x = BaseDec::width / 2.f, float y = BaseDec::height / 2.f) {
	Ball b;

	b.x = x;
	b.y = y;

	b.vx = (rand() % 400) - 200.f;
	b.vy = (rand() % 400) - 200.f;

	b.radius = 10.f + (rand() % 30);
	b.mass = b.radius * 0.1f;

	b.color = {
		(Uint8)(rand() % 255),
		(Uint8)(rand() % 255),
		(Uint8)(rand() % 255),
		255
	};

	balls.push_back(b);

	SDL_Log("Balls: %zu", balls.size());
}

// handle ball collisions
void ResolveBallCollisions() {
	for (int i = 0; i < balls.size(); i++) {
		for (int j = i + 1; j < balls.size(); j++) {

			Ball& a = balls[i];
			Ball& b = balls[j];

			float dx = b.x - a.x;
			float dy = b.y - a.y;
			float dist = sqrt(dx * dx + dy * dy);
			float minDist = a.radius + b.radius;

			if (dist < minDist && dist > 0) {

				// direction between balls
				float nx = dx / dist;
				float ny = dy / dist;

				// push them apart
				float overlap = minDist - dist;

				a.x -= nx * overlap / 2;
				a.y -= ny * overlap / 2;

				b.x += nx * overlap / 2;
				b.y += ny * overlap / 2;

				// velocity change
				float dvx = a.vx - b.vx;
				float dvy = a.vy - b.vy;

				float dot = dvx * nx + dvy * ny;
				float totalMass = a.mass + b.mass;

				a.vx -= (2 * b.mass / totalMass) * dot * nx;
				a.vy -= (2 * b.mass / totalMass) * dot * ny;

				b.vx += (2 * a.mass / totalMass) * dot * nx;
				b.vy += (2 * a.mass / totalMass) * dot * ny;

				// random colors after hit
				a.color = { (Uint8)(rand() % 255), (Uint8)(rand() % 255), (Uint8)(rand() % 255), 255 };
				b.color = { (Uint8)(rand() % 255), (Uint8)(rand() % 255), (Uint8)(rand() % 255), 255 };
			}
		}
	}
}

float speedMultiplier = 1.f;

// draw text on screen
void DrawText(SDL_Renderer* renderer, TTF_Font* font,
	const std::string& text, float x, float y) {

	SDL_Color white = { 255, 255, 255, 255 };

	SDL_Surface* surface =
		TTF_RenderText_Solid(font, text.c_str(), text.size(), white);

	SDL_Texture* texture =
		SDL_CreateTextureFromSurface(renderer, surface);

	SDL_FRect rect = { x, y, (float)surface->w, (float)surface->h };

	SDL_DestroySurface(surface);

	SDL_RenderTexture(renderer, texture, nullptr, &rect);

	SDL_DestroyTexture(texture);
}

float damping = 0.999f;

int main(int argc, char* argv[])
{
	init();

	TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 20);
	if (!font) {
		SDL_Log("Font failed: %s", SDL_GetError());
	}

	SDL_Log("Renderer: %s", SDL_GetRendererName(BaseDec::renderer));

	bool isDragging = false;
	float dragStartX, dragStartY;

	Uint64 Previous_time = SDL_GetTicks();

	bool gravityOn = false;
	float gravity = 500.f;

	SDL_Event event;
	bool running(true);

	while (running) {

		Uint64 current_time = SDL_GetTicks();
		float deltaTime = (current_time - Previous_time) / 1000.f;
		Previous_time = current_time;

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}

			// right click drag to shoot ball
			if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				if (event.button.button == SDL_BUTTON_RIGHT) {
					isDragging = true;
					dragStartX = event.button.x;
					dragStartY = event.button.y;
				}
			}

			if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
				if (event.button.button == SDL_BUTTON_RIGHT && isDragging) {

					float dx = dragStartX - event.button.x;
					float dy = dragStartY - event.button.y;

					Ball b;

					b.x = dragStartX;
					b.y = dragStartY;

					b.vx = dx * 5.f;
					b.vy = dy * 5.f;

					b.radius = 20.f;
					b.mass = b.radius * 0.1f;

					b.color = { 255, 255, 0, 255 };

					balls.push_back(b);

					isDragging = false;
				}
			}

			// left click = spawn ball
			if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					SpawnBall(event.button.x, event.button.y);
				}
			}

			if (event.type == SDL_EVENT_KEY_DOWN) {

				if (event.key.scancode == SDL_SCANCODE_SPACE) {
					SpawnBall();
				}

				if (event.key.scancode == SDL_SCANCODE_G) {
					gravityOn = !gravityOn;
				}

				if (event.key.scancode == SDL_SCANCODE_R) {
					if (!balls.empty()) {
						balls.pop_back();
					}
				}

				if (event.key.scancode == SDL_SCANCODE_C) {
					balls.clear();
				}
			}
		}

		const bool* keyState = SDL_GetKeyboardState(nullptr);

		// speed control
		if (keyState[SDL_SCANCODE_UP]) {
			speedMultiplier += 0.001f;
		}
		if (keyState[SDL_SCANCODE_DOWN]) {
			speedMultiplier -= 0.001f;
			if (speedMultiplier < 0.1f) speedMultiplier = 0.1f;
		}

		// friction control
		if (keyState[SDL_SCANCODE_D]) {
			damping -= 0.00001f;
			if (damping < 0.9f) damping = 0.9f;
		}
		if (keyState[SDL_SCANCODE_F]) {
			damping += 0.00001f;
			if (damping > 1.0f) damping = 1.0f;
		}

		for (auto& ball : balls) {

			if (gravityOn) {
				ball.vy += gravity * deltaTime;
			}

			// apply friction
			ball.vx *= damping;
			ball.vy *= damping;

			// move ball
			ball.x += ball.vx * deltaTime * speedMultiplier;
			ball.y += ball.vy * deltaTime * speedMultiplier;

			// wall bounce
			if (ball.x - ball.radius < 0) {
				ball.x = ball.radius;
				ball.vx *= -1;
			}
			if (ball.x + ball.radius > BaseDec::width) {
				ball.x = BaseDec::width - ball.radius;
				ball.vx *= -1;
			}
			if (ball.y - ball.radius < 0) {
				ball.y = ball.radius;
				ball.vy *= -1;
			}
			if (ball.y + ball.radius > BaseDec::height) {
				ball.y = BaseDec::height - ball.radius;
				ball.vy *= -1;
			}
		}

		ResolveBallCollisions();

		SDL_SetRenderDrawColor(BaseDec::renderer, 0, 0, 0, 255);
		SDL_RenderClear(BaseDec::renderer);

		// draw all balls
		for (auto& ball : balls) {
			SDL_SetRenderDrawColor(BaseDec::renderer,
				ball.color.r, ball.color.g, ball.color.b, 255);

			DrawFilledCircle(BaseDec::renderer,
				ball.x, ball.y, ball.radius);
		}

		// UI text
		DrawText(BaseDec::renderer, font,
			"Balls: " + std::to_string(balls.size()), 10, 10);

		DrawText(BaseDec::renderer, font,
			"Gravity: " + std::string(gravityOn ? "ON" : "OFF"), 10, 35);

		DrawText(BaseDec::renderer, font,
			"Speed: " + std::to_string(speedMultiplier).substr(0, 4), 10, 60);

		DrawText(BaseDec::renderer, font,
			"Damping: " + std::to_string(damping).substr(0, 5), 10, 85);

		DrawText(BaseDec::renderer, font,
			"SPACE/CLICK spawn | R remove | C clear",
			10, BaseDec::height - 55.f);

		DrawText(BaseDec::renderer, font,
			"G gravity | UP/DOWN speed | D/F friction",
			10, BaseDec::height - 30.f);

		// draw drag line
		if (isDragging) {
			float mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			SDL_SetRenderDrawColor(BaseDec::renderer, 255, 255, 255, 255);
			SDL_RenderLine(BaseDec::renderer,
				dragStartX, dragStartY, mouseX, mouseY);
		}

		SDL_RenderPresent(BaseDec::renderer);
	}

	TTF_CloseFont(font);
	TTF_Quit();

	quit();

	return 0;
}