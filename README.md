## Design and Control System of an Infrared Obstacle Avoidance Car Based on TM4C123

### Project Background
With the continuous development of technology, intelligent cars have broad application prospects in industrial automation, logistics, home services, and other fields. Designing an infrared obstacle avoidance car system based on the TM4C123 can perceive obstacles through infrared modules and autonomously avoid collisions. It can also be remotely controlled via a PS2 handle, demonstrating high practicality and application value.

### Research Objectives
- **Industrial Automation:** Can be used for internal logistics transportation in factories, achieving efficient execution of transportation tasks with autonomous obstacle avoidance, thus improving production efficiency.
- **Home Services:** Can be used for home cleaning robots to avoid collisions with furniture, walls, and other obstacles, providing convenient home cleaning services.
- **Educational Field:** As a teaching example, it helps students understand the design and control principles of embedded systems, enhancing their practical skills and innovation awareness.

### Methods and Technologies
- **Hardware Selection:**
  - **Main Control Chip:** TM4C123
  - **Perception Module:** HW201 Infrared Obstacle Avoidance Module
  - **Control Module:** PS2 handle and receiver, TB6612FNG motor driver module
- **Motor Control:** Speed adjustment of the car is achieved by controlling the motor's speed through PWM (Pulse Width Modulation).
- **Speed Closed-Loop Control:** Using QEI (Quadrature Encoder Interface) to obtain the car's speed information, and through an incremental PID control algorithm to achieve closed-loop speed control, improving the car's stability and precision.
- **Obstacle Avoidance Function:** Utilizing the HW201 infrared obstacle avoidance module to sense obstacles ahead. When an obstacle is detected, the car avoids it by controlling the motor's direction or stopping.

### Project Outcomes
- **Successful remote control and speed adjustment of the car.**
- **Realization of closed-loop speed control, enhancing the car's operational stability.**
- **Implementation of infrared obstacle avoidance, allowing the car to autonomously avoid obstacles ahead.**


All files in the repositories are a project of code composer studio v8.

For more information you may can view the report.

Vedio :https://www.bilibili.com/video/BV1c841117dH/
