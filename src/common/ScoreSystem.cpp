#include "common/ScoreSystem.h"  
#include "common/ConsoleUtils.h"  
#include <iostream>  
#include <iomanip>  
#include <algorithm>  
#include <sstream>  
#include <limits>  
  
const std::string ScoreCalculator::CSV_FILE = "buscaminas_scores.csv";
const std::string ScoreCalculator::MULTIPLAYER_CSV_FILE = "buscaminas_multiplayer_scores.csv";
void ScoreCalculator::createCSVHeader() {  
    std::ifstream checkFile(CSV_FILE);  
    if (!checkFile.good()) {  
        std::ofstream file(CSV_FILE);  
        if (file.is_open()) {  
            file << "PlayerName,TotalScore,Difficulty,GameTime,BoardSize,TotalMines,TotalClicks,BasePoints,TimeBonus,EfficiencyBonus,SizeMultiplier\n";  
            file.close();  
        }  
    }  
    checkFile.close();  
} 

  
std::string ScoreCalculator::escapeCSV(const std::string& field) {  
    if (field.find(',') != std::string::npos || field.find('"') != std::string::npos) {  
        std::string escaped = "\"";  
        for (char c : field) {  
            if (c == '"') escaped += "\"\"";  
            else escaped += c;  
        }  
        escaped += "\"";  
        return escaped;  
    }  
    return field;  
} 
  
void ScoreCalculator::displayScore(const GameScore& score) {  
    gotoxy(2, score.boardRows + 12);  
    std::cout << "\033[96m╔══════════════════════════════════════════════════╗\033[0m\n";  
    gotoxy(2, score.boardRows + 13);  
    std::cout << "\033[96m║                   PUNTUACIÓN                     ║\033[0m\n";  
    gotoxy(2, score.boardRows + 14);  
    std::cout << "\033[96m╠══════════════════════════════════════════════════╣\033[0m\n";  
      
    gotoxy(2, score.boardRows + 15);  
    std::cout << "\033[96m║\033[0m \033[97mResultado:\033[0m " << (score.won ? "\033[92mVICTORIA!\033[0m" : "\033[91mDerrota\033[0m");  
    std::cout << std::string(28 - (score.won ? 9 : 7), ' ') << "          \033[96m║\033[0m\n";  
      
    gotoxy(2, score.boardRows + 16);  
    std::cout << "\033[96m║\033[0m \033[97mDificultad:\033[0m \033[93m" << score.difficulty << "\033[0m";  
    std::cout << std::string(32 - score.difficulty.length(), ' ') << "      \033[96m║\033[0m\n";  
      
    gotoxy(2, score.boardRows + 17);  
    std::cout << "\033[96m║\033[0m \033[97mTiempo:\033[0m \033[94m" << std::fixed << std::setprecision(1) << score.gameTimeSeconds << "s\033[0m";  
    std::cout << std::string(35 - std::to_string((int)score.gameTimeSeconds).length(), ' ') << "   \033[96m║\033[0m\n";  
      
    gotoxy(2, score.boardRows + 18);  
    std::cout << "\033[96m║\033[0m \033[97mClics totales:\033[0m \033[94m" << score.totalClicks << "\033[0m";  
    std::cout << std::string(29 - std::to_string(score.totalClicks).length(), ' ') << "     \033[96m║\033[0m\n";  
      
    if (score.won) {  
        gotoxy(2, score.boardRows + 19);  
        std::cout << "\033[96m╠══════════════════════════════════════════════════╣\033[0m\n";  
          
        gotoxy(2, score.boardRows + 20);  
        std::cout << "\033[96m║\033[0m \033[97mPuntos base:\033[0m \033[92m+" << score.basePoints << "\033[0m";  
        std::cout << std::string(32 - std::to_string(score.basePoints).length(), ' ') << "   \033[96m║\033[0m\n";  
          
        gotoxy(2, score.boardRows + 21);  
        std::cout << "\033[96m║\033[0m \033[97mBonus tiempo:\033[0m \033[92m+" << score.timeBonus << "\033[0m";  
        std::cout << std::string(31 - std::to_string(score.timeBonus).length(), ' ') << "   \033[96m║\033[0m\n";  
          
        gotoxy(2, score.boardRows + 22);  
        std::cout << "\033[96m║\033[0m \033[97mBonus eficiencia:\033[0m \033[92m+" << score.efficiencyBonus << "\033[0m";  
        std::cout << std::string(27 - std::to_string(score.efficiencyBonus).length(), ' ') << "   \033[96m║\033[0m\n";  
          
        gotoxy(2, score.boardRows + 23);  
        std::cout << "\033[96m║\033[0m \033[97mMultiplicador:\033[0m \033[92mx" << score.sizeMultiplier << "\033[0m";  
        std::cout << std::string(31 - std::to_string(score.sizeMultiplier).length(), ' ') << "  \033[96m║\033[0m\n";  
    }  
      
    gotoxy(2, score.boardRows + (score.won ? 24 : 19));  
    std::cout << "\033[96m╠══════════════════════════════════════════════════╣\033[0m\n";  
      
    gotoxy(2, score.boardRows + (score.won ? 25 : 20));  
    std::cout << "\033[96m║\033[0m \033[97mPUNTUACIÓN TOTAL:\033[0m \033[93m" << score.totalScore << "\033[0m";  
    std::cout << std::string(29 - std::to_string(score.totalScore).length(), ' ') << "  \033[96m║\033[0m\n";  
      
    gotoxy(2, score.boardRows + (score.won ? 26 : 21));  
    std::cout << "\033[96m╚══════════════════════════════════════════════════╝\033[0m\n";  
}  
  

