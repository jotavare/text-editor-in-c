#include "../includes/text_editor.h"

/*
 * Handle errors and exit;
 * Clear the screen on exit;
 */
void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}

/*
 * Restore the terminal attributes to their original state;
 * Flush any inputted values;
 */
void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &G.original_termios) == -1)
        die("tcsetattr");
}

/*
 * Get and save the original state of terminal attributes;
 * Disable, change, and/or fix with flags (description in 'man termios');
 * The iflag and cflag are not necessary but were used in the old days;
 * VMIN set minimum number of bytes of input before read() returns;
 * Vtime value of time to wait before read() return (in tenths of a second 1/10);
 */
void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &G.original_termios) == -1)
        die("tcgetattr");

    atexit(disableRawMode);

    struct termios raw = G.original_termios;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/*
 * Wait for a keypress and return the pressed key;
 */
char editorReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    return (c);
}

/*
 * Use ioctl to get the size of the terminal window;
 * If ioctl fails, return -1, else return 0;
 */
int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        return (-1);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return (0);
    }
}