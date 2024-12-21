/************************************************************
 *  Sudoku Game Demo (C++ / Single-file Example)
 *
 *  �\�୫�I�G
 *    1) �D��� (�}�l�s�C�� / ���J�s�� / �Ʀ�] / ����)
 *    2) 9x9 �ƿW�L�� (��B�C�q 1 �}�l)
 *    3) system("cls") �M�� (Windows)
 *    4) �h������˼ƭp�� (pthread)
 *    5) �s��/Ū�ɡG���a�i�ۭq�ɦW
 *    6) ��@���J�R�O�Grow col val / 's' / 'q'
 *
 *  �`�N�G
 *    - �sĶ�ɥi��ݭn -lpthread
 *    - �Y�b Linux/macOS ���ҡA��� system("clear") �P sleep(1)
 *    - ���{���ȥܽd�֤��޿�A�ݦۦ��u��/����/�X�R
 ************************************************************/
#include <iostream>
#include <cstdio>      // fopen, fscanf, fprintf
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <ctime>
#include <vector>
#include <algorithm>
#include <sstream>     // stringstream (�ѪR���a��J)
#include <windows.h>   // Windows �U Sleep()�A�Y�b Linux/macOS ��� <unistd.h>

using namespace std;

/*
 * ==== ����`�ƻP�ܼƳ]�w ====
 */
static const int N = 9;           // �ƿW�L���j�p (�w�] 9)
int board[N][N];                  // ���쪺�ƿW�L��
bool fixedCell[N][N];            // �аO���Ǧ�m�O��l���w�A����ק�
int timeLimit = 300;             // �˼ƭp��(��) - �i�����פ���
bool timeUp = false;             // �p�ɬO�_����
bool gameFinished = false;       // ���a�O�_�����C��(���\�Ω��)

// �Ʀ�]�ε��c
struct ScoreRecord {
    string playerName;
    int usedTime;      // ���a�����ϥΪ����
    int difficulty;    // ���׵��� (1: ²��, 2: ����, 3: �x���K�i�ۦ�w�q)
};

// ���쪺�Ʀ�] (�d�ҥ� vector �Ȧs)
vector<ScoreRecord> scoreboard;

// ������A�Ω�h������O�@�@�θ귽
pthread_mutex_t mutexLock;

/************************************************************
 *  �ƿW�ͦ� / �ˬd��
 ************************************************************/

/* �ˬd�b row, col �� num �O�_�w�� (0-based) */
bool checkSafe(int grid[N][N], int row, int col, int num) {
    // �ˬd�� row
    for (int i = 0; i < N; i++) {
        if (grid[row][i] == num) return false;
    }
    // �ˬd�� col
    for (int i = 0; i < N; i++) {
        if (grid[i][col] == num) return false;
    }
    // �ˬd 3x3 ���
    int boxSize = 3; // �w�� 9x9
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

/* �Q�Φ^���k�ѥX�ƿW�]�Ω��ˬd�Υͦ��^*/
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
                        grid[row][col] = 0; // �^��
                    }
                }
                return false;
            }
        }
    }
    return true; // ������n
}

/* �H���ͦ��@��²�檺 9x9 �ƿW (²���ܨ�) */
void generateSudoku() {
    // ���� 0
    memset(board, 0, sizeof(board));

    /*
     * �o�̵��@�ӫD�`²�檺�ͦ��ܨҡG
     * 1. ���b�﨤�u�� 3 �� 3x3 �ϰ���H���Ʀr (���X�k)
     * 2. �A���ե� solveSudoku() ���ͧ����
     * 3. �̫���Ť@�w�ƶq�@���D�� (�����ץi���h�Τ�)
     */

    // step1: ��﨤�u
    srand((unsigned)time(NULL));
    int boxSize = 3; // for 9x9
    for (int k = 0; k < N; k += boxSize) {
        // �� (k,k) �� 3x3 �϶�
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

    // step2: �ѥX����L
    solveSudoku(board);

    // step3: ���} (������)
    int holes = 30; // ²��Ҧ��N�� 30 �� (�i�ۦ�վ�)
    for (int i = 0; i < holes; ) {
        int r = rand() % N;
        int c = rand() % N;
        if (board[r][c] != 0) {
            board[r][c] = 0;
            i++;
        }
    }

    // ��n fixedCell
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            fixedCell[r][c] = (board[r][c] != 0);
        }
    }
}