void ScoreCalculator::saveScoreToCSV(const GameScore& score, const std::string& playerName) {  
    if (!score.won) return; // Solo guardar puntuaciones ganadoras  
      
    createCSVHeader();  
      
    std::ofstream file(CSV_FILE, std::ios::app);  
    if (file.is_open()) {  
        file << escapeCSV(playerName) << ","  
             << score.totalScore << ","  
             << escapeCSV(score.difficulty) << ","  
             << std::fixed << std::setprecision(2) << score.gameTimeSeconds << ","  
             << score.boardRows << "x" << score.boardCols << ","  
             << score.totalMines << ","  
             << score.totalClicks << ","  
             << score.basePoints << ","  
             << score.timeBonus << ","  
             << score.efficiencyBonus << ","  
             << score.sizeMultiplier << "\n";  
        file.close();  
    }  
}  
  
std::vector<GameScore> ScoreCalculator::loadScoresFromCSV() {  
    std::vector<GameScore> scores;  
    std::ifstream file(CSV_FILE);  
    std::string line;  
      
    // Saltar header  
    if (std::getline(file, line)) {  
        while (std::getline(file, line)) {  
            std::stringstream ss(line);  
            std::string token;  
            GameScore score;  
              
            // Parse CSV fields  
            if (std::getline(ss, token, ',')) score.playerName = token;  
            if (std::getline(ss, token, ',')) score.totalScore = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.difficulty = token;  
            if (std::getline(ss, token, ',')) score.gameTimeSeconds = std::stod(token);  
            if (std::getline(ss, token, ',')) {  
                size_t pos = token.find('x');  
                if (pos != std::string::npos) {  
                    score.boardRows = std::stoi(token.substr(0, pos));  
                    score.boardCols = std::stoi(token.substr(pos + 1));  
                }  
            }  
            if (std::getline(ss, token, ',')) score.totalMines = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.totalClicks = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.basePoints = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.timeBonus = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.efficiencyBonus = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.sizeMultiplier = std::stoi(token);  
              
            score.won = true;  
            scores.push_back(score);  
        }  
    }  
      
    // Ordenar por puntuación descendente  
    std::sort(scores.begin(), scores.end(),   
              [](const GameScore& a, const GameScore& b) {  
                  return a.totalScore > b.totalScore;  
              });  
      
    return scores;  
} 

