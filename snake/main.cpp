#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cassert>
#include <cstddef>
#include <format>
#include <iostream>
#include <memory>
#include <random>
#include <span>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

template <typename T>
T *SCP(T *ptr) {
    if (!ptr) {
        std::cerr << "SDL error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    return ptr;
}

template <typename T>
void SCV(T val) {
    if (val == 0) {
        std::cerr << "SDL error: " << SDL_GetError() << std::endl;
        exit(1);
    }
}

constexpr int FPS = 144;
constexpr bool LIMIT_FPS = true;
constexpr float FPS_DT = 1000.0f / FPS;

struct Vec2 {
    float x, y;

    Vec2 operator+(const Vec2 &other) const {
        return {x + other.x, y + other.y};
    }

    Vec2 operator-(const Vec2 &other) const {
        return {x - other.x, y - other.y};
    }

    Vec2 operator-() const { return {-x, -y}; }

    bool operator==(const Vec2 &other) const {
        return x == other.x && y == other.y;
    }

    Vec2 &operator+=(const Vec2 &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2 operator*(float n) const { return {x * n, y * n}; }

    [[nodiscard]] Vec2 floor_to(float n) const {
        return Vec2{.x = std::floor(x / n) * n, .y = std::floor(y / n) * n};
    }
};

class Game {
   public:
#define SPREAD_COLOUR(colour)                                           \
    (colour >> 24 & 0xFF), (colour >> 16) & 0xFF, (colour >> 8) & 0xFF, \
        (colour) & 0xFF
    enum class State { Start, Running, End };

    using WindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

    Game()
        // : gen(std::random_device{}()),
        : window_(
              SCP(SDL_CreateWindow("Snake", SCREEN_WIDTH, SCREEN_HEIGHT, 0)),
              &SDL_DestroyWindow),
          gen(10),
          dist_x(0, TILE_COUNT_X),
          dist_y(0, TILE_COUNT_Y) {
        state_ = State::Start;

        SCV(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));
        SCV(TTF_Init());

        renderer_ = SCP(SDL_CreateRenderer(window_.get(), nullptr));

        apple_texture_ = load_texture_from_image("./assets/apple.png");

        font_ = SCP(TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSans.ttf",
                                 HEIGHT_AVAILABLE));

        music_sound = load_sound_from_file("./assets/music.wav");
        SDL_SetAudioStreamGain(music_sound.stream, 0.04f);
        food_sound = load_sound_from_file("./assets/food.wav");
        SDL_SetAudioStreamGain(food_sound.stream, 0.1f);
        gameover_sound = load_sound_from_file("./assets/gameover.wav");
        SDL_SetAudioStreamGain(gameover_sound.stream, 0.1f);
        move_sound = load_sound_from_file("./assets/move.wav");
        SDL_SetAudioStreamGain(move_sound.stream, 0.1f);
    }

    ~Game() {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyTexture(apple_texture_);
        SDL_DestroyTexture(score_texture_);
        SDL_DestroyAudioStream(music_sound.stream);
        SDL_DestroyAudioStream(gameover_sound.stream);
        SDL_DestroyAudioStream(food_sound.stream);
        SDL_DestroyAudioStream(move_sound.stream);

        delete music_sound.audio_buf.data();
        delete gameover_sound.audio_buf.data();
        delete food_sound.audio_buf.data();
        delete move_sound.audio_buf.data();

        TTF_CloseFont(font_);
        TTF_Quit();
        SDL_Quit();
    }

    void init() {
        // SDL_ResumeAudioStreamDevice(music_sound.stream);
    }

    void update_render_game(float dt) {
        if (SDL_GetAudioStreamQueued(music_sound.stream) <
            (int)music_sound.audio_buf.size()) {
            SDL_PutAudioStreamData(music_sound.stream,
                                   music_sound.audio_buf.data(),
                                   (int)music_sound.audio_buf.size());
        }
        if (state_ == State::Start) {
            // click to start{
            {
                // background
                bool dark = true;
                for (unsigned int j = 0; j < TILE_COUNT_Y; j++) {
                    for (unsigned int i = 0; i < TILE_COUNT_X; i++) {
                        if (dark) {
                            SDL_SetRenderDrawColor(
                                renderer_, SPREAD_COLOUR(LIGHT_BACKGROUND));
                        } else {
                            SDL_SetRenderDrawColor(
                                renderer_, SPREAD_COLOUR(DARK_BACKGROUND));
                        }
                        dark = !dark;
                        SDL_FRect rect{.x = static_cast<float>(i * TILE_SIZE),
                                       .y = static_cast<float>(j * TILE_SIZE),
                                       .w = TILE_SIZE,
                                       .h = TILE_SIZE};
                        SDL_RenderFillRect(renderer_, &rect);
                    }
                    dark = !dark;
                }
                std::string start_message =
                    std::format("Press <Space> to start");
                int w, h;
                TTF_GetStringSizeWrapped(font_, start_message.c_str(), 0, 0, &w,
                                         &h);
                SDL_Surface *text_surface = SCP(TTF_RenderText_Blended_Wrapped(
                    font_, start_message.c_str(), 0, SDL_Color{0, 0, 0, 255},
                    0));
                SDL_Texture *texture =
                    SCP(SDL_CreateTextureFromSurface(renderer_, text_surface));
                SDL_DestroySurface(text_surface);
                SDL_FRect final_message_pos = {
                    .x = SCREEN_WIDTH / 2.0f - w / 2.0f,
                    .y = SCREEN_HEIGHT / 2.0f - h / 2.0f,
                    .w = (float)w,
                    .h = (float)h,
                };
                SDL_RenderTexture(renderer_, texture, NULL, &final_message_pos);
                SDL_DestroyTexture(texture);
            }
            SDL_RenderPresent(renderer_);
        } else if (state_ == State::Running) {
            //----------------update-----------------------------------------------

            // turn
            float dl = snake_speed_ * dt;
            if (turn_to_dir_.x != 0) {
                const float y = head_position_.y / TILE_SIZE + 0.5f;
                const float y_diff = y - std::floor(y);
                if (y_diff >= 0.4f && y_diff <= 0.6f) {
                    play_once_and_put_data(&move_sound);
                    head_position_.y = std::floor(y) * TILE_SIZE;
                    head_direction_ = turn_to_dir_;
                    turn_to_dir_ = {0, 0};
                    if (body_sizes_.size() != 0) {
                        body_sizes_.insert(body_sizes_.begin(),
                                           -head_direction_ * dl);
                        if (body_sizes_[1].x != 0) {
                            body_sizes_[1].x -=
                                dl * (body_sizes_[1].x > 0 ? 1 : -1);
                        } else {
                            body_sizes_[1].y -=
                                dl * (body_sizes_[1].y > 0 ? 1 : -1);
                        }
                    }
                }

            } else if (turn_to_dir_.y != 0) {
                const float x = head_position_.x / TILE_SIZE + 0.5f;
                const float x_diff = x - std::floor(x);
                if (x_diff >= 0.4f && x_diff <= 0.6f) {
                    play_once_and_put_data(&move_sound);
                    head_position_.x = std::floor(x) * TILE_SIZE;
                    head_direction_ = turn_to_dir_;
                    turn_to_dir_ = {0, 0};
                    if (body_sizes_.size() != 0) {
                        body_sizes_.insert(body_sizes_.begin(),
                                           -head_direction_ * dl);
                        if (body_sizes_[1].x != 0) {
                            body_sizes_[1].x -=
                                dl * (body_sizes_[1].x > 0 ? 1 : -1);
                        } else {
                            body_sizes_[1].y -=
                                dl * (body_sizes_[1].y > 0 ? 1 : -1);
                        }
                    }
                }
            }

            // update position
            head_position_.x += head_direction_.x * dl * TILE_SIZE;
            head_position_.y += head_direction_.y * dl * TILE_SIZE;
            {
                if (body_sizes_.size() >= 1) {
                    // attached to head grows
                    if (body_sizes_[0].x != 0) {
                        body_sizes_[0].x += (body_sizes_[0].x > 0) ? dl : -dl;
                    } else {
                        body_sizes_[0].y += (body_sizes_[0].y > 0) ? dl : -dl;
                    }

                    // everything else same size

                    // tail shortens
                    float tail_shrink = dl;
                    if (start_growing_) {
                        float growth = std::min(tail_shrink, growing_length_);
                        growing_length_ -= growth;
                        tail_shrink -= growth;
                        if (growing_length_ <= 0.0f) {
                            start_growing_ = false;
                            growing_length_ = 0.0f;
                        }
                    }

                    while (tail_shrink > 0 && !body_sizes_.empty()) {
                        Vec2 &last_size = body_sizes_.back();
                        float &len =
                            (last_size.x != 0) ? last_size.x : last_size.y;
                        float abs_len = std::abs(len);
                        if (body_sizes_.size() > 1 && tail_shrink >= abs_len) {
                            tail_shrink -= abs_len;
                            body_sizes_.pop_back();
                        } else {
                            len -= (len > 0) ? tail_shrink : -tail_shrink;
                            tail_shrink = 0;
                        }
                    }
                }
            }

            // apple collision
            if (apple_present_) {
                if (std::abs(head_position_.x - apple_position_.x) <
                        TILE_SIZE / 2.0f &&
                    std::abs(head_position_.y - apple_position_.y) <
                        TILE_SIZE / 2.0f) {
                    play_once_and_put_data(&food_sound);
                    score_++;
                    score_changed_ = true;
                    apple_present_ = false;
                    time_to_apple_ = TIME_TO_APPLE;
                    start_growing_ = true;
                    growing_length_ += 1.0f;
                    if (score_ <= 100) {
                        snake_speed_ +=
                            (MAX_SNAKE_SPEED - MIN_SNAKE_SPEED) / 100;
                    }
                }
            }

            // collision
            if (head_position_.x < 0 ||
                head_position_.x > (SCREEN_WIDTH - TILE_SIZE) ||
                head_position_.y < 0 ||
                head_position_.y > (SCREEN_HEIGHT - TILE_SIZE)) {
                head_position_.x = std::min(std::max(head_position_.x, 0.0f),
                                            (float)(SCREEN_WIDTH - TILE_SIZE));
                head_position_.y = std::min(std::max(head_position_.y, 0.0f),
                                            (float)(SCREEN_HEIGHT - TILE_SIZE));
                state_ = State::End;
                turn_to_dir_ = {0, 0};
                play_once_and_put_data(&gameover_sound);
            }

            //----------------render-----------------------------------------------

            {
                // background
                bool dark = true;
                for (unsigned int j = 0; j < TILE_COUNT_Y; j++) {
                    for (unsigned int i = 0; i < TILE_COUNT_X; i++) {
                        if (dark) {
                            SDL_SetRenderDrawColor(
                                renderer_, SPREAD_COLOUR(LIGHT_BACKGROUND));
                        } else {
                            SDL_SetRenderDrawColor(
                                renderer_, SPREAD_COLOUR(DARK_BACKGROUND));
                        }
                        dark = !dark;
                        SDL_FRect rect{.x = static_cast<float>(i * TILE_SIZE),
                                       .y = static_cast<float>(j * TILE_SIZE),
                                       .w = TILE_SIZE,
                                       .h = TILE_SIZE};
                        SDL_RenderFillRect(renderer_, &rect);
                    }
                    dark = !dark;
                }
            }

            {
                // random apple
                if (apple_present_) {
                    SDL_RenderTexture(renderer_, apple_texture_, NULL,
                                      &apple_position_);
                } else {
                    time_to_apple_ -= dt;
                    if (time_to_apple_ <= 0) {
                        choose_random_apple_pos();
                        apple_present_ = true;
                    }
                }
            }

            {
                // snake
                SDL_FRect head_rect = {
                    .x = head_position_.x,
                    .y = head_position_.y,
                    .w = TILE_SIZE,
                    .h = TILE_SIZE,
                };
                SDL_SetRenderDrawColor(renderer_, 91, 123, 249, 255);
                SDL_RenderFillRect(renderer_, &head_rect);
                Vec2 start = {
                    head_position_.x +
                        (head_direction_.x < 0 ? (float)TILE_SIZE : 0.0f),
                    head_position_.y +
                        (head_direction_.y < 0 ? (float)TILE_SIZE : 0.0f)};

                SDL_FRect rect;
                Vec2 p_start = {
                    head_position_.x +
                        (head_direction_.x == 0
                             ? TILE_SIZE / 2.0f
                             : (head_direction_.x > 0 ? (float)TILE_SIZE
                                                      : 0.0f)),
                    head_position_.y +
                        (head_direction_.y == 0
                             ? TILE_SIZE / 2.0f
                             : (head_direction_.y > 0 ? (float)TILE_SIZE
                                                      : 0.0f))};
                for (size_t i = 0; i < body_sizes_.size(); ++i) {
                    SDL_SetRenderDrawColor(renderer_, 91, 123, 249, 255);
                    Vec2 b = body_sizes_[i];
                    Vec2 end = start + b * TILE_SIZE;
                    if (b.x != 0) {
                        rect = {
                            .x = std::min(start.x, end.x),
                            .y = start.y,
                            .w = std::abs(end.x - start.x),
                            .h = TILE_SIZE,
                        };
                    } else {
                        rect = {
                            .x = start.x,
                            .y = std::min(start.y, end.y),
                            .w = TILE_SIZE,
                            .h = std::abs(end.y - start.y),
                        };
                    }
                    SDL_RenderFillRect(renderer_, &rect);

                    Vec2 p_end;
                    if (i + 1 < body_sizes_.size()) {
                        Vec2 next_b = body_sizes_[i + 1];
                        Vec2 corner = {
                            (b.x != 0)
                                ? (b.x > 0 ? end.x - (float)TILE_SIZE : end.x)
                                : start.x,
                            (b.y != 0)
                                ? (b.y > 0 ? end.y - (float)TILE_SIZE : end.y)
                                : start.y};
                        p_end = {corner.x + TILE_SIZE / 2.0f,
                                 corner.y + TILE_SIZE / 2.0f};
                        start = {
                            corner.x + (next_b.x > 0 ? (float)TILE_SIZE : 0.0f),
                            corner.y +
                                (next_b.y > 0 ? (float)TILE_SIZE : 0.0f)};
                    } else {
                        p_end = (b.x != 0) ? Vec2{end.x, p_start.y}
                                           : Vec2{p_start.x, end.y};
                        start = end;
                    }

                    {
                        SDL_FRect highlight;
                        if (b.x != 0) {
                            highlight = {
                                .x = std::min(p_start.x, p_end.x),
                                .y = p_start.y - 2.5f,
                                .w = std::abs(p_end.x - p_start.x),
                                .h = 5.0f,
                            };
                        } else {
                            highlight = {
                                .x = p_start.x - 2.5f,
                                .y = std::min(p_start.y, p_end.y),
                                .w = 5.0f,
                                .h = std::abs(p_end.y - p_start.y),
                            };
                        }
                        SDL_SetRenderDrawColor(renderer_, 119, 146, 255, 255);
                        SDL_RenderFillRect(renderer_, &highlight);
                    }
                    p_start = p_end;

                    if (i >= 3 && check_rect_collision(head_rect, rect)) {
                        state_ = State::End;
                        play_once_and_put_data(&gameover_sound);
                        break;
                    }
                }
            }

            {
                // score
                SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 1);
                SDL_RenderRect(renderer_, &SCORE_BOX);
                SDL_RenderTexture(renderer_, apple_texture_, NULL,
                                  &SCORE_BOX_APPLE);
                if (score_changed_) {
                    std::string score = std::to_string(score_);
                    int w, h;
                    TTF_GetStringSize(font_, score.c_str(), 0, &w, &h);

                    float wf = std::min((float)w, WIDTH_AVAILABLE);
                    float hf = wf * (float)h / (float)w;
                    score_box_score_ = {
                        .x = SCORE_CENTER_X - wf / 2,
                        .y = SCORE_CENTER_Y - hf / 2,
                        .w = wf,
                        .h = hf,
                    };

                    SDL_Surface *text_surface = SCP(TTF_RenderText_Blended(
                        font_, score.c_str(), 0, SDL_Color{0, 0, 0, 255}));
                    score_texture_ = SCP(
                        SDL_CreateTextureFromSurface(renderer_, text_surface));
                    SDL_DestroySurface(text_surface);
                    SDL_RenderTexture(renderer_, score_texture_, NULL,
                                      &score_box_score_);
                    score_changed_ = false;
                } else {
                    SDL_RenderTexture(renderer_, score_texture_, NULL,
                                      &score_box_score_);
                }
            }

            SDL_RenderPresent(renderer_);
        } else if (state_ == State::End) {
            // just show a popup in the middle of screen
            // NOTE: do something better then create and destroy textures each
            // time
            std::string final_message = "Game Over";
            std::string score_message = std::format("Score: {}", score_);
            std::string restart_message =
                std::format("Press <Space> to restart");

            int w, h;
            TTF_GetStringSizeWrapped(font_, final_message.c_str(), 0, 0, &w,
                                     &h);
            SDL_Surface *text_surface = SCP(TTF_RenderText_Blended_Wrapped(
                font_, final_message.c_str(), 0, SDL_Color{0, 0, 0, 255}, 0));
            SDL_Texture *texture =
                SCP(SDL_CreateTextureFromSurface(renderer_, text_surface));
            SDL_DestroySurface(text_surface);
            SDL_FRect final_message_pos = {
                .x = SCREEN_WIDTH / 2.0f - w / 2.0f,
                .y = SCREEN_HEIGHT / 2.0f - 3.0f * h,
                .w = (float)w,
                .h = (float)h,
            };
            SDL_RenderTexture(renderer_, texture, NULL, &final_message_pos);
            SDL_DestroyTexture(texture);

            TTF_GetStringSizeWrapped(font_, score_message.c_str(), 0, 0, &w,
                                     &h);
            text_surface = SCP(TTF_RenderText_Blended_Wrapped(
                font_, score_message.c_str(), 0, SDL_Color{0, 0, 0, 255}, 0));
            texture =
                SCP(SDL_CreateTextureFromSurface(renderer_, text_surface));
            SDL_DestroySurface(text_surface);
            final_message_pos = {
                .x = SCREEN_WIDTH / 2.0f - (float)w / 2.0f,
                .y = SCREEN_HEIGHT / 2.0f - (float)h,
                .w = (float)w,
                .h = (float)h,
            };
            SDL_RenderTexture(renderer_, texture, NULL, &final_message_pos);
            SDL_DestroyTexture(texture);

            TTF_GetStringSizeWrapped(font_, restart_message.c_str(), 0, 0, &w,
                                     &h);
            w *= 3 / 2;
            h *= 3 / 2;
            text_surface = SCP(TTF_RenderText_Blended_Wrapped(
                font_, restart_message.c_str(), 0, SDL_Color{0, 0, 0, 255}, 0));
            texture =
                SCP(SDL_CreateTextureFromSurface(renderer_, text_surface));
            SDL_DestroySurface(text_surface);
            final_message_pos = {
                .x = SCREEN_WIDTH / 2.0f - (float)w / 2.0f,
                .y = SCREEN_HEIGHT / 2.0f + (float)h,
                .w = (float)w,
                .h = (float)h,
            };
            SDL_RenderTexture(renderer_, texture, NULL, &final_message_pos);
            SDL_DestroyTexture(texture);

            SDL_RenderPresent(renderer_);
        }
    }

    void re_start() {
        if (state_ == State::Running) return;
        state_ = State::Running;
        score_ = 0;

        choose_random_apple_pos();
        apple_present_ = true;
        start_growing_ = false;
        growing_length_ = 0;

        head_position_ = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
        head_position_ = head_position_.floor_to(TILE_SIZE);
        head_direction_ = {0, -1};
        turn_to_dir_ = {0, 0};

        body_sizes_.resize(0);
        body_sizes_.push_back({0, 1});
    }

    void receive_dir(Vec2 dir) {
        if (state_ != State::Running) return;
        if (head_direction_ == dir) return;
        if (head_direction_.x != 0 && dir.x != 0) return;
        if (head_direction_.y != 0 && dir.y != 0) return;
        turn_to_dir_ = dir;
    }

   private:
    struct Sound {
        std::span<Uint8> audio_buf;
        SDL_AudioStream *stream;
    };

    SDL_Texture *load_texture_from_image(const std::string_view filepath) {
        SDL_Texture *texture;
        int width, height, channels;
        unsigned char *pixels = stbi_load(filepath.data(), &width, &height,
                                          &channels, STBI_rgb_alpha);

        if (!pixels) {
            // load default texture
            SDL_Log("STBI failed to load image %s: %s", filepath.data(),
                    stbi_failure_reason());
            return nullptr;
        }

        SDL_Surface *surface = SDL_CreateSurfaceFrom(
            width, height, SDL_PIXELFORMAT_RGBA32, pixels, width * 4);
        texture = SCP(SDL_CreateTextureFromSurface(renderer_, surface));
        SDL_DestroySurface(surface);
        stbi_image_free(pixels);
        return texture;
    }

    Sound load_sound_from_file(const std::string_view filepath) {
        Sound sound;
        SDL_AudioSpec spec;
        Uint8 *audio_buf;
        Uint32 audio_len;
        SCV(SDL_LoadWAV(filepath.data(), &spec, &audio_buf, &audio_len));
        sound.stream = SCP(SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL));
        sound.audio_buf = std::span(audio_buf, audio_len);
        SDL_PutAudioStreamData(sound.stream, sound.audio_buf.data(),
                               (int)sound.audio_buf.size());
        return sound;
    }

    void play_once_and_put_data(Sound *sound) {
        SDL_ResumeAudioStreamDevice(sound->stream);
        if (SDL_GetAudioStreamAvailable(sound->stream) <
            (int)sound->audio_buf.size()) {
            SDL_PutAudioStreamData(sound->stream, sound->audio_buf.data(),
                                   (int)sound->audio_buf.size());
        }
    }

    void choose_random_apple_pos() {
        apple_position_ = {
            .x = (float)dist_x(gen) * TILE_SIZE + TILE_SIZE * 0.05f,
            .y = (float)dist_y(gen) * TILE_SIZE + TILE_SIZE * 0.05f,
            .w = TILE_SIZE * 0.9,
            .h = TILE_SIZE * 0.9,
        };

        // don't be around the score box
        if (apple_position_.x >= SCORE_BOX.x ||
            apple_position_.y >= SCORE_BOX.y) {
            choose_random_apple_pos();
        }
    }

    bool check_rect_collision(const SDL_FRect &a, const SDL_FRect &b) {
        constexpr float INSET = 2.0f;  // prevents edge-touching false positives
        return (a.x + INSET < b.x + b.w && a.x + a.w - INSET > b.x &&
                a.y + INSET < b.y + b.h && a.y + a.h - INSET > b.y);
    }

    static constexpr unsigned int SCREEN_WIDTH = 800;
    static constexpr unsigned int SCREEN_HEIGHT = 800;
    static constexpr unsigned int TILE_SIZE = SCREEN_HEIGHT / 20;
    static_assert(SCREEN_HEIGHT % TILE_SIZE == 0,
                  "SCREEN_HEIGHT must be divisible by TILE_SIZE");
    static_assert(SCREEN_WIDTH % TILE_SIZE == 0,
                  "SCREEN_WIDTH must be divisible by TILE_SIZE");
    static constexpr int TILE_COUNT_X = SCREEN_WIDTH / TILE_SIZE;
    static constexpr int TILE_COUNT_Y = SCREEN_HEIGHT / TILE_SIZE;
    static constexpr unsigned int DARK_BACKGROUND = 0xA5D03B00;
    static constexpr unsigned int LIGHT_BACKGROUND = 0xACD64300;

    State state_;

    unsigned int score_ = 0;
    bool score_changed_ = true;
    SDL_Texture *score_texture_;
    static constexpr SDL_FRect SCORE_BOX = {
        .x = 0.90 * SCREEN_WIDTH,
        .y = 0.94 * SCREEN_HEIGHT,
        .w = 0.09 * SCREEN_WIDTH,
        .h = 0.05 * SCREEN_HEIGHT,
    };
    static constexpr SDL_FRect SCORE_BOX_APPLE = {
        .x = SCORE_BOX.x + 2.5f,
        .y = SCORE_BOX.y + 5,
        .w = SCORE_BOX.h - 10,
        .h = SCORE_BOX.h - 10,
    };
    static constexpr float SCORE_CENTER_X = SCORE_BOX.x + 3 * SCORE_BOX.w / 4;
    static constexpr float SCORE_CENTER_Y = SCORE_BOX.y + SCORE_BOX.h / 2;
    static constexpr float WIDTH_AVAILABLE = SCORE_BOX.w / 2 - 5;
    static constexpr float HEIGHT_AVAILABLE = SCORE_BOX.h - 10;
    SDL_FRect score_box_score_;

    Vec2 head_position_;
    Vec2 head_direction_;
    Vec2 turn_to_dir_ =
        {};  // turn to this dir (based on key) at some later frame
    std::vector<Vec2> body_sizes_ = {};
    static constexpr float MIN_SNAKE_SPEED = 3 / 1000.0f;
    static constexpr float MAX_SNAKE_SPEED = 10 / 1000.0f;
    float snake_speed_ = MIN_SNAKE_SPEED;  // snake_speed is units of tile

    bool apple_present_ = false;
    static constexpr float TIME_TO_APPLE = 1000.0f;
    float time_to_apple_ = TIME_TO_APPLE;
    SDL_FRect apple_position_ = {};  // should be always valid in render_game
    bool start_growing_ = false;
    float growing_length_ = 0;  // 0 to tile_size

    WindowPtr window_;
    SDL_Renderer *renderer_;
    SDL_Texture *apple_texture_;

    TTF_Font *font_;

    Sound music_sound;
    Sound food_sound;
    Sound move_sound;
    Sound gameover_sound;

    std::mt19937 gen;
    std::uniform_int_distribution<int> dist_x;
    std::uniform_int_distribution<int> dist_y;
};

