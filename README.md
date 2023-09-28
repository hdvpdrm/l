# l
Alternative to ls classical application

# Output customization
In ~/.config/ .lconfig file is created at first run of program<br>
and it contains data about colors and number of element per line for table output.<br>
P.S. Every option can be customized in future.

# Note about regular expression support
Let's imagine you want to list all txt files in current dir, so you might think to use ```l *.txt```<br>,
and after you type this command nothing happens. You need to use ```l .*.txt```, because you can use "real"<br>
regexes to sort apllication's output.<br> 

# help page
Overall, this program is dedicated to show directory's content and its properties.<br>
It can show file/dir permissions, size, creation and modification time.<br>

It has 3 different modes of output: list, table, tree.<br>
All properties are shown in list mode, so if you wanna show size of files in tree mode,<br>
than program will be forced to print all requested data in list mode.<br>

To see all files and dirs just type l without any flags.<br>

If you don't provide any path as argument, l will show content of current dir,<br>
otherwise it will show content of provided directory.<br>

## Usage
### Common examples
```$ l```<br>
```$ l -flag1 -flagn```<br>

### Particular examples
```$ l ~/notes/.*.txt -f``` print all txt files in notes dir<br>
```$ l ../ -d ```<br>

List of flags:<br>
```-d``` show only directories<br>
```-f``` show only files<br>
```-s``` sort output( -d -f and -f -d are two ways to manage sorted output)<br>
```-r``` recursivly walk through directory<br>
```-p``` show permissions of files/dirs<br>
```-T``` show modification time<br>
```-C``` show creation time<br>
```-S``` show size of each file in directory<br>
```-l``` show as list(by default)<br>
```-m``` show as table<br>
```-t``` show as tree<br>
```-a``` show all information(permissions, size, creation/modification time)<br>
```-P``` print without colorizing, use standard color codes<br>
```-h``` print help page and break program execution<br>
```-n``` print total number of elements in directory<br>
