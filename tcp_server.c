#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define STACK_SIZE  100
#define TOKENS      200
#define BUFFER_SIZE 255
#define QUIT        -1

float stack[STACK_SIZE];
int top;
int input_error = 0;

void empty_stack(void);
void tokenize(char *expression, char **tokens, int *token_count);
void push(float operand);
void empty_stack(void);
bool is_empty(void);
bool is_full(void);
void push(float operand);
float pop(void);
void subtract(void);
void divide(void);
void warn_stack_overflow(void);
void warn_stack_underflow(void);
void warn_incomplete_expression(void);
float evaluate(char *expression);
void tokenize(char *expression, char **tokens, int *token_count);

int main(int argc,char* argv[]){
    int prog_counter;
    if(argc<2) {
   	printf("You forgot to specify the port number.\n");
	return 0;
    }
    int port = atoi(argv[1]);
	char buffer[256];
	//create a socket
    int server_socket,valread;
    server_socket = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;
	//bind the socket to our specified IP and port
	bind(server_socket,(struct sockaddr*) &server_address,sizeof(server_address));
    listen(server_socket,5);
	for(;;) {
		int client_socket;
        int counter;
        int converted_counter;
		client_socket = accept(server_socket,NULL,NULL);
		if(read(client_socket,&counter,sizeof(counter))) {
			converted_counter = ntohl(counter);
		}
        //do {
        while(converted_counter!=0) { 
            valread = read(client_socket,buffer,256);
            printf("Server Received:\"%s\"\n",buffer);
            //calculate the value.
            float final_val = evaluate(buffer);
           // printf("Value sent: %f\n", final_val);
		send(client_socket,&final_val,sizeof(final_val),0);
            converted_counter--;
	}
       //} while(converted_counter!=0);
    }
    close(server_socket);
	return 0;
}

void empty_stack(void)
{
  	top = 0;
}

bool is_empty(void)
{
  	return top == 0;
}

bool is_full(void)
{
  	return top == STACK_SIZE;
}

void push(float operand)
{
  	if (is_full()) {
    		warn_stack_overflow();
    		input_error = 1;
    		return;
  	}
  	stack[top++] = operand;
}

float pop(void)
{
  	if (is_empty()) {
    		warn_stack_underflow();
    		input_error = 1;
    		return 0;
  	}
  	return stack[--top];
}

void warn_stack_overflow(void)
{
  	fprintf(stderr, "Expression is too complex.\n");
}

void warn_stack_underflow(void)
{
  	fprintf(stderr, "Not enough operands in expression\n");
	//char test[256] = "Testing";
	//send(client_socket,test,sizeof(test),0);
}

void warn_incomplete_expression(void)
{
  	fprintf(stderr, "Incomplete expression\n");
}

void subtract(void)
{
  	float subtrahend = pop();
  	float minuend = pop();
  	push(minuend - subtrahend);
}

void divide(void)
{
  	float divisor = pop();
  	float dividend = pop();
  	push(dividend / divisor);
}
/*
 The evaluation and tokenize code is from https://github.com/twcamper/c-programming/blob/master/13/13.15-RPN-calculator.c
*/
float evaluate(char *expression)
{
    int i, token_count;
    float value;
    char *t_ptr;
	char *token_pointers[TOKENS];   /* list of addresses within the expression string */

  	empty_stack();

  	/* pepper our expression input string with terminators (NULL chars) */
  	tokenize(expression, token_pointers, &token_count);

  	for (i = 0; i < token_count; i++) {
    	/* the next substring in our NULL delimited input string */
    	t_ptr = token_pointers[i];
    /*
       Is the token a number?
       It may begin with (or be) a digit,
       OR if it's longer than 1 char,
         it must be '-' then at least one digit
    */
        if (isdigit(t_ptr[0]) || (strlen(t_ptr) > 1 && isdigit(t_ptr[1]) && t_ptr[0] == '-'))
            push(atof(t_ptr));
        else
            switch(t_ptr[0])  {
                case '+': push(pop() + pop());
                    break;
                case '-': subtract();
                    break;
                case '*': push(pop() * pop());
                    break;
                case '/': divide();
                    break;
                default:
                    /* Quit - neither an operator nor a digit operand */
                    input_error = QUIT;
            }
 //       if (input_error)
 //           return input_error;
  	}

  	value = pop();
  	if (!is_empty()) {
    		warn_incomplete_expression();
    		return (input_error = 1);
  	}
  	return value;
}

void tokenize(char *expression, char **tokens, int *token_count)
{
  register bool in_token = false;
  int t;
  char *ch_ptr;
  for (t = 0, ch_ptr = expression; *ch_ptr; ch_ptr++)  {
    if (isspace(*ch_ptr))  {
      if (in_token)
        /* this is the first char after a token,
         * and this is how we tokenize our string */
        *ch_ptr = '\0';
      in_token = false;
    } else  /* NOT A SPACE  */
      if (!in_token) {
        /* this is the frist char of our token, */
        /* so we put a pointer to it in our list */
        tokens[t++] = ch_ptr;
        in_token = true;
      }
  }
  *token_count = t;
}
