# GROUP FORMATIONS AND MOVEMENT

  # Description and goal

The goal of this project is to apply the seek steering behavior to a group of agents.

The agents will be capable to adopt a certain formation and keep it while moving.

The scope of the project includes:
  - 2 to 3 different formations
  - Group movement
  - Switching from one formation to another
  - Destination prediction for each agent in the formation
  
 
 The project will be done in unreal engine using the topdown template as a base. It will be C++ code based but blueprints will also be used for faster implementation of non AI related features.
 
  # Design and implementation
 
 After conducting the necessary research, I understood that a coordinated group movement or a group formation was based on the use of slots assigned to each unit and that the unit woukd follow. The position of all slots in the formation would be determined relatively to one leader slot and their orientation will be the one of the leader.
 
 To implement that, I created a base class formation from which would inherit every other formation class (line, circle etc). This bas class would contain a pointer to AActor, being the leader and an array of AFormationSlot, a class deriving from AActor. 
 I made the slot derived from AActor so i could spawn them in the game and use the built in AActor functions like GetActorLocation, GetActorRotation etc.
 
 For most of the formation an object the agent class (AGPP_ResearchCharacter) takes the role of the leader. That is, to simplify the movement behavior as the AGPP_ResearchCharacter class inherit from ACharacter and thus has a CharacterMovementComponent allowing me to move it around using AIMoveTo.
 
 When the group shortcut is pressed, the first unit selected becomes the leader and so the slots are spawn and placed relatively to it depending on the formation selected (line, circle, defense circle), then each slot is assigned to ons of the selected agents;
 
 In its update function, the class AGPP_ResearchCharacter seeks the assigned slot and so all of its objects move into formation.
 
 The last formation is a split group formation. When the group is already in formation and the solit function is called, the group will divide in two smaller group with the same formation. Pressing a second time will merge the group again.
 
 This is done with a Splited class deriving from formation which contains to Formation pointer. On initialization, a AFormationSlot object is spawn and will serve as leader. Then two Formation are created and their respective leader is assigned the current formation leader as the slot to follow. They are then placed at predetermined distance from it.
 
  # Possible applications and extensions
  
  A system that allows units to get into a specific formation, and keep while moving and even when getting close to obstacle would be obviously very useful to a real time strategy game. This opens the strategic possibilities for the player and allows a more in depth micro management.
  
  The movement destination prediction is a much needed player feedback in games with a lot of different units at all time on screen.
  
  Possible extensions and future projects:
    - With different unit types, formation could be designed to improve the effectiveness of a battle group.
    - On a bigger scale, coordinated units movement could allow the formation of batallion in some kind of Total War like games.
 
# Bibliography
 
- Artificial Intelligence For Games - Second Edition by Ian Millington and John Funge 
  - Chapter 3.7 -> Coordinated movement
  
 - Unreal online documentation
