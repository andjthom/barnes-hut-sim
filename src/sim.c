#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include "./csv.h"
#include "./constants.h"
#include "./stretchy_buffer.h"

static void InitSpace();
static void BH_Step();
static void BH_CreateTree();
static void BH_FreeTree();
static int BH_CreateCell(double x, double y, double z, double size);
static void BH_AddToCell(int c, int p);
static void BH_MakeSubcells(int c);
static int BH_IsInCell(int c, int p);
static int BH_CellMeetsCriteria(int c, int p);
static void BH_Kick();
static void BH_KickFromCell(int c, int p);
static void BH_Drift();

static double GenRand();
static double GenRandEx();
static double NormalDist(double mean, double std_dev);
static void RecordData();

typedef struct {
    int particle;
    int num_particles;

    Vec3 pos;
    Vec3 center;
    double size;

    int subcells[8];
} Cell;

static const double TIMESTEP_MYR = TIMESTEP * 1000;

static int g_num_particles;
static int g_num_steps;
static double g_init_rot;
static double g_mean_rand_vel;
static double g_mass;

static Vec3 *g_pos = NULL;
static Vec3 *g_vel = NULL;
static Cell *g_cells = NULL;
static int g_num_cells;

static Vec3 g_total_vel = {0, 0, 0};
static double g_total_ke = 0;
static double g_total_pe = 0;

int main(int argc, char *argv[])
{
    int steps_simulated, bar_pos, i;
    char pos_dir[256];
    char total_vel_filename[256];
    char total_ke_filename[256];
    char total_pe_filename[256];
    FILE *total_vel_file, *total_ke_file, *total_pe_file;

    srand(time(NULL));

    if (argc != 6)
    {
        printf("Usage: %s <directory> <num_particles> <num_steps> <init_rot(in rad/Gyr)> <mean_rand_vel>\n",
               argv[0]);
        return;
    }
    else
    {
        g_num_particles = atoi(argv[2]);
        g_num_steps = atoi(argv[3]);
        g_init_rot = atof(argv[4]);
        g_mean_rand_vel = atof(argv[5]);
    }

    if (!CSV_WritePropFile(argv[1], g_num_particles, g_num_steps, TIMESTEP, STEPS_PER_EPOCH, AREA_SIZE,
                           g_init_rot, g_mean_rand_vel))
    {
        return -1;
    }

    g_mass = TOTAL_MASS / g_num_particles;
    g_pos = (Vec3 *)malloc(g_num_particles * sizeof(Vec3));
    g_vel = (Vec3 *)malloc(g_num_particles * sizeof(Vec3));
    InitSpace();

    sprintf(pos_dir, "%spos/", argv[1]);
    sprintf(total_vel_filename, "%stotal_vel.csv", argv[1]);
    sprintf(total_ke_filename, "%stotal_ke.csv", argv[1]);
    sprintf(total_pe_filename, "%stotal_pe.csv", argv[1]);

    if (!CSV_WriteStepFile(pos_dir, "particle,pos_x,pos_y,pos_z", 0, g_num_particles, g_pos))
    {
        return -1;
    }

    total_vel_file = CSV_InitWrite(total_vel_filename, "step,total_vel_x,total_vel_y,total_vel_z");
    if (!total_vel_file)
    {
        return -1;
    }

    total_ke_file = CSV_InitWrite(total_ke_filename, "step,total_ke");
    if (!total_ke_file)
    {
        return -1;
    }

    total_pe_file = CSV_InitWrite(total_pe_filename, "step,total_pe");
    if (!total_pe_file)
    {
        return -1;
    }

    RecordData();
    CSV_WriteVec3(total_vel_file, 0, g_total_vel);
    CSV_WriteDouble(total_ke_file, 0, g_total_ke);
    CSV_WriteDouble(total_pe_file, 0, g_total_pe);

    steps_simulated = 0;
    while (steps_simulated < g_num_steps)
    {
        BH_Step();
        steps_simulated++;

        if (steps_simulated % STEPS_PER_EPOCH == 0)
        {
            RecordData();
            CSV_WriteStepFile(pos_dir, "particle,pos_x,pos_y,pos_z", steps_simulated, g_num_particles, g_pos);
            CSV_WriteVec3(total_vel_file, steps_simulated, g_total_vel);
            CSV_WriteDouble(total_ke_file, steps_simulated, g_total_ke);
            CSV_WriteDouble(total_pe_file, steps_simulated, g_total_pe);
        }

        /* Progress bar */
        bar_pos = BAR_WIDTH * steps_simulated / g_num_steps;
        printf("[");

        for (i = 0; i < BAR_WIDTH; i++)
        {
            if (i < bar_pos)
                printf("=");
            else if (i == bar_pos)
                printf(">");
            else
                printf("-");
        }

        printf("] %d / %d\r", steps_simulated, g_num_steps);
        fflush(stdout);
    }

    printf("\n");
    CSV_Close(total_vel_file);
    CSV_Close(total_ke_file);
    CSV_Close(total_pe_file);
    free(g_pos);
    free(g_vel);

    return 0;
}

