# <cy>Progress Bar Color</c>
This mod lets you customize your progress bar color!  
You can set your normal/practice progress bar manually or follow your player colors.  
Note that follow dual color needs Weebify's [Separate Dual Icons](mod:weebify.separate_dual_icons) to be loaded, or this mod will only take your main player's color. 
  
### <cy>Supports</c>
- Normal / Practice progress bars in official levels browser, level entrance, pause menu and retry level drop bar;
- Quest Progress Bars;
- List Progress Bars in List Page and List Cells;
  
### <cy>Modes</c>
- **Default**: As if mod not loaded;
- **Follow**: follow player's color;
- **Manual**: Pick the color by your own;
- **Chromatic**: Dynamic rainbow like;
- **Progress**: gradient relative with current percentage there;
- **Random**: As this word says;
- **Advanced**: Self defined for pro users.
  
### <cy>Manual for Time Varient Setups</c>
You can let your progress bars time varient in colors for current setups
- Chromatic (unless speed = 0)
- Progress (Time mode)
- Advanced (if the *phase* is used and really affected gl_FragColor)
You can set a common speed in mod settings popup, but if you check async option in a tab, your proogress bar will not sync this common speed but follow its own speed setup instead.  
You can see Async / Speed / Phase option in all thee three modes mentioned above, but all of the three async togglers point to a same value and will change together, same for speed / phase setup.
  
### <cy>Available Uniforms for GLSL file of Advanced Mode</c>
Now that the other six modes are only a set of introduction of what this mod supports to take as params for you, if you choose to write the shader yourself, these uniforms are availabe in your shader:
- **phase** = current cycle phase, takes 0-1 as value.
- **progress** = current percentage of this progress bar, takes 0-100 as value.
- **main** = your main player sprite color
- **second** = your second player sprite color
- **glow** = your main player glow outline color
- **main2** = your main player sprite color (p2 main color if SDI loaded)
- **second2** = your second player sprite color (p2 second color if SDI loaded)
- **glow2** = your main player glow outline color (p2 glow color if SDI loaded)
  
SDI = Separate Dual Icons by Weebify.  
  
Obviously I can't introduce how to code a cool shader as expected for you here, if you need help in shader coding, go ask our Deepseek / ChatGPT / Gemini friend or someone else...  
  
### <cy>Contact / Report Bugs</c>
- Github issue
- Discord: [Naxrin#6957](https://discordapp.com/users/414986613962309633) (recommended)  
- Twitter / X: [@Naxrin19](https://x.com/Naxrin19) (not really check it often)
- In-Game ID: Naxrin