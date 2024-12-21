/************************************************************
 *  Sudoku Game Demo (C++ / Single-file Example)
 *
 *  功能重點：
 *    1) 主選單 (開始新遊戲 / 載入存檔 / 排行榜 / 結束)
 *    2) 9x9 數獨盤面 (行、列從 1 開始)
 *    3) system("cls") 清屏 (Windows)
 *    4) 多執行緒倒數計時 (pthread)
 *    5) 存檔/讀檔：玩家可自訂檔名
 *    6) 單一行輸入命令：row col val / 's' / 'q'
 *
 *  注意：
 *    - 編譯時可能需要 -lpthread
 *    - 若在 Linux/macOS 環境，改用 system("clear") 與 sleep(1)
 *    - 本程式僅示範核心邏輯，需自行優化/除錯/擴充
 ************************************************************/
#include <iostream>
#include <cstdio>      // fopen, fscanf, fprintf
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <ctime>
#include <vector>
#include <algorithm>
#include <sstream>     // stringstream (解析玩家輸入)
#include <windows.h>   // Windows 下 Sleep()，若在 Linux/macOS 改用 <unistd.h>

using namespace std;

/*
 * ==== 全域常數與變數設定 ====
 */
static const int N = 9;           // 數獨盤面大小 (預設 9)
int board[N][N];                  // 全域的數獨盤面
bool fixedCell[N][N];            // 標記哪些位置是原始給定，不能修改
int timeLimit = 300;             // 倒數計時(秒) - 可依難度切換
bool timeUp = false;             // 計時是否結束
bool gameFinished = false;       // 玩家是否結束遊戲(成功或放棄)

// 排行榜用結構
struct ScoreRecord {
    string playerName;
    int usedTime;      // 玩家完成使用的秒數
    int difficulty;    // 難度等級 (1: 簡單, 2: 中等, 3: 困難…可自行定義)
};

// 全域的排行榜 (範例用 vector 暫存)
vector<ScoreRecord> scoreboard;

// 互斥鎖，用於多執行緒保護共用資源
pthread_mutex_t mutexLock;

/************************************************************
 *  數獨生成 / 檢查區
 ************************************************************/

/* 檢查在 row, col 放 num 是否安全 (0-based) */
bool checkSafe(int grid[N][N], int row, int col, int num) {
    // 檢查該 row
    for (int i = 0; i < N; i++) {
        if (grid[row][i] == num) return false;
    }
    // 檢查該 col
    for (int i = 0; i < N; i++) {
        if (grid[i][col] == num) return false;
    }
    // 檢查 3x3 方塊
    int boxSize = 3; // 針對 9x9
    int boxRow = (row / boxSize) * boxSize;
    int boxCol = (col / boxSize) * boxSize;
    for (int r = 0; r < boxSize; r++) {
        for (int c = 0; c < boxSize; c++) {
            if (grid[boxRow + r][boxCol + c] == num) {
                return false;
            }
        }
    }
    return true;
}

/* 利用回溯法解出數獨（用於檢查或生成）*/
bool solveSudoku(int grid[N][N]) {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (grid[row][col] == 0) {
                for (int num = 1; num <= N; num++) {
                    if (checkSafe(grid, row, col, num)) {
                        grid[row][col] = num;
                        if (solveSudoku(grid)) {
                            return true;
                        }
                        grid[row][col] = 0; // 回溯
                    }
                }
                return false;
            }
        }
    }
    return true; // 全部填好
}

/* 隨機生成一個簡單的 9x9 數獨 (簡易示例) */
void generateSudoku() {
    // 先填滿 0
    memset(board, 0, sizeof(board));

    /*
     * 這裡給一個非常簡單的生成示例：
     * 1. 先在對角線的 3 個 3x3 區域填滿隨機數字 (但合法)
     * 2. 再嘗試用 solveSudoku() 產生完整解
     * 3. 最後挖空一定數量作為題目 (依難度可挖多或少)
     */

    // step1: 填對角線
    srand((unsigned)time(NULL));
    int boxSize = 3; // for 9x9
    for (int k = 0; k < N; k += boxSize) {
        // 填 (k,k) 的 3x3 區塊
        for (int row = 0; row < boxSize; row++) {
            for (int col = 0; col < boxSize; col++) {
                int num;
                do {
                    num = rand() % N + 1;
                } while(!checkSafe(board, k+row, k+col, num));
                board[k+row][k+col] = num;
            }
        }
    }

    // step2: 解出完整盤
    solveSudoku(board);

    // step3: 挖洞 (依難度)
    int holes = 30; // 簡單模式就挖 30 格 (可自行調整)
    for (int i = 0; i < holes; ) {
        int r = rand() % N;
        int c = rand() % N;
        if (board[r][c] != 0) {
            board[r][c] = 0;
            i++;
        }
    }

    // 填好 fixedCell
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            fixedCell[r][c] = (board[r][c] != 0);
        }
    }
}