void ScoreCalculator::displayHighScores() {  
    auto scores = loadScoresFromCSV();  
      
    clearScreen();  
    gotoxy(1, 1);  
    std::cout << "\033[96m╔══════════════════════════════════════════════════════════════════════╗\033[0m\n";  
    std::cout << "\033[96m║                           MEJORES PUNTUACIONES                       ║\033[0m\n";  
    std::cout << "\033[96m╠══════════════════════════════════════════════════════════════════════╣\033[0m\n";  
    std::cout << "\033[96m║ #  Jugador      Puntos   Dificultad  Tiempo   Tablero   Minas  Clics ║\033[0m\n";  
    std::cout << "\033[96m╠══════════════════════════════════════════════════════════════════════╣\033[0m\n";  
      
    for (size_t i = 0; i < std::min(scores.size(), size_t(10)); ++i) {  
        const auto& score = scores[i];  
        std::cout << "\033[96m║\033[0m ";  
          
        // # - 2 caracteres  
        std::cout << std::setw(1) << std::right << (i + 1) << "  ";  
          
        // Jugador - 11 caracteres (incluyendo espacio)  
        std::cout << std::setw(11) << std::left << score.playerName.substr(0, 11) << "";  
          
        // Puntos - 7 caracteres  
        std::cout << std::setw(6) << std::right << score.totalScore << "       ";  
          
        // Dificultad - 11 caracteres  
        std::cout << std::setw(11) << std::left << score.difficulty.substr(0, 11) << "";  
          
        // Tiempo - 8 caracteres (incluyendo 's')  
        std::cout << std::setw(3) << std::right << std::fixed << std::setprecision(1) << score.gameTimeSeconds << "s ";  
          
        // Tablero - 8 caracteres  
        std::cout << std::setw(6) << std::right << (std::to_string(score.boardRows) + "x" + std::to_string(score.boardCols)) << " ";  
          
        // Minas - 5 caracteres  
        std::cout << std::setw(8) << std::right << score.totalMines << "";  
          
        // Clics - 5 caracteres  
        std::cout << std::setw(8) << std::right << score.totalClicks << "  ";  
          
        std::cout << "\033[96m║\033[0m\n";  
    }  
      
    if (scores.empty()) {  
        std::cout << "\033[96m║                        No hay puntuaciones guardadas                 ║\033[0m\n";  
    }  
      
    std::cout << "\033[96m╚══════════════════════════════════════════════════════════════════════╝\033[0m\n";  
    std::cout << "\nPresione Enter para continuar...";  
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  
    std::cin.get();  
}
     

GameScore ScoreCalculator::calculateScore(int difficulty, int rows, int cols,   
                                        double gameTime, int clicks, int flags, bool won) {  
    GameScore score;  
      
    // Puntos base por dificultad  
    switch(difficulty) {  
        case 1: score.basePoints = 100; break;  // Fácil  
        case 2: score.basePoints = 300; break;  // Medio  
        case 3: score.basePoints = 500; break;  // Difícil  
        default: score.basePoints = 100; break;  
    }  
      
    // Solo otorgar bonificaciones si ganó  
    if (won) {  
        score.timeBonus = calculateTimeBonus(gameTime, difficulty);  
        score.efficiencyBonus = calculateEfficiencyBonus(clicks, rows * cols, won);  
        score.sizeMultiplier = calculateSizeMultiplier(rows, cols);  
    } else {  
        // Si perdió, solo puntos parciales  
        score.basePoints /= 4;  // 25% de los puntos base  
        score.timeBonus = 0;  
        score.efficiencyBonus = 0;  
        score.sizeMultiplier = 1;  
    }  
      
    score.totalScore = (score.basePoints + score.timeBonus + score.efficiencyBonus) * score.sizeMultiplier;  
    score.gameTimeSeconds = gameTime;  
    score.totalClicks = clicks;  
    score.flagsUsed = flags;  
    score.won = won;  
    score.difficulty = getDifficultyName(difficulty);  
    score.boardRows = rows;  
    score.boardCols = cols;  
      
    // Calcular total de minas basado en dificultad  
    double porcentaje = (difficulty == 1) ? 0.2 : (difficulty == 2) ? 0.5 : 0.7;  
    score.totalMines = static_cast<int>((rows * cols) * porcentaje);  
      
    return score;  
}


std::string ScoreCalculator::getDifficultyName(int difficulty) {  
    switch(difficulty) {  
        case 1: return "Fácil";  
        case 2: return "Medio";  
        case 3: return "Difícil";  
        default: return "Desconocido";  
    }  
}  
  
int ScoreCalculator::calculateTimeBonus(double gameTime, int difficulty) {  
    // Bonus por tiempo: más puntos por ser más rápido  
    // Tiempo objetivo por dificultad (en segundos)  
    double targetTime = (difficulty == 1) ? 60.0 : (difficulty == 2) ? 120.0 : 180.0;  
      
    if (gameTime < targetTime) {  
        double ratio = (targetTime - gameTime) / targetTime;  
        return static_cast<int>(ratio * 200 * difficulty); // Hasta 200 puntos extra por dificultad  
    }  
    return 0;  
}  
  
