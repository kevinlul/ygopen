#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <thread>

#include "../ocgcore-proto/msg_codec.hpp"
#include "graphic_board.hpp"

static const char* MY_FIFO_NAME = "/tmp/ygomsgs";

std::mutex m2;

static_assert(sizeof(unsigned char)==1, "Unsigned char must have a size of 1");
void FifoReader(DuelBoardSDL* board)
{
	static unsigned char buffer[1024];
	YGOpen::MsgEncoder encoder;
	
	while (!m2.try_lock())
	{
		int fd = open(MY_FIFO_NAME, O_RDONLY);
		int r = read(fd, buffer, sizeof(buffer));
		close(fd);
		
		board->m1.lock();
		board->AppendMsg(encoder.Encode(buffer, r));
		if(board->IsRealtime())
			board->Forward();
		board->m1.unlock();
	}
	m2.unlock();
}

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	mkfifo(MY_FIFO_NAME, 0666);
	
	DuelBoardSDL board;
	
	board.FillPile(0, LocationMainDeck, 40);
	board.FillPile(0, LocationExtraDeck, 15);
	board.FillPile(1, LocationMainDeck, 40);
	board.FillPile(1, LocationExtraDeck, 15);
	board.SetLP(0, 8000);
	board.SetLP(1, 8000);
	
	std::thread t1(FifoReader, &board);
	
	m2.lock();
	board.Run();
	m2.unlock();
	
	puts("Game thread closed");
	t1.join();
	
	return 0;
}
