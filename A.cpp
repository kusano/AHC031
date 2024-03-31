#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
using namespace std;
using namespace std::chrono;

struct Line
{
    int a0 = 0;
    int a1 = 0;
    int h = 0;

    Line() {}
    Line(int a0, int a1, int h): a0(a0), a1(a1), h(h) {}
};

struct Area
{
    int x0 = 0;
    int y0 = 0;
    int x1 = 0;
    int y1 = 0;

    Area() {};
    Area(int x0, int y0, int x1, int y1): x0(x0), y0(y0), x1(x1), y1(y1) {}
    int area() {return (x1-x0)*(y1-y0);}
};

int main()
{
    int W, D, N;
    cin>>W>>D>>N;
    vector<vector<int>> a(D, vector<int>(N));
    for (int i=0; i<D; i++)
        for (int j=0; j<N; j++)
            cin>>a[i][j];

    system_clock::time_point start = system_clock::now();

    long long score = 1;
    vector<vector<bool>> H(W+1, vector<bool>(W));
    vector<vector<bool>> V(W, vector<bool>(W+1));

    for (int d=0; d<D; d++)
    {
        // サイズ0の予約を追加して偶数にする。
        int N2 = N;
        vector<int> A = a[d];
        if (N2%2!=0)
        {
            N2++;
            A.push_back(0);
        }

        // a0 と a1 をペアにしたときに必要な高さ。
        auto get_h = [&](int a0, int a1)
        {
            int h = (A[a0]+A[a1]+W-1)/W;
            if ((A[a0]+h-1)/h+(A[a1]+h-1)/h>W)
                h++;
            return h;
        };

        vector<Line> lines;

        // 余りが少なくなるようにペアを割り当てていく。
        {
            vector<bool> U(N2);
            for (int i=0; i<N2; i++)
                if (!U[i])
                {
                    int mr = W*W;
                    int mj = 0;
                    for (int j=0; j<N2; j++)
                        if (j!=i && !U[j])
                        {
                            int h = get_h(i, j);
                            int r = W*h-A[i]-A[j];
                            if (r<mr)
                            {
                                mr = r;
                                mj = j;
                            }
                        }

                    U[i] = U[mj] = true;

                    Line l(i, mj, get_h(i, mj));
                    if (A[l.a0]>A[l.a1])
                        swap(l.a0, l.a1);

                    lines.push_back(l);
                }
        }

        sort(lines.begin(), lines.end(), [&](const Line &l0, const Line &l1) {return l0.h<l1.h;});

        // イベントホールの高さを超えるなら、最後からライン減らす。
        {
            int h = 0;
            for (Line &line: lines)
                h += line.h;

            for (int i=N/2-1; i>=0 && h>W; i--)
            {
                int d = min(h, lines[i].h-1);
                lines[i].h -= d;
                h -= d;
            }
        }

        // 答えに変換。
        vector<Area> answer(N);
        {
            int y = 0;
            for (Line &line: lines)
            {
                int w = (A[line.a0]+line.h-1)/line.h;
                w = min(w, W-1);
                if (line.a0<N)
                {
                    answer[line.a0].x0 = 0;
                    answer[line.a0].y0 = y;
                    answer[line.a0].x1 = w;
                    answer[line.a0].y1 = y+line.h;
                }
                if (line.a1<N)
                {
                    answer[line.a1].x0 = w;
                    answer[line.a1].y0 = y;
                    answer[line.a1].x1 = W;
                    answer[line.a1].y1 = y+line.h;
                }
                y += line.h;
            }
        }

        // 出力。
        for (Area &area: answer)
            cout<<area.y0<<" "<<area.x0<<" "<<area.y1<<" "<<area.x1<<endl;

        // スコア計算。
        {
            for (int i=0; i<N; i++)
                if (answer[i].area()<a[d][i])
                    score += (a[d][i]-answer[i].area())*100;

            vector<vector<bool>> H2(W+1, vector<bool>(W));
            vector<vector<bool>> V2(W, vector<bool>(W+1));
            for (Area &area: answer)
            {
                for (int x=area.x0; x<area.x1; x++)
                {
                    H2[area.y0][x] = true;
                    H2[area.y1][x] = true;
                }
                for (int y=area.y0; y<area.y1; y++)
                {
                    V2[y][area.x0] = true;
                    V2[y][area.x1] = true;
                }
            }

            if (d>0)
            {
                for (int y=1; y<W; y++)
                    for (int x=0; x<W; x++)
                        if (H[y][x]!=H2[y][x])
                            score++;
                for (int y=0; y<W; y++)
                    for (int x=1; x<W; x++)
                        if (V[y][x]!=V2[y][x])
                            score++;
            }

            H = H2;
            V = V2;
        }
    }

    // デバッグ出力。
    {
        // 平均空き面積。
        long long remain = 0;
        for (int d=0; d<D; d++)
        {
            remain += W*W;
            for (int t: a[d])
                remain -= t;
        }
        remain = (remain+D/2)/D;

        // 計算時間。
        system_clock::time_point now = system_clock::now();
        double time = chrono::duration_cast<chrono::nanoseconds>(now-start).count()*1e-9;

        fprintf(stderr, " %2d %2d %6lld %8lld %5.3f\n", D, N, remain, score, time);
    }
}
