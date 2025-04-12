/*

do mining pool test in litecoinpool.org

*/

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_ATTEMPTS_TRY 3
#define MAX_MESSAGE 2048
#define MAX_ERROR_MESSAGE 32

static const char *HOST_POOL = "us2.litecoinpool.org";
static unsigned short PORT_POOL = 8080;

static char *data_buffer = NULL;
static char *error_msg = NULL;

void mining_test() {
  struct sockaddr_in server_addr;
  struct hostent *server;
  int sockfd, out = 0;
  data_buffer = (char*)malloc(MAX_MESSAGE);
  error_msg = (char*)calloc(1, MAX_ERROR_MESSAGE);
  int bytes_read = 0;
  size_t tries = 0;
  
  if (errno) {
		strcpy(error_msg, "at start");
  	goto end_mining;
  }
  
#define TRIAL_SUCCESS tries = 0
#define IF_TRIAL_FAIL if (++tries > MAX_ATTEMPTS_TRY) 
  
  server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons (PORT_POOL);
  if (!(server = gethostbyname(HOST_POOL)))
    goto end_mining;
	server_addr.sin_addr = *((struct in_addr *)server->h_addr);
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		strcpy(error_msg, "sockfd fail");
    goto end_mining;
  }
  if (connect (sockfd, (struct sockaddr *)&server_addr, sizeof (server_addr)) < 0) {
		strcpy(error_msg, "connect fail");
		goto end_sock_mining;
  }
	
	//subcribe + authentication
	sprintf(data_buffer,
		"{\"id\":1,\"method\":\"mining.subscribe\",\"params\":[\"%s\"]}\n"
		"{\"id\":2,\"method\":\"mining.authorize\",\"params\":[\"%s\",\"%s\"]}\n",
		// customize value 
		"TEST_MACHINE", "Ariasa.test", " 1234");

	for (size_t length = strlen(data_buffer), s; length > 0;) {
    s = send (sockfd, data_buffer, length, 0);
    if (s <= 0) {
  		IF_TRIAL_FAIL {
  			strcpy(error_msg, "send msg");
  			goto end_sock_mining;
  		}
      continue;
    }
    length -= s;
    memmove(data_buffer, data_buffer + s, length);
  }
  TRIAL_SUCCESS;

  // printf("m: first");
  while (!out) {
	  bytes_read = recv(sockfd, data_buffer, MAX_MESSAGE - 1, 0);
	  if(!bytes_read) {
	  	// maybe lag or lost or closed connection 
  		IF_TRIAL_FAIL {
  			strcpy(error_msg, "recv give null");
  			goto end_sock_mining;
  		}
	  	continue;
	  } else if (bytes_read < 0) {
	  	// error
			strcpy(error_msg, "err on recv");
	  	goto end_sock_mining;
	  }
  	TRIAL_SUCCESS;
  	
    data_buffer[bytes_read] = 0;
	  // proccess_received_msg(data_buffer, bytes_read);
	  // fflush(stdout);
  	printf("%s",data_buffer);
    scanf("%d",&out);
  }
  
end_sock_mining:
  close(sockfd);
end_mining:
	if (*error_msg || errno)
		printf("Err: %s, %s",error_msg, strerror(errno));
  free (data_buffer);
  free (error_msg);
	return;
  
  
#undef TRIAL_SUCCESS
#undef IF_TRIAL_FAIL
}

/*

{"id":1,"error":null,"result":[[["mining.notify","5700bdac04ab1e9f"],["mining.set_difficulty","5700bdac04ab1e9f2"]],"5700bdac",6]}
{"id":null,"method":"mining.set_difficulty","params":[131072]}
{"id":null,"method":"mining.notify","params":["719c","74d51d399e8a8c6b40ca8942a789771f55a2d38921c00754b6c2f041c6975c7e","01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4603a9e92b0467f933f42cfabe6d6d5204f974f658695dabb8446e86a4c1c9a403029a1381e52d1281d90e4cb211fc8000000000000000042f4c502f0a","ffffffff02f416b125000000001976a91457757ed2d68143967543d7c58579c33e8984548888ac0000000000000000266a24aa21a9ed38b63e3e6088f6f4d97782a546d480bfc39f8ef3629459561c16eef1684e5aa100000000",["820a3f331906b31148a234bc07476a02455ec9c5acb5266c51154afe16cdf6bc","fa7a453134d132412f6937cb356027ffa59d074e1556657d7f5d8f05ad612f02","e71b33d4111cd2ca1a1494a298221caaa73bff1bb9b04c35830b2c34b680c8de","ecdae541c92751d8f3fd8b13b65f15bedda40463ddd3b9baa49bcdfccf9fa729","31e06d6f264e6b295c7aa376b3f8c29ee1663022dfdf3b3ee3ff4ab056eac4a0","ebb7e8848716652d7741ee59726a7675634448c00edbf1b2c3f1c3c59e27cc4d","bd8956e1f12719079a7fb4f14bf3973bc5922f8b6751888374110ae6851529d2","d04fa14e2bb3bcb93f8985a2e2270db9982aeaa34dd2ee6f444a184ef8a132d9","5c0a4e369f5b7d5ad646a5866d2207afc001f673c3fa566c61e895987e724345","f6761f32aaaa2e89c27ed58ac2146e20217772eabab43f951cd8102aeb4856b8"],"20000000","1932f47c","67f933ef",false]}
{"id":2,"error":null,"result":false}
{"id":null,"method":"mining.notify","params":["719d","74d51d399e8a8c6b40ca8942a789771f55a2d38921c00754b6c2f041c6975c7e","01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4603a9e92b0467f934212cfabe6d6df21c219d98f16cb13f1040234e03452265f17f2fa3ab544298bec95effd8d64c8000000000000000042f4c502f0a","ffffffff021d90b725000000001976a91457757ed2d68143967543d7c58579c33e8984548888ac0000000000000000266a24aa21a9ed44e3b7e82188ab635b753d00b8bd2d0bdecc877f9cfe7a6778def2aefd24b2cb00000000",["820a3f331906b31148a234bc07476a02455ec9c5acb5266c51154afe16cdf6bc","fa7a453134d132412f6937cb356027ffa59d074e1556657d7f5d8f05ad612f02","983b9defe66cf2bf0131e8710f6da814d9e6bda520879f6fd70cad9ed5873669","6e0d19a3c549edf468d452d364a0a79e62cfc7f2f0999925d50a910d170c1cce","970d9ee249471c87dc5428bee5da4b21164bbab40e44b5e6cf403e67765be696","c7ae1008d9df745adee7aa505f7444bce8a3dd4fc8e1e8554247b643575f8cdd","752f6feff1bc4a85215261f8ff85674b63b9c95e6fa82b4da32ed666a20a9b04","755c9f3f6cea5d61516968dad1fe98c99ab1760d187476ead6a3747340d4c292","a55497a37449f20170bc72e40b92df2537bcefaff2d02b891fe7fa0c149d71a6","f0cf10a8a83a5c43d5aa28621b0466a465d14286a9003267169f4c8727df96ca","5725f86f669925bd2b2f52530eac2e5aa15987bbdffa55cf03916800022aa3ee"],"20000000","1932f47c","67f9341c",false]}

*/