/************************************************************
 *  顯示數獨盤面（以 1-based 方式印欄列）
 ************************************************************/
void printBoard() {
    system("cls");  // Windows 清屏

    cout << "==================== S U D O K U ====================\n\n";

    // 顯示欄位標題 (1~9)，並在每 3 行後留點空格分隔
    cout << "      ";
    for (int i = 1; i <= N; i++) {
        cout << i << " ";
        if (i % 3 == 0 && i < N) cout << "  ";  // 每 3 列多空一格
    }
    cout << "\n    ========================================\n";

    // 印出 9x9 內容 (內部仍是 0-based，但顯示成 1-based)
    for (int r = 0; r < N; r++) {
        // 列號 (r+1)
        cout << " " << (r + 1) << " | ";
        for (int c = 0; c < N; c++) {
            // 若為 0，顯示 '.'，否則顯示 board[r][c]
            if (board[r][c] == 0) {
                cout << ".";
            } else {
                cout << board[r][c];
            }

            // 在每一格後面留一空格，並在每 3 行再多印個分隔
            if ((c + 1) % 3 == 0 && c < N - 1) {
                cout << " | ";
            } else {
                cout << " ";
            }
        }
        cout << "\n";

        // 每 3 列後加分隔線（但最後一列不用）
        if ((r + 1) % 3 == 0 && r < N - 1) {
            cout << "    ----------------------------------------\n";
        }
    }
    cout << "=====================================================\n\n";
}

/* 檢查當前棋盤是否全部填滿(玩家勝利) */
bool isBoardComplete() {
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (board[r][c] == 0) {
                return false;
            }
        }
    }
    return true;
}

/************************************************************
 *  玩家輸入 & 單一行命令解析：
 *   1) row col val  (填寫)
 *   2) 's' (存檔)
 *   3) 'q' (放棄)
 ************************************************************/
void doPlayerCommand() {
    // 先顯示提示
    cout << "請輸入 (row col val) 或 's' 存檔, 'q' 放棄: ";

    // 讀取整行字串
    string input;
    // 為了避免上一輪輸入殘留，可酌情呼叫 ignore
    // 不過若確定每次呼叫 doPlayerCommand() 時都無殘留，可省略
    //cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    getline(cin, input);

    // 簡單去除前後空白的方式 (可再擴充)
    // 若要更安全可自行寫 trimString
    if (input.empty()) {
        cout << "[警告] 輸入為空，請重新輸入\n";
        Sleep(1000);
        return;
    }

    // 檢查是否 'q' 或 's'
    if (input == "q") {
        // 放棄遊戲
        gameFinished = true;
        return;
    }
    else if (input == "s") {
        // 存檔
        cout << "請輸入要存檔的檔案名稱 (例如: my_save.txt): ";
        string fileName;
        getline(cin, fileName);
        if (fileName.empty()) {
            cout << "[警告] 檔名為空，存檔取消\n";
            Sleep(1000);
            return;
        }

        FILE* fp = fopen(fileName.c_str(), "w");
        if (!fp) {
            cout << "存檔失敗: 無法開啟檔案\n";
            Sleep(1000);
            return;
        }
        fclose(fp); // 先關掉，確定能開啟檔案

        // 呼叫原本 saveGame 函式
        if (fileName.size() > 0 && !fileName.empty()) {
            if (/* 也可直接 saveGame(fileName.c_str()) */ true) {
                // 改寫一下(確保符合原本範例):
                extern bool saveGame(const char*);
                if (saveGame(fileName.c_str())) {
                    cout << "存檔成功！\n";
                } else {
                    cout << "存檔失敗，請檢查檔名或路徑。\n";
                }
            }
        }
        Sleep(1000);
        return;
    }
    else {
        // 嘗試把 input 分解成 row col val (1-based)
        int r, c, val;
        {
            std::stringstream ss(input);
            if (!(ss >> r >> c >> val)) {
                cout << "[警告] 格式錯誤，請輸入 (row col val) 或 's'/'q'\n";
                Sleep(1000);
                return;
            }
        }

        // 1-based 轉 0-based
        r -= 1;
        c -= 1;

        // 檢查範圍
        if (r < 0 || r >= N || c < 0 || c >= N || val < 1 || val > N) {
            cout << "[錯誤] 輸入不合法，請重新輸入。\n";
            Sleep(1000);
            return;
        }
        if (fixedCell[r][c]) {
            cout << "[警告] 這格是給定數字，無法修改。\n";
            Sleep(1000);
            return;
        }
        if (!checkSafe(board, r, c, val)) {
            cout << "[警告] 放置不合法，可能與行、列或九宮格衝突。\n";
            Sleep(1000);
            return;
        }

        // 合法就填上
        board[r][c] = val;
    }
}

