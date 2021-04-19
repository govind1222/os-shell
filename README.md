# os-shell
Creating a shell for a Linux environment using C. This project was to gain knowledge of how the command line works in Linux and also to learn more about child processes and how the execv command works. 

## Compilation
Compile the file using gcc 
```bash
gcc dash.c -o dash -Wall -Werror -O
```

## Usage
The shell supports most commands as well as output redirection and parallel commands. 

  For output redirection use the following command template.
  ```bash
  command > filename
  ```
  For example
  ```bash
  echo "Hello World" > myfile.txt
  ```

  To run parallel commands, follow this command template
  ```bash
   command1 & command2 
  ```
  Seperate commands with & symbol
  
  For example,
  ```bash
  ls -la & pwd
  ```
