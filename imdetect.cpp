#define SDL_MAIN_HANDLED
#define NOMINMAX
#include <iostream>
#include <string>
#include <filesystem>
#include <cmath>
#include <vector>
#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(int argc, char* argv[]) {
    if(argc < 2) {
        puts("argument error");
        return 1;
    }
    if(!std::filesystem::exists(std::filesystem::path(argv[1]))){
        puts("file not found");
        return 1;
    }
    std::string command("rembg i ");
    command += std::string(argv[1]);
    command += " hoge.png";

    STARTUPINFO si = {sizeof(STARTUPINFO)};
    PROCESS_INFORMATION pi = {};
    char comm[260];
    strcpy(comm, command.c_str());
    if(!CreateProcess(NULL, comm, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){
        puts("execute rembg error");
        return 1;
    }
    CloseHandle(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window_handle = SDL_CreateWindow("image", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 540, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer_handle = SDL_CreateRenderer(window_handle, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_Surface* origin_sur = IMG_Load(argv[1]);
    SDL_Surface* removed_sur = IMG_Load("hoge.png");
    SDL_Texture* origin_tex = SDL_CreateTextureFromSurface(renderer_handle, origin_sur);
    SDL_Texture* removed_tex = SDL_CreateTextureFromSurface(renderer_handle, removed_sur);

    SDL_FreeSurface(origin_sur);
    SDL_FreeSurface(removed_sur);

    int w, h;
    SDL_QueryTexture(origin_tex, NULL, NULL, &w, &h);

    bool quit = false;

    SDL_Event poll_event;

    SDL_RenderSetLogicalSize(renderer_handle, w, h);
    SDL_SetRenderDrawBlendMode(renderer_handle, SDL_BLENDMODE_BLEND);

    SDL_Rect src_rect = {
        0,
        0,
        w,
        h,
    };

    SDL_Rect dst_rect = src_rect;

    bool downing = false;

    int pos_x = 0, pos_y = 0, moved_x = 0, moved_y = 0, native_frame = 0, frame_count = 0;

    while(!quit){
        while(SDL_PollEvent(&poll_event) != 0) {
            switch(poll_event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if(poll_event.button.button != SDL_BUTTON_LEFT)
                        break;
                    downing = true;
                    pos_x = poll_event.button.x;
                    pos_y = poll_event.button.y;
                    break;
                case SDL_MOUSEMOTION:
                    if(!downing)
                        break;
                    moved_x = poll_event.motion.x - pos_x;
                    moved_y = poll_event.motion.y - pos_y;
                    break;
                case SDL_MOUSEBUTTONUP:
                    if(poll_event.button.button != SDL_BUTTON_LEFT)
                        break;
                    downing = false;
                    pos_x = 0;
                    pos_y = 0;
                    moved_x = 0;
                    moved_y = 0;
                    frame_count = 0;
                    native_frame = 0;
                    break;
            }
        }
        SDL_SetRenderDrawColor(renderer_handle, 0, 0, 0, 255);
        SDL_RenderClear(renderer_handle);
        if(downing) {
            native_frame++;
            frame_count = std::max(0, native_frame - 30);
        }
        SDL_RenderCopy(renderer_handle, origin_tex, &src_rect, &src_rect);
        if(frame_count != 0){
            SDL_SetRenderDrawColor(renderer_handle, 0, 0, 0, std::min(frame_count, 10) * 5);
            SDL_RenderFillRect(renderer_handle, &src_rect);
            SDL_SetTextureAlphaMod(removed_tex, 255 - std::min(frame_count, 10) * 2);
            dst_rect.x = moved_x;
            dst_rect.y = moved_y;
            dst_rect.w = (src_rect.w * (std::min(frame_count, 10) * 0.001 + 1));
            dst_rect.h = (src_rect.h * (std::min(frame_count, 10) * 0.001 + 1));
            SDL_RenderCopy(renderer_handle, removed_tex, &src_rect, &dst_rect);
        }
        SDL_RenderPresent(renderer_handle);
        SDL_Delay(10);
    }

    SDL_DestroyTexture(origin_tex);
    SDL_DestroyTexture(removed_tex);

    SDL_DestroyRenderer(renderer_handle);
    SDL_DestroyWindow(window_handle);
    

    SDL_Quit();
    return 0;
}