/************************************************************
 *  多執行緒：倒數計時
 ************************************************************/
void* countdown(void* arg) {
    int seconds = *(int*)arg; // 讀取傳入的參數
    while (seconds > 0 && !gameFinished) {
        Sleep(1000); // Windows 下用 Sleep(ms)
        seconds--;
    }
    pthread_mutex_lock(&mutexLock);
    if (!gameFinished) {
        timeUp = true;
        gameFinished = true;
    }
    pthread_mutex_unlock(&mutexLock);

    pthread_exit(NULL);
}

/************************************************************
 *  存檔 / 讀檔功能
 ************************************************************/
bool saveGame(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        cerr << "存檔失敗: 無法開啟檔案\n";
        return false;
    }
    // 寫入剩餘時間
    fprintf(fp, "%d\n", timeLimit);

    // 寫入棋盤 9x9 的狀態 (board)
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            fprintf(fp, "%d ", board[r][c]);  // 若空格為 0
        }
        fprintf(fp, "\n");
    }

    // 寫入哪些格子是固定的 (fixedCell)
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            fprintf(fp, "%d ", fixedCell[r][c] ? 1 : 0);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    return true;
}

bool loadGame(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        cerr << "讀檔失敗: 檔案不存在或無法開啟\n";
        return false;
    }
    fscanf(fp, "%d\n", &timeLimit);
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            fscanf(fp, "%d", &board[r][c]);
        }
    }
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            int tmp;
            fscanf(fp, "%d", &tmp);
            fixedCell[r][c] = (tmp == 1);
        }
    }
    fclose(fp);
    return true;
}

/************************************************************
 *  排行榜功能 (示例)
 ************************************************************/
void updateScoreBoard(const string &player, int usedTime, int difficulty) {
    ScoreRecord rec;
    rec.playerName = player;
    rec.usedTime   = usedTime;
    rec.difficulty = difficulty;
    scoreboard.push_back(rec);

    // 依完成時間由小到大排序
    sort(scoreboard.begin(), scoreboard.end(),
         [](const ScoreRecord &a, const ScoreRecord &b) {
             return a.usedTime < b.usedTime;
         });
}

void showScoreBoard() {
    system("cls");
    cout << "======= 排行榜 (依完成時間由小到大) =======\n";
    int rank = 1;
    for (auto &s : scoreboard) {
        cout << "#" << rank++
             << " 玩家: " << s.playerName
             << " | 時間(秒): " << s.usedTime
             << " | 難度: " << s.difficulty << "\n";
    }
    cout << "===========================================\n";
    cout << "按 Enter 回主選單...\n";
    cin.ignore();
    cin.get();
}

/************************************************************
 *  主程式流程
 ************************************************************/
