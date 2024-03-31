#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
using namespace std;
using namespace std::chrono;

int xor64() {
    static uint64_t x = 88172645463345263ULL;
    x ^= x<<13;
    x ^= x>> 7;
    x ^= x<<17;
    return int(x&0x7fffffff);
}

struct Line
{
    int a0 = 0;
    int a1 = 0;
    int h = 0;
    int x = 0;

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
    int overflow = 0;

    vector<vector<Line>> lines(D);

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
                    lines[d].push_back(Line(i, mj, get_h(i, mj)));
                }
        }

        // 山登り。
        {
            for (int step=0; step<8096; step++)
            {
                int r0 = xor64()%(N2/2);
                int r1 = xor64()%(N2/2);
                if (r0==r1)
                    continue;

                Line &l0 = lines[d][r0];
                Line &l1 = lines[d][r1];

                int t0 = xor64()%2;
                int t1 = xor64()%2;
                swap(
                    t0==0 ? l0.a0 : l0.a1,
                    10==0 ? l1.a0 : l1.a1);

                int h02 = get_h(l0.a0, l0.a1);
                int h12 = get_h(l1.a0, l1.a1);

                if (h02+h12<l0.h+l1.h)
                {
                    l0.h = h02;
                    l1.h = h12;
                }
                else
                    swap(
                        t0==0 ? l0.a0 : l0.a1,
                        10==0 ? l1.a0 : l1.a1);
            }

            for (Line &line: lines[d])
                if (A[line.a0]>A[line.a1])
                    swap(line.a0, line.a1);
        }

        sort(lines[d].begin(), lines[d].end(), [&](const Line &l0, const Line &l1) {return l0.h<l1.h;});

        // イベントホールの高さを超えるなら、最後からライン減らす。
        {
            int h = 0;
            for (Line &line: lines[d])
                h += line.h;

            for (int i=N/2-1; i>=0 && h>W; i--)
            {
                int dd = min(h-W, lines[d][i].h-1);
                lines[d][i].h -= dd;
                h -= dd;
            }
        }

        sort(lines[d].begin(), lines[d].end(), [&](const Line &l0, const Line &l1) {return l0.h<l1.h;});
    }

    // 高さの余りをなるべく前後と高さが合うように割り振る。
    {
        for (int d=0; d<D; d++)
        {
            int h = W;
            for (Line &line: lines[d])
                h -= line.h;

            vector<bool> F(lines[d].size());

            if (d>0)
            {
                vector<pair<int, int>> C;
                for (int i=0; i<(int)lines[d].size(); i++)
                    if (!F[i] &&
                        lines[d-1][i].h>=lines[d][i].h)
                        C.push_back({lines[d-1][i].h-lines[d][i].h, i});
                sort(C.begin(), C.end());

                for (auto c: C)
                    if (c.first<=h)
                    {
                        h -= c.first;
                        lines[d][c.second].h += c.first;
                        F[c.second] = true;
                    }
            }

            if (d<D-1)
            {
                vector<pair<int, int>> C;
                for (int i=0; i<(int)lines[d].size(); i++)
                    if (!F[i] &&
                        lines[d+1][i].h>=lines[d][i].h)
                        C.push_back({lines[d+1][i].h-lines[d][i].h, i});
                sort(C.begin(), C.end());

                for (auto c: C)
                    if (c.first<=h)
                    {
                        h -= c.first;
                        lines[d][c.second].h += c.first;
                        F[c.second] = true;
                    }
            }

            int n = 0;
            for (bool f: F)
                if (!f)
                    n++;
            if (n>0)
            {
                while (h>0)
                    for (int i=0; i<(int)lines[d].size() && h>0; i++)
                    {
                        h--;
                        lines[d][i].h++;
                    }
            }
            else
            {
                sort(lines[d].begin(), lines[d].end(), [&](const Line &l0, const Line &l1) {return l0.h<l1.h;});
                lines[d].back().h += h;
            }

            sort(lines[d].begin(), lines[d].end(), [&](const Line &l0, const Line &l1) {return l0.h<l1.h;});
        }
    }

    // 横の壁をなるべく作らないように並び替え。
    if (N<40)
    {
        for (int d=1; d<D; d++)
        {
            double tm = chrono::duration_cast<chrono::nanoseconds>(system_clock::now()-start).count()*1e-9;
            if (tm>2.0)
                break;

            vector<bool> PH(W+1);
            int y = 0;
            for (Line &line: lines[d-1])
            {
                PH[y] = true;
                y += line.h;
            }

            int n = (int)lines[d].size();

            vector<int> T(1<<n, -1);
            vector<vector<int>> TO(1<<n);
            T[0] = 0;

            for (int i=0; i<n; i++)
            {
                vector<int> P(1<<n, -1);
                vector<vector<int>> PO(1<<n);
                P.swap(T);
                PO.swap(TO);

                for (int b=0; b<1<<n; b++)
                    if (P[b]>=0)
                    {
                        int y = 0;
                        for (int o: PO[b])
                            y += lines[d][o].h;

                        for (int j=0; j<n; j++)
                            if ((b>>j&1)==0)
                            {
                                int t = P[b]+(PH[y+lines[d][j].h]?1:0);
                                if (t>T[b|1<<j])
                                {
                                    T[b|1<<j] = t;
                                    TO[b|1<<j] = PO[b];
                                    TO[b|1<<j].push_back(j);
                                }
                            }
                    }
            }

            vector<Line> temp = lines[d];
            lines[d].clear();
            for (int o: TO[(1<<n)-1])
                lines[d].push_back(temp[o]);
        }
    }

    // 壁のx座標を調整
    {
        vector<int> PV(W, -1);

        for (int d=0; d<D; d++)
        {
            int y = 0;
            for (Line &line: lines[d])
            {
                int xmin = ((line.a0<N?a[d][line.a0]:0)+line.h-1)/line.h;
                int xmax = W-((line.a1<N?a[d][line.a1]:0)+line.h-1)/line.h;
                xmax = min(W/2, xmax);
                if (xmin>xmax)
                    line.x = xmin;
                else
                {
                    int ms = -1;
                    int mx = 0;
                    for (int x=xmin; x<=xmax; x++)
                    {
                        int s = 1;
                        for (int dy=0; dy<line.h; dy++)
                            if (PV[y+dy]==x)
                                s++;
                        s = s*1000 - abs((xmin+xmax)/2-x);

                        if (s>ms)
                        {
                            ms = s;
                            mx = x;
                        }
                    }
                    line.x = mx;
                }

                line.x = min(W-1, line.x);
                line.x = max(line.a0<N?1:0, line.x);

                for (int dy=0; dy<line.h; dy++)
                    PV[y+dy] = line.x;

                y += line.h;
            }
        }
    }

    for (int d=0; d<D; d++)
    {
        // 答えに変換。
        vector<Area> answer(N);
        {
            int y = 0;
            for (Line &line: lines[d])
            {
                if (line.a0<N)
                {
                    answer[line.a0].x0 = 0;
                    answer[line.a0].y0 = y;
                    answer[line.a0].x1 = line.x;
                    answer[line.a0].y1 = y+line.h;
                }
                if (line.a1<N)
                {
                    answer[line.a1].x0 = line.x;
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
                {
                    score += (a[d][i]-answer[i].area())*100;
                    overflow++;
                    //cerr<<"!"<<d<<" "<<i<<" "<<a[d][i]<<" "<<answer[i].area()<<endl;
                }

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

        fprintf(stderr, " %2d %2d %6lld %8lld %2d %5.3f\n", D, N, remain, score, overflow, time);
    }
}
