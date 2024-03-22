//
// Created by jewoo on 2024-03-22.
//

#include <random>
#include <sstream>
#include <iostream>
#include <queue>
#include <chrono>
#include <algorithm>

namespace BeamSearchWithTime {
    using ScoreType = int64_t;

    constexpr const ScoreType INF = 1000000000LL;

    struct Coord {
        int y_;
        int x_;

        explicit Coord(const int x = 0, const int y = 0) : y_(y), x_(x) {}
    };

    constexpr const int H = 30;
    constexpr const int W = 30;
    constexpr int END_TURN = 10;

    class MazeState {
    private:
        int turn_{0};
        int points_[H][W] = {};
        static constexpr const int dx[4] = {1, -1, 0, 0};
        static constexpr const int dy[4] = {0, 0, 1, -1};

    public:
        Coord character_ = Coord();
        ScoreType evaluated_score_{0};
        int game_score_{0};
        int first_action_{-1};

        MazeState() = default;

        explicit MazeState(const int seed) {
            auto mt_for_construct = std::mt19937(seed);
            this->character_.y_ = mt_for_construct() % H;
            this->character_.x_ = mt_for_construct() % W;

            for (int y = 0; y < H; ++y) {
                for (int x = 0; x < W; ++x) {
                    if (y == character_.y_ && x == character_.x_) {
                        continue;
                    }
                    this->points_[y][x] = mt_for_construct() % 10;

                }

            }
        }


        bool isDone() const {
            return this->turn_ == END_TURN;
        }

        void advance(const int action) {
            this->character_.x_ += dx[action];
            this->character_.y_ += dy[action];
            auto &point = this->points_[this->character_.y_][this->character_.x_];
            if (point > 0) {
                this->game_score_ += point;
                point = 0;
            }
            this->turn_++;
        }

        [[nodiscard]] std::vector<int> legalActions() const {
            std::vector<int> actions;
            for (int action = 0; action < 4; ++action) {
                const int ny = this->character_.y_ + dy[action];
                const int nx = this->character_.x_ + dx[action];
                if (ny >= 0 && ny < H && nx >= 0 && nx < W) {
                    actions.emplace_back(action);
                }
            }
            return actions;
        }

        [[nodiscard]] std::string toString() const {
            std::stringstream ss;
            ss << "turn:\t" << this->turn_ << "\n";
            ss << "score:\t" << this->game_score_ << "\n";
            for (int h = 0; h < H; ++h) {
                for (int w = 0; w < W; ++w) {
                    if (this->character_.y_ == h && this->character_.x_ == w) {
                        ss << "@";
                    } else if (this->points_[h][w] > 0) {
                        ss << points_[h][w];
                    } else {
                        ss << ".";
                    }
                }

                ss << "\n";
            }
            return ss.str();
        }

        void evaluateScore() {
            this->evaluated_score_ = this->game_score_;
        }
    };

    using State = MazeState;
    std::mt19937 mt_for_action(0);

    int randomAction(const State &state) {
        const auto legal_actions = state.legalActions();
        return legal_actions[mt_for_action() % legal_actions.size()];
    }

    int greedyAction(const State &state) {
        auto legal_actions = state.legalActions();
        ScoreType best_score = -INF;
        int best_action = -1;
        for (const auto action: legal_actions) {
            State now_state = state;
            now_state.advance(action);
            now_state.evaluateScore();
            if (now_state.evaluated_score_ > best_score) {
                best_score = now_state.evaluated_score_;
                best_action = action;
            }
        }
        return best_action;
    }

    void playGame(const int seed) {
        auto state = State(seed);
        while (!state.isDone()) {
            std::cout << state.toString() << std::endl;
            const auto action = randomAction(state);
            state.advance(action);
        }
        std::cout << state.toString() << std::endl;

        while (!state.isDone()) {
            state.advance(greedyAction(state));
            std::cout << state.toString() << std::endl;
        }
    }

    bool operator<(const MazeState &maze_1, const MazeState &maze_2) {
        return maze_1.evaluated_score_ < maze_2.evaluated_score_;
    }

