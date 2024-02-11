# tear
CLI Text Editor

# UTF-8
char string
ł.ó.dź.\n
indexed
01234567

screen rendered string
łódź\n

actual char rendered string
ł.ó.dź.\n
indexed
01234567

Needed translations:
- Make operations UTF-8 aware
	- maybe a helper to
		1. move the cursors to the first UTF-8 byte
		2. 

# Things left to do

## Movement
- Add tabs to match the tabs in the previous line when pressing 'Enter' and creating
  a new line
