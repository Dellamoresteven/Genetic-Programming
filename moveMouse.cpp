#include <Windows.h>
#include <iostream>
#define print(x) std::cout << #x << " : " << x << std::endl;


static int xMouse = 200, yMouse = 200, t = 1;
POINT p;

void mouse_event(int _1, int dx, int dy, int _4, int _5)
{
  xMouse += dx; yMouse += dy;
  std::cout << "mouse_event(" << _1 << ", " << dx << ", " << dy << ", " << _4 << ", " << _5 << "): "
    << xMouse << ", " << yMouse << '\n';
    SetCursorPos(xMouse,yMouse);
}

void AccurateSleep(int delay)
{
  t += delay;
  std::cout << "AccurateSleep(" << delay << "): " << t << '\n';

}

void Sleep(int delay)
{
  t += delay;
  std::cout << "Sleep(" << delay << "): " << t << '\n';
}

void Smoothing(int smoothing, int delay, int x, int y,int mouseX, int mouseY) {
  x = x - mouseX;
  y = y - mouseY;
  // if(mouseX > x){
  //   x = x - mouseX;
  //   print('hi');
  // } else {
  //   x = x - mouseX;
  // }

  int x_ = 0, y_ = 0, t_ = 0;
  for (int i = 1; i <= smoothing; ++i) {
    // i / smoothing provides the interpolation paramter in [0, 1]
    int xI = i * x / smoothing;
    int yI = i * y / smoothing;
    int tI = i * delay / smoothing;
    mouse_event(1, xI - x_, yI - y_, 0, 0);
    AccurateSleep(tI - t_);
    x_ = xI; y_ = yI; t_ = tI;
  }
}

#define PRINT_AND_DO(...) std::cout << #__VA_ARGS__ << ";\n"; __VA_ARGS__ 

int main()
{
    GetCursorPos(&p);
    xMouse = p.x;
    yMouse = p.y; 
    PRINT_AND_DO(Smoothing(30, 132, 500, 900,xMouse,yMouse));
//   PRINT_AND_DO(xMouse = 0; yMouse = 0; t = 0);
//   PRINT_AND_DO(Smoothing(20, 15, 10, 0));
}