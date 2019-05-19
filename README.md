# taskfarming
HPC systems are always configured to increase productivity of large capability applications.
In some cases the granularity of the number of nodes allowed to be requested even blocks
serial codes automatically. However there are a number of cases where single processor codes
are not to be excluded: parameter sweeping applications, hard or impossible to paralelise
algorithms, untrained pool of users for a given HPC center, etc.
On Grid systems there is always a time overhead that render bulk submission impractical.
In these situations it would desirable if these systems could give the users an automatic
tool to deploy in high throughput a list of serial one processor tasks.

This work details the solution developed by us, as a very simple MPI code, for this situation.
