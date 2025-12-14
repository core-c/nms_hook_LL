## No Man's Sky Glitchbuilding Tool
### (nms_hook_LL)

This is a "No Man's Sky" Glitchbuilding Tool.

Usage instructions:
* Unzip the executable file from the zip (or download the .exe) from this github.  It is a super small program of only 17.5 KB.
* You can place it anywhere on your drive. It does not need any more files to run.
* Start the game (fullscreen).
* Run the tool.
* Switch back to the game, and start glitchbuilding.

The 2 extra mouse-buttons (usually at your thumb) are assigned now.
Xbutton 2 performs a wire-glitch (On PC that is: 'Q'+LMB).
Xbutton 1 performs a cache-glitch (On PC that is 'Q'+'C').
If you press Xbutton 1 while pressing the control-key, it will perform an adjacency glitch ('E'+LMB).

That is it!

> You do not need the file ***timings.txt*** when you do not compile this program yourself.



## Command Line Arguments

You can run this tool from the command line (terminal) and provide all new timing values.
Be sure to provide all 9 needed values, or it will not work.
Here is an example of how to use your own timing values:

`
    C:\>nms_hook_LL 1000 50000 50000 500 50000 50000 1000 50000 50000
`



## Info for people who compile this little program

If you compile this application yourself, you can enable reading timing values from file.
To do so, define the compiler directive: VALUES_FROM_FILE
`
    #define VALUES_FROM_FILE
`

The file named ***timings.txt*** holds all the needed values.
There are 3 glitches, and each glitch occupies a line in the file.
Each line has 3 values. The first value is the most important one.

The 1st line are the delays for a wire glitch
The 2nd line are the delays for a cache glitch
The 3rd line are the delays for an adjacency glitch

> Do not mess up the format.