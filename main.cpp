#include <windows.h>


// consty
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int BALL_SPEED = -5;
const int TIMER_ID = 1;
const int PADDLE_SPEED = 10;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;

// pileczka
struct Ball {
    int x, y;
    int speedX, speedY;
    int size;
} ball;

// paletka
struct Paddle {
    int x, y;
    int width, height;
} paddle;

// background
HBITMAP hBitmap;


int score = 0;
int heart = 3;
int zmTurbo = 0;


// nasze funckje
void UpdateBallPosition(HWND hWnd);
void DrawBall(HDC hdc);
void DrawBackgroundImage(HDC hdc);
void UpdatePaddlePosition(HWND hWnd, WPARAM wParam);
void DrawPaddle(HDC hdc, int xPos);
void DrawScore(HDC hdc);
void DrawHeart(HDC hdc);
bool CheckCollisionWithPaddle();
bool CheckCollisionWithFloor();
void ResetBall();
void ShowGameOver(HWND hWnd);
void PlaySoundtrack();





LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = "PingPongClass";

    RegisterClass(&wc);

    HWND hWnd = CreateWindow("PingPongClass", "Ping Pong Game", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = { 0 };
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.speedX = BALL_SPEED;
    ball.speedY = BALL_SPEED;
    ball.size = 20;

    paddle.width = PADDLE_WIDTH;
    paddle.height = PADDLE_HEIGHT;
    paddle.x = (SCREEN_WIDTH - paddle.width) / 2;
    paddle.y = SCREEN_HEIGHT - paddle.height - 50;

    hBitmap = (HBITMAP)LoadImage(NULL, "background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBitmap == NULL) {
        MessageBox(NULL, "Failed to load image!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    SetTimer(hWnd, TIMER_ID, 16, NULL); // timer 60 fps

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        DrawBackgroundImage(hdc);
        DrawBall(hdc);
        DrawPaddle(hdc, paddle.x);
        DrawScore(hdc);
        DrawHeart(hdc);
        EndPaint(hWnd, &ps);
        break;
    case WM_TIMER:
        UpdateBallPosition(hWnd);
        break;
    case WM_KEYDOWN:
        UpdatePaddlePosition(hWnd, wParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void DrawBackgroundImage (HDC hdc){
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BITMAP bitmap;
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    StretchBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);
}
void UpdateBallPosition(HWND hWnd) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    ball.x += ball.speedX;
    ball.y += ball.speedY;

    if (ball.x <= 0 || ball.x >= clientRect.right - ball.size) {
        ball.speedX = -ball.speedX;
    }
    if (ball.y <= 0 || ball.y >= clientRect.bottom - ball.size) {
        ball.speedY = -ball.speedY;
    }

    if (CheckCollisionWithPaddle()) {
        ball.speedY = -ball.speedY; // odbicie (paletki i pilki)

        score++;
        InvalidateRect(NULL, NULL, TRUE);

        zmTurbo++;
        if(zmTurbo > 5){
            ball.speedY--;
            zmTurbo = 0;
        }

    }

    if(CheckCollisionWithFloor()){
        heart--;

        if(heart == 0){
            ShowGameOver(hWnd);
        }


        InvalidateRect(NULL, NULL, TRUE);
    }

    InvalidateRect(hWnd, NULL, TRUE);
}

void DrawBall(HDC hdc) {
    Ellipse(hdc, ball.x, ball.y, ball.x + ball.size, ball.y + ball.size);
}

void UpdatePaddlePosition(HWND hWnd, WPARAM wParam) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    switch (wParam) {
    case VK_LEFT:
        paddle.x -= PADDLE_SPEED;
        if (paddle.x < 0) paddle.x = 0;
        break;
    case VK_RIGHT:
        paddle.x += PADDLE_SPEED;
        if (paddle.x > clientRect.right - paddle.width) paddle.x = clientRect.right - paddle.width;
        break;
    }

    InvalidateRect(hWnd, NULL, TRUE);
}

void DrawPaddle(HDC hdc, int xPos) {
    Rectangle(hdc, xPos, paddle.y, xPos + paddle.width, paddle.y + paddle.height);
}

void DrawScore(HDC hdc) {
    TCHAR scoreText[20];
    wsprintf(scoreText, TEXT("Score: %d"), score);

    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    HFONT hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    RECT textRect;
    GetClientRect(WindowFromDC(hdc), &textRect);
    DrawText(hdc, scoreText, -1, &textRect, DT_CALCRECT | DT_RIGHT | DT_TOP);

    TextOut(hdc, textRect.right - 50, 10, scoreText, lstrlen(scoreText));

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}


void DrawHeart(HDC hdc) {
    TCHAR scoreText[20];
    wsprintf(scoreText, TEXT("Life: %d"), heart);

    SetTextColor(hdc, RGB(240, 0, 0));
    HFONT hFont = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    RECT textRect;
    GetClientRect(WindowFromDC(hdc), &textRect);
    DrawText(hdc, scoreText, -1, &textRect, DT_CALCRECT | DT_RIGHT | DT_TOP);

    TextOut(hdc, SCREEN_WIDTH - 90, 12, scoreText, lstrlen(scoreText));

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}





bool CheckCollisionWithPaddle() {
    return ball.x + ball.size >= paddle.x && ball.x <= paddle.x + paddle.width &&
           ball.y + ball.size >= paddle.y && ball.y <= paddle.y + paddle.height;
}

bool CheckCollisionWithFloor() {
    return ball.y + ball.size + 35 >= SCREEN_HEIGHT ;
}



void ShowGameOver(HWND hWnd) {
    TCHAR scoreText[40];
    wsprintf(scoreText, TEXT("Game Over!!! Your final score is:: %d"), score);

    MessageBox(hWnd, scoreText, "Game Over!!!", MB_OK | MB_ICONINFORMATION);

    ball.x = -100;
    ball.y = -100;
    ball.speedX = 0;
    ball.speedY = 0;
    InvalidateRect(NULL, NULL, TRUE);
}



