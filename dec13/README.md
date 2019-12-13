
Note: this solution uses the Curses library. Use `-lcurses` when linking.

To play the game manually, use the command line

```
./main input.txt manual
```

where `input.txt` is your problem input. Control the paddle with the left and right arrow keys. The game will pause and wait for input; it's too fast for me otherwise :-(

If you don't supply the `manual` command-line argument, you get to watch the CPU play (with a delay so you can actually see what's going on).