static void InitSpace()
{
    int i;
    double omega_0;

    omega_0 = g_init_rot / g_num_particles / 1000; /* from rad Gyr^-1 to rad Myr^-1 */
    g_mean_rand_vel *= 0.98; /* from km/sec to pc/Myr */

#if 1
    for (i = 0; i < g_num_particles; i++)
    {
        Vec3 p, v;
        double v_rand, theta, phi;

        p.x = GenRandEx() * (AREA_SIZE / 2.0);
        p.y = GenRandEx() * (AREA_SIZE / 2.0);
        p.z = GenRandEx() * (AREA_SIZE / 2.0);

        v.x = -p.y * omega_0;
        v.y = p.x * omega_0;

        v_rand = NormalDist(g_mean_rand_vel, g_mean_rand_vel * 0.3);
        theta = M_PI * GenRand();
        phi = 2 * M_PI * GenRand();
        v.x = v.x + (v_rand * sin(theta) * cos(phi));
        v.y = v.y + (v_rand * sin(theta) * sin(phi));
        v.z = v_rand * cos(theta);

        g_pos[i] = p;
        g_vel[i] = v;
    }
#else
    for (i = 0; i < g_num_particles; i++)
    {
        g_pos[i].x = GenRandEx() * (AREA_SIZE / 2.0);
        g_pos[i].y = GenRandEx() * (AREA_SIZE / 2.0);
        g_pos[i].z = GenRandEx() * (AREA_SIZE / 2.0);
        g_vel[i].x = -g_pos[i].x * omega_0;
        g_vel[i].y = g_pos[i].y * omega_0;
        g_vel[i].z = 0;
    }
#endif
}

static void BH_Step()
{
    BH_CreateTree();
    BH_Kick();
    BH_FreeTree();
    BH_Drift();
}

/*
 * Create entire tree of cells.
 */
static void BH_CreateTree()
{
    int size = AREA_SIZE * 10000;
    int half_size = size / 2;
    int i;

    g_num_cells = 0;
    BH_CreateCell(-half_size, -half_size, -half_size, size);

    for (i = 0; i < g_num_particles; i++)
    {
        BH_AddToCell(0, i);
    }
}

static void BH_FreeTree()
{
    SB_FREE(g_cells);
}

static int BH_CreateCell(double x, double y, double z, double size)
{
    int c;
    Cell cell;

    cell.particle = -1;
    cell.num_particles = 0;
    cell.pos.x = x;
    cell.pos.y = y;
    cell.pos.z = z;
    cell.center.x = 0;
    cell.center.y = 0;
    cell.center.z = 0;
    cell.size = size;
    SB_PUSH(g_cells, cell);

    c = g_num_cells;
    g_num_cells++;
    return c;
}

/*
 * Adds a point p to cell c and update c's center of mass.
 * If the cell already has a point, we must subdivide and try to add the point to each subcell as well as the
 * old particle that was already in the cell.
 */