int main() {
    pthread_mutex_init(&mutexLock, NULL);

    while (true) {
        system("cls");
        cout << "================= 數獨遊戲選單 =================\n";
        cout << "1. 開始新遊戲\n";
        cout << "2. 載入存檔\n";
        cout << "3. 查看排行榜\n";
        cout << "4. 結束程式\n";
        cout << "==============================================\n";
        cout << "請輸入選項: ";
        int choice;
        cin >> choice;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          // 清除換行符，避免干擾後續 getline

        if (choice == 1) {
            // 讓玩家選難度
            cout << "選擇難度 (1: 簡單, 2: 中等, 3: 困難): ";
            int diff;
            cin >> diff;
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            switch (diff) {
                case 1: timeLimit = 300; break;  // 5 分鐘
                case 2: timeLimit = 180; break;  // 3 分鐘
                case 3: timeLimit = 120; break;  // 2 分鐘
                default: timeLimit = 300; break;
            }

            // 生成新題目
            generateSudoku();

            // 遊戲開始
            timeUp = false;
            gameFinished = false;

            // 啟動計時執行緒
            pthread_t timerThread;
            pthread_create(&timerThread, NULL, countdown, &timeLimit);

            // 取得遊戲開始時間，用來計算完成所需時間
            time_t startTime = time(NULL);

            // ====== 遊戲進行迴圈 ======
            while (!gameFinished) {
                printBoard();
                if (timeUp) {
                    cout << "[系統] 時間到！\n";
                    gameFinished = true;
                    break;
                }
                if (isBoardComplete()) {
                    cout << "[恭喜] 你完成了數獨！\n";
                    gameFinished = true;
                    break;
                }

                // 單一行指令 (row col val / 's' / 'q')
                doPlayerCommand();
            }

            // ====== 結算 ======
            time_t endTime = time(NULL);
            int usedTime = (int)(endTime - startTime);

            // 若玩家完成且非超時
            if (!timeUp && isBoardComplete()) {
                cout << "你成功完成，花費時間: " << usedTime << " 秒\n";
                cout << "請輸入你的名字: ";
                string name;
                cin.ignore();
                getline(cin, name);
                updateScoreBoard(name, usedTime, diff);
            }
            else if (timeUp) {
                cout << "時間已到，挑戰失敗。\n";
            }
            else if (!isBoardComplete()) {
                cout << "你已放棄或未完成。\n";
            }

            // 回到主選單前，等玩家按鍵
            cout << "按 Enter 回主選單...\n";
            cin.ignore();
            pthread_cancel(timerThread);
            pthread_join(timerThread, NULL);

        } else if (choice == 2) {
            // 載入存檔 (先問使用者要載哪個檔案)
            cout << "請輸入要載入的檔名 (例如: sudoku_save.txt): ";
            string fileName;
            getline(cin, fileName);
            if (fileName.empty()) {
                cout << "[警告] 檔名為空，無法讀檔。\n";
                Sleep(1000);
                continue;
            }
            if (loadGame(fileName.c_str())) {
                cout << "讀檔成功！\n";
                // 遊戲繼續
                timeUp = false;
                gameFinished = false;

                pthread_t timerThread;
                pthread_create(&timerThread, NULL, countdown, &timeLimit);

                time_t startTime = time(NULL);

                // ====== 遊戲迴圈 ======
                while (!gameFinished) {
                    printBoard();
                    if (timeUp) {
                        cout << "[系統] 時間到！\n";
                        gameFinished = true;
                        break;
                    }
                    if (isBoardComplete()) {
                        cout << "[恭喜] 你完成了數獨！\n";
                        gameFinished = true;
                        break;
                    }

                    // 單一行指令
                    doPlayerCommand();
                }

                // ====== 結算 ======
                time_t endTime = time(NULL);
                int usedTime = (int)(endTime - startTime);

                if (!timeUp && isBoardComplete()) {
                    cout << "你成功完成，花費時間: " << usedTime << " 秒\n";
                    cout << "請輸入你的名字: ";
                    string name;
                    cin.ignore();
                    getline(cin, name);
                    updateScoreBoard(name, usedTime, 1);
                      // 這裡難度未知，可自行儲存上次的難度
                }
                else if (timeUp) {
                    cout << "時間已到，挑戰失敗。\n";
                }
                else if (!isBoardComplete()) {
                    cout << "你已放棄或未完成。\n";
                }

                cout << "按 Enter 回主選單...\n";
                cin.ignore();
                pthread_cancel(timerThread);
                pthread_join(timerThread, NULL);

            } else {
                cout << "讀檔失敗，請確認檔案 " << fileName << " 存在。\n";
                cout << "按 Enter 回主選單...\n";
                cin.ignore();
            }

        } else if (choice == 3) {
            // 排行榜
            showScoreBoard();

        } else if (choice == 4) {
            // 結束程式
            cout << "再見！\n";
            break;
        } else {
            cout << "無效選項，請重新輸入。\n";
            Sleep(1000);
        }
    }

    pthread_mutex_destroy(&mutexLock);
    return 0;
}
