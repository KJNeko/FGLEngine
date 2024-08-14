# This document is for the high level functionality of the engine



# Frame Begin

## Phase 1 (Simulation)
- Simulation tick
- Determine if the frame should be rendered. If not, We skip to the next 'frame'
- If the frame should render. We go to phase 2 (1.5 actually)

## Phase 1.5 (Transfer)
- Any data needing to be transferred to the device (GPU) will be done so within reason. If the buffer is full then we simply transfer what we could fit and move on.

## Phase 2 (Cameras)
- Cameras will have a flag to determine if they are active or not. Active cameras are populated into a global list and then processed
- Once this list is gathered, Each camera's frustum will be updated (from the simulation tick)
- Game objects are culled in each Camera, The camera will contain a list of objects in it's view until the next frame.

## Phase 3 (Lights)
- Once the cameras know what objects are in their views they will begin with a pre-depth pass. This also reduces the amount of overdraw
- Each light in the scene that is not culled (Based on distance from the camera and other flags) will have their shadow (depth) pass be performed

## Phase 4 (Off-Screen Rendering)
- Each camera is rendered, Both textureless and textured objects are rendered in their respective pipelines.

## Phase 5 (Composition)
- GBuffer composition
- Cameras within the view of other cameras are composited (How the ***FUCK*** do I do this? *No idea*)