    int beamSearchAction(const State &state, const int beam_width,
                         const int beam_depth) {
        std::priority_queue<State> now_beam;
        State best_state;
        now_beam.push(state);

        for (int t = 0; t < beam_depth; ++t) {
            std::priority_queue<State> next_beam;
            for (int i = 0; i < beam_width; ++i) {
                if (now_beam.empty()) {
                    break;
                }
                State now_state = now_beam.top();
                now_beam.pop();
                const auto legal_actions = now_state.legalActions();
                for (const auto action: legal_actions) {
                    State next_state = now_state;
                    next_state.advance(action);
                    next_state.evaluateScore();
                    if (t == 0)
                        next_state.first_action_ = action;
                    next_beam.push(next_state);
                }
            }
            now_beam = next_beam;
            best_state = now_beam.top();

            if (best_state.isDone()) {
                break;
            }
        }
        return best_state.first_action_;
    }

    class TimeKeeper {
    private:
        std::chrono::high_resolution_clock::time_point start_time_;
        int64_t time_threshold_;

    public:
        TimeKeeper(const int64_t &time_threshold) : start_time_(
                std::chrono::high_resolution_clock::now()), time_threshold_(time_threshold) {}

        bool isTimeOver() const {
            using std::chrono::duration_cast;
            using std::chrono::milliseconds;
            auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
            return duration_cast<milliseconds>(diff).count() >= time_threshold_;
        }

    };

    int beamSearchActionWithTimeThreshold(
            const State &state,
            const int beam_width,
            const int64_t time_threshold
    ) {
        std::priority_queue<State> now_beam;
        State best_state;
        now_beam.push(state);

        auto time_keeper = TimeKeeper(time_threshold);

        for (int t = 0;; ++t) {
            std::priority_queue<State> next_beam;
            for (int i = 0; i < beam_width; ++i) {
                if (time_keeper.isTimeOver()) {
                    return best_state.first_action_;
                }
                if (now_beam.empty()) {
                    break;
                }
                State now_state = now_beam.top();
                now_beam.pop();
                const auto legal_actions = now_state.legalActions();
                for (const auto action: legal_actions) {
                    State next_state = now_state;
                    next_state.advance(action);
                    next_state.evaluateScore();
                    if (t == 0)
                        next_state.first_action_ = action;
                    next_beam.push(next_state);
                }
            }
            now_beam = next_beam;
            best_state = now_beam.top();

            if (best_state.isDone()) {
                break;
            }
        }
        return best_state.first_action_;
    }

    int beamSearchActionByNthElement(const State &state, const int beam_width,
                                     const int beam_depth) {
        std::vector<State> now_beam;
        State best_state;
        now_beam.push_back(state);
        for (int t = 0; t < beam_depth; ++t) {
            std::vector<State> next_beam;
            for (const State &now_state: now_beam) {
                auto legal_actions = now_state.legalActions();
                for (const auto &action: legal_actions) {
                    State next_state = now_state;
                    next_state.advance(action);
                    next_state.evaluateScore();
                    if (t == 0)
                        next_state.first_action_ = action;
                    next_beam.emplace_back(next_state);
                }
            }
            if (next_beam.size() > beam_width) {
                std::nth_element(next_beam.begin(), next_beam.begin() + beam_width,
                                 next_beam.end(),  [](const State &a, const State &b) {
                            return a.evaluated_score_ > b.evaluated_score_;
                        });
                next_beam.resize(beam_width);
            }
            now_beam = next_beam;
            if (now_beam[0].isDone()) {
                break;
            }
        }
        for (const State &now_state: now_beam) {
            if (now_state.evaluated_score_ > best_state.evaluated_score_) {
                best_state = now_state;
            }
        }
        return best_state.first_action_;
    }

    void testAiScore(const int game_number) {
        using std::cout;
        using std::endl;

        std::mt19937 mt_for_construct(0);
        double score_mean{0};

        for (int i = 0; i < game_number; ++i) {
            auto state = State(mt_for_construct());

            while (!state.isDone()) {
//                state.advance(greedyAction(state));
                state.advance(beamSearchActionWithTimeThreshold(state, 5, 10));
            }
            auto score = state.game_score_;
            score_mean += score;
        }
        score_mean /= (double) game_number;
        cout << "mean score: " << score_mean << endl;
    }
}