int main() {
    // for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
    //     std::cout << SDL_GetRenderDriver(i)<<std::endl;
    // }
    //
    // std::cout<<SDL_GetRendererProperties(renderer)<<std::endl;
    // std::cout<<SDL_GetRendererName(renderer)<<std::endl;

    Game game{};

    const uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t start;
    float dt = 0.0;
    bool quit = false;
    bool is_paused = false;
    SDL_Event ev;
    game.init();
    while (!quit) {
        start = SDL_GetPerformanceCounter();
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch (ev.key.key) {
                        case SDLK_W:
                        case SDLK_UP:
                            game.receive_dir(Vec2{0, -1});
                            break;
                        case SDLK_A:
                        case SDLK_LEFT:
                            game.receive_dir(Vec2{-1, 0});
                            break;
                        case SDLK_S:
                        case SDLK_DOWN:
                            game.receive_dir(Vec2{0, 1});
                            break;
                        case SDLK_D:
                        case SDLK_RIGHT:
                            game.receive_dir(Vec2{1, 0});
                            break;

                        case SDLK_SPACE:
                            game.re_start();
                            break;

                        case SDLK_P:
                            is_paused = !is_paused;
                            break;
                    }
                    break;
            }
        }

        if (!is_paused) {
            game.update_render_game(dt);
        }

        uint64_t end = SDL_GetPerformanceCounter();
        dt = 1000.0f * (float)(end - start) / (float)freq;
        if (LIMIT_FPS) {
            if (dt <= FPS_DT) {
                SDL_Delay(static_cast<uint32_t>(FPS_DT - dt));
                dt = FPS_DT;
            }
        }
    }

    return 0;
}
