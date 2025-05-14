**Problem Statement: Spring Workers**
Simulate a tree laden with fruits. Launch three “picker” processes and a “loader” process in parallel. If there is
fruit on the tree, the picker picks it and places it in a slot of a crate with 12 slots. When a picker finds that the
crate is full, it calls the loader. It waits for the loader to place this crate in a truck. Then, the loader furnishes a
new crate for the pickers. We assume there is enough space in the truck for all crates. All pickers return to the
main function when the tree is bare. In the end, the loader places any partially filled crate in the truck if present.
If a picker is adding to the last crate, the loader waits for it to complete the action.
Points to note:
• The number of fruits on the tree is known globally.
• This tree is implemented as an integer array to represent different pieces of fruit.
• The main function provides a shared empty crate when execution starts.
• A piece of fruit can be picked only once and by only one picker for obvious reasons.
Synchronize the processes, enforcing mutual exclusion where necessary. Produce a working code such that the
output identifies each action and the agent performing this action.