static void BH_AddToCell(int c, int p)
{
    int num_particles = g_cells[c].num_particles;

    if (num_particles > 0)
    {
        int i;

        if (num_particles == 1)
        {
            BH_MakeSubcells(c);
            for (i = 0; i < 8; i++)
            {
                if (BH_IsInCell(g_cells[c].subcells[i], g_cells[c].particle))
                {
                    BH_AddToCell(g_cells[c].subcells[i], g_cells[c].particle);
                    break;
                }
            }

            g_cells[c].particle = -1;
        }

        for (i = 0; i < 8; i++)
        {
            if (BH_IsInCell(g_cells[c].subcells[i], p))
            {
                BH_AddToCell(g_cells[c].subcells[i], p);
                break;
            }
        }
    }
    else
    {
        g_cells[c].particle = p;
    }

    g_cells[c].center.x = (num_particles * g_cells[c].center.x + g_pos[p].x) / (num_particles + 1);
    g_cells[c].center.y = (num_particles * g_cells[c].center.y + g_pos[p].y) / (num_particles + 1);
    g_cells[c].center.z = (num_particles * g_cells[c].center.z + g_pos[p].z) / (num_particles + 1);
    g_cells[c].num_particles++;
}

/*
 * Makes 8 subcells for parent cell c;
 */
static void BH_MakeSubcells(int c)
{
    int cell;
    double size = g_cells[c].size / 2.0;
    double x = g_cells[c].pos.x;
    double y = g_cells[c].pos.y;
    double z = g_cells[c].pos.z;

    cell = BH_CreateCell(x       , y       , z       , size);
    g_cells[c].subcells[0] = cell;
    cell = BH_CreateCell(x + size, y       , z       , size);
    g_cells[c].subcells[1] = cell;
    cell = BH_CreateCell(x + size, y       , z + size, size);
    g_cells[c].subcells[2] = cell;
    cell = BH_CreateCell(x       , y       , z + size, size);
    g_cells[c].subcells[3] = cell;
    cell = BH_CreateCell(x       , y + size, z       , size);
    g_cells[c].subcells[4] = cell;
    cell = BH_CreateCell(x + size, y + size, z       , size);
    g_cells[c].subcells[5] = cell;
    cell = BH_CreateCell(x + size, y + size, z + size, size);
    g_cells[c].subcells[6] = cell;
    cell = BH_CreateCell(x       , y + size, z + size, size);
    g_cells[c].subcells[7] = cell;
}

/*
 * Returns non-zero if particle p is within cell c.
 */
static int BH_IsInCell(int c, int p)
{
    return g_pos[p].x > g_cells[c].pos.x && g_pos[p].x <= (g_cells[c].pos.x + g_cells[c].size) &&
           g_pos[p].y > g_cells[c].pos.y && g_pos[p].y <= (g_cells[c].pos.y + g_cells[c].size) &&
           g_pos[p].z > g_cells[c].pos.z && g_pos[p].z <= (g_cells[c].pos.z + g_cells[c].size);
}

/*
 * Returns 0 if cell is not far away enough to approximate the force on particle p.
 * Returns non-zero otherwise or if the cell has no subcells.
 */
static int BH_CellMeetsCriteria(int c, int p)
{
    if (g_cells[c].num_particles > 1)
    {
        double rx = g_pos[p].x - g_cells[c].center.x;
        double ry = g_pos[p].y - g_cells[c].center.y;
        double rz = g_pos[p].z - g_cells[c].center.z;
        double r = sqrt(rx*rx + ry*ry + rz*rz);

        /* TODO: Verify that r > SOFTENING has correct behavior */
#if 1
        return ((r / g_cells[c].size) > THETA && r > SOFTENING);
#else
        return ((r / g_cells[c].size) > THETA);
#endif
    }
    else
    {
        return 1;
    }
}

/*
 * Calculate force acting on all points.
 * Uses a stretchy buffer as a queue for cells to examine.
 */
static void BH_Kick()
{
    int head, i;
    int *queue = NULL;

    for (i = 0; i < g_num_particles; i++)
    {
        head = 0;
        SB_PUSH(queue, 0);
        while (head < SB_LEN(queue))
        {
            int c = queue[head];
            if (BH_CellMeetsCriteria(c, i))
            {
                if (g_cells[c].num_particles > 0 && g_cells[c].particle != i)
                {
                    BH_KickFromCell(c, i);
                }
            }
            else
            {
                int j;
                for (j = 0; j < 8; j++)
                {
                    SB_PUSH(queue, g_cells[c].subcells[j]);
                }
            }

            head++;
        }

        SB_FREE(queue);
    }
}