/************************************************************
 *  ��ܼƿW�L���]�H 1-based �覡�L��C�^
 ************************************************************/
void printBoard() {
    system("cls");  // Windows �M��

    cout << "==================== S U D O K U ====================\n\n";

    // ��������D (1~9)�A�æb�C 3 ���d�I�Ů���j
    cout << "      ";
    for (int i = 1; i <= N; i++) {
        cout << i << " ";
        if (i % 3 == 0 && i < N) cout << "  ";  // �C 3 �C�h�Ť@��
    }
    cout << "\n    ========================================\n";

    // �L�X 9x9 ���e (�������O 0-based�A����ܦ� 1-based)
    for (int r = 0; r < N; r++) {
        // �C�� (r+1)
        cout << " " << (r + 1) << " | ";
        for (int c = 0; c < N; c++) {
            // �Y�� 0�A��� '.'�A�_�h��� board[r][c]
            if (board[r][c] == 0) {
                cout << ".";
            } else {
                cout << board[r][c];
            }

            // �b�C�@��᭱�d�@�Ů�A�æb�C 3 ��A�h�L�Ӥ��j
            if ((c + 1) % 3 == 0 && c < N - 1) {
                cout << " | ";
            } else {
                cout << " ";
            }
        }
        cout << "\n";

        // �C 3 �C��[���j�u�]���̫�@�C���Ρ^
        if ((r + 1) % 3 == 0 && r < N - 1) {
            cout << "    ----------------------------------------\n";
        }
    }
    cout << "=====================================================\n\n";
}

/* �ˬd��e�ѽL�O�_������(���a�ӧQ) */
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
 *  ���a��J & ��@��R�O�ѪR�G
 *   1) row col val  (��g)
 *   2) 's' (�s��)
 *   3) 'q' (���)
 ************************************************************/
void doPlayerCommand() {
    // ����ܴ���
    cout << "�п�J (row col val) �� 's' �s��, 'q' ���: ";

    // Ū�����r��
    string input;
    // ���F�קK�W�@����J�ݯd�A�i�u���I�s ignore
    // ���L�Y�T�w�C���I�s doPlayerCommand() �ɳ��L�ݯd�A�i�ٲ�
    //cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    getline(cin, input);

    // ²��h���e��ťժ��覡 (�i�A�X�R)
    // �Y�n��w���i�ۦ�g trimString
    if (input.empty()) {
        cout << "[ĵ�i] ��J���šA�Э��s��J\n";
        Sleep(1000);
        return;
    }

    // �ˬd�O�_ 'q' �� 's'
    if (input == "q") {
        // ���C��
        gameFinished = true;
        return;
    }
    else if (input == "s") {
        // �s��
        cout << "�п�J�n�s�ɪ��ɮצW�� (�Ҧp: my_save.txt): ";
        string fileName;
        getline(cin, fileName);
        if (fileName.empty()) {
            cout << "[ĵ�i] �ɦW���šA�s�ɨ���\n";
            Sleep(1000);
            return;
        }

        FILE* fp = fopen(fileName.c_str(), "w");
        if (!fp) {
            cout << "�s�ɥ���: �L�k�}���ɮ�\n";
            Sleep(1000);
            return;
        }
        fclose(fp); // �������A�T�w��}���ɮ�

        // �I�s�쥻 saveGame �禡
        if (fileName.size() > 0 && !fileName.empty()) {
            if (/* �]�i���� saveGame(fileName.c_str()) */ true) {
                // ��g�@�U(�T�O�ŦX�쥻�d��):
                extern bool saveGame(const char*);
                if (saveGame(fileName.c_str())) {
                    cout << "�s�ɦ��\�I\n";
                } else {
                    cout << "�s�ɥ��ѡA���ˬd�ɦW�θ��|�C\n";
                }
            }
        }
        Sleep(1000);
        return;
    }
    else {
        // ���է� input ���Ѧ� row col val (1-based)
        int r, c, val;
        {
            std::stringstream ss(input);
            if (!(ss >> r >> c >> val)) {
                cout << "[ĵ�i] �榡���~�A�п�J (row col val) �� 's'/'q'\n";
                Sleep(1000);
                return;
            }
        }

        // 1-based �� 0-based
        r -= 1;
        c -= 1;

        // �ˬd�d��
        if (r < 0 || r >= N || c < 0 || c >= N || val < 1 || val > N) {
            cout << "[���~] ��J���X�k�A�Э��s��J�C\n";
            Sleep(1000);
            return;
        }
        if (fixedCell[r][c]) {
            cout << "[ĵ�i] �o��O���w�Ʀr�A�L�k�ק�C\n";
            Sleep(1000);
            return;
        }
        if (!checkSafe(board, r, c, val)) {
            cout << "[ĵ�i] ��m���X�k�A�i��P��B�C�ΤE�c��Ĭ�C\n";
            Sleep(1000);
            return;
        }

        // �X�k�N��W
        board[r][c] = val;
    }
}

