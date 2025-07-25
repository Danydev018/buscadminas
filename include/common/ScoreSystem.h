#ifndef SCORE_SYSTEM_H  
#define SCORE_SYSTEM_H  
  
#include <string>  
#include <chrono>  
#include <fstream>  
#include <vector>  
  
struct GameScore {  
    int basePoints;  
    int timeBonus;  
    int efficiencyBonus;  
    int sizeMultiplier;  
    int totalScore;  
    double gameTimeSeconds;  
    int totalClicks;  
    int flagsUsed;  
    bool won;  
    std::string playerName;  
    std::string difficulty;  
    int boardRows;  
    int boardCols;  
    int totalMines;  
};  
  
class ScoreCalculator {  
public:  
    static void saveMultiplayerScoreToCSV(const GameScore& score, const std::string& playerName = "Player");  
    static std::vector<GameScore> loadMultiplayerScoresFromCSV();  
    static void displayMultiplayerHighScores();  
    static void displayScoreSubmenu();
    static GameScore calculateScore(int difficulty, int rows, int cols,   
                                  double gameTime, int clicks, int flags, bool won);  
    static void displayScore(const GameScore& score);  
    static void saveScoreToCSV(const GameScore& score, const std::string& playerName = "Player");  
    static std::vector<GameScore> loadScoresFromCSV();  
    static void displayHighScores();  
    static void displayMultiplayerResults(const GameScore& player1Score,   
                                              const GameScore& player2Score,  
                                              const std::string& player1Name,  
                                              const std::string& player2Name);
    static void displayLiveStats(int playerClicks, int playerFlags, double gameTime,   
                           int opponentClicks, int opponentFlags,  
                           const std::string& playerName, const std::string& opponentName,  
                           int boardRows);
    static void updateTimeOnly(double gameTime, int boardRows);
  
private:  
    static const std::string CSV_FILE;  
    static const std::string MULTIPLAYER_CSV_FILE;
    static std::string getDifficultyName(int difficulty);  
    static int calculateTimeBonus(double gameTime, int difficulty);  
    static int calculateEfficiencyBonus(int clicks, int totalCells, bool won);  
    static int calculateSizeMultiplier(int rows, int cols);  
    static void createCSVHeader();  
    static std::string escapeCSV(const std::string& field);  
};  
  
#endif // SCORE_SYSTEM_H