int ScoreCalculator::calculateEfficiencyBonus(int clicks, int totalCells, bool won) {  
    if (!won) return 0;  
      
    // Bonus por eficiencia: menos clics = más puntos  
    // Clics ideales sería revelar todas las celdas seguras  
    double efficiency = static_cast<double>(totalCells) / clicks;  
      
    if (efficiency > 1.0) {  
        return static_cast<int>((efficiency - 1.0) * 50); // Hasta 50 puntos extra por eficiencia  
    }  
    return 0;  
}  
  
int ScoreCalculator::calculateSizeMultiplier(int rows, int cols) {  
    // Multiplicador por tamaño del tablero  
    int totalCells = rows * cols;  
      
    if (totalCells <= 81) return 1;        // 9x9 o menor  
    else if (totalCells <= 150) return 2;  // Tableros medianos  
    else return 3;                         // Tableros grandes  
}

void ScoreCalculator::displayMultiplayerResults(const GameScore& player1Score,   
                                              const GameScore& player2Score,  
                                              const std::string& player1Name,  
                                              const std::string& player2Name) {  
    clearScreen();  
    gotoxy(1, 1);  
      
    std::cout << "\033[96m╔══════════════════════════════════════════════════╗\033[0m\n";  
    std::cout << "\033[96m║                RESULTADOS MULTIJUGADOR           ║\033[0m\n";  
    std::cout << "\033[96m╠══════════════════════════════════════════════════╣\033[0m\n";  
      
    // Mostrar puntuación de ambos jugadores  
    std::cout << "\033[96m║\033[0m               " << std::setw(4) << std::left << player1Name;  
    std::cout << "  vs    " << std::setw(10) << std::left << player2Name << "             \033[96m║\033[0m\n";  
      
    std::cout << "\033[96m║\033[0m Puntuación:   " << std::setw(6) << player1Score.totalScore;  
    std::cout << "       " << std::setw(8) << player2Score.totalScore << "              \033[96m║\033[0m\n";  
      
    std::string winner = (player1Score.totalScore > player2Score.totalScore) ? player1Name :   
                        (player2Score.totalScore > player1Score.totalScore) ? player2Name : "EMPATE";  
    std::cout << "\033[96m╠══════════════════════════════════════════════════╣\033[0m\n";     
    std::cout << "\033[96m║\033[0m GANADOR: " << std::setw(35) << std::left << winner << "     \033[96m║\033[0m\n";  
    std::cout << "\033[96m╚══════════════════════════════════════════════════╝\033[0m\n";  
}

void ScoreCalculator::displayLiveStats(int playerClicks, int playerFlags, double gameTime,  
                                     int opponentClicks, int opponentFlags,   
                                     const std::string& playerName, const std::string& opponentName,  
                                     int boardRows) {  
    // Mostrar estadísticas en tiempo real  
    gotoxy(2, boardRows + 3);  
    std::cout << "\033[94m┌─ ESTADÍSTICAS EN VIVO ───────────────────┐\033[0m\n";  
      
    gotoxy(2, boardRows + 4);  
    std::cout << "\033[94m│\033[0m \033[97m            " << std::setw(8) << std::left << playerName << "\033[0m";  
    std::cout << " vs      \033[97m" << std::setw(8) << std::left << opponentName << "\033[0m";  
    std::cout << "    \033[94m│\033[0m\n";  
      
    gotoxy(2, boardRows + 5);  
    std::cout << "\033[94m│\033[0m Clics: \033[93m      " << std::setw(3) << playerClicks << "\033[0m";  
    std::cout << "               \033[93m" << std::setw(3) << opponentClicks << "\033[0m";  
    std::cout << "       \033[94m│\033[0m\n";  
      
    gotoxy(2, boardRows + 6);  
    std::cout << "\033[94m│\033[0m Banderas:    \033[92m" << std::setw(2) << playerFlags << "\033[0m";  
    std::cout << "                \033[92m" << std::setw(2) << opponentFlags << "\033[0m";  
    std::cout << "        \033[94m│\033[0m\n";  
      
    gotoxy(2, boardRows + 7);  
    std::string timeStr = std::to_string(gameTime);  
    timeStr = timeStr.substr(0, timeStr.find('.') + 2) + "s"; // Mantener 1 decimal  
    int padding = 23 - timeStr.length(); // 23 es el espacio total disponible  
    std::cout << "\033[94m│\033[0m Tiempo: \033[96m" << std::fixed << std::setprecision(1) << gameTime << "s\033[0m";  
    std::cout << std::string(padding, ' ') << "          \033[94m│\033[0m\n"; 
      
    gotoxy(2, boardRows + 8);  
    std::cout << "\033[94m└──────────────────────────────────────────┘\033[0m\n";  
}