/************************************************************
 *  �h������G�˼ƭp��
 ************************************************************/
void* countdown(void* arg) {
    int seconds = *(int*)arg; // Ū���ǤJ���Ѽ�
    while (seconds > 0 && !gameFinished) {
        Sleep(1000); // Windows �U�� Sleep(ms)
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
 *  �s�� / Ū�ɥ\��
 ************************************************************/
bool saveGame(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        cerr << "�s�ɥ���: �L�k�}���ɮ�\n";
        return false;
    }
    // �g�J�Ѿl�ɶ�
    fprintf(fp, "%d\n", timeLimit);

    // �g�J�ѽL 9x9 �����A (board)
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            fprintf(fp, "%d ", board[r][c]);  // �Y�Ů欰 0
        }
        fprintf(fp, "\n");
    }

    // �g�J���Ǯ�l�O�T�w�� (fixedCell)
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
        cerr << "Ū�ɥ���: �ɮפ��s�b�εL�k�}��\n";
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
 *  �Ʀ�]�\�� (�ܨ�)
 ************************************************************/
void updateScoreBoard(const string &player, int usedTime, int difficulty) {
    ScoreRecord rec;
    rec.playerName = player;
    rec.usedTime   = usedTime;
    rec.difficulty = difficulty;
    scoreboard.push_back(rec);

    // �̧����ɶ��Ѥp��j�Ƨ�
    sort(scoreboard.begin(), scoreboard.end(),
         [](const ScoreRecord &a, const ScoreRecord &b) {
             return a.usedTime < b.usedTime;
         });
}

void showScoreBoard() {
    system("cls");
    cout << "======= �Ʀ�] (�̧����ɶ��Ѥp��j) =======\n";
    int rank = 1;
    for (auto &s : scoreboard) {
        cout << "#" << rank++
             << " ���a: " << s.playerName
             << " | �ɶ�(��): " << s.usedTime
             << " | ����: " << s.difficulty << "\n";
    }
    cout << "===========================================\n";
    cout << "�� Enter �^�D���...\n";
    cin.ignore();
    cin.get();
}

/************************************************************
 *  �D�{���y�{
 ************************************************************/
