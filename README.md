# HighLevelPuzzle
- A simpilified implementation of SIGGRAPH 2022 paper "Computational Design of High-level Interlocking Puzzles"
- Course Project of ZJU "Advances in Computer Graphics" of 2023-2024 spring & summer semester

## TODO
- 06-07 ~ 06-11
  - [X] Basic Architecture (rendering, puzzle representation, etc.)

- 06-12 Wed
  - [X] _MaxMovableDistance & optimizations
  - [X] calculate neighbor configs
  - [X] rename all `pieceNo` to `pieceID`
  - [X] more DEBUG options
  - [X] add a variable to label the "depth" of configurations
  - [X] display all neighbor configs

- 06-13 Thu
  - [X] fix the bug of inconsistent piece colors between configs (do not re-assign colors!)
  - [X] fix the bug of potential acesss violation for configs that some pieces have already been removed (since I assume the vector indices are the ID of puzzle piece, if a puzzle piece is removed, problems will occur)
  - [X] adjust the camera position so that it always focuses on the center of the puzzle whatever the config is

- 06-14 Fri
  - [X] the criteria of determining two configs are the same should be carefully devised (absolute or relative?)
  - [X] compute the kernel disassembly graph and level of difficulty

- 06-15 Sat
  - [ ] display the kernel disassembly graph
  - [ ] compute the complete disassembly graph, display it
  - [ ] puzzle design?
