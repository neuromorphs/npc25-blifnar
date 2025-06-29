#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <unistd.h>
//           _01234567_
char art[] = " .-:*#%0";
int framebuffer[64*32];


void load(int* frame)
{
    unsigned char header[256];
    unsigned char data[32*64*4];
    FILE* f = fopen("bounce.bmp", "rb");
    fread(header, 138, 1, f);
    fread(data, 32*64*4, 1, f);

    for (int q = 0; q < 32*64; ++q)
        frame[q] = data[q*4];
}
void display(const int* frame, int w = 64, int h = 32)
{
    printf(",---.---.---.---.---.---.---.---  --.---.---.---.---.---.---.--.\n");
    for (int hh = 0; hh < h; hh++) {
        for (int ww = 0; ww < w; ww++)
            printf("%c", art[frame[hh*w+ww]/(256/8)]);
            // printf("%c", art[ww%8]);
        printf("\n");
    }
    printf("`---.---.---.---.---.---.---.---  --.---.---.---.---.---.---.--'\n");
}


struct Particle {
    float pos[2];
    float vel[2];
    int color;
};

Particle particles [64*32];
Particle particles_[64*32];
Particle particlesO[64*32];

void load(Particle* ps, int w = 64, int h = 32)
{
    unsigned char header[256];
    unsigned char data[32*64*4];
    FILE* f = fopen("bounce.bmp", "rb");
    fread(header, 138, 1, f);
    fread(data, 32*64*4, 1, f);

    for (int hh = 0; hh < h; hh++)
        for (int ww = 0; ww < w; ww++) {
            int q = hh*w+ww;
            ps[q].pos[0] = ww;//(float)(ww - 32) / 32.0f;
            ps[q].pos[1] = hh;//(float)(hh - 16) / 16.0f;
            ps[q].vel[0] = .0f;//-0.2f;
            ps[q].vel[1] = .0f;//0.1f;//0.01f;
            ps[q].color = data[q*4];
    }
}