void ScoreCalculator::updateTimeOnly(double gameTime, int boardRows) {  
    gotoxy(2, boardRows + 6);  
    std::cout << "\033[94m┌──────────────────────────────────────────┐\033[0m\n";

    gotoxy(2, boardRows + 7);  
    std::string timeStr = std::to_string(gameTime);  
    timeStr = timeStr.substr(0, timeStr.find('.') + 2) + "s"; // Mantener 1 decimal  
    int padding = 29 - timeStr.length(); // 29 es el espacio total disponible para esta función  
    std::cout << "\033[94m│\033[0m Tiempo: \033[96m" << std::fixed << std::setprecision(1) << gameTime << "s\033[0m";  
    std::cout << std::string(padding, ' ') << "    \033[94m│\033[0m";  

    gotoxy(2, boardRows + 8);  
    std::cout << "\033[94m└──────────────────────────────────────────┘\033[0m\n";
}

void ScoreCalculator::saveMultiplayerScoreToCSV(const GameScore& score, const std::string& playerName) {  
    std::cout << "DEBUG: Intentando guardar puntuación multijugador para: " << playerName << std::endl;  
      
    if (!score.won) {  
        std::cout << "DEBUG: No se guarda porque no ganó" << std::endl;  
        return;  
    }  
      
    std::cout << "DEBUG: Creando archivo: " << MULTIPLAYER_CSV_FILE << std::endl;  
      
    
    if (!score.won) return; // Solo guardar puntuaciones ganadoras  
      
    // Crear header para archivo multijugador si no existe  
    std::ifstream checkFile(MULTIPLAYER_CSV_FILE);  
    if (!checkFile.good()) {  
        std::ofstream file(MULTIPLAYER_CSV_FILE);  
        if (file.is_open()) {  
            file << "PlayerName,TotalScore,Difficulty,GameTime,BoardSize,TotalMines,TotalClicks,BasePoints,TimeBonus,EfficiencyBonus,SizeMultiplier\n";  
            file.close();  
        }  
    }  
    checkFile.close();  
      
    std::ofstream file(MULTIPLAYER_CSV_FILE, std::ios::app);  
    if (file.is_open()) {  
        // Usar el nombre pasado como argumento y también actualizar el campo playerName del score
        GameScore scoreCopy = score;
        scoreCopy.playerName = playerName;
        file << escapeCSV(scoreCopy.playerName) << ","  
             << scoreCopy.totalScore << ","  
             << escapeCSV(scoreCopy.difficulty) << ","  
             << std::fixed << std::setprecision(2) << scoreCopy.gameTimeSeconds << ","  
             << scoreCopy.boardRows << "x" << scoreCopy.boardCols << ","  
             << scoreCopy.totalMines << ","  
             << scoreCopy.totalClicks << ","  
             << scoreCopy.basePoints << ","  
             << scoreCopy.timeBonus << ","  
             << scoreCopy.efficiencyBonus << ","  
             << scoreCopy.sizeMultiplier << "\n";  
        file.close();  
    }  
}  
  
std::vector<GameScore> ScoreCalculator::loadMultiplayerScoresFromCSV() {  
    std::vector<GameScore> scores;  
    std::ifstream file(MULTIPLAYER_CSV_FILE);  
    std::string line;  
      
    // Saltar header  
    if (std::getline(file, line)) {  
        while (std::getline(file, line)) {  
            std::stringstream ss(line);  
            std::string token;  
            GameScore score;  
              
            // Parse CSV fields (mismo código que loadScoresFromCSV)  
            if (std::getline(ss, token, ',')) score.playerName = token;  
            if (std::getline(ss, token, ',')) score.totalScore = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.difficulty = token;  
            if (std::getline(ss, token, ',')) score.gameTimeSeconds = std::stod(token);  
            if (std::getline(ss, token, ',')) {  
                size_t pos = token.find('x');  
                if (pos != std::string::npos) {  
                    score.boardRows = std::stoi(token.substr(0, pos));  
                    score.boardCols = std::stoi(token.substr(pos + 1));  
                }  
            }  
            if (std::getline(ss, token, ',')) score.totalMines = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.totalClicks = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.basePoints = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.timeBonus = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.efficiencyBonus = std::stoi(token);  
            if (std::getline(ss, token, ',')) score.sizeMultiplier = std::stoi(token);  
              
            score.won = true;  
            scores.push_back(score);  
        }  
    }  
      
    // Ordenar por puntuación descendente  
    std::sort(scores.begin(), scores.end(),  
              [](const GameScore& a, const GameScore& b) {  
                  return a.totalScore > b.totalScore;  
              });  
      
    return scores;  
}  
  
