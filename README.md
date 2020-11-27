# Barnes Hut Simulation
An n-body simulation using the Barnes-Hut algorithm.

## Building
To build both the simulation and viewer:
```bash
mkdir build
cd build
cmake ..
```

If only the simulation is wanted:
````bash
mkdir build
cd build
cmake .. -DNO_VIEWER=1
````

## Running
To run a simulation:
```bash
./sim <directory> <num_particles> <num_steps> <init_rot> <mean_rand_vel>
```
`directory` is the path to the directory where the resulting csv files will go. This directory must have a folder named `pos` in it before running. For example:
```bash
mkdir my_simulation
cd my_simulation
mkdir pos
cd ..
./sim my_simulation/
```

`num_particles` is the number of particles in the simulation.
`num_steps` is the number of 0.0001`Gyr` timesteps to run the simulation for.
`init_rot` is the initial total angular velocity of the particles in `rad/Gyr`.
`mean_rand_vel` is the mean value of random velocities with a standard deviation of 30% the mean in `km/s`.

## Output
The simulation outputs .csv files containing pertinent information.
`pos` contains a .csv file with every particles position at each epoch in `pc`.
`properties.csv` contains information about the simulation itself.
`total_vel.csv` contains the total velocity at every epoch in `pc/Myr`.
`total_ke.csv` contains the total kinetic energy at every epoch in `(M_sol pc^2)/(Myr^2)`.
`total_pe.csv` contains the total kinetic energy at every epoch in `(M_sol pc^2)/(Myr^2)`.
Parameters can further be tweaked in the `constants.h` file.