/*
 * Calculate new force and velocity of particle p from cell c.
 * Force equations derived from Dyer, Charles & Ip, Peter. (1993).
 */
static void BH_KickFromCell(int c, int p)
{
    double rx, ry, rz, r2;

    rx = g_cells[c].center.x - g_pos[p].x;
    ry = g_cells[c].center.y - g_pos[p].y;
    rz = g_cells[c].center.z - g_pos[p].z;
    r2 = rx*rx + ry*ry + rz*rz;

    if (r2 > SOFTENING * SOFTENING)
    {
        g_vel[p].x += G * TIMESTEP_MYR * rx * g_mass / pow(r2, 1.5);
        g_vel[p].y += G * TIMESTEP_MYR * ry * g_mass / pow(r2, 1.5);
        g_vel[p].z += G * TIMESTEP_MYR * rz * g_mass / pow(r2, 1.5);
    }
    else
    {
        double r = sqrt(r2);
        double x = r / SOFTENING;
        double f = x * (8 - 9*x + 2*x*x*x);
        g_vel[p].x += G * TIMESTEP_MYR * f * rx * g_mass / (SOFTENING * SOFTENING * r);
        g_vel[p].y += G * TIMESTEP_MYR * f * ry * g_mass / (SOFTENING * SOFTENING * r);
        g_vel[p].z += G * TIMESTEP_MYR * f * rz * g_mass / (SOFTENING * SOFTENING * r);
    }
}

/*
 * Calculate new position for each particle. Also update total velocity and KE.
 */
static void BH_Drift()
{
    int i;

    for (i = 0; i < g_num_particles; i++)
    {
        g_pos[i].x += g_vel[i].x * TIMESTEP_MYR;
        g_pos[i].y += g_vel[i].y * TIMESTEP_MYR;
        g_pos[i].z += g_vel[i].z * TIMESTEP_MYR;
    }
}

/*
 * Random number from 0 to 1.
 */
static double GenRand()
{
    return (rand() / (double)RAND_MAX);
}

/*
 * Random number from -1 to 1.
 */
static double GenRandEx()
{
    return (rand() / (double)RAND_MAX * 2.0 - 1.0);
}

/*
 * Box-Muller transform
 */
static double NormalDist(double mu, double sigma)
{
    static int second_call = 0;
    static double z1;
    double z0, u1, u2;

    if (second_call == 1)
    {
        second_call = 0;
        return z1;
    }

    second_call = 1;
    u1 = GenRand();
    u2 = GenRand();

    z0 = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
    z0 = z0 * sigma + mu;

    z1 = sqrt(-2 * log(u1)) * sin(2 * M_PI * u2);
    z1 = z1 * sigma + mu;

    return z0;
}

static void RecordData()
{
    int i;

    g_total_vel.x = 0;
    g_total_vel.y = 0;
    g_total_vel.z = 0;
    g_total_ke = 0;
    g_total_pe = 0;

    for (i = 0; i < g_num_particles; i++)
    {
        int j;
        double speed2;

        g_total_vel.x += g_vel[i].x;
        g_total_vel.y += g_vel[i].y;
        g_total_vel.z += g_vel[i].z;

        speed2 = g_vel[i].x*g_vel[i].x + g_vel[i].y*g_vel[i].y + g_vel[i].z*g_vel[i].z;
        g_total_ke += 0.5 * g_mass * speed2;

        for (j = 0; j < g_num_particles; j++)
        {
            if (i != j)
            {
                double rx, ry, rz, r;

                rx = g_pos[i].x - g_pos[j].x;
                ry = g_pos[i].y - g_pos[j].y;
                rz = g_pos[i].z - g_pos[j].z;
                r = sqrt(rx*rx + ry*ry + rz*rz);

                if (r > SOFTENING)
                {
                    g_total_pe += (G * g_mass * g_mass) / r;
                }
                else
                {
                    double x = r / SOFTENING;
                    double f = x * (8 - 9*x + 2*x*x*x);
                    g_total_pe += (G * f * g_mass * g_mass) / SOFTENING;
                }
            }
        }
    }
}
