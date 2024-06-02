# FreeRTOS Multi-Task Communication with Queue and Timers
## Description
This project demonstrates communication between four tasks using a FreeRTOS queue on an emulation board provided through Eclipse CDT Embedded.

## System Design
![image](https://github.com/AliMamdouh2025/FreeRTOS_Task_Coordination_Simulation/assets/144431914/767088d1-570b-45b4-9375-2142f6ba9a46)
![image](https://github.com/AliMamdouh2025/FreeRTOS_Task_Coordination_Simulation/assets/144431914/06536792-03b0-4177-9423-599b04f7d06d)
![image](https://github.com/AliMamdouh2025/FreeRTOS_Task_Coordination_Simulation/assets/144431914/521ed5dc-686f-4893-9bb6-8907bac4b375)

- Tasks:
1. Three Sender Tasks (two with same priority, one with higher priority)
2.  One Receiver Task
- Communication: Tasks communicate through a fixed-size queue.
- Data: Messages contain the string "Time is XYZ" where XYZ is the current system time in ticks.
## Sender Tasks
Each sender task sleeps for a random period Tsender drawn from a uniform distribution defined by two arrays. Upon waking, it sends a message to the queue. If the queue is full, the operation fails, and a counter for blocked messages is incremented. On successful send, a counter for transmitted messages is incremented. Counters are maintained per task. After sending, the task sleeps again for another random period. 
## Receiver Task
Sleeps for a fixed period Treceiver (100 ms). Wakes up and checks for a message in the queue. If a message exists, it reads it, increments the total received messages counter, and sleeps again. If no message is available, it sleeps again immediately. Reads only one message at a time, even if multiple messages are present.
## Timers
Each task has a dedicated timer for sleep/wake control.
## Timer Callbacks
- Sender Timer Callback:
Releases a dedicated semaphore upon being called.
Unblocks the sender task for sending to the queue.
- Receiver Timer Callback:
Releases a dedicated semaphore upon being called.
Unblocks the receiver task for message retrieval.
If 1000 messages are received, it calls the "Reset" function.
## Reset Function
- Prints total successfully sent messages, total blocked messages, and statistics per sender task.
- Resets total sent, blocked, and received message counters.
- Clears the queue.
- Updates Tsender values from arrays defining the lower and upper bounds of the uniform distribution.The arrays hold values for different iterations.Starts with values 50 and 150 milliseconds.
- If all values in the arrays are used:
Destroys timers. Prints "Game Over" and stops execution.