void ScoreCalculator::displayMultiplayerHighScores() {  
    auto scores = loadMultiplayerScoresFromCSV();  
      
    clearScreen();  
    gotoxy(1, 1);  
    std::cout << "\033[96m╔══════════════════════════════════════════════════════════════════════╗\033[0m\n";  
    std::cout << "\033[96m║                    MEJORES PUNTUACIONES MULTIJUGADOR                 ║\033[0m\n";  
    std::cout << "\033[96m╠══════════════════════════════════════════════════════════════════════╣\033[0m\n";  
    std::cout << "\033[96m║ #  Jugador      Puntos   Dificultad  Tiempo   Tablero   Minas  Clics ║\033[0m\n";  
    std::cout << "\033[96m╠══════════════════════════════════════════════════════════════════════╣\033[0m\n";  
      
    for (size_t i = 0; i < std::min(scores.size(), size_t(10)); ++i) {  
        const auto& score = scores[i];  
        std::cout << "\033[96m║\033[0m ";  
          
        // # - 2 caracteres  
        std::cout << std::setw(1) << std::right << (i + 1) << "  ";  
          
        // Jugador - 11 caracteres  
        std::cout << std::setw(11) << std::left << score.playerName.substr(0, 11) << "";  
          
        // Puntos - 7 caracteres  
        std::cout << std::setw(6) << std::right << score.totalScore << "       ";  
          
        // Dificultad - 11 caracteres  
        std::cout << std::setw(11) << std::left << score.difficulty.substr(0, 11) << "";  
          
        // Tiempo - 8 caracteres  
        std::cout << std::setw(3) << std::right << std::fixed << std::setprecision(1) << score.gameTimeSeconds << "s ";  
          
        // Tablero - 8 caracteres  
        std::cout << std::setw(6) << std::right << (std::to_string(score.boardRows) + "x" + std::to_string(score.boardCols)) << " ";  
          
        // Minas - 5 caracteres  
        std::cout << std::setw(8) << std::right << score.totalMines << "";  
          
        // Clics - 5 caracteres  
        std::cout << std::setw(8) << std::right << score.totalClicks << "  ";  
          
        std::cout << "\033[96m║\033[0m\n";  
    }  
      
    if (scores.empty()) {  
        std::cout << "\033[96m║                   No hay puntuaciones multijugador guardadas         ║\033[0m\n";  
    }  
      
    std::cout << "\033[96m╚══════════════════════════════════════════════════════════════════════╝\033[0m\n";  
    std::cout << "\nPresione Enter para continuar...";  
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  
    std::cin.get();  
}  
  
void ScoreCalculator::displayScoreSubmenu() {  
    while (true) {  
        clearScreen();  
        std::cout << "\033[96m╔══╗\033[92m═════════════════\033[31m═══════════════\033[96m╔══╗\033[0m\n";  
        std::cout << "\033[92m╠══╝          \033[0mPUNTUACIONES ALTAS\033[31m    ╚══╣\033[0m\n";  
        std::cout << "\033[92m╠════════════════════\033[31m══════════════════╣\033[0m\n";  
        std::cout << "\033[96m║  \033[0m1.\033[96m Modo Individual                  ║\033[0m\n";  
        std::cout << "\033[96m║  \033[0m2.\033[96m Modo Multijugador                ║\033[0m\n";  
        std::cout << "\033[96m║  \033[0m3.\033[96m Volver al menú principal         ║\033[0m\n";  
        std::cout << "\033[92m╚════════════════════\033[31m══════════════════╝\033[0m\n";  
        gotoxy(0, 7);  
        std::cout << "\033[0m└─────────────────────────┐\033[0m\n";
        std::cout << " Seleccione una opción: ";  
          
        int opcion;  
        std::cin >> opcion;  
          
        if (opcion == 1) {  
            displayHighScores();  
        } else if (opcion == 2) {  
            displayMultiplayerHighScores();  
        } else if (opcion == 3) {  
            break;  
        } else {  
            std::cout << "Opción no válida." << std::endl;  
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  
            std::cin.get();  
        }  
    }  
}