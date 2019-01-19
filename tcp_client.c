#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define STACK_SIZE  100
#define TOKENS      200
#define BUFFER_SIZE 255
#define QUIT        -1

/* globals */
float stack[STACK_SIZE];
int top;
int input_error = 0;

#define min(a,b) (((a) < (b)) ? (a) : (b)) 

/* prototypes */
void empty_stack(void);
bool is_empty(void);
bool is_full(void);
void push(float operand);
float pop(void);
void warn_stack_overflow(void);
void warn_stack_underflow(void);
void warn_incomplete_expression(void);
char* integer_to_string(int);
void tokenize(char *expression, char **tokens, int *token_count);

int main(int argc,char* argv[]){
    if(argc==1) {
   	printf("You forgot to specify the port number. And please type the input in double quots after the port number with a space in between. \n");
        return 0;
    }
	if(argc==2) {
		printf("You forgot to type the input in double quots after the port number with a space in between. \n");
        	return 0;
	}
	int port = atoi(argv[1]);
	//create a socket
	int network_socket;
	network_socket = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;
	
	char expression[256];
	char string1[256];
    char string2[256];
    char client_message_original[256];
	char client_message[256];
	strcpy(client_message_original,argv[2]);
	
	int connection_status = connect(network_socket,(struct sockaddr *) &server_address,sizeof(server_address));
	//check connection error
	if(connection_status == -1){
		printf("Error in connection to the remote socket \n\n");
		return 0;
	}

	//add space in the input
	for(int i = 0, j = 0; i<strlen(client_message_original); i++, j++) {
		if(client_message_original[i] == '+' || client_message_original[i] == '-' || client_message_original[i] == '*'||client_message_original[i] == '/') {
            		client_message[j] =' ';
            		j++;
            		client_message[j] = client_message_original[i];
            		j++;
            		client_message[j] = ' ';
        	} else
            		client_message[j] = client_message_original[i];
	}
	//printf("client_message: %s\n",client_message);
	
	//return 0;
	int counter = 0;
	int counter_int = 0;
	int counter_others = 0;
	//for (int i=0; i<strlen(client_message); client_message[i]=='+' || client_message[i]=='/' || client_message[i]=='*' || client_message[i]=='-'? i++, counter++ : i++);
	for(int i=0; i<strlen(client_message);i++){
		if(client_message[i]=='+' || client_message[i]=='/' || client_message[i]=='*' || client_message[i]=='-')
			counter++;
		else if(isdigit(client_message[i]))
			counter_int++;	
		
		else {
			if(isspace(client_message[i])) continue;
			printf("Invalid input: contain other characters.\n");
			return 0;
		} 
	}
	//printf("counter, counter_int: %d %d\n",counter, counter_int);
	//if( counter != counter_int-1) {
	//	printf("Please type a valid input.\n");
	//	//close(network_socket);
	//	return 0;
	//}
	int converted_counter = htonl(counter);
	send(network_socket,&converted_counter,sizeof(converted_counter),0);
	counter--;
	
    int i, token_count, value;
    bool flag = false;
    char *t_ptr;
    char *token_pointers[TOKENS];   /* list of addresses within the expression string */
    //char* in_string = malloc(sizeof(char) * sizeof(int) * 4 + 1);
    char in_string[256];
    //char temp[256]="";
    char temp[256];
    char space[256]=" ";
    empty_stack();

    /* pepper our expression input string with terminators (NULL chars) */
    tokenize(client_message, token_pointers, &token_count);

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
          push(atoi(t_ptr));
        else
          switch(t_ptr[0])  {
            case '+':
                  gcvt(pop(), 6, temp);
                 // printf("+ temp: %s\n",temp);
			gcvt(pop(), 6, in_string);
		//printf("+ in_string: %s\n",in_string);
                  strcat(in_string,space);
                  strcat(in_string,temp);
                  strcat(in_string," +");
                  flag = true;
                  break;
            case '-':
                  gcvt(pop(), 6, temp);
                  gcvt(pop(), 6, in_string);
                  strcat(in_string,space);
                  strcat(in_string,temp);
                  strcat(in_string," -");
                  flag = true;
                  break;
            case '*':
                  gcvt(pop(), 6, temp);
                  gcvt(pop(), 6, in_string);
                  strcat(in_string,space);
                  strcat(in_string,temp);
                  strcat(in_string," *");
                  flag = true;
                  break;
            case '/':
                  gcvt(pop(), 6, temp);
                  gcvt(pop(), 6, in_string);
                  strcat(in_string,space);
                  strcat(in_string,temp);
                  strcat(in_string," /\0");
                  flag = true;
                  break;
            default:
              /* Quit - neither an operator nor a digit operand */
              input_error = QUIT;
                  printf("Input Error: not a valid postfix expression.\n");
          }
        if (input_error)
          return input_error;
        if(flag) {
            //printf("in_string is: %s\n",in_string);
//            int checker = 0;
//            for(int i = 0; i<strlen(in_string); i++) {
//                if(in_string[i] == '+' || in_string[i] == '-' ||in_string[i] == '*'||in_string[i] == '/')
//                    checker = 1;
//            }
//            printf("in_string: %s\n",in_string);
//            printf("check: %d\n",checker);
//            if(checker == 0) {
//                    printf("Not enough operator.\n");
//                    return 0;
//            }
            send(network_socket,in_string,sizeof(in_string),0);
            float server_response;
            recv(network_socket,&server_response,sizeof(server_response),0);
            if(counter==0) {
                printf("Client received final: %f\n",server_response);
            } else {
                printf("Client received: %f\n",server_response);
            }
            counter--;
            //clear in_string for next round;
            strcpy(in_string,"");
            push(server_response);
            flag = false;
        }
    }
	close(network_socket);
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
}
void warn_incomplete_expression(void)
{
  fprintf(stderr, "Incomplete expression\n");
}

/*
 The tokenize code is from https://github.com/twcamper/c-programming/blob/master/13/13.15-RPN-calculator.c
 */
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

char* integer_to_string(int x)
{
    char* buffer = malloc(sizeof(char) * sizeof(int) * 4 + 1);
    if (buffer)
    {
         sprintf(buffer, "%d", x);
    }
    return buffer; // caller is expected to invoke free() on this buffer to release memory
}