void update(Particle* ps, const Particle* ps_last_t, const Particle* ps_orig, float dt, int w = 64, int h = 32)
{
    bool flag = false;

    // for (int q = 0; q < 32*64; ++q) {
    //     ps[q].pos[0] += ps[q].vel[0] * dt;
    //     ps[q].pos[1] += ps[q].vel[1] * dt;
    // }

    // int neighbour_offset[] = {-1, 1, -64, 64};
    int neighbour_offset[] = {-1, 1, -64, 64,
                              -64-1, -64+1, 64-1, 64+1};
    float neighbour_distance[] = {1, 1, 1, 1,
                              sqrtf(2.0f), sqrtf(2.0f), sqrtf(2.0f), sqrtf(2.0f)};

    for (int hh = 1; hh < h - 1; hh++)
        for (int ww = 1; ww < w - 1; ww++) {
            int q = hh*w+ww;
            Particle& me = ps[q];

            me.vel[0] *= 0.99f;
            me.vel[1] *= 0.99f;

            for (int n = 0; n < 8; n++) {
                const Particle& you = ps_last_t[q + neighbour_offset[n]];
                if (!flag && you.color == 0)
                    continue;

                float dx = you.pos[0] - me.pos[0];
                float dy = you.pos[1] - me.pos[1];
                float norm = sqrtf(dx*dx + dy*dy);
                me.vel[0] += dx * (norm - neighbour_distance[n]) * 0.33f;
                me.vel[1] += dy * (norm - neighbour_distance[n]) * 0.33f;
            }

            for (int n = 0; n < 64*32; n++) {
                if (n == q)
                    continue;

                bool is_neighboor = false;
                for (int nn = 0; nn < 8; nn++)
                    is_neighboor |= (n == q + neighbour_offset[nn]);
                if (is_neighboor)
                    continue;
                // if (n == q + neighbour_offset[0] ||
                //     n == q + neighbour_offset[1] ||
                //     n == q + neighbour_offset[2] ||
                //     n == q + neighbour_offset[3] ||
                //     n == q + neighbour_offset[4] ||
                //     n == q + neighbour_offset[5] ||
                //     n == q + neighbour_offset[6] ||
                //     n == q + neighbour_offset[7])
                //     continue;

                const Particle& you = ps_last_t[n];
                if (!flag && you.color == 0)
                    continue;

                float dx = you.pos[0] - me.pos[0];
                float dy = you.pos[1] - me.pos[1];
                float norm = sqrtf(dx*dx + dy*dy) + 1e-9f;
                if (norm < 0.99f)
                {
                    me.vel[0] -= (dx / norm) * 0.1f;
                    me.vel[1] -= (dy / norm) * 0.1f;
                    // me.vel[0] = 0;
                    // me.vel[1] = 0;
                }
            }

            float fx = me.vel[0];
            float fy = me.vel[1];
            float fnorm = sqrtf(fx*fx + fy*fy) + 1e-9f;
    
            me.vel[0] = (fx / fnorm) * fminf(fnorm, 1.0f);
            me.vel[1] = (fy / fnorm) * fminf(fnorm, 1.0f);

            me.vel[1] += 0.002f;
            me.vel[0] += 0.0005f;

            // me.vel[1] += 0.02f;
            // me.vel[0] += 0.005f;

            if (ps_orig)
            {
                const Particle& orig = ps_orig[q];

                float fx = orig.pos[0] - me.pos[0];
                float fy = orig.pos[1] - me.pos[1];
                float fnorm = sqrtf(fx*fx + fy*fy) + 1e-9f;
                me.vel[0] += (fx / fnorm) * fminf(fnorm, .005f);
                me.vel[1] += (fy / fnorm) * fminf(fnorm, .005f);
            }


            me.pos[0] += me.vel[0] * dt;
            me.pos[1] += me.vel[1] * dt;

            me.pos[0] = fmax(me.pos[0], 2.0f);
            me.pos[1] = fmax(me.pos[1], 2.0f);
            me.pos[0] = fmin(me.pos[0], 64.f);
            me.pos[1] = fmin(me.pos[1], 32.f);
        }

    // for (int hh = 1; hh < h - 1; hh++)
    //     for (int ww = 1; ww < w - 1; ww++) {
    //         int q = hh*w+ww;
    //         float new0 = ps[q].pos[0] + ps[q].vel[0] * dt;
    //         float new1 = ps[q].pos[1] + ps[q].vel[1] * dt;
    //         int addr = int(new1)*64 + new0;
    //         if (ps_last_t[addr].color > 0)
    //             continue;
    //         ps[q].pos[0] = new0;
    //         ps[q].pos[1] = new1;
    //     }
}

void display(const Particle* ps, int* frame)
{
    memset(framebuffer, 0, 64*32*4);
    for (int q = 0; q < 32*64; ++q) {
        if (ps[q].color == 0)
            continue;
        if (ps[q].pos[0] < 0 || ps[q].pos[0] >= 64 ||
            ps[q].pos[1] < 0 || ps[q].pos[1] >= 32)
            continue;
        int addr = int(ps[q].pos[1])*64 + ps[q].pos[0];
        frame[addr] = ps[q].color;
    }
    display(frame);
}


int main(void)
{
    load(particles);
    memcpy(particlesO, particles, sizeof(particles));
    for (int frame = 0; frame < 2000; ++frame) {
        memcpy(particles_, particles, sizeof(particles));
        update(particles, particles_, (frame % 500 < 250) ? NULL : particlesO, 1.0f);
        display(particles, framebuffer);
        usleep(10000);
    }

    // load(framebuffer);
    // for (int q = 0; q < 100; ++q) {
    //     memcpy(framebuffer, framebuffer+1, 64*32*4);
    //     display(framebuffer);
    //     usleep(20000);
    // }


    // for (int x = 0; x < 64; x++)
    //     for (int y = 0; y < 32; y++)
    //         framebuffer[y*64+x] = x*4;

    // load(framebuffer);
    // display(framebuffer);

    // for (int x = 0; x < 64; x++)
    //     for (int y = 0; y < 32; y++)
    //         framebuffer[y*64+x] = y*8;
    // display(framebuffer);
    // int a = 'A';
    // printf("Hello world!!!\n");
    // // printf("%d\n", a);//chr('A'));
    return 0;
}