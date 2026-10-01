#include <iostream>
#include <memory>

#include <SDL3/SDL.h>    

#include "AudioEngine.h"
#include "EditorUI.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

int main()
{
    if (!SDL_Init(SDL_INIT_AUDIO) || !SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
        return -1;
    }

    try
    {
        SDL_Window* window = SDL_CreateWindow(
            "Audio Engine Editor",
            1200,
            800,
            SDL_WINDOW_RESIZABLE
        );
        if (!window)
        {
			std::cerr << "Window creation failed: " << SDL_GetError() << '\n';
            SDL_Quit();
            return -1;
        }

		SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer)
        {
			std::cerr << "Renderer creation failed: " << SDL_GetError() << '\n';
            SDL_DestroyWindow(window);
            SDL_Quit();
			return -1;
        }

        IMGUI_CHECKVERSION();
		ImGui::CreateContext();

        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
		ImGui_ImplSDLRenderer3_Init(renderer);


		AudioEngine audioEngine;
        if (!audioEngine.initialize())
        {
            std::cerr << "Failed to initialize Audio Engine\n";
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
		}

        EditorUI editor(audioEngine);

		bool running = true;

		// MAIN LOOP //

        while (running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
				ImGui_ImplSDL3_ProcessEvent(&event);

                if (event.type == SDL_EVENT_QUIT)
                {
                    running = false;
                }
            }
            
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

			editor.render();

			ImGui::Render();

            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderClear(renderer);

			ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

			SDL_RenderPresent(renderer);
		}

		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();

		ImGui::DestroyContext();

        audioEngine.shutdown();

		SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        SDL_Quit();

        return 1;
    }

	catch (...)
    {
        std::cerr << "Unknown error occurred\n";
        SDL_Quit();

        return 1;
    }

    SDL_Quit();

    return 0;
}