int main() {
    pthread_mutex_init(&mutexLock, NULL);

    while (true) {
        system("cls");
        cout << "================= �ƿW�C����� =================\n";
        cout << "1. �}�l�s�C��\n";
        cout << "2. ���J�s��\n";
        cout << "3. �d�ݱƦ�]\n";
        cout << "4. �����{��\n";
        cout << "==============================================\n";
        cout << "�п�J�ﶵ: ";
        int choice;
        cin >> choice;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          // �M������šA�קK�z�Z���� getline

        if (choice == 1) {
            // �����a������
            cout << "������� (1: ²��, 2: ����, 3: �x��): ";
            int diff;
            cin >> diff;
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            switch (diff) {
                case 1: timeLimit = 300; break;  // 5 ����
                case 2: timeLimit = 180; break;  // 3 ����
                case 3: timeLimit = 120; break;  // 2 ����
                default: timeLimit = 300; break;
            }

            // �ͦ��s�D��
            generateSudoku();

            // �C���}�l
            timeUp = false;
            gameFinished = false;

            // �Ұʭp�ɰ����
            pthread_t timerThread;
            pthread_create(&timerThread, NULL, countdown, &timeLimit);

            // ���o�C���}�l�ɶ��A�Ψӭp�⧹���һݮɶ�
            time_t startTime = time(NULL);

            // ====== �C���i��j�� ======
            while (!gameFinished) {
                printBoard();
                if (timeUp) {
                    cout << "[�t��] �ɶ���I\n";
                    gameFinished = true;
                    break;
                }
                if (isBoardComplete()) {
                    cout << "[����] �A�����F�ƿW�I\n";
                    gameFinished = true;
                    break;
                }

                // ��@����O (row col val / 's' / 'q')
                doPlayerCommand();
            }

            // ====== ���� ======
            time_t endTime = time(NULL);
            int usedTime = (int)(endTime - startTime);

            // �Y���a�����B�D�W��
            if (!timeUp && isBoardComplete()) {
                cout << "�A���\�����A��O�ɶ�: " << usedTime << " ��\n";
                cout << "�п�J�A���W�r: ";
                string name;
                cin.ignore();
                getline(cin, name);
                updateScoreBoard(name, usedTime, diff);
            }
            else if (timeUp) {
                cout << "�ɶ��w��A�D�ԥ��ѡC\n";
            }
            else if (!isBoardComplete()) {
                cout << "�A�w���Υ������C\n";
            }

            // �^��D���e�A�����a����
            cout << "�� Enter �^�D���...\n";
            cin.ignore();
            pthread_cancel(timerThread);
            pthread_join(timerThread, NULL);

        } else if (choice == 2) {
            // ���J�s�� (���ݨϥΪ̭n�������ɮ�)
            cout << "�п�J�n���J���ɦW (�Ҧp: sudoku_save.txt): ";
            string fileName;
            getline(cin, fileName);
            if (fileName.empty()) {
                cout << "[ĵ�i] �ɦW���šA�L�kŪ�ɡC\n";
                Sleep(1000);
                continue;
            }
            if (loadGame(fileName.c_str())) {
                cout << "Ū�ɦ��\�I\n";
                // �C���~��
                timeUp = false;
                gameFinished = false;

                pthread_t timerThread;
                pthread_create(&timerThread, NULL, countdown, &timeLimit);

                time_t startTime = time(NULL);

                // ====== �C���j�� ======
                while (!gameFinished) {
                    printBoard();
                    if (timeUp) {
                        cout << "[�t��] �ɶ���I\n";
                        gameFinished = true;
                        break;
                    }
                    if (isBoardComplete()) {
                        cout << "[����] �A�����F�ƿW�I\n";
                        gameFinished = true;
                        break;
                    }

                    // ��@����O
                    doPlayerCommand();
                }

                // ====== ���� ======
                time_t endTime = time(NULL);
                int usedTime = (int)(endTime - startTime);

                if (!timeUp && isBoardComplete()) {
                    cout << "�A���\�����A��O�ɶ�: " << usedTime << " ��\n";
                    cout << "�п�J�A���W�r: ";
                    string name;
                    cin.ignore();
                    getline(cin, name);
                    updateScoreBoard(name, usedTime, 1);
                      // �o�����ץ����A�i�ۦ��x�s�W��������
                }
                else if (timeUp) {
                    cout << "�ɶ��w��A�D�ԥ��ѡC\n";
                }
                else if (!isBoardComplete()) {
                    cout << "�A�w���Υ������C\n";
                }

                cout << "�� Enter �^�D���...\n";
                cin.ignore();
                pthread_cancel(timerThread);
                pthread_join(timerThread, NULL);

            } else {
                cout << "Ū�ɥ��ѡA�нT�{�ɮ� " << fileName << " �s�b�C\n";
                cout << "�� Enter �^�D���...\n";
                cin.ignore();
            }

        } else if (choice == 3) {
            // �Ʀ�]
            showScoreBoard();

        } else if (choice == 4) {
            // �����{��
            cout << "�A���I\n";
            break;
        } else {
            cout << "�L�Ŀﶵ�A�Э��s��J�C\n";
            Sleep(1000);
        }
    }

    pthread_mutex_destroy(&mutexLock);
    return 0;
}
