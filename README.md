## No Man's Sky Glitchbuilding Tool
### (nms_hook_LL)

This is a "No Man's Sky" Glitchbuilding Tool.

Usage instructions:
* Unzip the files from the zip from this github.  It is a super small program of only 36.5 KB.
* You can place the files anywhere on your drive.
* Start the game (fullscreen).
* Run the tool.
* Switch back to the game, and start glitchbuilding.

The 2 extra mouse-buttons (usually at your thumb) are assigned now.  
Xbutton 2 performs a wire-glitch (On PC that is: 'Q'+LMB).  
Xbutton 1 performs a cache-glitch (On PC that is 'Q'+'C').  
If you press Xbutton 1 while pressing the control-key, it will perform an adjacency glitch ('E'+LMB).

That is it!

![screenshot](/screenshot.png)



## Command Line Arguments

You can run this tool from the command line (terminal) and provide some arguments.  
If you run this tool without any argument, it will (try to) load the timing values from disk.  
But if you want to edit the values, you also want the working values to be saved to file for next times.  

Here are the possible command line arguments you can provide:  

Show the current timing values:  
`
    C:\>nms_hook_LL t
`

Provide all 9 timing values:  
`
    C:\>nms_hook_LL 750 50000 50000 500 50000 50000 1000 50000 50000
`
> See below explanation what those 9 values mean


Provide only the 3 main timing values:  
`
    C:\>nms_hook_LL 750 500 1000
`
> See below explanation what those 3 values mean

There is also the possibility to save your new values to file, so you have to do the tuning only once.  
To save the values to file, append an extra argument 's' to your values, like so:  
`
    C:\>nms_hook_LL 750 500 1000 s
`
or:  
`
    C:\>nms_hook_LL 750 50000 50000 500 50000 50000 1000 50000 50000 s
`

> Note: Once your new values are saved to file, for next times it is enough to start the tool without any argument

### The meaning of the 9 values

Each glitch uses 3 values, and there are 3 glitches, so there are 3 groups of 3 values:  

`
    nms_hook_LL <wire1> <wire2> <wire3> <cache1> <cache2> <cache3> <adjacency1> <adjacency2> <adjacency3>
`

The wire-glitch involves the 'Q'-key and the left-mouse-button. Both actions have a key/button press, and a key/button release.  
Here is when the delays are executed for the wire-glitch:  

    - Press Q-key
	- Delay <wire1> microseconds
	- Press Left-Mouse-Button
	- Delay <wire2> microseconds
	- Release Q-key
	- Delay <wire3> microseconds
	- Release Left-Mouse-Button

> The same for the other two glitches.

### The meaning of the 3 values

When you use only 3 arguments, you provide the values:
`
<wire1> <cache1> <adjacency1>
`

Those 3 first delays of each glitch are the main/important delays.  
It should be enough to tune only these main values to get it to work for you.  



## The file ***timings.txt***

The file named ***timings.txt*** holds all the needed timing values.  
There are 3 glitches, and each glitch occupies a line in the file.  
Each line has 3 values. The first value (on each line) is the most important one.  

The 1st line of the file contains the delays for a wire glitch.  
`
<wire1> <wire2> <wire3>
`

The 2nd line of the file contains the delays for a cache glitch.  
`
<cache1> <cache2> <cache3>
`

The 3rd line of the file contains the delays for an adjacency glitch.  
`
<adjacency1> <adjacency2> <adjacency3>
`

The program will maintain this file, but you can edit it manually if you like.  
Just do not mess up the (simple) format.