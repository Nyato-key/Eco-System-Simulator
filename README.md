# Eco-System-Simulator

**Full Changelog**: https://github.com/Nyato-key/Eco-System-Simulator/commits/Simulator

### _Simulation of Natural Selection_
The Digital Organism Ecosystem Simulation is designed to vividly demonstrate how evolutionary dynamics unfold in a controlled, interactive virtual environment. In it, digital organisms must secure resources and adapt to environmental constraints, revealing the essential drivers of evolutionary change. The primary objective of the project is to develop an accessible, scientifically sound simulation that illustrates natural selection, resource competition, and genetic inheritance. By observing evolving populations and manipulating environmental conditions, users gain direct insight into the mechanisms driving evolution and ecosystem dynamics. Through sophisticated data tracking and visualization, the project captures the emergent behaviors and evolutionary trends that develop from simple initial conditions. The simulation serves as both an educational tool for understanding population dynamics and an experimental platform for testing how various factors influence evolutionary pathways in constrained ecosystems

## About
• Simulate the process of evolution through natural and sexual selection in a controlled virtual environment
• Implement genetic inheritance systems with trait-based advantages and adaptive mutation mechanisms
• Demonstrate how population genetics, change over time in response to environmental pressures and resource availability
• Analyze the emergence of optimal trait combinations through evolutionary pressure
• Visualize real-time evolutionary processes and their statistical outcomes through interactive graphs and tooltips

## Algorithm Design

**1. Natural Selection Algorithm:**
The simulation implements fitness-based natural selection where grubs with lower overall fitness have reduced survival rates. Fitness is calculated as a weighted combination of traits (40% speed, 40% sight, 20% metabolism efficiency), with periodic culling of the weakest individuals based on survival rates determined by resource availability.

**2. Sexual Selection Algorithm:**
Grubs select mates based on attractiveness scores calculated from fitness and proximity. The algorithm evaluates potential mates within a visible range, ranks them by attractiveness (fitness/distance ratio), and selects from the top three candidates for reproduction.

> _Effective Reproduction Chance_   =   B(1-N/C)

Reproduction Conditions: 
       Can Reproduce = alive 
                    hunger ≥ 0.75 	 
                       thirst ≥ 0.75      
              Cool down ≤ 0.0

**3. Genetic Inheritance System:**
• Trait Inheritance: Offspring inherit traits through a 50/50 random selection from either parent, rather than averaging
• Adaptive Mutation: Mutation rates scale based on population fitness - struggling populations experience larger mutations (increased exploration), while fit populations experience smaller refinements (exploitation)
• Trait Advantages: Grubs with higher sight can detect resources from farther distances; faster grubs move more efficiently toward targets.
• Spatial Optimization: Uniform grid partitioning enables efficient collision detection and neighbor queries

**4. Trait-Based Advantages:**
• Speed: Higher speed reduces movement cost and increases efficiency toward targets
• Sight: Extended sight range multiplies effective detection distance
• Metabolism: Lower metabolism reduces base resource consumption
• Trait Costs: Higher speed increases hunger cost; higher sight increases thirst cost


## Evolutionary Loop:
1.	Initialization: Create population with genetic diversity within trait bounds.
2.	Resource Acquisition: Grubs seek food/water based on needs and trait advantages.
3.	Reproduction Cycle:
• 	Check reproductive readiness (sufficient resources, no cool down)
• 	Find mate using fitness-based selection
• 	Generate offspring with inherited/mutated traits
• 	Apply reproductive energy cost to parents
4.	Statistics Collection: Sample population metrics at regular intervals.
5.	Termination: End simulation when population extinct, time limit reached, or user stopped.

## Simulation Core
• Population Management (Grub lifecycle, reproduction, death)
• Resource System (Food/water generation, consumption, respawning)
• Genetics Engine (Trait inheritance, mutation, fitness calculation)
•  Spatial System (Uniform grid for efficient spatial queries)
•  Selection Algorithms (Natural and sexual selection logic)
• Statistics Tracking (Data sampling, CSV export, graph generation)

## Limitations
• Simplified Genetics: The simulation uses a simplified genetic model without complex gene interactions, epistasis, or pleiotropy
• Deterministic Environment: Resource distribution and environmental factors remain constant, lacking seasonal variations or catastrophic events
• Limited Behavioral Complexity: Grub behaviors are limited to basic resource seeking and reproduction without complex social behaviors or learning
• Computational Constraints: Large populations (>5000) may experience performance degradation despite spatial optimization
• Memory Assumptions: The simulation assumes sufficient memory for genetic data storage and statistical sampling
• Real-time Constraints: The fixed timestep simulation may not accurately model variable time scales in real evolutionary processes

## Results and Analysis
The simulation demonstrates several key evolutionary principles:
1.	Adaptive Radiation: Initial genetically diverse populations converge toward optimal trait combinations based on environmental pressures
2.	Trade-off Evolution: The system naturally evolves trade-offs between speed (hunger cost) and sight (thirst cost) based on resource availability
3.	Mutation Rate Optimization: Populations self-regulate mutation rates - high during stress, low during stability
4.	Selective Pressure Response: Trait distributions shift predictably in response to modified reproduction rates and crowding factors
5.	Extinction Events: Populations can collapse when resource consumption outpaces regeneration, demonstrating carrying capacity limits

## Conclusion
The simulation successfully demonstrates evolutionary principles and population
dynamics. It provides insights into how genetic traits affect survival and reproduction,
while serving as an educational tool for understanding ecosystem modeling. The modular
design allows for future enhancements and more complex simulations.